#include "cmsis.h"
#include "device_manager.h"
#include "stdio_device.h"
#include "romfs.h"

#include "FreeRTOS.h"
#include "task.h"

#ifdef CORE_HAS_MPU
#include "mpu_manager.h"
#endif

void SystemInit()
{
#ifdef CORE_HAS_MPU
	MpuManager_Init();
#endif
	DeviceManager_Init();
	RomFS_Init();

	printf("FreeRTOS Kernel " tskKERNEL_VERSION_NUMBER
			" for " PLAT_NAME " booted.\n");
}

void BootInit()
{
	extern void BoardInit();
	extern void LowLevelInit();
	extern void BoardDeviceInit();
	extern int _Z10__cxx_mainv();

	LowLevelInit();
	BoardInit();
	SystemInit();
	BoardDeviceInit();

	// Call into C++ main (name mangled because it's C++) via a shim that
	// ensures global constructors get called etc.
	_Z10__cxx_mainv();
}

