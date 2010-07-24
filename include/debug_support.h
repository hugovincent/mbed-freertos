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
void Debug_PrintSavedRegisterState(const unsigned int *regs);
void Debug_PrintCPSR(const unsigned int spsr);
void Debug_PrintBacktrace(const unsigned int *fp);

// FIXME these should be pulled in magically though linker-foo
#define RAM_BASE (0x40000000)
#define RAM_LENGTH (0x8000)

#define Debug_ValidMemory(addr) (addr >= RAM_BASE && addr < (RAM_BASE + RAM_LENGTH))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef Debug_Support_h


