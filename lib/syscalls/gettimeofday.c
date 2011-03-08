#include <stdlib.h>
#include <sys/time.h>

#include <reent.h>
#include "FreeRTOS.h"
#include "task.h"
#include "lib/syscalls/syscalls_util.h"
#include "mpu_wrappers.h"

PRIVILEGED_FUNCTION int _gettimeofday_r(struct _reent *ptr, struct timeval * tp, void * tzvp) 
{
	struct timezone *tzp = (struct timezone *)tzvp;
	if (tp)
	{
		// FIXME this is uptime not time of day...
		unsigned long long uptime_usec = ullTaskGetSchedulerUptime();
		tp->tv_sec = uptime_usec / 1000000;
		tp->tv_usec = uptime_usec % 1000000;
	}

	/* Return fixed data for the timezone.  */
	if (tzp)
	{
		tzp->tz_minuteswest = 0;
		tzp->tz_dsttime = 0;
	}

	return 0;
}

