#include <errno.h>

#include <reent.h>

int _link_r(struct _reent *ptr, const char *oldpath, const char *newpath)
{
	errno = ENOSYS;
	return -1;
}

