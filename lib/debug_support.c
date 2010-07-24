#include "debug_support.h"
#include "hardware/uart.h"

/* Preallocated buffer for Debug_Printf() */
char Debug_MsgBuffer[64];

/* In combination with uart0PutChar_debug, this is guaranteed to be safe when
 * IRQs are disabled, and can also be used at other times when the OS/scheduler,
 * stack or memory integrity can not be relied on.
 */
void Debug_Puts(const char *str)
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

/* Assumes a full APCS stack frame, so compile with -mapcs-frame and don't
   use -fomit-frame-pointer (enabled by default for -O2).
   
   The structure of a stack trace is as follows,
		low: previous fp (frame pointer)  -12 (3 word)
			 previous sp (stack pointer)   -8 (2 word)
			 previous lr (link register)   -4 (1 word)
		fp-> previous pc (the function)     0

   See http://csg.lbl.gov/pipermail/vxwexplo/2004-February/004397.html
 */   
void Debug_PrintBacktrace(const unsigned int *fp)
{
	int depth = 0;
	unsigned int function;

	while (Debug_ValidMemory((unsigned int)fp))
	{
		depth++;

		function = (*fp) - 16;
		Debug_Printf("\t#%d: [<%08x>] called from [<%08x>]\n", depth,
				function, *(fp - 1) - 8);

		// FIXME would ideally print stack local variables here too?

		fp = (unsigned int *)*(fp - 3);
	}
	if (depth == 0)
		Debug_Puts("\t(Stack frame corrupt?)\n");
}


void Debug_PrintCPSR(const unsigned int spsr)
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
	Debug_Printf("\tpsr: %08x (%c%c%c%c...%c%c%c %s-mode)\n", 
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
void Debug_PrintSavedRegisterState(const unsigned int *regs)
{
	for (int i  = 0; i < 10; i++)
	{
		Debug_Printf("\tr%d : %08x", i, regs[i]);
		if (((i + 1) % 3) == 0)
			Debug_Puts("\n");
	}
	Debug_Printf("\tr10: %08x\tfp : %08x\n", 
			regs[10], regs[11]);
	Debug_Printf("\tip : %08x\tsp : %08x\tlr : %08x\n",
			regs[12], regs[13], regs[14]);
}
