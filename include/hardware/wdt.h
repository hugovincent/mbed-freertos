#ifndef Watchdog_h
#define Watchdog_h

#ifdef __cplusplus
extern "C" {
#endif

void WDT_Init(const unsigned int timeout_seconds); 
void WDT_Feed(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef Watchdog_h

