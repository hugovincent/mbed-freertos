#include <unistd.h>
#include <errno.h>
#include "mpu_wrappers.h"

ssize_t _readlink_r(struct _reent *ptr, const char *restrict path, char *restrict buf, size_t bufsize) PRIVILEGED_FUNCTION
{
	ptr->_errno = EACCES;
	return -1;
}

