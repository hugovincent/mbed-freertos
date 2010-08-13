#include <stdlib.h>
#include <string.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"
#include "mpu_wrappers.h"

int _rename_r(struct _reent *ptr, const char * oldpath, const char * newpath) PRIVILEGED_FUNCTION
{
	int block[4];
	block[0] = (int)oldpath;
	block[1] = strlen(oldpath);
	block[2] = (int)newpath;
	block[3] = strlen(newpath);
	return checkerror(do_AngelSWI(AngelSWI_Reason_Rename, block)) ? -1 : 0;
}
