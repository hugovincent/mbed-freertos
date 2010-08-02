#include "FreeRTOS.h"
#include "console.h"

void Console_Init()
{
	// FIXME:
	extern void initialise_stdio();
	initialise_stdio();
}

void Console_SingleMode()
{
	portDISABLE_INTERRUPTS();
	// FIXME switch over stdio output
}

