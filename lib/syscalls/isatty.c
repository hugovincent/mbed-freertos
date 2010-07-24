#include <stdlib.h>
#include <errno.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"

int _isatty_r(struct _reent *ptr, int fd)
{
	struct fdent *pfd;

	pfd = findslot(fd);
	if (pfd == NULL)
	{
		errno = EBADF;
		return -1;
	}

	return checkerror(do_AngelSWI(AngelSWI_Reason_IsTTY, &pfd->handle));
}
