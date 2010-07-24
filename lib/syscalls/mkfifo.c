#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int mkfifo(const char *path, mode_t mode)
{
	errno = EROFS;
	return -1;
}

