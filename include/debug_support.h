/* This file provides utility functions to make debugging nicer.
 *
 * Hugo Vincent, 23 July 2010.
 */

#ifndef Debug_Support_h
#define Debug_Support_h

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Debug_RegisterDump {
	unsigned int r[13];
	unsigned int pc;
	unsigned int pc_ptr;
	unsigned int cpsr;
	unsigned int lr;
	unsigned int sp;
};

#define MAX_BACKTRACE_FRAMES		(12)
#define MAX_BACKTRACE_STACKDETAIL	(16)
void Debug_PrintBacktrace(unsigned int *fp, int skip_frames);
void Debug_PrintBacktraceHere(int skip_frames);

void Debug_PrintSavedRegisterState(struct Debug_RegisterDump *regs);
void Debug_PrintCPSR(unsigned int cpsr);
bool Debug_ValidAddress_RAM(unsigned int *addr);
bool Debug_ValidAddress_Flash(unsigned int *addr);

/* ------------------------------------------------------------------------- */
/* JTAG Debug Communications Channel (for use with a JTAG debugger): */
void DCC_Putc(char msg);
void DCC_Puts(const char *msg);
void DCC_Write(const unsigned char *val, long len);

/* ------------------------------------------------------------------------- */
/* Semihosting calls (for use with a JTAG debugger or mbed interface chip): */

enum SemihostReasons {
	// Standard ARM Semihosting Commands:
	Semihost_SYS_OPEN   = 0x1,
	Semihost_SYS_CLOSE  = 0x2,
	Semihost_SYS_WRITE  = 0x5,
	Semihost_SYS_READ   = 0x6,
	Semihost_SYS_ISTTY  = 0x9,
	Semihost_SYS_SEEK   = 0xa,
	Semihost_SYS_ENSURE = 0xb,
	Semihost_SYS_FLEN   = 0xc,

	// Custom Functions:
	Semihost_USR_XFFIND = 0x100, // mbed, getting listings of files
	Semihost_USR_UID    = 0x101, // mbed, get unique ID and ethernet MAC address
};

static inline int SemihostCall(enum SemihostReasons reason, void *arg)
{
	// Based on arm/syscalls.c from Newlib and James Snyder's semifs.

	// For Thumb-2 code use the BKPT instruction instead of SWI.
#ifdef __thumb2__
	#define AngelSWI 		0xAB
	#define AngelSWIInsn	"bkpt"
#else
	#define AngelSWI		0x123456
	#define AngelSWIInsn	"swi"
#endif

	int value;
	asm volatile ("mov r0, %1; mov r1, %2; " AngelSWIInsn " %a3; mov %0, r0"
			: "=r" (value) // Outputs
			: "r" (reason), "r" (arg), "i" (AngelSWI) // Inputs
			: "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc");
	return value;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef Debug_Support_h


