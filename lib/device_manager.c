#include <errno.h>
#include "device_manager.h"
#include "device_manager_driver.h"

struct DeviceManager_FdStruct *DeviceManager_NewFd()
{
	return NULL;
}

struct DeviceManager_FdStruct *DeviceManager_FdStructForFd(int fd)
{
	errno = EBADF;
	return NULL;
}

void DeviceManager_ReleaseFd(struct DeviceManager_FdStruct *fdstruct)
{

}

void DeviceManager_Init()
{
	// Create initial search tree
}

int  DeviceManager_Register(struct FileLikeObj *dev)
{
	return -1;
}

struct FileLikeObj *DeviceManager_GetDeviceForPath(const char *path)
{
	return (struct FileLikeObj *)NULL;
}

struct FileLikeObj *DeviceManager_GetDeviceForFd(int fd)
{
	return (struct FileLikeObj *)NULL;
}
