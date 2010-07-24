/* These files provides default handlers for hardware exceptions (data and prefetch aborts,
 * undefined instruction, unhandled FIQ/IRQ) on the ARM architecture. It is the responsibiltiy
 * of the bootcode to attach the handlers to the exception vectors.
 *
 * Hugo Vincent, 23 July 2010.
 */

#include <stdio.h>
#include <exception_handlers.h>
#include "hardware/uart.h"

// FIXME these should be pulled in magically though linker-foo
#define RAM_BASE (0x40000000)
#define RAM_LENGTH (0x8000)


/* This array is written to with register contents by the assembler part of the
 * exception handlers (in hardware/cpu-X/crt0.s). The CPSR goes on the end.
 */
unsigned int SavedRegs[16];


#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))
#define IN_VALID_MEM(addr) (addr >= RAM_BASE && addr < (RAM_BASE + RAM_LENGTH))


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

// FIXME: 
static char msgBuffer[64];
#define DebugPrintf(fmt, args...) { \
	sprintf(msgBuffer, fmt, args); \
	DebugPrint(msgBuffer); \
}


/* Assumes a full APCS stack frame, so compile with -mapcs-frame and don't
   use -fomit-frame-pointer (enabled by default for -O2).
   
   The structure of a stack trace is as follows,
		low: previous fp (frame pointer)  -12 (3 word)
			 previous sp (stack pointer)   -8 (2 word)
			 previous lr (link register)   -4 (1 word)
		fp-> previous pc (the function)     0

   See http://csg.lbl.gov/pipermail/vxwexplo/2004-February/004397.html
 */   
void PrintBacktrace(unsigned int *fp)
{
	int depth = 0;
	unsigned int function;

	while (IN_VALID_MEM((unsigned int)fp))
	{
		depth++;

		function = (*fp) - 16;
		DebugPrintf("\t#%d: [<%08x>] called from [<%08x>]\n", depth,
				function, *(fp - 1) - 8);

		// FIXME would ideally print stack local variables here too?

		fp = (unsigned int *)*(fp - 3);
	}
	if (depth == 0)
		DebugPrint("\t(Stack frame corrupt?)\n");
}


void PrintCPSR(unsigned int spsr)
{
	char *mode;
	switch (spsr & 0x1f)
	{
		case 0x10: mode = "user";		break;
		case 0x11: mode = "fiq";		break;
		case 0x12: mode = "irq";		break;
		case 0x13: mode = "svc";		break;
		case 0x17: mode = "abort";		break;
		case 0x1b: mode = "und";		break;
		case 0x1f: mode = "sys";		break;
		default:   mode = "unknown";	break;
	}
	DebugPrintf("\tpsr: %08x (%c%c%c%c...%c%c%c %s-mode)\n", 
			spsr,
			(spsr & 1<<31) ? 'N' : 'n',
			(spsr & 1<<30) ? 'Z' : 'z',
			(spsr & 1<<29) ? 'C' : 'c',
			(spsr & 1<<28) ? 'V' : 'v',
			(spsr &  1<<7) ? 'F' : 'f',
			(spsr &  1<<6) ? 'I' : 'i',
			(spsr &  1<<5) ? 'T' : 't',
			mode);
}


/* Print as much info as we can about the processor state pre-exception. */
__attribute__ ((noreturn)) void PrintAbortInfo(unsigned int addr)
{
	DebugPrintf(" at pc : [<%08x>]\n", addr);

	// Print registers
	DebugPrint("\nProcesor State:\n");
	for (int i  = 0; i < 10; i++)
	{
		DebugPrintf("\tr%d : %08x", i, SavedRegs[i]);
		if (((i + 1) % 3) == 0)
			DebugPrint("\n");
	}
	DebugPrintf("\tr10: %08x\tfp : %08x\n", 
			SavedRegs[10], SavedRegs[11]);
	DebugPrintf("\tip : %08x\tsp : %08x\tlr : %08x\n",
			SavedRegs[12], SavedRegs[13], SavedRegs[14]);

	PrintCPSR(SavedRegs[15]);

	DebugPrint("\nBacktrace:\n");
	PrintBacktrace((void *)SavedRegs[11]); // r11 is the frame pointer

	// FIXME some FreeRTOS-specific thread information should go here?

	DebugPrint("\nHalting.\n\n");

	// Put processor core into sleep mode to conserve power.
	while (1) LPC_SC->PCON |= 0x5;
}


/* Lock down the processor. (For some reason, interrupts don't get automatically disabled?!) */
void LockDownProcessor(void)
{
	portDISABLE_INTERRUPTS();
#if defined(MBED_LPC23xx)
	LPC_VIC->IntEnClr = 0xffffffff;
#elif defined(MBED_LPC17xx)
	NVIC->ICER[0] = 0xffffffff;		// FIXME?
	NVIC->ICER[1] = 0x07;			// FIXME?
#endif
}


/**** Entry points from assembly language stubs in crt0.s ********************/

__attribute__ ((noreturn)) void Exception_PrefetchAbort(unsigned int addr)
{
	LockDownProcessor();
	DebugPrint("\n[FreeRTOS] Fatal Error: Prefetch Abort");
	PrintAbortInfo(addr);
}

__attribute__ ((noreturn)) void Exception_DataAbort(unsigned int addr)
{
	LockDownProcessor();
	DebugPrint("\n[FreeRTOS] Fatal Error: Data Abort");
	PrintAbortInfo(addr);
}

__attribute__ ((noreturn)) void Exception_UndefinedInstruction(unsigned int addr)
{
	LockDownProcessor();
	DebugPrint("\n[FreeRTOS] Fatal Error: Undefined Instruction");
	PrintAbortInfo(addr);
}

__attribute__ ((noreturn)) void Exception_UnhandledIRQ()
{
	LockDownProcessor();
	DebugPrint("\n[FreeRTOS] Fatal Error: Unhandled/Spurious IRQ.\n");
	// FIXME try to print *which* interrupt it was...
	while (1);
}

__attribute__ ((noreturn)) void Exception_UnhandledFIQ()
{
	LockDownProcessor();
	DebugPrint("\n[FreeRTOS] Fatal Error: Unhandled/Spurious FIQ.\n");
	// FIXME try to print *which* interrupt it was...
	while (1);
}

