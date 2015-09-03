#include "open.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
//#include "../../../../platform-include/sys/fcntl.h"
#include "../../../../libc/include/fcntl.h"

#ifndef O_NOFOLLOW
#	define O_NOFOLLOW 0x0100
#endif
#ifndef O_CLOEXEC
#	define O_CLOEXEC 0x1000000
#endif
#ifndef O_DIRECTORY
#	define O_DIRECTORY 0x100000
#endif

int oflags_bsd_to_linux(int flags);

long sys_open(const char* filename, int flags, unsigned int mode)
{
	int ret, linux_flags;

	linux_flags = oflags_bsd_to_linux(flags);

	if (sizeof(void*) == 4)
	{
		linux_flags |= LINUX_O_LARGEFILE;
	}

	// TODO: Check filename, handle wrong case

	ret = LINUX_SYSCALL(__NR_open, filename, linux_flags, mode);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

int oflags_bsd_to_linux(int flags)
{
	int linux_flags = 0;

	if (flags & O_RDONLY) /* always false */
		linux_flags |= LINUX_O_RDONLY;
	if (flags & O_WRONLY)
		linux_flags |= LINUX_O_WRONLY;
	if (flags & O_RDWR)
		linux_flags |= LINUX_O_RDWR;
	if (flags & O_NONBLOCK)
		linux_flags |= LINUX_O_NONBLOCK;
	if (flags & O_APPEND)
		linux_flags |= LINUX_O_APPEND;
	if (flags & O_CREAT)
		linux_flags |= LINUX_O_CREAT;
	if (flags & O_TRUNC)
		linux_flags |= LINUX_O_TRUNC;
	if (flags & O_EXCL)
		linux_flags |= LINUX_O_EXCL;
	if (flags & O_CLOEXEC)
		linux_flags |= LINUX_O_CLOEXEC;
	if (flags & O_NOFOLLOW)
		linux_flags |= LINUX_O_NOFOLLOW;
	if (flags & O_DIRECTORY)
		linux_flags |= LINUX_O_DIRECTORY;
	if (flags & O_CLOEXEC)
		linux_flags |= LINUX_O_CLOEXEC;

	return linux_flags;
}

