#include "device_manager.h"

void DeviceManager_Init()
{

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
