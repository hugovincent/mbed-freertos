#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"
#include "FreeRTOS.h"
#include "task.h"

#include "hardware/uart.h"

/* fd, is a user file descriptor. */
int _write_r(struct _reent *ptr, int fd, const void * buf, size_t len)
{
	//-------------------------------------------------------------------------
	// FIXME temporary...

	/* Which output method to use? */
	signed portBASE_TYPE (*putcharHandler)(signed portCHAR, portTickType blocking)
		= &uart0PutChar_debug;
	/*if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
	{
		putcharHandler = &uart0PutChar;
	}*/
	
	char *tmp = (char *)buf;
	while (tmp < ((char *)buf + len))
	{
		/* Make line endings behave like normal serial terminals. */
		if (*tmp == '\n')
		{
			(*putcharHandler)('\r', 0);
		}
		(*putcharHandler)(*tmp++, 0);
	}
	return len;

	// End FIXME
	//-------------------------------------------------------------------------

	int res;
	struct fdent *pfd;
	int block[3];

	pfd = findslot(fd);
	if (pfd == NULL)
	{
		errno = EBADF;
		return -1;
	}

	block[0] = pfd->handle;
	block[1] = (int) buf;
	block[2] = len;

	/* fh, is a valid internal file handle.
	   Returns the number of bytes *not* written. */
	res = checkerror(do_AngelSWI(AngelSWI_Reason_Write, block));

	/* Clearly an error. */
	if (res < 0)
		return -1;

	pfd->pos += len - res;

	/* We wrote 0 bytes? 
	   Retrieve errno just in case. */
	if ((len - res) == 0)
		return error (0);

	return (len - res);
}
