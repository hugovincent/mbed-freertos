/* These files provides default handlers for hardware exceptions (data and prefetch aborts,
 * undefined instruction, unhandled FIQ/IRQ) on the ARM architecture. It is the responsibiltiy
 * of the bootcode to attach the handlers to the exception vectors.
 *
 * Hugo Vincent, 6 May 2010.
 */

#include <stdio.h>
#include <exception_handlers.h>
#include "hardware/uart.h"


/* This array is written to with register contents by the assembler part of these
 * exception handlers.
 */
unsigned int AbortRegisterState[13];

char msgBuffer[32]; /* Temporary space for sprintf'ing into. */

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

/* In combination with uart0PutChar_debug, this is guaranteed to be safe when
 * IRQs are disabled, and can also be used at other times when the OS/scheduler,
 * stack or memory integrity can not be relied on.
 */
void DebugPrint(const char *str)
{
	while (*str)
	{
		/* Make line endings behave like normal serial terminals. */
		if (*str == '\n')
		{
			uart0PutChar_debug('\r', 0);
		}
		uart0PutChar_debug(*str++, 0);
	}
}

/* Print as much info as we can about the processor state pre-exception. */
__attribute__ ((noreturn)) void PrintAbortInfo(unsigned int addr)
{
	sprintf(msgBuffer, " at PC = 0x%08x\n", addr);
	DebugPrint(msgBuffer);

	// Print registers R0...R12
	DebugPrint("Procesor State:\n");
	for (int i  = 0; i <= 12; i++)
	{
		if (i < 10)
			sprintf(msgBuffer, "\tR%d  = 0x%08x", i, AbortRegisterState[i]);
		else
			sprintf(msgBuffer, "\tR%d = 0x%08x", i, AbortRegisterState[i]);
		DebugPrint(msgBuffer);

		if (((i + 1) % 3) == 0)
			DebugPrint("\n");
	}
	DebugPrint("\nHalting.\n\n");

	// Put processor core into sleep mode to conserve power.
	while (1);
		LPC_SC->PCON |= 0x5;
}

/* Lock down the processor. (For some reason, interrupts don't get automatically disabled?!) */
void LockDownProcessor(void)
{
	portDISABLE_INTERRUPTS();
	LPC_VIC->IntEnClr = 0xffffffff;
}

__attribute__ ((interrupt, noreturn)) void Exception_PrefetchAbort(unsigned int addr)
{
	LockDownProcessor();
	DebugPrint("\n[Fatal Error] Prefetch Abort");
	PrintAbortInfo(addr);
}

__attribute__ ((interrupt, noreturn)) void Exception_DataAbort(unsigned int addr)
{
	LockDownProcessor();
	DebugPrint("\n[Fatal Error] Data Abort");
	PrintAbortInfo(addr);
}

__attribute__ ((interrupt, noreturn)) void Exception_UndefinedInstruction(unsigned int addr)
{
	LockDownProcessor();
	DebugPrint("\n[Fatal Error] Undefined Instruction");
	PrintAbortInfo(addr);
}

__attribute__ ((interrupt, noreturn)) void Exception_UnhandledIRQ()
{
	LockDownProcessor();
	DebugPrint("\n[Fatal Error] Unhandled/Spurious IRQ.\n");
	// FIXME try to print *which* interrupt it was...
	while (1);
}

__attribute__ ((interrupt, noreturn)) void Exception_UnhandledFIQ()
{
	LockDownProcessor();
	DebugPrint("\n[Fatal Error] Unhandled/Spurious FIQ.\n");
	// FIXME try to print *which* interrupt it was...
	while (1);
}

