#include <string.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"
#include "mpu_wrappers.h"

PRIVILEGED_FUNCTION int _unlink_r(struct _reent *ptr, const char *path)
{
	int res;
	int block[2];
	block[0] = (int)path;
	block[1] = strlen(path);
	res = do_AngelSWI(AngelSWI_Reason_Remove, block);

	if (res == -1)
		return error (res);

	return 0;
}

