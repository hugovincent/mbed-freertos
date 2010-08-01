#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "device_manager.h"
extern "C" {
	extern struct FileLikeObj SemiFS_FLO;
	struct FileLikeObj *semi = &SemiFS_FLO;

	extern struct FileLikeObj RomFS_FLO;
	struct FileLikeObj *rom = &RomFS_FLO;
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
			printf("Failed to write (returned %d): %s\n", nwritten, strerror(errno));
		semi->close_(fd);
	}
	else
		printf("Failed to open /semifs/test.txt for writing\n");
}

void TestRomFS()
{
	printf("Trying to open a file...\n");
	int fd = rom->open_("test.txt", O_RDONLY, 0);
	if (fd != -1)
	{
		char buf[72];
		printf("Trying to read %d bytes from /romfs/test.txt...\n", sizeof(buf));
		ssize_t nread = rom->read_(fd, &buf, sizeof(buf));
		if (nread < 0)
			printf("Failed to read (returned %d): %s\n", nread, strerror(errno));
		buf[nread] = 0;
		printf("Read %d bytes: \"%s\"\n", nread, buf);
		rom->close_(fd);
	}
	else
		printf("Failed to open /romfs/test.txt for reading\n");
}

