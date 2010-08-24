#include <errno.h>
#include <reent.h>
#include "mpu_wrappers.h"

PRIVILEGED_FUNCTION int _link_r(struct _reent *ptr, const char *oldpath, const char *newpath) 
{
	ptr->_errno = ENOSYS;
	return -1;
}

