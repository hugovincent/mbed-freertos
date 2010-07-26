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

/* This global struct is filled out with register state by the assembler
 * component of the exception handlers (in hardware/cpu-X/crt0.s).
 */
struct Debug_RegisterDump SavedRegs;

enum ExceptionType
{
	DataAbort = 0,
	PrefetchAbort,
	UndefinedInstruction
};

/* Print as much info as we can about the processor state pre-exception. */
static __attribute__ ((noreturn)) void PrintAbortInfo(enum ExceptionType type)
{
	// pc is the actual address, and pc_ptr is the actually opcode at that address
	switch (type)
	{
		case DataAbort:
			Debug_Puts("\n[FreeRTOS] Fatal Error: Data Abort");
			Debug_Printf(" at pc : [<%08x>] -> 0x%08x\n", SavedRegs.pc, SavedRegs.pc_ptr);
			break;

		case PrefetchAbort:
			Debug_Puts("\n[FreeRTOS] Fatal Error: Prefetch Abort");
			Debug_Printf(" at pc : [<%08x>]\n", SavedRegs.pc);
			break;

		case UndefinedInstruction:
			Debug_Puts("\n[FreeRTOS] Fatal Error: Undefined Instruction");
			Debug_Printf(" 0x%08x at pc : [<%08x>]\n", SavedRegs.pc_ptr, SavedRegs.pc);
			break;
	}

	Debug_Puts("\nProcesor State:\n");
	Debug_PrintSavedRegisterState(&SavedRegs);

	Debug_Puts("\nBacktrace:\n");
	Debug_PrintBacktrace(SavedRegs.r[11]); // r11 is the frame pointer

	// FIXME some FreeRTOS-specific thread information should go here?

	Debug_Puts("\nHalting.\n\n");

	// Put processor core into sleep mode to conserve power.
	PowerManagement_PowerDown();
}


__attribute__ ((noreturn)) void Exception_PrefetchAbort()
{
	portDISABLE_INTERRUPTS();
	PrintAbortInfo(PrefetchAbort);
}


__attribute__ ((noreturn)) void Exception_DataAbort()
{
	portDISABLE_INTERRUPTS();
	PrintAbortInfo(DataAbort);
}


__attribute__ ((noreturn)) void Exception_UndefinedInstruction()
{
	portDISABLE_INTERRUPTS();
	PrintAbortInfo(UndefinedInstruction);
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

