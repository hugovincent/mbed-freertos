#include "FreeRTOS.h"
#include "console.h"

void Console_EarlyInit()
{
	// FIXME:
	extern void initialise_stdio();
	initialise_stdio();
}

void Console_LateInit()
{
	// FIXME register with device manager
}

void Console_SingleMode()
{
	portDISABLE_INTERRUPTS();
	// FIXME switch over stdio output
}

