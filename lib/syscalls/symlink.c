#include <unistd.h>
#include <errno.h>
#include "mpu_wrappers.h"

int _symlink_r(struct _reent *ptr, const char *path1, const char *path2) PRIVILEGED_FUNCTION
{
	ptr->_errno = EROFS;
	return -1;
}

