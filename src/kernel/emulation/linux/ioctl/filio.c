#include "ioctl.h"
#include "filio.h"
#include "../fcntl/fcntl.h"
#include "../fcntl/open.h"
#include <sys/errno.h>
#include "../errno.h"
#include "../unistd/readlink.h"
#include "../fdpath.h"
#include "../simple.h"
#include <stddef.h>
#include <stdbool.h>

#define	D_TAPE	1
#define	D_DISK	2
#define	D_TTY	3


extern int sprintf(char* buf, const char* fmt, ...);
extern int strncmp(const char *, const char *, __SIZE_TYPE__);
extern __SIZE_TYPE__ strlen(const char*);
extern void* memmove(void*, void*, __SIZE_TYPE__);

static bool get_fd_path(int fd, char* buf, size_t len)
{
	int ret;

	ret = fdpath(fd, buf, len);
	if (ret < 0)
		return false;

	if (strncmp(buf, "/Volumes/SystemRoot/dev/", 24) == 0)
	{
		// Remove /Volumes/SystemRoot from the start
		memmove(buf, buf + 19, strlen(buf + 19) + 1);
	}
	return true;
}


int handle_filio(int fd, int cmd, void* arg, int* retval)
{
	switch (cmd)
	{
		case BSD_FIODTYPE:
		{
			char orig_path[256];
			int* iarg = (int*) arg;

			if (iarg == NULL)
			{
				*retval = -EFAULT;
				return IOCTL_HANDLED;
			}

			if (!get_fd_path(fd, orig_path, sizeof(orig_path)))
			{
				*retval = -EBADF;
				return IOCTL_HANDLED;
			}
			__simple_kprintf("dtype for fd %d -> %s\n", fd, orig_path);

			*retval = 0;
			if (strncmp(orig_path, "/dev/pts", 8) == 0
					|| strncmp(orig_path, "/dev/tty", 8) == 0)
			{
				*iarg = D_TTY;
			}
			else if (strncmp(orig_path, "/dev/st", 7) == 0
					|| strncmp(orig_path, "/dev/nst", 8) == 0)
			{
				*iarg = D_TAPE;
			}
			else
			{
				*iarg = D_DISK;
			}
			return IOCTL_HANDLED;
		}
		case BSD_FIONBIO:
		{
			*retval = __real_ioctl(fd, 0x5421, arg);
			if (*retval < 0)
				*retval = errno_linux_to_bsd(*retval);
			
			return IOCTL_HANDLED;
		}
		case BSD_FIOASYNC:
		{
			*retval = __real_ioctl(fd, 0x5452, arg);
			if (*retval < 0)
				*retval = errno_linux_to_bsd(*retval);
			
			return IOCTL_HANDLED;
		}
		case BSD_FIOCLEX:
		{
			*retval = __real_ioctl(fd, 0x5451, arg);
			if (*retval < 0)
				*retval = errno_linux_to_bsd(*retval);
			
			return IOCTL_HANDLED;
		}
		case BSD_FIONCLEX:
		{
			*retval = __real_ioctl(fd, 0x5450, arg);
			if (*retval < 0)
				*retval = errno_linux_to_bsd(*retval);
			
			return IOCTL_HANDLED;
		}
		case BSD_FIONREAD:
		{
			*retval = __real_ioctl(fd, 0x541B, arg);
			if (*retval < 0)
				*retval = errno_linux_to_bsd(*retval);
			
			return IOCTL_HANDLED;
		}
		case BSD_FIOGETOWN:
		{
			*retval = __real_ioctl(fd, 0x8903, arg);
			if (*retval < 0)
				*retval = errno_linux_to_bsd(*retval);
			
			return IOCTL_HANDLED;
		}
		case BSD_FIOSETOWN:
		{
			*retval = __real_ioctl(fd, 0x8901, arg);
			if (*retval < 0)
				*retval = errno_linux_to_bsd(*retval);
			
			return IOCTL_HANDLED;
		}
		/*
		case BSD_FIOASYNC:
		{
			int flags;
			int* iarg = (int*) arg;
			
			if (!iarg)
				*retval = -EINVAL;
			
			*retval = sys_fcntl(fd, F_GETFL, 0);
			if (*retval < 0)
				return IOCTL_HANDLED;
			
			flags = *retval;
			
			if (*iarg)
				flags |= BSD_O_ASYNC;
			else
				flags &= ~BSD_O_ASYNC;
			
			*retval = sys_fcntl(fd, F_SETFL, flags);
			
			return IOCTL_HANDLED;
		}
		*/
		default:
			return IOCTL_PASS;
	}
}

