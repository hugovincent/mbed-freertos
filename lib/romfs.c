#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>

#include "device_manager.h"
#include "romfs.h"

#define ROMFS_MAX_FDS	4
static struct {
	int handle;
	int pos;
} RomFS_fd_table[ROMFS_MAX_FDS];

static int RomFS_num_fd;

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
	int fd, fh;

	if (RomFS_num_fd == ROMFS_MAX_FDS)
	{
		errno = ENFILE;
		return -1;
	}

	if ((flags & O_RDWR) || (flags & O_WRONLY) || (flags & O_APPEND)
			|| (flags & O_CREAT) || (flags & O_TRUNC))
	{
		errno = EROFS;
		return -1;
	}

	// Find file in filesystem
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

	// Find an empty fd (we know there is at least one available here)
	for (int i = 0; i < ROMFS_MAX_FDS; i ++)
		if (RomFS_fd_table[i].handle == -1)
			fd = i;
	RomFS_num_fd++;

	RomFS_fd_table[fd].handle = fh;
	RomFS_fd_table[fd].pos = 0;

	return fd;

}

static int RomFS_Close(int fd)
{
	RomFS_fd_table[fd].handle = -1;
	RomFS_num_fd--;
	return 0;
}

static ssize_t RomFS_Read(int fd, void *ptr, size_t len)
{
	int fh = RomFS_fd_table[fd].handle;

	int bytes = romfs_header[fh].length - RomFS_fd_table[fd].pos;
	if (bytes > len)
		bytes = len;

	// It is possible to lseek past the end of the file. read needs
	// to reset offset if necessary first.
	if (bytes < 0)
	{
		RomFS_fd_table[fd].pos = romfs_header[fh].length;
		bytes = 0;
	}

	if (bytes == 0)
	{
		return 0;
	}

	int offset = romfs_header[fh].offset + RomFS_fd_table[fd].pos;
	memcpy(ptr, (void*)&(romfs_data[offset]), bytes);

	RomFS_fd_table[fd].pos += bytes;
	return bytes;
}

static off_t RomFS_Lseek(int fd, off_t off, int whence)
{
	int fh = RomFS_fd_table[fd].handle;

	switch (whence)
	{
		case SEEK_CUR:
			// seek from current position
			off += RomFS_fd_table[fd].pos;
			break;

		case SEEK_END:
			// seek from end of file
			off += romfs_header[fh].length;
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
	RomFS_fd_table[fd].pos = off;
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
	for (int i = 0; i < ROMFS_MAX_FDS; i++)
		RomFS_fd_table[i].handle = -1;

	DeviceManager_Register(&RomFS_FLO);
}

