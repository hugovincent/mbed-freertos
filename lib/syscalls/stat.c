#include <stdlib.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>
#include <errno.h>

#include <reent.h>
#include "lib/syscalls/syscalls_util.h"

/* In lib/syscalls/fstat.c */
extern int _swistat(int fd, struct stat * st);

int _stat_r(struct _reent *ptr, const char *fname, struct stat *st)
{
	int fd, res;
	memset(st, 0, sizeof (* st));
	/* The best we can do is try to open the file readonly.  
	   If it exists, then we can guess a few things about it. */
	if ((fd = _open_r(ptr, fname, O_RDONLY, 0)) == -1)
		return -1;
	st->st_mode |= S_IFREG | S_IREAD;
	res = _swistat(fd, st);
	/* Not interested in the error. */
	_close_r(ptr, fd); 
	return res;
}

