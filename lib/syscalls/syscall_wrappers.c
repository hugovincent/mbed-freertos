/* This file serves two purposes: it provides the non-reentrant-looking (non-"_r")
 * wrappers for the newlib syscalls and (if building with MPU support) inserts
 * the necessary MPU privilege raising/lowering machinery around the calls into
 * privileged code.
 *
 * Hugo Vincent, 13 August 2010.
 */

#include "FreeRTOS.h"
#include <reent.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <wchar.h>
#include <signal.h>
#include <string.h>

#ifdef CORE_HAS_MPU

// MPU privilege handling from kernel/port/ARM_CM3_MPU/port.c

static portBASE_TYPE prvRaisePrivilege(void) __attribute__(( naked ));
static portBASE_TYPE prvRaisePrivilege(void)
{
	__asm volatile
	(
		"	mrs r0, control						\n"
		"	tst r0, #1							\n" /* Is the task running privileged? */
		"	itte ne								\n"
		"	movne r0, #0						\n" /* CONTROL[0]!=0, return false. */
		"	svcne %0							\n" /* Switch to privileged. */
		"	moveq r0, #1						\n" /* CONTROL[0]==0, return true. */
		"	bx lr								\n"
		:: "i" (portSVC_RAISE_PRIVILEGE) : "r0"
	);

	return 0;
}

#define MPU_START()		portBASE_TYPE xRunningPrivileged = prvRaisePrivilege()
#define MPU_END()		if( xRunningPrivileged != pdTRUE ) \
	__asm volatile ( " mrs r0, control \n orr r0, #1 \n msr control, r0" :::"r0" )

#else

#define MPU_START()		do {} while (0)
#define MPU_END()		do {} while (0)

#endif

/* ------------------------------------------------------------------------- */
// MPU and Reentrancy Wrapper macros:

// Plain MPU wrapper:
#define MPU_WRAPPER(return_type, syscall, argslist, ...)					\
	return_type syscall(__VA_ARGS__) {										\
		MPU_START();														\
		return_type ret = _##syscall##_r argslist;							\
		MPU_END();															\
		return ret;															\
	}

// Plain MPU wrapper for functions returning void:
#define MPU_WRAPPER_VOID(syscall, argslist, ...)							\
	void syscall(__VA_ARGS__) {												\
		MPU_START();														\
		_##syscall##_r argslist;											\
		MPU_END();															\
	}

// MPU wrapper for variadic functions in the ioctl family that have a
// corresponding va_list variant (as with printf/vprintf):
#define MPU_WRAPPER_VA(return_type, syscall, argslist, ...)					\
	return_type syscall(__VA_ARGS__, ...) {									\
		va_list arglist;													\
		va_start(arglist, request);											\
		MPU_START();														\
		return_type ret = _v##syscall##_r argslist;							\
		MPU_END();															\
		va_end(arglist);													\
		return ret;															\
	}

// Plain reent wrapper:
#define REENT_WRAPPER(return_type, syscall, argslist, ...)					\
	return_type syscall(__VA_ARGS__) {										\
		return_type ret = _##syscall##_r argslist;							\
		return ret;															\
	}

// Plain reent wrapper for functions returning void:
#define REENT_WRAPPER_VOID(syscall, argslist, ...)							\
	void syscall(__VA_ARGS__) {												\
		_##syscall##_r argslist;											\
	}

// Reent wrapper for variadic functions in the printf family that have a
// corresponding va_list variant (as with printf/vprintf):
#define REENT_WRAPPER_VA(return_type, syscall, argslist, ...)				\
	return_type syscall(__VA_ARGS__, ...) {									\
		va_list arglist;													\
		va_start(arglist, fmt);												\
		return_type ret = _v##syscall##_r argslist;							\
		va_end(arglist);													\
		return ret;															\
	}

/* ------------------------------------------------------------------------- */
// Foward declarations needed by the wrappers:

struct _reent *__getreent_mpu();
extern int _chmod_r(struct _reent *ptr, const char *path, mode_t mode);
extern int _chown_r(struct _reent *ptr, const char *path, uid_t owner, gid_t group);
extern int _fsync_r(struct _reent *ptr, int fd);
extern int _vioctl_r(struct _reent *ptr, int fd, unsigned long request, va_list args);
extern int _mkdir_r(struct _reent *ptr, const char *path, mode_t mode);
extern int _mkfifo_r(struct _reent *ptr, const char *path, mode_t mode);
extern ssize_t _readlink_r(struct _reent *ptr, const char *restrict path, char *restrict buf, size_t bufsize);
extern int _rename_r(struct _reent *ptr, const char * oldpath, const char * newpath);
extern int __sigtramp_r(struct _reent *ptr, int sig);
extern int _init_signal_r(struct _reent *ptr);
extern int _nanosleep_r(struct _reent *ptr, const struct timespec *rqtp, struct timespec *rmtp);
extern int _usleep_r(struct _reent *ptr, useconds_t useconds);
extern unsigned int _sleep_r(struct _reent *ptr, unsigned int seconds);
extern int _symlink_r(struct _reent *ptr, const char *path1, const char *path2);
extern void *_memalign_r(struct _reent *ptr, size_t blocksize, size_t bytes);

/* ------------------------------------------------------------------------- */
// The following are syscall wrappers.

MPU_WRAPPER(int,		close,			(__getreent_mpu(), fd), int fd)
MPU_WRAPPER(int,		chmod,			(__getreent_mpu(), path, mode), const char *path, mode_t mode)
MPU_WRAPPER(int,		chown,			(__getreent_mpu(), path, owner, group), const char *path, uid_t owner, gid_t group)
//MPU_WRAPPER(int, 		exit,			(__getreent_mpu(), status), int status)
MPU_WRAPPER(int,		fcntl,			(__getreent_mpu(), fd, a, b), int fd, int a, int b)
MPU_WRAPPER(int,		fsync,			(__getreent_mpu(), fd), int fd)
MPU_WRAPPER(int,		fstat,			(__getreent_mpu(), fd, buf), int fd, struct stat *buf)
MPU_WRAPPER(pid_t,		getpid,			(__getreent_mpu()))
MPU_WRAPPER(int,		gettimeofday,	(__getreent_mpu(), tp, tzp), struct timeval *restrict tp, void *restrict tzp)
MPU_WRAPPER_VA(int,		ioctl,			(__getreent_mpu(), fd, request, arglist), int fd, unsigned long request)
MPU_WRAPPER(int,		isatty,			(__getreent_mpu(), fd), int fd)
MPU_WRAPPER(int,		kill,			(__getreent_mpu(), pid, sig), pid_t pid, int sig)
MPU_WRAPPER(int,		link,			(__getreent_mpu(), path1, path2), const char *path1, const char *path2)
MPU_WRAPPER(off_t,		lseek,			(__getreent_mpu(), fd, offs, whence), int fd, off_t offs, int whence)
MPU_WRAPPER(int,		mkdir,			(__getreent_mpu(), path, mode), const char *path, mode_t mode)
MPU_WRAPPER(int,		mkfifo,			(__getreent_mpu(), path, mode), const char *path, mode_t mode)
MPU_WRAPPER(int,		nanosleep,		(__getreent_mpu(), rqtp, rmtp), const struct timespec *rqtp, struct timespec *rmtp)
MPU_WRAPPER(int,		open,			(__getreent_mpu(), path, flags, mode), const char *path, int flags, int mode)
MPU_WRAPPER(int,		raise,			(__getreent_mpu(), sig), int sig)
MPU_WRAPPER(ssize_t,	read,			(__getreent_mpu(), fd, buf, nbyte), int fd, void *buf, size_t nbyte)
MPU_WRAPPER(ssize_t,	readlink,		(__getreent_mpu(), path, buf, bufsize), const char *path, char *buf, size_t bufsize)
MPU_WRAPPER(int,		rename,			(__getreent_mpu(), oldpath, newpath), const char *oldpath, const char *newpath)
MPU_WRAPPER(void *,		sbrk,			(__getreent_mpu(), incr), int incr)
MPU_WRAPPER(_sig_func_ptr,signal,		(__getreent_mpu(), sig, func), int sig, _sig_func_ptr func)
MPU_WRAPPER(int,		init_signal,	(__getreent_mpu()))
MPU_WRAPPER(int,		_sigtramp,		(__getreent_mpu(), sig), int sig)
MPU_WRAPPER(int,		sleep,			(__getreent_mpu(), seconds), unsigned int seconds)
MPU_WRAPPER(int,		stat,			(__getreent_mpu(), path, buf), const char *restrict path, struct stat *restrict buf)
MPU_WRAPPER(int,		symlink,		(__getreent_mpu(), path1, path2), const char *path1, const char *path2)
MPU_WRAPPER(clock_t,	times,			(__getreent_mpu(), buffer), struct tms *buffer)
MPU_WRAPPER(int,		unlink,			(__getreent_mpu(), path), const char *path)
MPU_WRAPPER(int,		usleep,			(__getreent_mpu(), usec), useconds_t usec)
MPU_WRAPPER(ssize_t, 	write, 			(__getreent_mpu(), fd, buf, nbyte), int fd, const void *buf, size_t nbyte)

// The following is a list of non-"_r" symbols used in the Newlib build that
// are referenced but not defined (and thus need to be provided here). Similarly
// to the above MPU wrappers, these need to call __getreent() thus need to raise
// and lower their privileges.

REENT_WRAPPER(void *,	calloc,			(__getreent(), count, size), size_t count, size_t size)
REENT_WRAPPER(int,		fclose,			(__getreent(), stream), FILE *stream)
REENT_WRAPPER(int,		fflush,			(__getreent(), stream), FILE *stream)
REENT_WRAPPER(int,		fputc,			(__getreent(), c, stream), int c, FILE *stream)
REENT_WRAPPER(int,		fputs,			(__getreent(), str, stream), const char *restrict str, FILE *restrict stream)
REENT_WRAPPER(size_t,	fread,			(__getreent(), ptr, size, nitems, stream), void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream)
REENT_WRAPPER_VOID(		free, 			(__getreent(), ptr), void *ptr)
REENT_WRAPPER(size_t,	fwrite,			(__getreent(), ptr, size, nitems, stream), const void *ptr, size_t size, size_t nitems, FILE *stream)
REENT_WRAPPER(int,		getc_unlocked,	(__getreent(), stream), FILE *stream)
REENT_WRAPPER(char *,	getenv,			(__getreent(), name), const char *name)
REENT_WRAPPER(void *, 	malloc, 		(__getreent(), size), size_t size)
REENT_WRAPPER(size_t,	mbrtowc,		(__getreent(), pwc, s, n, ps), wchar_t *restrict pwc, const char *restrict s, size_t n, mbstate_t *restrict ps)
REENT_WRAPPER(void *, 	memalign, 		(__getreent(), blocksize, bytes), size_t blocksize, size_t bytes)
REENT_WRAPPER(int,		mkstemp,		(__getreent(), template), char *template)
REENT_WRAPPER(int,		putc_unlocked,	(__getreent(), c, stream), int c, FILE *stream)
REENT_WRAPPER(int,		puts,			(__getreent(), str), const char *str)
REENT_WRAPPER(void *,	realloc,		(__getreent(), ptr, size), void *ptr, size_t size)
REENT_WRAPPER(char *,	strdup,			(__getreent(), s1), const char *s1)
REENT_WRAPPER(long,		strtol,			(__getreent(), str, endptr, base), const char *restrict str, char **restrict endptr, int base)
REENT_WRAPPER(unsigned long, strtoul,	(__getreent(), str, endptr, base), const char *restrict str, char **restrict endptr, int base)
REENT_WRAPPER(int,		vsnprintf,		(__getreent(), s, n, format, ap), char *restrict s, size_t n, const char *restrict format, va_list ap);
REENT_WRAPPER_VA(int, 	sprintf,		(__getreent(), s, fmt, arglist), char *s, const char *fmt)
REENT_WRAPPER_VA(int, 	snprintf,		(__getreent(), s, n, fmt, arglist), char *s, size_t n, const char *fmt)
REENT_WRAPPER_VA(int,	fprintf,		(__getreent(), stream, fmt, arglist), FILE *stream, const char *fmt)
REENT_WRAPPER_VA(int,	sscanf,			(__getreent(), s, fmt, arglist), const char *s, const char *fmt)
REENT_WRAPPER_VA(int, 	printf,			(__getreent(), fmt, arglist), const char *fmt)

// fiprintf?

/* ------------------------------------------------------------------------- */
// Reentrancy stuff: we have two variants of __getreent(); the first is a 
// privileged function and can only be called from within MPU raised priv blocks,
// and the other is a wrapper: unprivileged but slower as it must raise and lower
// the tasks privilege to get the reentrancy structure.

struct _reent *__getreent_mpu() PRIVILEGED_FUNCTION
{
	// FIXME do newlib <-> FreeRTOS thread integration here. 

	// This is an example for Nucleus RTOS:
#if 0
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
#endif
	return _impure_ptr;
}

struct _reent *__getreent()
{
	MPU_START();
	struct _reent *ret = __getreent_mpu();
	MPU_END();
	return ret;
}

int *__errno( void )
{
	return &__getreent()->_errno;
}

