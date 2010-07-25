/* Power management for LPC1768 and LPC2368 (and probably others).
 *
 * Hugo Vincent, 24 July 2010.
 */

#include "power_management.h"
#include "cmsis.h"
#include "FreeRTOS.h"

void PowerManagement_Sleep()
{
	// Put processor core into Sleep Mode to conserve power.
	LPC_SC->PCON |= 0x5;

	// And we're back... let's just NOP for a bit just in case.
	portNOP();
	portNOP();
	portNOP();
	portNOP();

	// Reenable & reconnect PLL
	//FIXME

	// Reconfigure USBCLK, CCLK dividers.
	//FIXME
}

__attribute__ ((noreturn)) void PowerManagement_PowerDown()
{
	portDISABLE_INTERRUPTS();
	while (1) 
		LPC_SC->PCON |= 0x3;
}

void PowerManagement_Idle()
{
	// Put processor core into Idle Mode to conserve power.
	LPC_SC->PCON |= 0x1;

	// And we're back... let's just NOP for a bit just in case.
	portNOP();
	portNOP();
	portNOP();
	portNOP();

	// No special reconfiguration needed for Idle mode.
}
