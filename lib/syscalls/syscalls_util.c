#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "lib/syscalls/syscalls_util.h"
#include "mpu_wrappers.h"

#define CHECK_INIT(ptr) 					\
	do										\
	{										\
		if ((ptr) && !(ptr)->__sdidinit)	\
		__sinit (ptr);						\
	}										\
	while (0)

static int monitor_stdin;
static int monitor_stdout;
static int monitor_stderr;

/* Return a pointer to the structure associated with
   the user file descriptor fd. */
PRIVILEGED_FUNCTION struct fdent* findslot(int fd) 
{
	CHECK_INIT(_REENT);

	/* User file descriptor is out of range. */
	if ((unsigned int)fd >= MAX_OPEN_FILES)
		return NULL;

	/* User file descriptor is open? */
	if (openfiles[fd].handle == -1)
		return NULL;

	/* Valid. */
	return &openfiles[fd];
}

/* Return the next lowest numbered free file
   structure, or -1 if we can't find one. */
PRIVILEGED_FUNCTION int newslot(void) 
{
	int i;

	for (i = 0; i < MAX_OPEN_FILES; i++)
		if (openfiles[i].handle == -1)
			break;

	if (i == MAX_OPEN_FILES)
		return -1;

	return i;
}

PRIVILEGED_FUNCTION void initialise_stdio(void) 
{
	/* Open the standard file descriptors by opening the debug UART and
	 * attaching it write-only to stdout and stderr, and read-only to stdin.
	 */

	int i;
	static int initialized = 0;
	if (initialized)
		return;

	initialized = 1;

	int volatile block[3];
	block[0] = (int) ":tt";
	block[2] = 3;     /* length of filename */
	block[1] = 0;     /* mode "r" */
	monitor_stdin = do_AngelSWI(AngelSWI_Reason_Open, (void *) block);

	block[0] = (int) ":tt";
	block[2] = 3;     /* length of filename */
	block[1] = 4;     /* mode "w" */
	monitor_stdout = do_AngelSWI(AngelSWI_Reason_Open, (void *) block);

	block[0] = (int) ":tt";
	block[2] = 3;     /* length of filename */
	block[1] = 8;     /* mode "a" */
	monitor_stderr = do_AngelSWI(AngelSWI_Reason_Open, (void *) block);

	/* If we failed to open stderr, redirect to stdout. */
	if (monitor_stderr == -1)
		monitor_stderr = monitor_stdout;

	memset(openfiles, 0, sizeof (openfiles));
	for (i = 0; i < MAX_OPEN_FILES; i ++)
		openfiles[i].handle = -1;

	openfiles[0].handle = monitor_stdin;
	openfiles[0].pos = 0;
	openfiles[1].handle = monitor_stdout;
	openfiles[1].pos = 0;
	openfiles[2].handle = monitor_stderr;
	openfiles[2].pos = 0;
}

/* FIXME a mess: */

/* Set errno and return result. */
PRIVILEGED_FUNCTION int error(int result) 
{
	errno = do_AngelSWI(AngelSWI_Reason_Errno, NULL);
	return result;
}

/* Check the return and set errno appropriately. */
PRIVILEGED_FUNCTION int checkerror(int result) 
{
	if (result == -1)
		return error (-1);
	return result;
}

