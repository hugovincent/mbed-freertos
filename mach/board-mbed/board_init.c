/* Board-specific hardware initialisation. Must set up the console serial 
 * port. 
 *
 * Hugo Vincent, 2 May 2010.
 */

#include "os_init.h"
#include "semifs.h"

#include "drivers/gpio.h"
#include "drivers/uart.h"
#include "drivers/wdt.h"
#include "drivers/rtc.h"

void Board_EarlyInit( void )
{
	// FIXME This is where pinmux configuration should get done

	UART_Init(/* UART: */ 0, /* baud rate: */ 115200, /* buffer size: */ 128);
	WDT_Init(/* timeout in seconds: */ 6);
}

void Board_LateInit()
{
	GPIO_Init();
	//RTC_Init();

	SemiFS_Init();
}

