/* Board-specific Initialisation.
 * Hugo Vincent, 2 May 2010.
 */

#include "FreeRTOSConfig.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

// This initialises the stdio layer in Newlib.
extern void initialise_stdio (void);

void BoardInit( void )
{
	// Setup the debug UART (talks to the PC through the mbed's second
	// microcontroller) and connect it to stdio.
	uart0Init(115200, 128);
	initialise_stdio();

	// Setup the led's on the mbed board.
	vGpioInitialise();
}

// FIXME put WDT code into it's own peripheral driver
#if 0

// Constants to setup the WDT (4MHz RC clock with fixed divide-by-4 -- 3s timeout)
#define mainWDT_TIMEOUT		( 1000000 * 3 )

static void WDT_InitWatchdog() 
{
	// Setup the watchdog timer (3 second timeout).
	// FIXME check/clear reset-reason for WDT reset
	WDMOD = 0x03; // enable and reset
	WDTC = mainWDT_TIMEOUT;
	WDFEED = 0xAA; WDFEED = 0x55;
}

static void WDT_FeedWatchdog( void )
{
	taskENTER_CRITICAL();
	{
		WDFEED = 0xAA; WDFEED = 0x55;
	}
	taskEXIT_CRITICAL();
}
#endif

#if configGENERATE_RUN_TIME_STATS == 1
/* This uses Timer 1 to record task run-time statistics. Allows FreeRTOS
 * to generate a nice, tabular `top`-style CPU-usage listing. 
 */
void ConfigureTimerForRunTimeStats( void )
{
	/* Power up and feed the timer with a clock. */
	LPC_SC->PCONP |= 0x1<<2;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3<<4))) | (0x01<<4);

	/* Reset Timer 1. */
	LPC_TIM1->TCR = 0x1<<1;

	/* Prescale to a frequency that is good enough to get a decent resolution,
	   but not too fast so as to overflow all the time. */
	LPC_TIM1->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;

	/* Start the counter, counting up. */
	LPC_TIM1->CTCR = 0x0;
	LPC_TIM1->TCR = 0x1<<0;
}
#endif

