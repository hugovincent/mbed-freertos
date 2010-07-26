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


// FIXME these should be pulled in magically though linker-foo
#define RAM_BASE (0x40000000)
#define RAM_LENGTH (0x8000)
int  Debug_ValidMemory(unsigned int addr)
{
	return (addr >= RAM_BASE && addr < (RAM_BASE + RAM_LENGTH)) ? 1 : 0;
}


/* Assumes a full AAPCS stack frame, so compile with -mapcs-frame and don't
   use -fomit-frame-pointer (enabled by default for -O2).
   
   See http://csg.lbl.gov/pipermail/vxwexplo/2004-February/004397.html
 */
struct StackFrame {
	struct StackFrame *fp;
	unsigned int sp;
	unsigned int lr; // LR is offset by 8 due to pipelining
	unsigned int pc; // frame pointer points here (high address end of struct)
};
void Debug_PrintBacktrace(unsigned int fp)
{
	int depth = 0;
	struct StackFrame *frame = (struct StackFrame *)(fp - 3 * sizeof(int*));

	// Walk the linked list as long as we remain in seemingly-valid frames
	while (Debug_ValidMemory((unsigned int)frame) && depth < MAX_BACKTRACE_FRAMES)
	{
		depth++;

		Debug_Printf("\t#%d: [<%08x>] called from [<%08x>]\n", depth,
				frame->pc, frame->lr - 8); // LR is offset by 8 due to CPU pipeline

		// FIXME would ideally print stack local variables here too?

		frame = (struct StackFrame *)(frame->fp - 3 * sizeof(int*));
	}
	if (depth == 0)
		Debug_Puts("\t(Stack frame corrupt?)\n");
	else if (depth == MAX_BACKTRACE_FRAMES)
		Debug_Puts("\t... truncated ...\n");
}


void Debug_PrintCPSR(unsigned int psr)
{
	char *mode;
	switch (psr & 0x1f)
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
			psr,
			(psr & 1<<31) ? 'N' : 'n',
			(psr & 1<<30) ? 'Z' : 'z',
			(psr & 1<<29) ? 'C' : 'c',
			(psr & 1<<28) ? 'V' : 'v',
			(psr &  1<<7) ? 'F' : 'f',
			(psr &  1<<6) ? 'I' : 'i',
			(psr &  1<<5) ? 'T' : 't',
			mode);
}


/* Print as much info as we can about the processor state pre-exception. */
void Debug_PrintSavedRegisterState(struct Debug_RegisterDump *regs)
{
	for (int i  = 0; i < 10; i++)
	{
		Debug_Printf("\tr%d : %08x", i, regs->r[i]);
		if (((i + 1) % 3) == 0)
			Debug_Puts("\n");
	}
	Debug_Printf("\tr10: %08x\tfp : %08x\n", 
			regs->r[10], regs->r[11]);
	Debug_Printf("\tip : %08x\tsp : %08x\tlr : %08x\n",
			regs->r[12], regs->sp, regs->lr);

	Debug_PrintCPSR(regs->cpsr);
}
