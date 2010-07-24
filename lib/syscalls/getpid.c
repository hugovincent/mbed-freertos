#include <reent.h>

int _getpid_r(struct _reent *ptr)
{
	return 1;
}
