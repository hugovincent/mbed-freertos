#ifndef Console_h_
#define Console_h_

void Console_Init();

// SingleMode pauses the scheduler, disables IRQs and FIQs, and switches the
// console to a unbuffered, blocking, debug version safe to use during
// exception recovery etc.
void Console_SingleMode();
void Console_NormalMode();

#endif // ifndef Console_h_

