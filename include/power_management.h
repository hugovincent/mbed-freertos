#ifndef Power_Management_h
#define Power_Management_h

#ifdef __cplusplus
extern "C" {
#endif

void PowerManagement_Idle();
void PowerManagement_Sleep();
__attribute__ ((noreturn)) void PowerManagement_PowerDown();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef Power_Management_h

