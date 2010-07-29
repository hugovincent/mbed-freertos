/* Device manager for FreeRTOS.
 *
 *	Hugo Vincent, 24 July 2010.
 */

#ifndef DeviceManager_h
#define DeviceManager_h

#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function pointer prototypes
typedef int (*FileLikeObj_RdWr)(int fd, void *buf, size_t len);
typedef int (*FileLikeObj_Ioctl)(int fd, unsigned long request, ...);
typedef int (*FileLikeObj_Open)(const char *path, int flags, int mode); 
typedef int (*FileLikeObj_FdOp)(int fd); 
typedef int (*FileLikeObj_Fstat)(int fd, struct stat *st);
typedef int (*FileLikeObj_Lseek)(int fd, _off_t offs, int dir);

/* A file-like object (FLO) is a node in the root filesystem, and thus behaves 
 * like a file. This encompasses regular files, and notably devices such as
 * serial ports. A FileLikeObj is located by its path, which can be either a
 * prefix, for example, "/flash/" (in which case it handles all decendent 
 * paths as well; denoted by the trailing forward slash) or a leaf such
 * as "/dev/uart/1".
 */
struct FileLikeObj {

	// This is the prefix, e.g. "/dev/". Can't be "/".
	const char *pathprefix;

	// 1 if this FLO has no children in the filesystem, 0 if is a hierarchy.
	int is_leaf;

	// Standard operations:
	FileLikeObj_RdWr 	read_;
	FileLikeObj_RdWr 	write_;
	FileLikeObj_Ioctl	ioctl_;
	FileLikeObj_Open 	open_;
	FileLikeObj_FdOp	close_;
	FileLikeObj_Fstat	fstat_;
	FileLikeObj_FdOp	fsync_;
	FileLikeObj_FdOp	isatty_;
	FileLikeObj_Lseek	lseek_;
};

// Initialise internal datastructures of the device manager. Must be called
// before registering any devices.
void DeviceManager_Init();

// Register a device or filesystem with the device manager.
int  DeviceManager_Register(struct FileLikeObj *dev);

// Retrieve the FileLikeObj struct that is responsible for handling a given path.
struct FileLikeObj *DeviceManager_GetDeviceForPath(const char *path);

// Retrieve the FileLikeObj struct in use by a specific file descriptor.
struct FileLikeObj *DeviceManager_GetDeviceForFd(int fd);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef Device_Manager_h
