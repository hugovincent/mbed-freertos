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
#include "semifs.h"

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

void BoardDeviceInit()
{
	extern void initialise_stdio();
	initialise_stdio();

	SemiFS_Init();
}

