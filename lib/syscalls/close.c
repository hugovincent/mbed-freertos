#include <reent.h>
#include <errno.h>
#include <stdlib.h>
#include "lib/syscalls/syscalls_util.h"

#include "mpu_wrappers.h"

/* fd, is a user file descriptor. */
PRIVILEGED_FUNCTION int _close_r(struct _reent *ptr, int fd) 
{
	int res;
	struct fdent *pfd;

	pfd = findslot(fd);
	if (pfd == NULL)
	{
		ptr->_errno = EBADF;
		return -1;
	}

	/* Handle stderr == stdout. */
	if ((fd == 1 || fd == 2)
			&& (openfiles[1].handle == openfiles[2].handle))
	{
		pfd->handle = -1;
		return 0;
	}

	/* Attempt to close the handle. */
	res = checkerror(do_AngelSWI (AngelSWI_Reason_Close, &f(pfd->handle)));

	/* Reclaim handle? */
	if (res == 0)
		pfd->handle = -1;

	return res;
}

