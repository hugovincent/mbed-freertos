/* Setup PLL, clock dividers, power gating, memory accelerator module and
 * low-level generic (board-independent) GPIO.
 *
 * IMPORTANT NOTE: This runs before the C/C++ main() function and can't use any
 * standard library functions. If you need memcpy/bzero/etc, use __builtin_memcpy() etc.
 *
 * Hugo Vincent, 2 May 2010.
 */

#include <lpc23xx.h>

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

void LowLevelInit(void)
{
	/* Disconnect the PLL if it's already connected. */
	if (PLLCON & (PLL_CONNECT | PLL_ENABLE))
	{
		PLLCON = PLL_ENABLE;
		PLLFEED = 0xAA; PLLFEED = 0x55;
	}

	/* Disable the PLL. */
	PLLCON = 0;
	PLLFEED = 0xAA; PLLFEED = 0x55;

	/* Turn on the oscillator clock source and wait for it to start.
	 * Also, enable fast mode on GPIO ports 0 and 1.
	 */
	SCS |= OSC_ENABLE | FAST_GPIO;
	while( !( SCS & OSC_STAT ) );
	CLKSRCSEL = OSC_SELECT;

	/* Setup the PLL to multiply the XTAL input up to Fcco = 288 MHz. */
	PLLCFG =  (PLL_MUL - 1) | (((PLL_DIV - 1) << 16));
	PLLFEED = 0xAA; PLLFEED = 0x55;

	/* Turn on and wait for the PLL to lock. */
	PLLCON = PLL_ENABLE;
	PLLFEED = 0xAA; PLLFEED = 0x55;
	while( !( PLLSTAT & PLL_LOCK ) );

	/* Set clock dividors for CPU and USB blocks. */
	CCLKCFG = (CPU_CLK_DIV - 1);
	USBCLKCFG = (USB_CLK_DIV - 1);

	/* Connect the PLL and wait for it to connect. */
	PLLCON = PLL_CONNECT | PLL_ENABLE;
	PLLFEED = 0xAA; PLLFEED = 0x55;
	while( !( PLLSTAT & PLL_CONNECTED ) );

	/* Setup and turn on the MAM.  Four cycle access is used due to the fast
	 * PLL used.  It is possible faster overall performance could be obtained by
	 * tuning the MAM and PLL settings.
	 */
	MAMCR = 0;
	MAMTIM = MAM_TIM_4;
	MAMCR = MAM_MODE_FULL;
}

