/* Device manager: driver-private API
 *
 * Hugo Vincent, 4 October 2010.
 */

#ifndef DeviceManagerDriver_h
#define DeviceManagerDriver_h

#include "device_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

// Register a device or filesystem with the device manager.
int DeviceManager_Register(struct FileLikeObj *dev);

// Data of use to user of device-manager (i.e. driver)
struct DeviceManager_FdStruct {
	unsigned short handle;	// internal (driver-specific) handle
	unsigned short fd;		// global (device-manager) fd
	int pos;				// position in file (if applicable)
	void *userdata;			// other user data (managed by driver)
};

// Return a new, currently unused file descriptor (global, excludes
// per-process stdin/out/err), wrapped in struct for convenience.
struct DeviceManager_FdStruct *DeviceManager_NewFd();

// Get userdata for a given fd
struct DeviceManager_FdStruct *DeviceManager_FdStructForFd(int fd);

// Release an fd struct (and fd) when no longer needed
void DeviceManager_ReleaseFd(struct DeviceManager_FdStruct *fdstruct);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef DeviceManagerDriver_h
