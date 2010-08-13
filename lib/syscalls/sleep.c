#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "mpu_wrappers.h"

// FIXME

int _nanosleep_r(struct _reent *ptr, const struct timespec *rqtp, struct timespec *rmtp) PRIVILEGED_FUNCTION
{
	ptr->_errno = ENOSYS;
	return -1;
}

int _usleep_r(struct _reent *ptr, useconds_t useconds) PRIVILEGED_FUNCTION
{
	ptr->_errno = EINTR;
	return -1;
}

unsigned int _sleep_r(struct _reent *ptr, unsigned int seconds) PRIVILEGED_FUNCTION
{
	return 0;
}

