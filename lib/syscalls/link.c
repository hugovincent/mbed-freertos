#include <errno.h>
#include <reent.h>
#include "mpu_wrappers.h"

int _link_r(struct _reent *ptr, const char *oldpath, const char *newpath) PRIVILEGED_FUNCTION
{
	ptr->_errno = ENOSYS;
	return -1;
}

