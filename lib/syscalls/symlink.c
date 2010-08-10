#include <unistd.h>
#include <errno.h>

int symlink(const char *path1, const char *path2)
{
	errno = EROFS;
	return -1;
}

