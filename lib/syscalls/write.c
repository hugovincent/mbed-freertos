#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"
#include "FreeRTOS.h"
#include "task.h"

/* fd, is a user file descriptor. */
int _write_r(struct _reent *ptr, int fd, const void * buf, size_t len)
{
	//-------------------------------------------------------------------------
	// FIXME temporary...	
	extern int uart0write(const char *buff, size_t len);
	return uart0write(buf, len);

	// End FIXME
	//-------------------------------------------------------------------------

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
	   Returns the number of bytes *not* written. */
	res = checkerror(do_AngelSWI(AngelSWI_Reason_Write, block));

	/* Clearly an error. */
	if (res < 0)
		return -1;

	pfd->pos += len - res;

	/* We wrote 0 bytes? 
	   Retrieve errno just in case. */
	if ((len - res) == 0)
		return error (0);

	return (len - res);
}
