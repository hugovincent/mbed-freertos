#include <unistd.h>
#include <errno.h>

int fsync(int fildes)
{
	errno = EINVAL;
	return -1;
}

