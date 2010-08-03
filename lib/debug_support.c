/* Debug support functions.
 *
 * Hugo Vincent, 23 July 2010.
 */

#include <unwind.h>
#include <cmsis.h>
#include "debug_support.h"
#include "drivers/uart.h"

bool Debug_ValidAddress_RAM(unsigned int *addr)
{
	/* These symbols are defined by the linker script. */
	extern unsigned int __SRAM_segment_start__, __SRAM_segment_end__;

	return (addr >= &__SRAM_segment_start__
			&& addr < &__SRAM_segment_end__) ? true : false;
}

bool Debug_ValidAddress_Flash(unsigned int *addr)
{
	/* These symbols are defined by the linker script. */
	extern unsigned int __FLASH_segment_start__, __FLASH_segment_end__;

	return (addr >= &__FLASH_segment_start__
			&& addr < &__FLASH_segment_end__) ? true : false;
}

/* Stack backtrace helper function */
_Unwind_Reason_Code trace_fcn(_Unwind_Context *ctx, void *d)
{
	int *priv_data = (int*)d;
	int *depth = &priv_data[0], *skip_frames = &priv_data[1];

	if (*depth >= *skip_frames)
	{
		printf("\t#%d: [<%08x>]\n", *depth - *skip_frames, _Unwind_GetIP(ctx));
	}
	(*depth)++;

	return _URC_NO_REASON;
}

void Debug_PrintBacktraceHere(int skip_frames)
{
	int priv_data[2] = {0, skip_frames + 1};
  	_Unwind_Backtrace(&trace_fcn, &priv_data[0]);
}

void Debug_PrintBacktrace(unsigned int *fp, int skip_frames)
{
	// FIXME the old method I had here relied on obsolete ABI that could be made
	// to work on ARM7 but can not work on ARM-CM3 with EABI. Need to misuse
	// <unwind.h>, .debug_frame, and the exception handling infrastructure to 
	// make this work...
	puts("\t(Stack frame corrupt?)");

	// FIXME?
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
	printf("\tpsr: %08x (%c%c%c%c...%c%c%c %s-mode)\n",
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
		printf("\tr%d : %08x", i, regs->r[i]);
		if (((i + 1) % 3) == 0)
			putchar('\n');
	}
	printf("\tr10: %08x\tfp : %08x\n",
			regs->r[10], regs->r[11]);
	printf("\tip : %08x\tsp : %08x\tlr : %08x\n",
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

