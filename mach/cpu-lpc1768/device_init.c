/* Setup PLL, clock dividers, power gating, memory accelerator module and
 * low-level generic (board-independent) GPIO.
 *
 * IMPORTANT NOTE: This runs before the C/C++ main() function and can't use any
 * standard library functions. If you need memcpy/bzero/etc, use __builtin_memcpy() etc.
 *
 * Rob Turner, 20 July 2010.
 */

#include "os_init.h"
#include "FreeRTOSConfig.h"
#include <cmsis.h>

#define PCONP_PCGPIO    	(0x00008000)
#define PLL0_FEED()			{ LPC_SC->PLL0FEED = 0xAA; LPC_SC->PLL0FEED = 0x55; }
#define PLL1_FEED()			{ LPC_SC->PLL1FEED = 0xAA; LPC_SC->PLL1FEED = 0x55; }

#define SHCSR_USGFAULTENA	(1 << 18)
#define SHCSR_BUSFAULTENA	(1 << 17)
#define SHCSR_MEMFAULTENA	(1 << 16)

uint32_t SystemCoreClock = 99000000;

void LowLevel_Init(void)
{
	/*************************************************************************/
	// Core Clock Setup

	// Disable peripherals power.
	LPC_SC->PCONP = 0;

	// Enable GPIO power.
	LPC_SC->PCONP = PCONP_PCGPIO;

	// Disable TPIU.
	LPC_PINCON->PINSEL10 = 0;

	if (LPC_SC->PLL0STAT & (1 << 25))
	{
		// Enable PLL, disconnected.
		LPC_SC->PLL0CON = 1;
		PLL0_FEED();
	}

	// Disable PLL, disconnected.
	LPC_SC->PLL0CON = 0;
	PLL0_FEED();

	// Enable main OSC.
	LPC_SC->SCS |= 0x20;
	while (!(LPC_SC->SCS & 0x40));

	// Select main OSC (= 12MHz) as the PLL clock source.
	LPC_SC->CLKSRCSEL = 0x1;
	LPC_SC->PLL0CFG = 0x20031;
	PLL0_FEED();

	// Enable PLL, disconnected.
	LPC_SC->PLL0CON = 1;
	PLL0_FEED();

	// Set clock divider.
	LPC_SC->CCLKCFG = 0x03;

	// Configure flash accelerator.
	LPC_SC->FLASHCFG = 0x403a;

	// Check lock bit status.
	while (((LPC_SC->PLL0STAT & (1 << 26)) == 0));

	// Enable and connect.
	LPC_SC->PLL0CON = 3;
	PLL0_FEED();
	while (((LPC_SC->PLL0STAT & (1 << 25)) == 0));

	/*************************************************************************/
	// USB Clock Setup

	// Configure the clock for the USB.
	if (LPC_SC->PLL1STAT & (1 << 9))
	{
		// Enable PLL, disconnected.
		LPC_SC->PLL1CON = 1;
		PLL1_FEED();
	}

	// Disable PLL, disconnected.
	LPC_SC->PLL1CON = 0;
	PLL1_FEED();
	LPC_SC->PLL1CFG = 0x23;
	PLL1_FEED();

	// Enable PLL, disconnected.
	LPC_SC->PLL1CON = 1;
	PLL1_FEED();
	while (((LPC_SC->PLL1STAT & (1 << 10)) == 0));

	// Enable and connect.
	LPC_SC->PLL1CON = 3;
	PLL1_FEED();
	while (((LPC_SC->PLL1STAT & (1 << 9)) == 0));

	// Setup the peripheral bus to be the same as the PLL output (64 MHz).
	LPC_SC->PCLKSEL0 = 0x05555555;

	/*************************************************************************/
	// Set system call and system timer tick interrupts, and MPU fault setup

	SCB->SHCSR |= SHCSR_USGFAULTENA | SHCSR_BUSFAULTENA; // SHCSR_MEMFAULTENA gets set by kernel when MPU is init

	// Setup FreeRTOS vectors (priorities for these are set by kernel)
	extern void vPortSVCHandler( void );
	extern void xPortPendSVHandler(void);
	extern void xPortSysTickHandler(void);
	NVIC_SetVector(SVCall_IRQn, (unsigned int)vPortSVCHandler);
	NVIC_SetVector(PendSV_IRQn, (unsigned int)xPortPendSVHandler);
	NVIC_SetVector(SysTick_IRQn, (unsigned int)xPortSysTickHandler);
}

#if configGENERATE_RUN_TIME_STATS == 1
/* FIXME this was written for LPC2368 - check it works here too */
/* This uses Timer 1 to record task run-time statistics. Allows FreeRTOS
 * to generate a nice, tabular `top`-style CPU-usage listing. 
 */
void ConfigureTimerForRunTimeStats( void )
{
	// Power up and feed the timer with a clock.
	LPC_SC->PCONP |= 0x1<<2;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3<<4))) | (0x01<<4);

	// Reset Timer 1.
	LPC_TIM1->TCR = 0x1<<1;

	// Prescale to a frequency that is good enough to get a decent resolution,
	// but not too fast so as to overflow all the time.
	LPC_TIM1->PR =  ( SystemCoreClock / 10000UL ) - 1UL;

	// Start the counter, counting up.
	LPC_TIM1->CTCR = 0x0;
	LPC_TIM1->TCR = 0x1<<0;
}
#endif

