#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "device_manager.h"
extern "C" {
	extern struct FileLikeObj SemiFS_FLO;
	struct FileLikeObj *semi = &SemiFS_FLO;
}

void TestSemiFS()
{
	printf("Trying to open a file...\n");
	int fd = semi->open_("test.txt", O_CREAT | O_APPEND, 0x755);
	if (fd != -1)
	{
		char str[] = "hello world\n";
		printf("Trying to write %d bytes to /semifs/test.txt...\n", strlen(str));
		ssize_t nwritten = semi->write_(fd, str, strlen(str));
		if (nwritten != (ssize_t)strlen(str))
			printf("Failed to write (%d): %s\n", nwritten, strerror(errno));
		semi->close_(fd);
	}
	else
		printf("Failed to open /semifs/test.txt for writing\n");
}

