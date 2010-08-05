#include "os_init.h"
#include "cmsis.h"
 
#include "power_management.h"
#include "device_manager.h"
#include "console.h"
#include "romfs.h"

#ifdef CORE_HAS_MPU
#include "mpu_manager.h"
#endif

/* For kernel version message: */
#include "FreeRTOS.h"
#include "task.h"

void OS_Init();
void Board_EarlyInit();
void Board_LateInit();

extern void __libc_init_array(void);
extern int __main() __attribute__ ((weak));
extern int main(int, char **, char **);
extern void exit(int) __attribute__ ((noreturn, weak));

void Boot_Init()
{
	SystemInit(); // CMSIS system initialization
	Board_EarlyInit();
	Console_EarlyInit();

	// Libc-provided function to initialize global structures e.g.
	// calling constructors on global C++ objects
	__libc_init_array();

	OS_Init();
	Console_LateInit();
	Board_LateInit();

	if (__main)
		__main();
	int return_code = main(0, NULL, NULL);

	// If main() returns, call the finalizers/destructors
	extern void __libc_fini_array();
	__libc_fini_array();

	// If we have exit() call that
	if (exit)
		exit(return_code);

	// If exit() returns (!) power down the board
	PowerManagement_PowerDown();
}

void OS_Init()
{
#ifdef CORE_HAS_MPU
	MPUManager_Init();
#endif
	DeviceManager_Init();
	RomFS_Init();

	printf("FreeRTOS Kernel " tskKERNEL_VERSION_NUMBER " for " PLAT_NAME 
			" @ %lu MHz booted.\n", SystemCoreClock / 1000000);
}

