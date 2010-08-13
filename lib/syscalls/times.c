#include <stdlib.h>
#include <sys/times.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"
#include "mpu_wrappers.h"

/* Return a clock that ticks at 100Hz.  */
clock_t _times_r(struct _reent *ptr, struct tms * tp) PRIVILEGED_FUNCTION
{
	clock_t timeval = do_AngelSWI(AngelSWI_Reason_Clock, NULL);

	if (tp)
	{
		tp->tms_utime  = timeval;	/* user time */
		tp->tms_stime  = 0;	/* system time */
		tp->tms_cutime = 0;	/* user time, children */
		tp->tms_cstime = 0;	/* system time, children */
	}

	return timeval;
}

