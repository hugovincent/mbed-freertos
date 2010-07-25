/* This file provides default handlers for hardware exceptions (data and prefetch
 * aborts, undefined instruction, unhandled FIQ/IRQ) on the ARM architecture. It
 * is the responsibiltiy of the bootcode to attach the handlers to the exception
 * vectors, and the bootcode must also fill out the SavedRegs array. 
 *
 * Hugo Vincent, 23 July 2010.
 */

#include <stdio.h>
#include "exception_handlers.h"
#include "debug_support.h"
#include "power_management.h"

#include "FreeRTOS.h"

/* This array is written to with register contents by the assembler part of the
 * exception handlers (in hardware/cpu-X/crt0.s). The CPSR goes on the end.
 */
unsigned int SavedRegs[16];

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
	PowerManagement_PowerDown();
}

__attribute__ ((noreturn)) void Exception_PrefetchAbort(unsigned int addr)
{
	portDISABLE_INTERRUPTS();
	Debug_Puts("\n[FreeRTOS] Fatal Error: Prefetch Abort");
	PrintAbortInfo(addr);
}

__attribute__ ((noreturn)) void Exception_DataAbort(unsigned int addr)
{
	portDISABLE_INTERRUPTS();
	Debug_Puts("\n[FreeRTOS] Fatal Error: Data Abort");
	PrintAbortInfo(addr);
}

__attribute__ ((noreturn)) void Exception_UndefinedInstruction(unsigned int addr)
{
	portDISABLE_INTERRUPTS();
	Debug_Puts("\n[FreeRTOS] Fatal Error: Undefined Instruction");
	PrintAbortInfo(addr);
}

__attribute__ ((noreturn)) void Exception_UnhandledIRQ()
{
	portDISABLE_INTERRUPTS();
	Debug_Puts("\n[FreeRTOS] Fatal Error: Unhandled/Spurious IRQ.\n");
	// FIXME try to print *which* interrupt it was...
	PowerManagement_PowerDown();
}

__attribute__ ((noreturn)) void Exception_UnhandledFIQ()
{
	portDISABLE_INTERRUPTS();
	Debug_Puts("\n[FreeRTOS] Fatal Error: Unhandled/Spurious FIQ.\n");
	// FIXME try to print *which* interrupt it was...
	PowerManagement_PowerDown();
}

