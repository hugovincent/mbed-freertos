#include <unistd.h>
#include <errno.h>

ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize)
{
	errno = EACCES;
	return -1;
}

