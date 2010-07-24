#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"

/* fd, is a user file descriptor. */
_off_t _lseek_r(struct _reent *ptr, int fd, _off_t offs, int dir)
{
	int res;
	struct fdent *pfd;

	/* Valid file descriptor? */
	pfd = findslot(fd);
	if (pfd == NULL)
	{
		errno = EBADF;
		return -1;
	}

	/* Valid whence? */
	if ((dir != SEEK_CUR)
			&& (dir != SEEK_SET)
			&& (dir != SEEK_END))
	{
		errno = EINVAL;
		return -1;
	}

	/* Convert SEEK_CUR to SEEK_SET */
	if (dir == SEEK_CUR)
	{
		offs = pfd->pos + offs;
		/* The resulting file offset would be negative. */
		if (offs < 0)
		{
			errno = EINVAL;
			if ((pfd->pos > 0) && (offs > 0))
				errno = EOVERFLOW;
			return -1;
		}
		dir = SEEK_SET;
	}

	int block[2];
	if (dir == SEEK_END)
	{
		block[0] = pfd->handle;
		res = checkerror(do_AngelSWI(AngelSWI_Reason_FLen, block));
		if (res == -1)
			return -1;
		offs += res;
	}

	/* This code only does absolute seeks.  */
	block[0] = pfd->handle;
	block[1] = offs;
	res = checkerror(do_AngelSWI(AngelSWI_Reason_Seek, block));

	/* At this point offs is the current file position. */
	if (res >= 0)
	{
		pfd->pos = offs;
		return offs;
	}
	else
		return -1;
}

