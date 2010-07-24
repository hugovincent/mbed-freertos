#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"

int _open_r(struct _reent *ptr, const char * path, int flags, int mode)
{
	int aflags = 0, fh;
	int block[3];

	int fd = newslot ();

	if (fd == -1)
	{
		errno = EMFILE;
		return -1;
	}

	/* It is an error to open a file that already exists. */
	if ((flags & O_CREAT) 
			&& (flags & O_EXCL))
	{
		struct stat st;
		int res;
		res = _stat_r (ptr, path, &st);
		if (res != -1)
		{
			errno = EEXIST;
			return -1;
		}
	}

	/* The flags are Unix-style, so we need to convert them. */ 
#ifdef O_BINARY
	if (flags & O_BINARY)
		aflags |= 1;
#endif

	/* In O_RDONLY we expect aflags == 0. */

	if (flags & O_RDWR) 
		aflags |= 2;

	if ((flags & O_CREAT)
			|| (flags & O_TRUNC)
			|| (flags & O_WRONLY))
		aflags |= 4;

	if (flags & O_APPEND)
	{
		/* Can't ask for w AND a; means just 'a'.  */
		aflags &= ~4;
		aflags |= 8;
	}

	block[0] = (int)path;
	block[2] = strlen(path);
	block[1] = aflags;

	fh = do_AngelSWI(AngelSWI_Reason_Open, block);

	/* Return a user file descriptor or an error. */
	if (fh >= 0)
	{
		openfiles[fd].handle = fh;
		openfiles[fd].pos = 0;
		return fd;
	}
	else
		return error (fh);
}


