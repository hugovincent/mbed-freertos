#include "device_manager.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis.h"

// This initialises the stdio layer in Newlib.
extern void initialise_stdio (void);

void SystemInit()
{
	DeviceManager_Init();
	initialise_stdio();

	printf("FreeRTOS Kernel " tskKERNEL_VERSION_NUMBER 
			" for " PLAT_NAME " booted.\n");
}
