/* These files provides default handlers for hardware exceptions (data and prefetch aborts,
 * undefined instruction, unhandled FIQ/IRQ) on the ARM architecture. It is the responsibiltiy
 * of the bootcode to attach the handlers to the exception vectors.
 *
 * Hugo Vincent, 6 May 2010.
 */

#include <stdio.h>
#include <exception_handlers.h>
#include "hardware/uart.h"

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

__attribute__ ((interrupt, noreturn)) void Exception_PrefetchAbort()
{
	DebugPrint("[Exception] Prefetch Abort.\n");
	while (1);
}

__attribute__ ((interrupt, noreturn)) void Exception_DataAbort()
{
	DebugPrint("[Exception] Data Abort.\n");
	while (1);
}

__attribute__ ((interrupt, noreturn)) void Exception_UndefinedInstruction()
{
	DebugPrint("[Exception] Undefined Instruction.\n");
	while (1);
}

__attribute__ ((interrupt, noreturn)) void Exception_UnhandledIRQ()
{
	DebugPrint("[Exception] Unhandled/Spurious IRQ.\n");
	while (1);
}

__attribute__ ((interrupt, noreturn)) void Exception_UnhandledFIQ()
{
	DebugPrint("[Exception] Unhandled/Spurious FIQ.\n");
	while (1);
}

