#include "cmsis.h"
#include "device_manager.h"

#include "FreeRTOS.h"
#include "task.h"

#ifdef TARGET_LPC1768
#include "mpu_manager.h"
#endif

void SystemInit()
{
	// This initialises the stdio layer in Newlib. FIXME temporary
	extern void initialise_stdio();

#ifdef TARGET_LPC1768
	MpuManager_Init();
#endif
	DeviceManager_Init();
	initialise_stdio();

	printf("FreeRTOS Kernel " tskKERNEL_VERSION_NUMBER
			" for " PLAT_NAME " booted.\n");
}

void BootInit()
{
	void BoardInit();
	void LowLevelInit();
	extern int _Z10__cxx_mainv();

	LowLevelInit();
	BoardInit();
	SystemInit();

	// Call into C++ main (name mangled because it's C++) via a shim that
	// ensures global constructors get called etc.
	_Z10__cxx_mainv();
}

