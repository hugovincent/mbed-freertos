/* These files provides default handlers for hardware exceptions (data and prefetch aborts, 
 * undefined instruction, unhandled FIQ/IRQ) on the ARM architecture. It is the responsibiltiy
 * of the bootcode to attach the handlers to the exception vectors. 
 *
 * Hugo Vincent, 6 May 2010.
 */

#ifndef Exception_Handlers_h
#define Exception_Handlers_h

#ifdef __cplusplus
extern "C" {
#endif

void Exception_PrefetchAbort(void);
void Exception_DataAbort(void);
void Exception_UndefinedInstruction(void);
void Exception_UnhandledIRQ(void);
void Exception_UnhandledFIQ(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef Exception_Handlers_h

