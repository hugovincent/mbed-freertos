#include "os_init.h"
#include "device_manager.h"

// This initialises the stdio layer in Newlib.
extern void initialise_stdio (void);

void OperatingSystemInit()
{
	DeviceManager_Init();
	initialise_stdio();
}
