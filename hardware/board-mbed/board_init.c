/* Board-specific Initialisation.
 * Hugo Vincent, 2 May 2010.
 */

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
