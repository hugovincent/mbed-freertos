#include <unistd.h>
#include <errno.h>
#include "mpu_wrappers.h"

PRIVILEGED_FUNCTION int _symlink_r(struct _reent *ptr, const char *path1, const char *path2) 
{
	ptr->_errno = EROFS;
	return -1;
}

