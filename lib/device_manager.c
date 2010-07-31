#include "device_manager.h"
#include "semifs.h"

void DeviceManager_Init()
{
	// FIXME check this board actually has a semihosted filesystem?
	SemiFS_Init();
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
