/* This file provides utility functions to make debugging nicer.
 *
 * Hugo Vincent, 23 July 2010.
 */

#ifndef Debug_Support_h
#define Debug_Support_h

#ifdef __cplusplus
extern "C" {
#endif

void Debug_Puts(const char *str);
#define Debug_Printf(fmt, args...) { \
	extern char Debug_MsgBuffer[64]; \
	sprintf(Debug_MsgBuffer, fmt, args); \
	Debug_Puts(Debug_MsgBuffer); \
}

struct Debug_RegisterDump {
	unsigned int pc;
	unsigned int pc_ptr;
	unsigned int cpsr;
	unsigned int lr;
	unsigned int sp;
	unsigned int r[13];
};

void Debug_PrintSavedRegisterState(struct Debug_RegisterDump *regs);
void Debug_PrintCPSR(unsigned int cpsr);
void Debug_PrintBacktrace(unsigned int fp);

#define MAX_BACKTRACE_FRAMES	(12)

// FIXME these should be pulled in magically though linker-foo
#define RAM_BASE (0x40000000)
#define RAM_LENGTH (0x8000)

#define Debug_ValidMemory(addr) (addr >= RAM_BASE && addr < (RAM_BASE + RAM_LENGTH))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef Debug_Support_h


