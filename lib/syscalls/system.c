#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"

int _system(const char *s)
{
	int block[2];
	int e;

	/* Hmmm.  The ARM debug interface specification doesn't say whether
	   SYS_SYSTEM does the right thing with a null argument, or assign any
	   meaning to its return value.  Try to do something reasonable....  */
	if (!s)
		return 1;  /* maybe there is a shell available? we can hope. :-P */
	block[0] = (int)s;
	block[1] = strlen(s);
	e = checkerror(do_AngelSWI(AngelSWI_Reason_System, block));
	if ((e >= 0) && (e < 256))
	{
		/* We have to convert e, an exit status to the encoded status of
		   the command.  To avoid hard coding the exit status, we simply
		   loop until we find the right position.  */
		int exit_code;

		for (exit_code = e; e && WEXITSTATUS(e) != exit_code; e <<= 1)
			continue;
	}
	return e;
}

