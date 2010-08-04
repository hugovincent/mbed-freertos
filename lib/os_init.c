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
	Console_EarlyInit();

	// Libc-provided function to initialize global structures e.g.
	// calling constructors on global C++ objects
	//extern void __libc_init_array();
	//__libc_init_array();

	System_Init();
	Console_LateInit();
	Board_LateInit();

	extern int __main() __attribute__ ((weak));
	if (__main)
		__main();
	extern int main();
	main();

	// If main() returns, call the finalizers/destructors
	extern void __libc_fini_array();
	__libc_fini_array();
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

