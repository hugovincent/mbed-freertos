/* This file provides default handlers for hardware exceptions (data and prefetch aborts,
 * undefined instruction, unhandled FIQ/IRQ) on the ARM architecture. It is the responsibiltiy
 * of the bootcode to attach the handlers to the exception vectors.
 *
 * Hugo Vincent, 23 July 2010.
 */

#include <stdio.h>
#include "exception_handlers.h"
#include "debug_support.h"

#include "FreeRTOS.h"

/* This array is written to with register contents by the assembler part of the
 * exception handlers (in hardware/cpu-X/crt0.s). The CPSR goes on the end.
 */
unsigned int SavedRegs[16];

/* Lock down the processor. (For some reason, interrupts don't get automatically disabled?!) */
static void LockDownProcessor(void)
{
	portDISABLE_INTERRUPTS();
#if defined(MBED_LPC23xx)
	LPC_VIC->IntEnClr = 0xffffffff;
#elif defined(MBED_LPC17xx)
	NVIC->ICER[0] = 0xffffffff;		// FIXME?
	NVIC->ICER[1] = 0x07;			// FIXME?
#endif
}

/* Print as much info as we can about the processor state pre-exception. */
__attribute__ ((noreturn)) void PrintAbortInfo(const unsigned int addr)
{
	Debug_Printf(" at pc : [<%08x>]\n", addr);

	// Print registers
	Debug_Puts("\nProcesor State:\n");
	Debug_PrintSavedRegisterState(SavedRegs);
	Debug_PrintCPSR(SavedRegs[15]);

	Debug_Puts("\nBacktrace:\n");
	Debug_PrintBacktrace((void *)SavedRegs[11]); // r11 is the frame pointer

	// FIXME some FreeRTOS-specific thread information should go here?

	Debug_Puts("\nHalting.\n\n");

	// Put processor core into sleep mode to conserve power.
	while (1) 
		LPC_SC->PCON |= 0x5;
}

__attribute__ ((noreturn)) void Exception_PrefetchAbort(unsigned int addr)
{
	LockDownProcessor();
	Debug_Puts("\n[FreeRTOS] Fatal Error: Prefetch Abort");
	PrintAbortInfo(addr);
}

__attribute__ ((noreturn)) void Exception_DataAbort(unsigned int addr)
{
	LockDownProcessor();
	Debug_Puts("\n[FreeRTOS] Fatal Error: Data Abort");
	PrintAbortInfo(addr);
}

__attribute__ ((noreturn)) void Exception_UndefinedInstruction(unsigned int addr)
{
	LockDownProcessor();
	Debug_Puts("\n[FreeRTOS] Fatal Error: Undefined Instruction");
	PrintAbortInfo(addr);
}

__attribute__ ((noreturn)) void Exception_UnhandledIRQ()
{
	LockDownProcessor();
	Debug_Puts("\n[FreeRTOS] Fatal Error: Unhandled/Spurious IRQ.\n");
	// FIXME try to print *which* interrupt it was...
	while (1);
}

__attribute__ ((noreturn)) void Exception_UnhandledFIQ()
{
	LockDownProcessor();
	Debug_Puts("\n[FreeRTOS] Fatal Error: Unhandled/Spurious FIQ.\n");
	// FIXME try to print *which* interrupt it was...
	while (1);
}

