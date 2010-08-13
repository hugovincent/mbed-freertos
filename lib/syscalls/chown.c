#include <unistd.h>
#include <errno.h>

#include "mpu_wrappers.h"

int _chown_r(struct _reent *ptr, const char *path, uid_t owner, gid_t group) PRIVILEGED_FUNCTION
{
	ptr->_errno = EROFS;
	return -1;
}

