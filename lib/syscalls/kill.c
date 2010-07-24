#include <signal.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"

int _kill_r(struct _reent *ptr, int pid, int sig)
{
	(void) pid;
	(void) sig;

	/* Note: The pid argument is thrown away.  */
	switch (sig)
	{
		case SIGABRT:
			return do_AngelSWI(AngelSWI_Reason_ReportException,
					(void *)ADP_Stopped_RunTimeError);
		default:
			return do_AngelSWI(AngelSWI_Reason_ReportException,
					(void *)ADP_Stopped_ApplicationExit);
	}
}

