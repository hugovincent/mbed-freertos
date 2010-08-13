#include <reent.h>

#include "mpu_wrappers.h"

int _getpid_r(struct _reent *ptr) PRIVILEGED_FUNCTION
{
	return 1;
}

