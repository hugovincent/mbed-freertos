#include <sys/signal.h>

// FIXME this file is a placeholder for now.

void _exit(int status)
{
	/* There is only one SWI for both _exit and _kill. For _exit, call
	   the SWI with the second argument set to -1, an invalid value for
	   signum, so that the SWI handler can distinguish the two calls.
	   Note: The RDI implementation of _kill throws away both its
	   arguments.  */
	//kill(status, -1);

	while (1); // wait indefinitely
}
