#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>

#include "device_manager_driver.h"
#include "romfs.h"

struct file_entry {
	const char *filename;
	unsigned int offset;
	unsigned int length;
};

// This is the auto-generated filesystem data:
#include "lib/romfs_data.h"

/*****************************************************************************/

static int RomFS_Open(const char *path, int flags, int mode /* <-- ignored */)
{
	if ((flags & O_RDWR) || (flags & O_WRONLY) || (flags & O_APPEND)
			|| (flags & O_CREAT) || (flags & O_TRUNC))
	{
		errno = EROFS;
		return -1;
	}

	// Find file in filesystem
	unsigned short fh; 
	for (fh = 0; fh < romfs_numfiles; fh++)
	{
		const char *filename = romfs_header[fh].filename;
		if (strcmp(filename, path) == 0)
			break;
	}
	if (fh == romfs_numfiles)
	{
		errno = ENOENT;
		return -1;
	}

	// Create a device-manager fd
	struct DeviceManager_FdStruct *fdstruct = DeviceManager_NewFd();
	fdstruct->handle = fh;
	fdstruct->pos = 0;
	
	return fdstruct->fd;

}

static int RomFS_Close(int fd)
{
	struct DeviceManager_FdStruct *fdstruct = DeviceManager_FdStructForFd(fd);
	if (fdstruct == NULL)
		return -1;
	DeviceManager_ReleaseFd(fdstruct);
	return 0;
}

static ssize_t RomFS_Read(int fd, void *ptr, size_t len)
{
	struct DeviceManager_FdStruct *fdstruct = DeviceManager_FdStructForFd(fd);
	if (fdstruct == NULL)
		return -1;

	int bytes = romfs_header[fdstruct->handle].length - fdstruct->pos;
	if (bytes > len)
		bytes = len;

	// It is possible to lseek past the end of the file. read needs
	// to reset offset if necessary first.
	if (bytes < 0)
	{
		fdstruct->pos = romfs_header[fdstruct->handle].length;
		errno = EIO;
		return -1;
	}

	if (bytes > 0)
	{
		int offset = romfs_header[fdstruct->handle].offset + fdstruct->pos;
		memcpy(ptr, (void*)&(romfs_data[offset]), bytes);

		fdstruct->pos += bytes;
	}
	return bytes;
}

static off_t RomFS_Lseek(int fd, off_t off, int whence)
{
	struct DeviceManager_FdStruct *fdstruct = DeviceManager_FdStructForFd(fd);
	if (fdstruct == NULL)
		return -1;

	switch (whence)
	{
		case SEEK_CUR:
			// seek from current position
			off += fdstruct->pos;
			break;

		case SEEK_END:
			// seek from end of file
			off += romfs_header[fdstruct->handle].length;
			break;

		case SEEK_SET:
			break;

		default:
			{
				errno = EINVAL;
				return -1;
			}
			break;
	}

	if (off < 0)
	{
		errno = EINVAL;
		return -1;
	}
	fdstruct->pos = off;
	return off;
}

/* FIXME:
static DIR *RomFS_OpenDir(const char *dirname)
{

}

static int RomFS_CloseDir(DIR *dirp)
{

}

static struct dirent *RomFS_ReadDir(DIR *dirp)
{

}
*/

struct FileLikeObj RomFS_FLO =
{
	.pathprefix = "/rom/",
	.is_leaf = 0,

	.read_  = RomFS_Read,
	.write_ = NULL,
	.ioctl_ = NULL,
	.open_  = RomFS_Open,
	.close_ = RomFS_Close,
	.fstat_ = NULL,
	.fsync_ = NULL,
	.lseek_ = RomFS_Lseek,
	/*
	.opendir_  = RomFS_OpenDir,
	.readdir_  = RomFS_ReadDir,
	.closedir_ = RomFS_CloseDir,
	*/
};

void RomFS_Init()
{
	DeviceManager_Register(&RomFS_FLO);
}

