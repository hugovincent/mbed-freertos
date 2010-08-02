/* Setup PLL, clock dividers, power gating, memory accelerator module and
 * low-level generic (board-independent) GPIO.
 *
 * IMPORTANT NOTE: This runs before the C/C++ main() function and can't use any
 * standard library functions. If you need memcpy/bzero/etc, use __builtin_memcpy() etc.
 *
 * Hugo Vincent, 2 May 2010.
 */

#include "os_init.h"
#include "FreeRTOSConfig.h"
#include <cmsis.h>
#include <exception_handlers.h>

/* Constants to setup the PLL and clock dividers:
 *
 *     External crystal = 12 MHz, Fcco (PLL output) = 288 MHz,
 *     Fcpu = 72 MHz, Fusb = 48 MHz.
 */
#define PLL_MUL			((unsigned int)12)
#define PLL_DIV			((unsigned int)1)
#define CPU_CLK_DIV		((unsigned int)4)
#define USB_CLK_DIV		((unsigned int)6)

/* Constants to program the PLL */
#define PLL_ENABLE		((unsigned int)0x0001)
#define PLL_CONNECT		((unsigned int)0x0002)
#define PLL_LOCK		((unsigned int)0x4000000)
#define PLL_CONNECTED	((unsigned int)0x2000000)
#define OSC_ENABLE		((unsigned int)0x20)
#define OSC_STAT		((unsigned int)0x40)
#define OSC_SELECT		((unsigned int)0x01)
#define FAST_GPIO		((unsigned int)0x01)

/* Constants to setup the MAM. */
#define MAM_TIM_4		((unsigned char)0x04)
#define MAM_MODE_FULL	((unsigned char)0x02)

#define PLL_FEED()		{ LPC_SC->PLL0FEED = 0xAA; LPC_SC->PLL0FEED = 0x55; }

uint32_t SystemCoreClock = 72000000;

void LowLevel_Init(void)
{
	// FIXME get/store reset-reason.

	/*************************************************************************/
	// Core Clock Setup

	// Disconnect the PLL if it's already connected.
	if (LPC_SC->PLL0CON & (PLL_CONNECT | PLL_ENABLE))
	{
		LPC_SC->PLL0CON = PLL_ENABLE;
		PLL_FEED();
	}

	// Disable the PLL.
	LPC_SC->PLL0CON = 0;
	PLL_FEED();

	// Turn on the oscillator clock source and wait for it to start.
	// Also, enable fast mode on GPIO ports 0 and 1.
	LPC_SC->SCS |= OSC_ENABLE | FAST_GPIO;
	while (!(LPC_SC->SCS & OSC_STAT));
	LPC_SC->CLKSRCSEL = OSC_SELECT;

	// Setup the PLL to multiply the XTAL input up to Fcco = 288 MHz.
	LPC_SC->PLL0CFG = (PLL_MUL - 1) | (((PLL_DIV - 1) << 16));
	PLL_FEED();

	// Turn on and wait for the PLL to lock.
	LPC_SC->PLL0CON = PLL_ENABLE;
	PLL_FEED();
	while (!(LPC_SC->PLL0STAT & PLL_LOCK));

	// Set clock dividors for CPU and USB blocks.
	LPC_SC->CCLKCFG = (CPU_CLK_DIV - 1);
	LPC_SC->USBCLKCFG = (USB_CLK_DIV - 1);

	// Connect the PLL and wait for it to connect.
	LPC_SC->PLL0CON = PLL_CONNECT | PLL_ENABLE;
	PLL_FEED();
	while (!(LPC_SC->PLL0STAT & PLL_CONNECTED));

	/*************************************************************************/
	// Memory and VIC setup

	// Setup and turn on the MAM.  Four cycle access is used due to the fast
	// PLL used.  It is possible faster overall performance could be obtained by
	// tuning the MAM and PLL settings.
	LPC_SC->MAMCR = 0;
	LPC_SC->MAMTIM = MAM_TIM_4;
	LPC_SC->MAMCR = MAM_MODE_FULL;

	// Set vectors for unhandled and/or spurious interrupts in the VIC.
	for (int irq = 0; irq < 32; irq++)
	{
		LPC_VIC->VectAddr[irq] = (unsigned int)Exception_UnhandledIRQ;
		LPC_VIC->VectPriority[irq] = 15; // lowest
	}
}

#if configGENERATE_RUN_TIME_STATS == 1
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

