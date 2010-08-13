#include <unistd.h>
#include <errno.h>
#include "mpu_wrappers.h"

int _fsync_r(struct _reent *ptr, int fd) PRIVILEGED_FUNCTION
{
	ptr->_errno = EINVAL;
	return -1;
}

