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
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>

#include "device_manager.h"
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

#define SEMIFS_MAX_FDS	4
static struct {
	int handle;
	int pos;
} SemiFS_fd_table[SEMIFS_MAX_FDS];

static int SemiFS_num_fd;

/*****************************************************************************/
// Semihosting calls, some portions based on arm/syscalls.c from Newlib

enum {
	// Standard ARM Semihosting Commands:
	SYS_OPEN   = 0x1,
	SYS_CLOSE  = 0x2,
	SYS_WRITE  = 0x5,
	SYS_READ   = 0x6,
	SYS_ISTTY  = 0x9,
	SYS_SEEK   = 0xa,
	SYS_ENSURE = 0xb,
	SYS_FLEN   = 0xc,

	// Custom Functions for mbed getting listings of files:
	USR_XFFIND = 0x100,
};

static inline int __semihost(int reason, void *arg)
{
	// For Thumb-2 code use the BKPT instruction instead of SWI.
#ifdef __thumb2__
	#define AngelSWI 		0xAB
	#define AngelSWIInsn	"bkpt"
#else
	#define AngelSWI		0x123456
	#define AngelSWIInsn	"swi"
#endif

	int value;
	asm volatile ("mov r0, %1; mov r1, %2; " AngelSWIInsn " %a3; mov %0, r0"
			: "=r" (value) // Outputs
			: "r" (reason), "r" (arg), "i" (AngelSWI) // Inputs
			: "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc");
			// Clobbers r0 and r1, and lr if in supervisor mode.

	/* Accordingly to page 13-77 of ARM DUI 0040D other registers
	   can also be clobbered.  Some memory positions may also be
	   changed by a system call, so they should not be kept in
	   registers. Note: we are assuming the manual is right and
	   Angel is respecting the APCS.  */

	return value;
}

/*****************************************************************************/

static int SemiFS_Open(const char *path, int flags, int mode /* <-- ignored */)
{
	int aflags = 0, fd, fh;
	uint32_t args[3];

	if (SemiFS_num_fd == SEMIFS_MAX_FDS)
	{
		errno = ENFILE;
		return -1;
	}

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

	// Find an empty fd (we know there is at least one available here)
	for (int i = 0; i < SEMIFS_MAX_FDS; i ++)
		if (SemiFS_fd_table[i].handle == -1)
			fd = i;
	SemiFS_num_fd++;

	args[0] = (uint32_t)path;
	args[1] = (uint32_t)aflags;
	args[2] = (uint32_t)strlen(path);
	fh =  __semihost(SYS_OPEN, args);

	if (fh >= 0)
	{
		SemiFS_fd_table[fd].handle = fh;
		SemiFS_fd_table[fd].pos = 0;
	}

	return fd;
}

static int SemiFS_Close(int fd)
{
	int fh = SemiFS_fd_table[fd].handle;

	if (fd != SEMIFS_MAX_FDS)
		SemiFS_fd_table[fd].handle = -1;

	SemiFS_num_fd--;

	return __semihost(SYS_CLOSE, &fh);
}

static ssize_t SemiFS_Write(int fd, void *ptr, size_t len)
{
	int fh = SemiFS_fd_table[fd].handle, x;
	uint32_t args[3];

	args[0] = (uint32_t)fh;
	args[1] = (uint32_t)ptr;
	args[2] = (uint32_t)len;
	x = __semihost(SYS_WRITE, args);

	if (x == -1 || x == len)
	{
		errno = EIO;
		return -1;
	}

	if (fd != SEMIFS_MAX_FDS)
		SemiFS_fd_table[fd].pos += len - x;

	return len - x;
}

static ssize_t SemiFS_Read(int fd, void *ptr, size_t len)
{
	int fh = SemiFS_fd_table[fd].handle, x;
	uint32_t args[3];

	args[0] = (uint32_t)fh;
	args[1] = (uint32_t)ptr;
	args[2] = (uint32_t)len;
	x = __semihost(SYS_READ, args);

	if (x < 0)
	{
		errno = EIO;
		return -1;
	}

	if (fd != SEMIFS_MAX_FDS)
		SemiFS_fd_table[fd].pos += len - x;

	return len - x;
}

static off_t SemiFS_Lseek(int fd, off_t off, int whence)
{
	int fh = SemiFS_fd_table[fd].handle, res;
	uint32_t args[2];

	switch (whence)
	{
		case SEEK_CUR:
			{
				// seek from current position
				if (fd == SEMIFS_MAX_FDS)
					return -1;
				off += SemiFS_fd_table[fd].pos;
				whence = SEEK_SET;
			}
			break;

		case SEEK_END:
			{
				// seek from end of file
				args[0] = fh;
				off += __semihost(SYS_FLEN, &args);
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
	args[0] = (uint32_t)fh;
	args[1] = (uint32_t)off;
	res = __semihost(SYS_SEEK, args);

	if (fd != SEMIFS_MAX_FDS && res == 0)
		SemiFS_fd_table[fd].pos = off;

	/* This is expected to return the position in the file.  */
	return res == 0 ? off : -1;
}

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
#if 0
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

	res = __semihost(USR_XFFIND, param);
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
	.pathprefix = "/SemiFS/",
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
	for (int i = 0; i < SEMIFS_MAX_FDS; i++)
		SemiFS_fd_table[i].handle = -1;

	DeviceManager_Register(&SemiFS_FLO);
}

