#include <FreeRTOS.h>
#include <task.h>
#include "hardware/wdt.h"

void WDT::init(const unsigned int timeout_seconds)
{
	/* Enable and reset watchdog. */
	LPC_WDT->WDMOD = 0x03;

	/* The WDT counter is clocked from the 4MHz internal RC clock with a
	 * fixed divide-by-4, so multiply by 1 million to set timeout in seconds.
	 */
	LPC_WDT->WDTC = 1000000 * timeout_seconds;

	/* Feed to get settings to take and start the watchdog. */
	LPC_WDT->WDFEED = 0xAA; LPC_WDT->WDFEED = 0x55;
}

void WDT::feed()
{
	taskENTER_CRITICAL();
	{
		LPC_WDT->WDFEED = 0xAA; LPC_WDT->WDFEED = 0x55;
	}
	taskEXIT_CRITICAL();
}

