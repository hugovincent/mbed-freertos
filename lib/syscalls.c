/* This is based on the stubs in Newlib (specifically, the ARM syscalls.c in libgloss).
 *
 * Hugo Vincent, 28 April 2010.
 *
 * Modifications:
 *		- Added FreeRTOS-specific memory allocation functions
 *		- Switched to reentrant versions of stubs
 */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <reent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#if 1
// FIXME-----------------------------------------------------------------------
#define AngelSWI_Reason_Open		0x01
#define AngelSWI_Reason_Close		0x02
#define AngelSWI_Reason_WriteC		0x03
#define AngelSWI_Reason_Write0		0x04
#define AngelSWI_Reason_Write		0x05
#define AngelSWI_Reason_Read		0x06
#define AngelSWI_Reason_ReadC		0x07
#define AngelSWI_Reason_IsTTY		0x09
#define AngelSWI_Reason_Seek		0x0A
#define AngelSWI_Reason_FLen		0x0C
#define AngelSWI_Reason_TmpNam		0x0D
#define AngelSWI_Reason_Remove		0x0E
#define AngelSWI_Reason_Rename		0x0F
#define AngelSWI_Reason_Clock		0x10
#define AngelSWI_Reason_Time		0x11
#define AngelSWI_Reason_System		0x12
#define AngelSWI_Reason_Errno		0x13
#define AngelSWI_Reason_GetCmdLine 	0x15
#define AngelSWI_Reason_HeapInfo 	0x16
#define AngelSWI_Reason_EnterSVC 	0x17
#define AngelSWI_Reason_ReportException 0x18
#define ADP_Stopped_ApplicationExit ((2 << 16) + 38)
#define ADP_Stopped_RunTimeError 	((2 << 16) + 35)
#define do_AngelSWI(a, b) 			(0)
// END FIXME-------------------------------------------------------------------
#endif

#include <FreeRTOS.h>
#include <task.h>
#include "hardware/uart.h"

struct _reent *__get_reent	_PARAMS ((void));

static int	checkerror	_PARAMS ((int));
static int	error		_PARAMS ((int));

/* This relies on linker magic to get stack and heap pointers */
extern unsigned int __start_of_heap__, __stack_min__;
static void *heap_end = NULL;

/* Struct used to keep track of the file position, just so we
   can implement fseek(fh,x,SEEK_CUR).  */
struct fdent
{
	int handle;
	int pos;
};

#define MAX_OPEN_FILES 10

/* User file descriptors (fd) are integer indexes into 
   the openfiles[] array. Error checking is done by using
   findslot(). 

   This openfiles array is manipulated directly by only 
   these 5 functions:

   findslot() - Translate entry.
   newslot() - Find empty entry.
   initialise_stdio() - Initialize entries.
   _open() - Initialize entry.
   _close() - Handle stdout == stderr case.

   Every other function must use findslot().  */

static struct fdent openfiles [MAX_OPEN_FILES];

static struct fdent* 	findslot	_PARAMS ((int));
static int				newslot		_PARAMS ((void));

/* following is copied from libc/stdio/local.h to check std streams */
extern void   _EXFUN(__sinit,(struct _reent *));
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
static struct fdent* findslot (int fd)
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
static int newslot (void)
{
	int i;

	for (i = 0; i < MAX_OPEN_FILES; i++)
		if (openfiles[i].handle == -1)
			break;

	if (i == MAX_OPEN_FILES)
		return -1;

	return i;
}

void initialise_stdio (void)
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
	monitor_stdin = do_AngelSWI (AngelSWI_Reason_Open, (void *) block);

	block[0] = (int) ":tt";
	block[2] = 3;     /* length of filename */
	block[1] = 4;     /* mode "w" */
	monitor_stdout = do_AngelSWI (AngelSWI_Reason_Open, (void *) block);

	block[0] = (int) ":tt";
	block[2] = 3;     /* length of filename */
	block[1] = 8;     /* mode "a" */
	monitor_stderr = do_AngelSWI (AngelSWI_Reason_Open, (void *) block);


	/* If we failed to open stderr, redirect to stdout. */
	if (monitor_stderr == -1)
		monitor_stderr = monitor_stdout;

	__builtin_memset (openfiles, 0, sizeof (openfiles));
	for (i = 0; i < MAX_OPEN_FILES; i ++)
		openfiles[i].handle = -1;

	openfiles[0].handle = monitor_stdin;
	openfiles[0].pos = 0;
	openfiles[1].handle = monitor_stdout;
	openfiles[1].pos = 0;
	openfiles[2].handle = monitor_stderr;
	openfiles[2].pos = 0;
}

/* Set errno and return result. */
static int error (int result)
{
	errno = do_AngelSWI (AngelSWI_Reason_Errno, NULL);
	return result;
}

/* Check the return and set errno appropriately. */
static int checkerror (int result)
{
	if (result == -1)
		return error (-1);
	return result;
}

/* fd, is a valid user file handle. 
   Translates the return of _read into
   bytes read. */
int _read_r (struct _reent *ptr, int fd, void * buf, size_t len)
{
	int res;
	struct fdent *pfd;
	int block[3];

	pfd = findslot (fd);
	if (pfd == NULL)
	{
		errno = EBADF;
		return -1;
	}

	block[0] = pfd->handle;
	block[1] = (int) buf;
	block[2] = len;
	/* fh, is a valid internal file handle.
	   buf, is a null terminated string.
	   len, is the length in bytes to read. 
	   Returns the number of bytes *not* written. */
	res = checkerror (do_AngelSWI (AngelSWI_Reason_Read, block));

	if (res == -1)
		return res;

	pfd->pos += len - res;

	/* res == len is not an error, 
	   at least if we want feof() to work.  */
	return len - res;
}

/* fd, is a user file descriptor. */
_off_t _lseek_r (struct _reent *ptr, int fd, _off_t offs, int dir)
{
	int res;
	struct fdent *pfd;

	/* Valid file descriptor? */
	pfd = findslot (fd);
	if (pfd == NULL)
	{
		errno = EBADF;
		return -1;
	}

	/* Valid whence? */
	if ((dir != SEEK_CUR)
			&& (dir != SEEK_SET)
			&& (dir != SEEK_END))
	{
		errno = EINVAL;
		return -1;
	}

	/* Convert SEEK_CUR to SEEK_SET */
	if (dir == SEEK_CUR)
	{
		offs = pfd->pos + offs;
		/* The resulting file offset would be negative. */
		if (offs < 0)
		{
			errno = EINVAL;
			if ((pfd->pos > 0) && (offs > 0))
				errno = EOVERFLOW;
			return -1;
		}
		dir = SEEK_SET;
	}

	int block[2];
	if (dir == SEEK_END)
	{
		block[0] = pfd->handle;
		res = checkerror (do_AngelSWI (AngelSWI_Reason_FLen, block));
		if (res == -1)
			return -1;
		offs += res;
	}

	/* This code only does absolute seeks.  */
	block[0] = pfd->handle;
	block[1] = offs;
	res = checkerror (do_AngelSWI (AngelSWI_Reason_Seek, block));

	/* At this point offs is the current file position. */
	if (res >= 0)
	{
		pfd->pos = offs;
		return offs;
	}
	else
		return -1;
}

/* fd, is a user file descriptor. */
int _write_r (struct _reent *ptr, int fd, const void * buf, size_t len)
{
	//-------------------------------------------------------------------------
	// FIXME temporary...

	/* Which output method to use? */
	signed portBASE_TYPE (*putcharHandler)(signed portCHAR, portTickType blocking);
	if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
	{
		putcharHandler = uart0PutChar;
	}
	else
	{
		putcharHandler = uart0PutChar_debug;
	}
	
	char *tmp = (char *)buf;
	while (tmp < ((char *)buf + len))
	{
		/* Make line endings behave like normal serial terminals. */
		if (*tmp == '\n')
		{
			putcharHandler('\r', 0);
		}
		putcharHandler(*tmp++, 0);
	}
	return len;

	// End FIXME
	//-------------------------------------------------------------------------

	int res;
	struct fdent *pfd;
	int block[3];

	pfd = findslot (fd);
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
	res = checkerror (do_AngelSWI (AngelSWI_Reason_Write, block));

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

int _open_r (struct _reent *ptr, const char * path, int flags, int w)
{
	int aflags = 0, fh;
	int block[3];

	int fd = newslot ();

	if (fd == -1)
	{
		errno = EMFILE;
		return -1;
	}

	/* It is an error to open a file that already exists. */
	if ((flags & O_CREAT) 
			&& (flags & O_EXCL))
	{
		struct stat st;
		int res;
		res = _stat_r (ptr, path, &st);
		if (res != -1)
		{
			errno = EEXIST;
			return -1;
		}
	}

	/* The flags are Unix-style, so we need to convert them. */ 
#ifdef O_BINARY
	if (flags & O_BINARY)
		aflags |= 1;
#endif

	/* In O_RDONLY we expect aflags == 0. */

	if (flags & O_RDWR) 
		aflags |= 2;

	if ((flags & O_CREAT)
			|| (flags & O_TRUNC)
			|| (flags & O_WRONLY))
		aflags |= 4;

	if (flags & O_APPEND)
	{
		/* Can't ask for w AND a; means just 'a'.  */
		aflags &= ~4;
		aflags |= 8;
	}

	block[0] = (int) path;
	block[2] = strlen (path);
	block[1] = aflags;

	fh = do_AngelSWI (AngelSWI_Reason_Open, block);

	/* Return a user file descriptor or an error. */
	if (fh >= 0)
	{
		openfiles[fd].handle = fh;
		openfiles[fd].pos = 0;
		return fd;
	}
	else
		return error (fh);
}

/* fd, is a user file descriptor. */
int _close_r (struct _reent *ptr, int fd)
{
	int res;
	struct fdent *pfd;

	pfd = findslot (fd);
	if (pfd == NULL)
	{
		errno = EBADF;
		return -1;
	}

	/* Handle stderr == stdout. */
	if ((fd == 1 || fd == 2)
			&& (openfiles[1].handle == openfiles[2].handle))
	{
		pfd->handle = -1;
		return 0;
	}

	/* Attempt to close the handle. */
	res = checkerror (do_AngelSWI (AngelSWI_Reason_Close, &f(pfd->handle)));

	/* Reclaim handle? */
	if (res == 0)
		pfd->handle = -1;

	return res;
}

int _getpid_r (struct _reent *ptr)
{
	return 1;
}

/* Low-level bulk RAM allocator -- used by Newlib's Malloc */
void *_sbrk_r (struct _reent *ptr, ptrdiff_t incr)
{
	void *prev_heap_end, *next_heap_end;

	/* Initialize on first call */
	if (heap_end == NULL)
	{
		heap_end = (void *)&__start_of_heap__;
	}

	prev_heap_end = heap_end;

	/* Align to always be on 8-byte boundaries */
	next_heap_end = (void *)((((unsigned int)heap_end + incr) + 7) & ~7);  

	/* Check if this allocation would collide with the heap */
	if (next_heap_end > (void *)&__stack_min__)
	{
		errno = ENOMEM;
		return NULL;
	}
	else
	{
		heap_end = next_heap_end;
		return (void *)prev_heap_end;
	}
}

int _swistat (int fd, struct stat * st)
{
	struct fdent *pfd;
	int res;

	pfd = findslot (fd);
	if (pfd == NULL)
	{
		errno = EBADF;
		return -1;
	}

	/* Always assume a character device,
	   with 1024 byte blocks. */
	st->st_mode |= S_IFCHR;
	st->st_blksize = 1024;
	res = checkerror (do_AngelSWI (AngelSWI_Reason_FLen, &pfd->handle));

	if (res == -1)
		return -1;
	/* Return the file size. */
	st->st_size = res;
	return 0;
}

int _fstat_r (struct _reent *ptr, int fd, struct stat * st)
{
	memset (st, 0, sizeof (* st));
	return _swistat (fd, st);
}

int _stat_r (struct _reent *ptr, const char *fname, struct stat *st)
{
	int fd, res;
	memset (st, 0, sizeof (* st));
	/* The best we can do is try to open the file readonly.  
	   If it exists, then we can guess a few things about it. */
	if ((fd = _open_r (ptr, fname, O_RDONLY, 0)) == -1)
		return -1;
	st->st_mode |= S_IFREG | S_IREAD;
	res = _swistat (fd, st);
	/* Not interested in the error. */
	_close_r (ptr, fd); 
	return res;
}

int _link_r (struct _reent *ptr, const char *oldpath, const char *newpath)
{
	errno = ENOSYS;
	return -1;
}

int _unlink_r (struct _reent *ptr, const char *path)
{
	int res;
	int block[2];
	block[0] = (int)path;
	block[1] = strlen(path);
	res = do_AngelSWI (AngelSWI_Reason_Remove, block);

	if (res == -1) 
		return error (res);
	return 0;
}

int _gettimeofday_r (struct _reent *ptr, struct timeval * tp, void * tzvp)
{
	struct timezone *tzp = tzvp;
	if (tp)
	{
		/* Ask the host for the seconds since the Unix epoch.  */
		tp->tv_sec = do_AngelSWI (AngelSWI_Reason_Time,NULL);
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

/* Return a clock that ticks at 100Hz.  */
clock_t _times_r (struct _reent *ptr, struct tms * tp)
{
	clock_t timeval = do_AngelSWI (AngelSWI_Reason_Clock,NULL);

	if (tp)
	{
		tp->tms_utime  = timeval;	/* user time */
		tp->tms_stime  = 0;	/* system time */
		tp->tms_cutime = 0;	/* user time, children */
		tp->tms_cstime = 0;	/* system time, children */
	}

	return timeval;
};

int _isatty_r (struct _reent *ptr, int fd)
{
	struct fdent *pfd;

	pfd = findslot (fd);
	if (pfd == NULL)
	{
		errno = EBADF;
		return -1;
	}

	return checkerror (do_AngelSWI (AngelSWI_Reason_IsTTY, &pfd->handle));
}

#if 0
int _system (const char *s)
{
	int block[2];
	int e;

	/* Hmmm.  The ARM debug interface specification doesn't say whether
	   SYS_SYSTEM does the right thing with a null argument, or assign any
	   meaning to its return value.  Try to do something reasonable....  */
	if (!s)
		return 1;  /* maybe there is a shell available? we can hope. :-P */
	block[0] = (int)s;
	block[1] = strlen (s);
	e = checkerror (do_AngelSWI (AngelSWI_Reason_System, block));
	if ((e >= 0) && (e < 256))
	{
		/* We have to convert e, an exit status to the encoded status of
		   the command.  To avoid hard coding the exit status, we simply
		   loop until we find the right position.  */
		int exit_code;

		for (exit_code = e; e && WEXITSTATUS (e) != exit_code; e <<= 1)
			continue;
	}
	return e;
}
#endif

int _rename_r (struct _reent *ptr, const char * oldpath, const char * newpath)
{
	int block[4];
	block[0] = (int)oldpath;
	block[1] = strlen(oldpath);
	block[2] = (int)newpath;
	block[3] = strlen(newpath);
	return checkerror (do_AngelSWI (AngelSWI_Reason_Rename, block)) ? -1 : 0;
}

void _exit(int status)
{
	while (1); // wait indefinitely
}

int _kill_r (struct _reent *ptr, int pid, int sig)
{
	(void) pid; (void) sig;
	/* Note: The pid argument is thrown away.  */
	switch (sig)
	{
		case SIGABRT:
			return do_AngelSWI (AngelSWI_Reason_ReportException,
					(void *) ADP_Stopped_RunTimeError);
		default:
			return do_AngelSWI (AngelSWI_Reason_ReportException,
					(void *) ADP_Stopped_ApplicationExit);
	}
}

///////////////////////////////////////////////////////////////////////////////
//                    FreeRTOS Specific stuff below                          //
///////////////////////////////////////////////////////////////////////////////

/*
__attribute__ ((weak)) void vPortInitialiseBlocks( void )
{
	// FIXME
}
*/

__attribute__ ((weak)) size_t xPortGetFreeHeapSize( void )
{
	/* FIXME this isn't really correct, it reports only the RAM which is available to the
	 * system, but which hasn't yet been assigned to Newlibs Malloc implementation. */
	
	/* Initialize on first call */
	if (heap_end == 0)
		heap_end = (void *)&__start_of_heap__;

	return &__stack_min__ - (unsigned int *)heap_end;
}

#if 0
// Function to add multithread support to newlib
struct _reent *__getreent( void )
{
	NU_HISR *HisrPtr;
	NU_TASK *TaskPtr;

	if ((HisrPtr = TCC_Current_HISR_Pointer()) == NULL)
	{
		// Running in normal task mode
		if ((TaskPtr = TCC_Current_Task_Pointer()) == NULL)
		{
			// No valid tasks are running currently return global space
			return _impure_ptr;
		}
		return TaskPtr->_impure_ptr;
	}
	return HisrPtr->_impure_ptr;
	return _impure_ptr;
}
#endif

#if 0
/*** From reent.c ***/

/* Interim cleanup code */

void
_DEFUN (cleanup_glue, (ptr, glue),
     struct _reent *ptr _AND
     struct _glue *glue)
{
  /* Have to reclaim these in reverse order: */
  if (glue->_next)
    cleanup_glue (ptr, glue->_next);

  _free_r (ptr, glue);
}

void
_DEFUN (_reclaim_reent, (ptr),
     struct _reent *ptr)
{
  if (ptr != _impure_ptr)
    {
      /* used by mprec routines. */
#ifdef _REENT_SMALL
      if (ptr->_mp)	/* don't bother allocating it! */
#endif
      if (_REENT_MP_FREELIST(ptr))
	{
	  int i;
	  for (i = 0; i < 15 /* _Kmax */; i++) 
	    {
	      struct _Bigint *thisone, *nextone;
	
	      nextone = _REENT_MP_FREELIST(ptr)[i];
	      while (nextone)
		{
		  thisone = nextone;
		  nextone = nextone->_next;
		  _free_r (ptr, thisone);
		}
	    }    

	  _free_r (ptr, _REENT_MP_FREELIST(ptr));
	}
      if (_REENT_MP_RESULT(ptr))
	_free_r (ptr, _REENT_MP_RESULT(ptr));

#ifdef _REENT_SMALL
      if (ptr->_emergency)
	_free_r (ptr, ptr->_emergency);
      if (ptr->_mp)
	_free_r (ptr, ptr->_mp);
      if (ptr->_r48)
	_free_r (ptr, ptr->_r48);
      if (ptr->_localtime_buf)
	_free_r (ptr, ptr->_localtime_buf);
      if (ptr->_asctime_buf)
	_free_r (ptr, ptr->_asctime_buf);
      if (ptr->_atexit->_on_exit_args_ptr)
	_free_r (ptr, ptr->_atexit->_on_exit_args_ptr);
#else
      /* atexit stuff */
      if ((ptr->_atexit) && (ptr->_atexit != &ptr->_atexit0))
	{
	  struct _atexit *p, *q;
	  for (p = ptr->_atexit; p != &ptr->_atexit0;)
	    {
	      q = p;
	      p = p->_next;
	      _free_r (ptr, q);
	    }
	}
#endif

      if (ptr->_cvtbuf)
	_free_r (ptr, ptr->_cvtbuf);

      if (ptr->__sdidinit)
	{
	  /* cleanup won't reclaim memory 'coz usually it's run
	     before the program exits, and who wants to wait for that? */
	  ptr->__cleanup (ptr);

	  if (ptr->__sglue._next)
	    cleanup_glue (ptr, ptr->__sglue._next);
	}

      /* Malloc memory not reclaimed; no good way to return memory anyway. */

    }
}

/*
 *  Do atexit() processing and cleanup
 *
 *  NOTE:  This is to be executed at task exit.  It does not tear anything
 *         down which is used on a global basis.
 */

void
_DEFUN (_wrapup_reent, (ptr), struct _reent *ptr)
{
  register struct _atexit *p;
  register int n;

  if (ptr == 0)
      ptr = _REENT;

#ifdef _REENT_SMALL
  for (p = &ptr->_atexit, n = p->_ind; --n >= 0;)
    (*p->_fns[n]) ();
#else
  for (p = ptr->_atexit; p; p = p->_next)
    for (n = p->_ind; --n >= 0;)
      (*p->_fns[n]) ();
#endif
  if (ptr->__cleanup)
    (*ptr->__cleanup) (ptr);
}

#endif

/*** From impure.c ***/
#if 0
#ifndef __ATTRIBUTE_IMPURE_PTR__
#define __ATTRIBUTE_IMPURE_PTR__
#endif
#ifndef __ATTRIBUTE_IMPURE_DATA__
#define __ATTRIBUTE_IMPURE_DATA__
#endif

static struct _reent __ATTRIBUTE_IMPURE_DATA__ impure_data = _REENT_INIT (impure_data);
struct _reent *__ATTRIBUTE_IMPURE_PTR__ _impure_ptr = &impure_data;
struct _reent *_CONST __ATTRIBUTE_IMPURE_PTR__ _global_impure_ptr = &impure_data;

#endif

