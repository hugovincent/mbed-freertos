#include <unistd.h>
#include <errno.h>
#include "mpu_wrappers.h"

PRIVILEGED_FUNCTION ssize_t _readlink_r(struct _reent *ptr, const char *restrict path, char *restrict buf, size_t bufsize) 
{
	ptr->_errno = EACCES;
	return -1;
}

