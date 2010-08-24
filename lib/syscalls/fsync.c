#include <unistd.h>
#include <errno.h>
#include "mpu_wrappers.h"

PRIVILEGED_FUNCTION int _fsync_r(struct _reent *ptr, int fd) 
{
	ptr->_errno = EINVAL;
	return -1;
}

