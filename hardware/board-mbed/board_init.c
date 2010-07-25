/* Board-specific hardware initialisation. Must set up the console serial 
 * port. 
 *
 * IMPORTANT NOTE: This runs before the C/C++ main() function and can't use a
 * number of standard library functions, including standard IO. 
 *
 * Hugo Vincent, 2 May 2010.
 */

#include "cmsis.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/wdt.h"

void BoardInit( void )
{
	// This is where things like pinmux configuration should get done.

	// Start the watchdog timer.
	WDT_Init(6);

	// Setup the debug UART (talks to the PC through the mbed's second
	// microcontroller).
	uart0Init(115200, 128);

	// Setup the led's on the mbed board.
	vGpioInitialise();
}

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
	LPC_TIM1->PR =  ( SystemCoreClock / 10000UL ) - 1UL;

	/* Start the counter, counting up. */
	LPC_TIM1->CTCR = 0x0;
	LPC_TIM1->TCR = 0x1<<0;
}
#endif

