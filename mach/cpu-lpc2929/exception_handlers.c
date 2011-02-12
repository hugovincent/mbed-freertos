/* This file provides default handlers for hardware exceptions (data and prefetch
 * aborts, undefined instruction, unhandled FIQ/IRQ) on the ARM7 architecture.
 *
 * Hugo Vincent, 23 July 2010.
 */

#include <stdio.h>
#include "console.h"
#include "debug_support.h"
#include "power_management.h"

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
static __attribute__ ((noreturn)) void __print_info(enum ExceptionType type)
{
	// pc is the actual address, and pc_ptr is the actually opcode at that address
	switch (type)
	{
		case DataAbort:
			printf("\n[FreeRTOS] Fatal Error: Data Abort at pc : ");
			printf("[<%08x>] -> 0x%08x\n", SavedRegs.pc, SavedRegs.pc_ptr);
			break;

		case PrefetchAbort:
			printf("\n[FreeRTOS] Fatal Error: Prefetch Abort at pc : ");
			printf("[<%08x>]\n", SavedRegs.pc);
			break;

		case UndefinedInstruction:
			printf("\n[FreeRTOS] Fatal Error: Undefined Instruction ");
			printf("0x%08x at pc : [<%08x>]\n", SavedRegs.pc_ptr, SavedRegs.pc);
			break;
	}

	puts("\nProcesor State:");
	Debug_PrintSavedRegisterState(&SavedRegs);

	puts("\nBacktrace:");
	Debug_PrintBacktrace((unsigned int *)SavedRegs.r[11], 0); // r11 is the frame pointer

	// FIXME some FreeRTOS-specific thread information should go here?

	puts("\nHalting.\n");

	// Put processor core into sleep mode to conserve power.
	PowerManagement_PowerDown();
}


__attribute__ ((noreturn)) void Exception_PrefetchAbort()
{
	Console_SingleMode();
	__print_info(PrefetchAbort);
}


__attribute__ ((noreturn)) void Exception_DataAbort()
{
	Console_SingleMode();
	__print_info(DataAbort);
}


__attribute__ ((noreturn)) void Exception_UndefinedInstruction()
{
	Console_SingleMode();
	__print_info(UndefinedInstruction);
}


__attribute__ ((noreturn)) void Exception_UnhandledIRQ()
{
	Console_SingleMode();
	puts("\n[FreeRTOS] Fatal Error: Unhandled/Spurious IRQ.");
	// FIXME try to print *which* interrupt it was...
	PowerManagement_PowerDown();
}


__attribute__ ((noreturn)) void Exception_UnhandledFIQ()
{
	Console_SingleMode();
	puts("\n[FreeRTOS] Fatal Error: Unhandled/Spurious FIQ.");
	// FIXME try to print *which* interrupt it was...
	PowerManagement_PowerDown();
}

__attribute__ ((noreturn)) void Exception_HardFault()
{
	Console_SingleMode();
	puts("\n[FreeRTOS] Fatal Error: HardFault.");
	// FIXME try to print all the details about it
	PowerManagement_PowerDown();
}

