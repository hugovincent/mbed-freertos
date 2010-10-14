/* Power management for LPC1768.
 *
 * Hugo Vincent, 31 July 2010.
 */

#include "power_management.h"
#include "debug_support.h"
#include "cmsis.h"
#include "FreeRTOS.h"

void PowerManagement_Sleep()
{
	// FIXME
}

__attribute__ ((noreturn)) void PowerManagement_PowerDown()
{
	// FIXME
	while (1)
		;
}

void PowerManagement_Idle()
{
	// Cortex-M3 "wait for interrupt" instruction
	__WFI();
}

void PowerManagement_MbedInterfacePowerdown()
{
	int dummy;
	SemihostCall(Semihost_USR_POWERDOWN, &dummy);
}

