#include "fcntl.h"
#include "open.h"
#include "../base.h"
#include "../simple.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"
#include "../bsdthread/cancelable.h"

#ifndef O_NOFOLLOW
#   define O_NOFOLLOW 0x0100
#endif
#ifndef O_DIRECTORY
#   define O_DIRECTORY 0x100000
#endif
#ifndef MAXPATHLEN
#	define MAXPATHLEN 1024
#endif

int oflags_bsd_to_linux(int flags);
int oflags_linux_to_bsd(int flags);

extern int sprintf(char *str, const char *format, ...);
long sys_readlink(const char* path, char* buf, unsigned long bsize);

long sys_fcntl(int fd, int cmd, long arg)
{
	CANCELATION_POINT();
	return sys_fcntl_nocancel(fd, cmd, arg);
}

long sys_fcntl_nocancel(int fd, int cmd, long arg)
{
	int ret, linux_cmd;

	switch (cmd)
	{
		case F_DUPFD:
			linux_cmd = LINUX_F_DUPFD;
			break;
		case F_GETFD:
			linux_cmd = LINUX_F_GETFD;
			break;
		case F_SETFD:
			linux_cmd = LINUX_F_SETFD;

			if (arg & ~FD_CLOEXEC)
				return -EINVAL;
			break;
		case F_GETFL:
			linux_cmd = LINUX_F_GETFL;
			break;
		case F_SETFL:
			linux_cmd = LINUX_F_SETFL;
			arg = oflags_bsd_to_linux(arg);
			break;
		case F_GETOWN:
			linux_cmd = LINUX_F_GETOWN;
			break;
		case F_SETOWN:
			linux_cmd = LINUX_F_SETOWN;
			break;
		case F_GETPATH:
		{
			char buf[100];
			int len;

			__simple_sprintf(buf, "/proc/self/fd/%d", fd);
			len = sys_readlink(buf, (char*) arg, MAXPATHLEN);

			if (len >= 0)
				((char*) arg)[len] = '\0';

			return 0;
		}
		case F_SETLK:
		case F_SETLKW:
		case F_GETLK:
			// TODO
			return 0;
		// TODO: implement remaining commands
		default:
			return -EINVAL;
	}

#ifdef __NR_fcntl64
	ret = LINUX_SYSCALL(__NR_fcntl64, fd, linux_cmd, arg);
#else
	ret = LINUX_SYSCALL(__NR_fcntl, fd, linux_cmd, arg);
#endif
	if (ret < 0)
	{
		ret = errno_linux_to_bsd(ret);
		return ret;
	}

	switch (cmd)
	{
		case F_GETFL:
			ret = oflags_linux_to_bsd(ret);
			break;
	}

	return ret;
}

