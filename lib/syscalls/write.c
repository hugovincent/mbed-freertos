#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mpu_wrappers.h"

/* fd, is a user file descriptor. */
int _write_r(struct _reent *ptr, int fd, const void * buf, size_t len) PRIVILEGED_FUNCTION
{
	//-------------------------------------------------------------------------
	// FIXME temporary...
	extern int uart0write(const char *buf, size_t len);
	return uart0write(buf, len);
	//-------------------------------------------------------------------------
#if 0
	int res;
	struct fdent *pfd;
	int block[3];

	pfd = findslot(fd);
	if (pfd == NULL)
	{
		ptr->_errno = EBADF;
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
#endif
}

