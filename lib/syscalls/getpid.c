#include <reent.h>

#include "mpu_wrappers.h"

PRIVILEGED_FUNCTION int _getpid_r(struct _reent *ptr) 
{
	return 1;
}

