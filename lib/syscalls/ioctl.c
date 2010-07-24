#include <sys/ioctl.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

int ioctl(int fd, unsigned long request, ...)
{
	va_list ap;

	va_start(ap, request);
	printf("first variable arg %d\n", va_arg(ap, int));
	va_end(ap);

	errno = ENOTTY;
	return -1;
}

