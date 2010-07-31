#include <cmsis.h>
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


int  Debug_ValidMemory(unsigned int *addr)
{
	/* These symbols are defined by the linker script. */
	extern unsigned int __data_start__, __top_of_stack__;

	return (addr >= &__data_start__
			&& addr < &__top_of_stack__) ? 1 : 0;
}

void Debug_PrintBacktraceHere(int skip_frames)
{
	register unsigned int *fp asm("r11");
	Debug_PrintBacktrace(fp, skip_frames + 1); // We don't want Debug_PrintBacktraceHere in the backtrace
}

/* Assumes a full AAPCS stack frame, so compile with:
		-mapcs-frame -fno-omit-frame-pointer.

   See http://csg.lbl.gov/pipermail/vxwexplo/2004-February/004397.html,
   and the stack dump implementations in Ethernet Nut/OS and LostARM.
 */
#define NEXT_FRAME_VALID() (next_frame < (frame - sizeof(struct StackFrame)) \
		&& next_frame > next_frame - 1024)
void Debug_PrintBacktrace(unsigned int *fp, int skip_frames)
{
	struct StackFrame {
		struct StackFrame *fp;
		unsigned int sp;
		unsigned int lr; // LR is offset by 8 due to CPU pipeline
		unsigned int pc;
	} *frame, *next_frame;

	// The frame pointer points to the end of the struct, so we need to
	// manually offset the pointer address
	frame = (struct StackFrame *)(fp - sizeof(struct StackFrame) / sizeof(int*));

	// Walk the linked list as long as we remain in seemingly-valid frames
	int depth = 0;
	while (Debug_ValidMemory((unsigned int *)frame)
			&& depth < (MAX_BACKTRACE_FRAMES + skip_frames))
	{
		// Get the next frame (similarly offset as above)
		next_frame = (struct StackFrame *)(frame->fp - 3);

		depth++;
		if (depth > skip_frames)
		{
			Debug_Printf("\t#%d: [<%08x>] called from [<%08x>]\n",
					depth - skip_frames, frame->pc, frame->lr - 8);

			Debug_Printf("\t\tframe : %p    next_frame : %p\n", frame, next_frame);

			// Print relevant part of the stack itself
			if (NEXT_FRAME_VALID())
			{
				unsigned int *j = (unsigned int *)next_frame + 4;
				unsigned int *k = (unsigned int *)frame;

				int count = 0;
				for (; j < k && count < MAX_BACKTRACE_STACKDETAIL; j++)
				{
					count++;
					if ((count % 3) == 1);
						Debug_Puts("\t");
					Debug_Printf("\t%08x", *j);
					if ((count % 3) == 0)
						Debug_Puts("\n");
				}
			}
		}

		// Does the next frame appear somewhat valid?
		if (NEXT_FRAME_VALID())
			frame = next_frame;
		else
			break;
	}

	if (depth == 0)
		Debug_Puts("\t(Stack frame corrupt?)\n");
	else if (depth >= MAX_BACKTRACE_FRAMES)
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

/*****************************************************************************/

/* Debug Communications Channel Output (libdcc):
 * Provided with OpenOCD in the contrib/ directory.
 *
 *	Copyright (C) 2008 by Dominic Rath
 *	Dominic.Rath@gmx.de
 *	Copyright (C) 2008 by Spencer Oliver
 *	spen@spen-soft.co.uk
 *	Copyright (C) 2008 by Frederik Kriewtz
 *	frederik@kriewitz.eu
 */

#define TARGET_REQ_DEBUGMSG_ASCII			0x01
#define TARGET_REQ_DEBUGMSG_HEXMSG(size)	(0x01 | ((size & 0xff) << 8))
#define TARGET_REQ_DEBUGCHAR				0x02

#if defined(TARGET_LPC17xx)

/* We use the Cortex-M3 DCRDR reg to simulate a ARM7/9 dcc channel
 *     DCRDR[7:0] is used by target for status
 *     DCRDR[15:8] is used by target for write buffer
 *     DCRDR[23:16] is used for by host for status
 *     DCRDR[31:24] is used for by host for write buffer
 */
static void __dcc_write(unsigned long dcc_data)
{
	int len = 4;

	while (len--)
	{
		/* wait for data ready */
		while (CoreDebug->DCRDR & 1 /* Busy */);

		/* write our data and set write flag - tell host there is data*/
		CoreDebug->DCRDR = (unsigned short)(((dcc_data & 0xff) << 8) | 1 /* Busy */);
		dcc_data >>= 8;
	}
}

#elif defined(TARGET_LPC23xx)

static void __dcc_write(unsigned long dcc_data)
{
	unsigned long dcc_status;

	do {
		asm volatile("mrc p14, 0, %0, c0, c0" : "=r" (dcc_status));
	} while (dcc_status & 0x2);

	asm volatile("mcr p14, 0, %0, c1, c0" : : "r" (dcc_data));
}

#else
#error libdcc does not support this target!
#endif

void DCC_Write(const unsigned char *val, long len)
{
	unsigned long dcc_data;

	__dcc_write(TARGET_REQ_DEBUGMSG_HEXMSG(1) | ((len & 0xffff) << 16));

	while (len > 0)
	{
		dcc_data = val[0]
			| ((len > 1) ? val[1] << 8 : 0x00)
			| ((len > 2) ? val[2] << 16 : 0x00)
			| ((len > 3) ? val[3] << 24 : 0x00);

		__dcc_write(dcc_data);

		val += 4;
		len -= 4;
	}
}

void DCC_Putc(char msg)
{
	__dcc_write(TARGET_REQ_DEBUGCHAR | ((msg & 0xff) << 16));
}

void DCC_Puts(const char *msg)
{
	long len;
	unsigned long dcc_data;

	for (len = 0; msg[len] && (len < 65536); len++);

	__dcc_write(TARGET_REQ_DEBUGMSG_ASCII | ((len & 0xffff) << 16));

	while (len > 0)
	{
		dcc_data = msg[0]
			| ((len > 1) ? msg[1] << 8 : 0x00)
			| ((len > 2) ? msg[2] << 16 : 0x00)
			| ((len > 3) ? msg[3] << 24 : 0x00);
		__dcc_write(dcc_data);

		msg += 4;
		len -= 4;
	}
}

