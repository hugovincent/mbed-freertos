#ifndef OS_Init_h
#define OS_Init_h

#ifdef __cplusplus
extern "C" {
#endif

void LowLevel_Init();
void Boot_Init();
void System_Init();
void Board_EarlyInit();
void Board_LateInit();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef OS_Init_h

