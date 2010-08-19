/* TODO:
 *   - Newlib integration.
 *   - let RTC alarm be a POSIX signal, and register alarm with POSIX alarm().
 *   - /dev/rtc as per Linux (www.kernel.org/doc/man-pages/online/pages/man4/rtc.4.html)
 *
 * Hugo Vincent, 14 August 2010.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>

#include "FreeRTOS.h"
#include "task.h"

#include "drivers/rtc.h"

/* ------------------------------------------------------------------------- */
// Register bits:

#define RTC_ILR_RTCCIF		(0x00000001)
#define RTC_ILR_RTCALF		(0x00000002)
#define RTC_ILR_MASK		(0x00000003)

#define RTC_CCR_CLKEN		(0x00000001)
#define RTC_CCR_CTCRST		(0x00000002)

#define RTC_AMR_AMRMASK		(0x000000ff)

#define RTC_AUX_OSCF		(0x00000010)
#define RTC_AUXEN_OSCFEN	(0x00000010)

#define RTC_CIIR_IMSEC		(0x00000001)
#define RTC_CIIR_IMMIN		(0x00000002)
#define RTC_CIIR_IMHOUR		(0x00000004)
#define RTC_CIIR_IMDOM		(0x00000008)
#define RTC_CIIR_IMDOW		(0x00000010)
#define RTC_CIIR_IMDOY		(0x00000020)
#define RTC_CIIR_IMMON		(0x00000040)
#define RTC_CIIR_IMYEAR		(0x00000080)
#define RTC_CIIR_IMMASK		(0x000000ff)

/* ------------------------------------------------------------------------- */

void RTC_ISR();

// Place RTC into low power state
static inline void RTC_Sleep()
{
	LPC_SC->PCONP &= ~(0x1<<10);
}

// Prepare RTC for interactive use
static inline void RTC_Wake()
{
	LPC_SC->PCONP |= 0x1<<10;
}

void RTC_GetTime(struct tm *tm)
{
	taskENTER_CRITICAL();
	{
		RTC_Wake();

		// Read clock registers into the tm structure
		tm->tm_sec   = LPC_RTC->SEC;
		tm->tm_min   = LPC_RTC->MIN;
		tm->tm_hour  = LPC_RTC->HOUR;
		tm->tm_wday  = LPC_RTC->DOW;
		tm->tm_mday  = LPC_RTC->DOM;
		tm->tm_yday  = LPC_RTC->DOY - 1;
		tm->tm_mon   = LPC_RTC->MONTH - 1;
		tm->tm_year  = LPC_RTC->YEAR - 1900;
		tm->tm_isdst = 0;

		RTC_Sleep();
	}
	taskEXIT_CRITICAL();
}

// Set clock to new values.
int RTC_SetTime(struct tm *tm)
{
	taskENTER_CRITICAL();
	{
		RTC_Wake();

		// Disable clock and reset clock divider counter
		LPC_RTC->CCR = 0;
		LPC_RTC->CCR = RTC_CCR_CTCRST;

		// Set time
		LPC_RTC->SEC   = tm->tm_sec;
		LPC_RTC->MIN   = tm->tm_min;
		LPC_RTC->HOUR  = tm->tm_hour;
		LPC_RTC->DOW   = tm->tm_wday;
		LPC_RTC->DOM   = tm->tm_mday;
		LPC_RTC->DOY   = tm->tm_yday + 1;
		LPC_RTC->MONTH = tm->tm_mon + 1;
		LPC_RTC->YEAR  = tm->tm_year + 1900;

		// Reenable clock
		LPC_RTC->CCR = RTC_CCR_CLKEN;

		RTC_Sleep();
	}
	taskEXIT_CRITICAL();

	return 0;
}

void RTC_Init()
{
	// Reset clock divider and enable clock
	LPC_RTC->CCR = 0;
	LPC_RTC->CCR = RTC_CCR_CTCRST;
	LPC_RTC->CCR = RTC_CCR_CLKEN;

	// Set alarm mask (all fields (hour, minute, day etc) must match for alarm)
	LPC_RTC->AMR = RTC_AMR_AMRMASK;

	// Disable all periodic interrupts
	LPC_RTC->CIIR = 0;

#if defined(TARGET_LPC17xx)
	// Enable the RTC oscillator failure detection interrupt
	LPC_RTC->RTC_AUX   = RTC_AUX_OSCF;
	LPC_RTC->RTC_AUXEN = RTC_AUXEN_OSCFEN;
#endif

	// Check for a sensible-looking time
	bool nonsense = false;
	if ((LPC_RTC->SEC < 0)		|| (LPC_RTC->SEC > 59))		nonsense = true;
	if ((LPC_RTC->MIN < 0)		|| (LPC_RTC->MIN > 59))		nonsense = true;
	if ((LPC_RTC->HOUR < 0)		|| (LPC_RTC->HOUR > 23))	nonsense = true;
	if ((LPC_RTC->DOW < 0)		|| (LPC_RTC->DOW > 6))		nonsense = true;
	if ((LPC_RTC->DOM < 1)		|| (LPC_RTC->DOM > 31))		nonsense = true;
	if ((LPC_RTC->DOY < 1)		|| (LPC_RTC->DOY > 366))	nonsense = true;
	if ((LPC_RTC->MONTH < 1)	|| (LPC_RTC->MONTH > 12))	nonsense = true;
	if ((LPC_RTC->YEAR < 1970)	|| (LPC_RTC->YEAR > 2050))	nonsense = true;

	// Set the clock to Jan 1, 1970 00:00:00
	if (nonsense)
		RTC_SetTime_Epoch((time_t) 0);

	// Clear pending interrupts
	LPC_RTC->ILR = RTC_ILR_MASK;

	// Put the RTC to sleep (continues to keep time whilst asleep)
	RTC_Sleep();

	// Enable the interrupt
	NVIC_SetVector(RTC_IRQn, (uint32_t)RTC_ISR);
	NVIC_EnableIRQ(RTC_IRQn);
}

int RTC_SetAlarm(struct tm *tm)
{
	time_t now;
	RTC_GetTime_Epoch(&now);

	// Check alarm is in the future
	if (!tm || (mktime(tm) < now))
		return -1;

	taskENTER_CRITICAL();
	{
		RTC_Wake();

		LPC_RTC->ALYEAR = tm->tm_year + 1900;
		LPC_RTC->ALMON  = tm->tm_mon + 1;
		LPC_RTC->ALDOY  = tm->tm_yday + 1;
		LPC_RTC->ALDOM  = tm->tm_mday;
		LPC_RTC->ALDOW  = tm->tm_wday;
		LPC_RTC->ALHOUR = tm->tm_hour;
		LPC_RTC->ALMIN  = tm->tm_min;
		LPC_RTC->ALSEC  = tm->tm_sec;

		RTC_Sleep();
	}
	taskEXIT_CRITICAL();

	return 0;
}

void RTC_GetAlarm(struct tm *tm)
{
	if (tm)
	{
		memset(tm, 0, sizeof(* tm));

		taskENTER_CRITICAL();
		{
			RTC_Wake();

			tm->tm_sec  = LPC_RTC->ALSEC;
			tm->tm_min  = LPC_RTC->ALMIN;
			tm->tm_hour = LPC_RTC->ALHOUR;
			tm->tm_wday = LPC_RTC->ALDOW;
			tm->tm_mday = LPC_RTC->ALDOM;
			tm->tm_yday = LPC_RTC->ALDOY - 1;
			tm->tm_mon  = LPC_RTC->ALMON - 1;
			tm->tm_year = LPC_RTC->ALYEAR - 1900;

			RTC_Sleep();
		}
		taskEXIT_CRITICAL();
	}
}

void RTC_PeriodicAlarm(RTC_PeriodicInt_Interval_t interval, bool enable)
{
	RTC_Wake();

	switch (interval)
	{
		case Periodic_PerSec:
			{
				if (!enable)
					LPC_RTC->CIIR &= ~RTC_CIIR_IMSEC;
				else
					LPC_RTC->CIIR |= RTC_CIIR_IMSEC;
			}
			break;
		case Periodic_PerMin:
			{
				if (!enable)
					LPC_RTC->CIIR &= ~RTC_CIIR_IMMIN;
				else
					LPC_RTC->CIIR |= RTC_CIIR_IMMIN;
			}
			break;
		case Periodic_PerHour:
			{
				if (!enable)
					LPC_RTC->CIIR &= ~RTC_CIIR_IMHOUR;
				else
					LPC_RTC->CIIR |= RTC_CIIR_IMHOUR;
			}
			break;
		case Periodic_PerDay:
			{
				if (!enable)
					LPC_RTC->CIIR &= ~RTC_CIIR_IMDOY;
				else
					LPC_RTC->CIIR |= RTC_CIIR_IMDOY;
			}
			break;
		case Periodic_PerMonth:
			{
				if (!enable)
					LPC_RTC->CIIR &= ~RTC_CIIR_IMMON;
				else
					LPC_RTC->CIIR |= RTC_CIIR_IMMON;
			}
			break;
		case Periodic_PerYear:
			{
				if (!enable)
					LPC_RTC->CIIR &= ~RTC_CIIR_IMYEAR;
				else
					LPC_RTC->CIIR |= RTC_CIIR_IMYEAR;
			}
			break;
	}

	RTC_Sleep();
}

/* ------------------------------------------------------------------------- */
// Unix Epoch wrapper functions:

void RTC_GetTime_Epoch(time_t *now)
{
	struct tm tm;
	RTC_GetTime(&tm);
	*now = mktime(&tm);
}

void RTC_SetTime_Epoch(time_t now)
{
	struct tm tm;
	localtime_r(&now, &tm);
	RTC_SetTime(&tm);
}

void RTC_GetAlarm_Epoch(time_t *then)
{
	struct tm tm;
	RTC_GetAlarm(&tm);
	*then = mktime(&tm);
}

void RTC_SetAlarm_Epoch(time_t then)
{
	struct tm tm;
	localtime_r(&then, &tm);
	RTC_SetAlarm(&tm);
}

/* ------------------------------------------------------------------------- */
// Interrupt service routine:

#if defined(TARGET_LPC23xx)
void RTC_ISR_Handler();
__attribute__((interrupt ("IRQ"))) void RTC_ISR()
{
	portSAVE_CONTEXT();
	RTC_ISR_Handler();
	portRESTORE_CONTEXT();
}
void RTC_ISR_Handler()
#elif defined(TARGET_LPC17xx)
__attribute__((interrupt)) void RTC_ISR()
#endif
{
	portBASE_TYPE higherPriorityTaskWoken = pdFALSE;

	RTC_Wake();

	// Periodic interrupt
	if (LPC_RTC->ILR & RTC_ILR_RTCCIF)
	{
		// FIXME dispatch interrupt

		// Clear interrupt
		LPC_RTC->ILR = RTC_ILR_RTCCIF;
	}

	// Alarm interrupt
	if (LPC_RTC->ILR & RTC_ILR_RTCALF)
	{
		// FIXME dispatch interrupt

		// Clear interrupt
		LPC_RTC->ILR = RTC_ILR_RTCALF;
	}

#if defined(TARGET_LPC17xx)
	// Oscillator failure interrupt
	if (LPC_RTC->RTC_AUX & RTC_AUX_OSCF)
	{
		// FIXME dispatch interrupt

		// Clear the interrupt
		LPC_RTC->RTC_AUX = RTC_AUX_OSCF;
	}
#endif

	RTC_Sleep();

#if defined(TARGET_LPC17xx)
	portEND_SWITCHING_ISR(higherPriorityTaskWoken);
#elif defined(TARGET_LPC23xx)
	LPC_VIC->Address = 0; // Clear the interrupt
	if (higherPriorityTaskWoken) { vPortYieldFromISR(); }
#endif
}

