#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "mpu_wrappers.h"

// FIXME

PRIVILEGED_FUNCTION int _nanosleep_r(struct _reent *ptr, const struct timespec *rqtp, struct timespec *rmtp) 
{
	ptr->_errno = ENOSYS;
	return -1;
}

PRIVILEGED_FUNCTION int _usleep_r(struct _reent *ptr, useconds_t useconds) 
{
	ptr->_errno = EINTR;
	return -1;
}

PRIVILEGED_FUNCTION unsigned int _sleep_r(struct _reent *ptr, unsigned int seconds) 
{
	return 0;
}

