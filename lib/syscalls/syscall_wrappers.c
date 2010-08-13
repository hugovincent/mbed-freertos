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

#define MPU_WRAPPER(return_type, syscall, argslist, ...)					\
	return_type syscall(__VA_ARGS__) {										\
		MPU_START();														\
		return_type ret = _##syscall##_r argslist;							\
		MPU_END();															\
		return ret;															\
	}

#define MPU_WRAPPER_VOID(syscall, argslist, ...)							\
	void syscall(__VA_ARGS__) {												\
		MPU_START();														\
		_##syscall##_r argslist;											\
		MPU_END();															\
	}

#define MPU_WRAPPER_VA(return_type, syscall, argslist, ...)					\
	return_type syscall(__VA_ARGS__, const char *fmt, ...) {				\
		va_list arglist;													\
		va_start(arglist, fmt);												\
		MPU_START();														\
		return_type ret = _v##syscall##_r argslist;							\
		MPU_END();															\
		va_end(arglist);													\
		return ret;															\
	}

#define MPU_WRAPPER_VA_1(return_type, syscall, argslist)					\
	return_type syscall(const char *fmt, ...) {								\
		va_list arglist;													\
		va_start(arglist, fmt);												\
		MPU_START();														\
		return_type ret = _v##syscall##_r argslist;							\
		MPU_END();															\
		va_end(arglist);													\
		return ret;															\
	}


MPU_WRAPPER(int,		close,			(__getreent(), fd), int fd)
//MPU_WRAPPER(int, 		exit,			(__getreent(), status), int status)
MPU_WRAPPER(int,		fcntl,			(__getreent(), fd, a, b), int fd, int a, int b)
MPU_WRAPPER(int,		fstat,			(__getreent(), fd, buf), int fd, struct stat *buf)
MPU_WRAPPER(pid_t,		getpid,			(__getreent()))
MPU_WRAPPER(int,		gettimeofday,	(__getreent(), tp, tzp), struct timeval *restrict tp, void *restrict tzp)
MPU_WRAPPER(int,		isatty,			(__getreent(), fd), int fd)
MPU_WRAPPER(int,		kill,			(__getreent(), pid, sig), pid_t pid, int sig)
MPU_WRAPPER(int,		link,			(__getreent(), path1, path2), const char *path1, const char *path2)
MPU_WRAPPER(off_t,		lseek,			(__getreent(), fd, offs, whence), int fd, off_t offs, int whence)
MPU_WRAPPER(int,		open,			(__getreent(), path, flags, mode), const char *path, int flags, int mode)
MPU_WRAPPER(ssize_t,	read,			(__getreent(), fd, buf, nbyte), int fd, void *buf, size_t nbyte)
MPU_WRAPPER(void *,		sbrk,			(__getreent(), incr), int incr)
MPU_WRAPPER(int,		stat,			(__getreent(), path, buf), const char *restrict path, struct stat *restrict buf)
MPU_WRAPPER(clock_t,	times,			(__getreent(), buffer), struct tms *buffer)
MPU_WRAPPER(int,		unlink,			(__getreent(), path), const char *path)
MPU_WRAPPER(ssize_t, 	write, 			(__getreent(), fd, buf, nbyte), int fd, const void *buf, size_t nbyte)

// The following is a list of non-"_r" symbols used in the Newlib build that
// are referenced but not defined (and thus need to be provided here). Similarly
// to the above MPU wrappers, these need to call __getreent() thus need to raise
// and lower their privileges.

MPU_WRAPPER(void *,		calloc,			(__getreent(), count, size), size_t count, size_t size)
MPU_WRAPPER(int,		fclose,			(__getreent(), stream), FILE *stream)
MPU_WRAPPER(int,		fflush,			(__getreent(), stream), FILE *stream)
MPU_WRAPPER(int,		fputc,			(__getreent(), c, stream), int c, FILE *stream)
MPU_WRAPPER(int,		fputs,			(__getreent(), str, stream), const char *restrict str, FILE *restrict stream)
MPU_WRAPPER(size_t,		fread,			(__getreent(), ptr, size, nitems, stream), void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream)
MPU_WRAPPER_VOID(		free, 			(__getreent(), ptr), void *ptr)
MPU_WRAPPER(size_t,		fwrite,			(__getreent(), ptr, size, nitems, stream), const void *ptr, size_t size, size_t nitems, FILE *stream)
MPU_WRAPPER(int,		getc_unlocked,	(__getreent(), stream), FILE *stream)
MPU_WRAPPER(char *,		getenv,			(__getreent(), name), const char *name)
MPU_WRAPPER(void *, 	malloc, 		(__getreent(), size), size_t size)
MPU_WRAPPER(size_t,		mbrtowc,		(__getreent(), pwc, s, n, ps), wchar_t *restrict pwc, const char *restrict s, size_t n, mbstate_t *restrict ps)
MPU_WRAPPER(int,		mkstemp,		(__getreent(), template), char *template)
MPU_WRAPPER(int,		putc_unlocked,	(__getreent(), c, stream), int c, FILE *stream)
MPU_WRAPPER(int,		puts,			(__getreent(), str), const char *str)
MPU_WRAPPER(int,		raise,			(__getreent(), sig), int sig)
MPU_WRAPPER(void *,		realloc,		(__getreent(), ptr, size), void *ptr, size_t size)
MPU_WRAPPER(char *,		strdup,			(__getreent(), s1), const char *s1)
MPU_WRAPPER(long,		strtol,			(__getreent(), str, endptr, base), const char *restrict str, char **restrict endptr, int base)
MPU_WRAPPER(unsigned long, strtoul,		(__getreent(), str, endptr, base), const char *restrict str, char **restrict endptr, int base)
MPU_WRAPPER_VA(int, 	sprintf,		(__getreent(), s, fmt, arglist), char *s)
MPU_WRAPPER_VA(int, 	snprintf,		(__getreent(), s, n, fmt, arglist), char *s, size_t n)
MPU_WRAPPER_VA(int,		fprintf,		(__getreent(), stream, fmt, arglist), FILE *stream)
MPU_WRAPPER_VA(int,		sscanf,			(__getreent(), s, fmt, arglist), const char *s)
MPU_WRAPPER_VA_1(int, 	printf,			(__getreent(), fmt, arglist))

// fiprintf?

