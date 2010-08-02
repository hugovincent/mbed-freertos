#include "os_init.h"

#include "device_manager.h"
#include "console.h"
#include "romfs.h"

#ifdef CORE_HAS_MPU
#include "mpu_manager.h"
#endif

/* For kernel version message: */
#include "FreeRTOS.h"
#include "task.h"

void Boot_Init()
{
	LowLevel_Init();
	Board_EarlyInit();
	Console_Init();
	System_Init();
	Board_LateInit();

	// Call into C++ main (name mangled because it's C++) via a shim that
	// ensures global constructors get called etc.
	extern int _Z10__cxx_mainv();
	_Z10__cxx_mainv();
}

void System_Init()
{
#ifdef CORE_HAS_MPU
	MpuManager_Init();
#endif
	DeviceManager_Init();
	RomFS_Init();

	printf("FreeRTOS Kernel " tskKERNEL_VERSION_NUMBER
			" for " PLAT_NAME " booted.\n");
}

