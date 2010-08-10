#include <time.h>
#include <unistd.h>
#include <errno.h>

// FIXME

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
	errno = ENOSYS;
	return -1;
}

int usleep(useconds_t useconds)
{
	errno = EINTR;
	return -1;
}

unsigned int sleep(unsigned int seconds)
{
	return 0;
}

