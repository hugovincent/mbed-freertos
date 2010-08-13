#include <sys/stat.h>
#include <errno.h>
#include "mpu_wrappers.h"

int _mkdir_r(struct _reent *ptr, const char *path, mode_t mode) PRIVILEGED_FUNCTION
{
	ptr->_errno = EROFS;
	return -1;
}

