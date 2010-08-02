#ifndef Console_h_
#define Console_h_

#ifdef __cplusplus
extern "C" {
#endif

void Console_Init();

// SingleMode pauses the scheduler, disables IRQs and FIQs, and switches the
// console to a unbuffered, blocking, debug version safe to use during
// exception recovery etc.
void Console_SingleMode();
void Console_NormalMode();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef Console_h_

