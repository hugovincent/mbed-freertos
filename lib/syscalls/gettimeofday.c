#include <stdlib.h>
#include <sys/time.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"
#include "mpu_wrappers.h"

PRIVILEGED_FUNCTION int _gettimeofday_r(struct _reent *ptr, struct timeval * tp, void * tzvp) 
{
	struct timezone *tzp = (struct timezone *)tzvp;
	if (tp)
	{
		/* Ask the host for the seconds since the Unix epoch.  */
		tp->tv_sec = do_AngelSWI(AngelSWI_Reason_Time, NULL);
		tp->tv_usec = 0;
	}

	/* Return fixed data for the timezone.  */
	if (tzp)
	{
		tzp->tz_minuteswest = 0;
		tzp->tz_dsttime = 0;
	}

	return 0;
}

