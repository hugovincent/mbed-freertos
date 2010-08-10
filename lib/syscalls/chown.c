#include <unistd.h>
#include <errno.h>

int chown(const char *path, uid_t owner, gid_t group)
{
	errno = EROFS;
	return -1;
}


