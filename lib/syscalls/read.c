#include <stdlib.h>
#include <errno.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"

/* fd, is a valid user file handle. 
   Translates the return of _read into
   bytes read. */
int _read_r(struct _reent *ptr, int fd, void * buf, size_t len)
{
	int res;
	struct fdent *pfd;
	int block[3];

	pfd = findslot(fd);
	if (pfd == NULL)
	{
		errno = EBADF;
		return -1;
	}

	block[0] = pfd->handle;
	block[1] = (int) buf;
	block[2] = len;
	/* fh, is a valid internal file handle.
	   buf, is a null terminated string.
	   len, is the length in bytes to read. 
	   Returns the number of bytes *not* written. */
	res = checkerror(do_AngelSWI(AngelSWI_Reason_Read, block));

	if (res == -1)
		return res;

	pfd->pos += len - res;

	/* res == len is not an error, 
	   at least if we want feof() to work.  */
	return len - res;
}

