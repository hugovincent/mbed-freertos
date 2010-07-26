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

#define MAX_BACKTRACE_FRAMES	(12)
void Debug_PrintBacktrace(unsigned int fp);

void Debug_PrintSavedRegisterState(struct Debug_RegisterDump *regs);
void Debug_PrintCPSR(unsigned int cpsr);
int  Debug_ValidMemory(unsigned int addr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef Debug_Support_h


