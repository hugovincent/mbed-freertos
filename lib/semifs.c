/* Copyright (c) 2010 James Snyder, eLua Project.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the right
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>

#include "device_manager_driver.h"
#include "debug_support.h"
#include "semifs.h"

// These structures provided by Simon Ford of mbed
struct SemiFS_FTime {
	uint8_t  hr;						// Hours    [0..23]
	uint8_t  min;						// Minutes  [0..59]
	uint8_t  sec;						// Seconds  [0..59]
	uint8_t  day;						// Day      [1..31]
	uint8_t  mon;						// Month    [1..12]
	uint16_t year;						// Year     [1980..2107]
};

// File Search info record
struct SemiFS_XF_Info {
	char name[32];						// File - 32-bytes
	uint32_t size;						// File size in bytes - 4-bytes
	uint16_t fileID;					// System File Identification  - 2-bytes
	struct SemiFS_FTime create_time;	// Date & time file was created
	struct SemiFS_FTime write_time;		// Date & time of last write
};
struct SemiFS_SearchInfo {
	const char *pattern;
	struct SemiFS_XF_Info file_info;
};

/*****************************************************************************/

static int SemiFS_Open(const char *path, int flags, int mode /* <-- ignored */)
{
	int aflags = 0;
	unsigned short fh;
	uint32_t args[3];

	if (flags & O_RDWR)
		aflags |= 2;

	if (flags & O_CREAT)
		aflags |= 4;

	if (flags & O_TRUNC)
		aflags |= 4;

	if (flags & O_APPEND)
	{
		aflags &= ~4; // Can't ask for w AND a; means just 'a'.
		aflags |= 8;
	}

	// Find file and open it (via semihosting call)
	args[0] = (uint32_t)path;
	args[1] = (uint32_t)aflags;
	args[2] = (uint32_t)strlen(path);
	fh =  SemihostCall(Semihost_SYS_OPEN, args);

	if (fh >= 0)
	{
		// Create a device-manager fd
		struct DeviceManager_FdStruct *fdstruct = DeviceManager_NewFd();
		fdstruct->handle = fh;
		fdstruct->pos = 0;
		return fdstruct->fd;
	}
	else
	{
		errno = ENOENT; // FIXME query actual error and report
		return -1;
	}
}

static int SemiFS_Close(int fd)
{
	struct DeviceManager_FdStruct *fdstruct = DeviceManager_FdStructForFd(fd);
	if (fdstruct == NULL)
		return -1;
	int res = SemihostCall(Semihost_SYS_CLOSE, &fdstruct->handle);
	if (res == 0)
		DeviceManager_ReleaseFd(fdstruct);
	return res;
}

static ssize_t SemiFS_Write(int fd, void *ptr, size_t len)
{
	struct DeviceManager_FdStruct *fdstruct = DeviceManager_FdStructForFd(fd);
	if (fdstruct == NULL)
		return -1;

	uint32_t args[3];
	args[0] = (uint32_t)fdstruct->handle;
	args[1] = (uint32_t)ptr;
	args[2] = (uint32_t)len;

	// Perform write
	int res = SemihostCall(Semihost_SYS_WRITE, args);
	if (res == -1 || res == len)
	{
		errno = EIO;
		return -1;
	}

	// Update position
	fdstruct->pos += len - res;
	return len - res;
}

static ssize_t SemiFS_Read(int fd, void *ptr, size_t len)
{
	struct DeviceManager_FdStruct *fdstruct = DeviceManager_FdStructForFd(fd);
	if (fdstruct == NULL)
		return -1;

	uint32_t args[3];
	args[0] = (uint32_t)fdstruct->handle;
	args[1] = (uint32_t)ptr;
	args[2] = (uint32_t)len;

	// Perform read
	int res = SemihostCall(Semihost_SYS_READ, args);
	if (res < 0)
	{
		errno = EIO;
		return -1;
	}

	// Update position
	fdstruct->pos += len - res;
	return len - res;
}

static off_t SemiFS_Lseek(int fd, off_t off, int whence)
{
	uint32_t args[2];

	struct DeviceManager_FdStruct *fdstruct = DeviceManager_FdStructForFd(fd);
	if (fdstruct == NULL)
		return -1;

	switch (whence)
	{
		case SEEK_CUR:
			{
				// seek from current position
				off += fdstruct->pos;
				whence = SEEK_SET;
			}
			break;

		case SEEK_END:
			{
				// seek from end of file
				args[0] = fdstruct->handle;
				off += SemihostCall(Semihost_SYS_FLEN, &args);
			}
			break;

		case SEEK_SET:
			{

			}
			break;

		default:
			{
				return -1;
			}
	}
	// Do absolute seek
	args[0] = (uint32_t)fdstruct->handle;
	args[1] = (uint32_t)off;
	int res = SemihostCall(Semihost_SYS_SEEK, args);

	if (res == 0)
		fdstruct->pos = off;

	/* This is expected to return the position in the file.  */
	return res == 0 ? off : -1;
}

#if 0
static void* SemiFS_OpenDir(const char *dname)
{
	static const char *testpattern = "*";
	static struct SemiFS_SearchInfo SemiFS_dir;

	SemiFS_dir.file_info.fileID = 0;
	SemiFS_dir.pattern = testpattern;
	return (void *)&SemiFS_dir;
}

static int SemiFS_CloseDir(void *d)
{
	return 0;
}

/* FIXME this is broken and needs integrating with Device_Manager too: */
#define DM_MAX_FNAME_LENGTH	32
static struct dm_dirent* SemiFS_ReadDir(void *d)
{
	extern struct dm_dirent dm_shared_dirent;
	extern char dm_shared_fname[DM_MAX_FNAME_LENGTH + 1 ];

	struct SemiFS_SearchInfo *dir = (struct SemiFS_SearchInfo *)d;
	struct SemiFS_XF_Info *SemiFS_file_info = &dir->file_info;
	struct dm_dirent* pent = &dm_shared_dirent;
	int res;

	uint32_t param[4];
	param[0] = (uint32_t)dir->pattern;
	param[1] = (uint32_t)strlen((const char *)dir->pattern);
	param[2] = (uint32_t)SemiFS_file_info;
	param[3] = (uint32_t)sizeof(struct SemiFS_XF_Info);

	res = SemihostCall(Semihost_USR_XFFIND, param);
	if (res != 0)
		return NULL;

	strncpy(dm_shared_fname, SemiFS_file_info->name, DM_MAX_FNAME_LENGTH);
	pent->fname = dm_shared_fname;
	pent->fsize = SemiFS_file_info->size;
	pent->ftime = 0; // need to convert from struct to UNIX time?!
	return pent;
}
#endif

struct FileLikeObj SemiFS_FLO =
{
	.pathprefix = "/mbed/",
	.is_leaf = 0,

	.read_  = SemiFS_Read,
	.write_ = SemiFS_Write,
	.ioctl_ = NULL,
	.open_  = SemiFS_Open,
	.close_ = SemiFS_Close,
	.fstat_ = NULL,
	.fsync_ = NULL,
	.lseek_ = SemiFS_Lseek,
	/*
	.opendir  = SemiFS_OpenDir,
	.readdir  = SemiFS_ReadDir,
	.closedir = SemiFS_CloseDir,
	*/
};

void SemiFS_Init()
{
	DeviceManager_Register(&SemiFS_FLO);
}

