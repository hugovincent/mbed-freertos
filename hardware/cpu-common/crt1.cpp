
//*****************************************************************************
#if defined (__cplusplus)
extern "C" {
#endif

void CrtStart(void);
extern void Reset_Handler(void);
extern void BoardInit(void);
extern void LowLevelInit(void);
extern void MpuManager_Init(void);
extern void SystemInit();

#if defined (__cplusplus)
} // extern "C"
#endif


extern int __cxx_main(void);

void CrtStart() {

	LowLevelInit();
	BoardInit();
	SystemInit();
#ifdef TARGET_LPC1768
	MpuManager_Init();
#endif

	__cxx_main();

	//
	// main() shouldn't return, but if it does reset
	//
	Reset_Handler();
}


