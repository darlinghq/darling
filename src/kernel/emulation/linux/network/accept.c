#include "accept.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "socket.h"
#include "duct.h"
#include "../bsdthread/cancelable.h"

long sys_accept(int fd, void* from, int* socklen)
{
	CANCELATION_POINT();
	return sys_accept_nocancel(fd, from, socklen);
}

long sys_accept_nocancel(int fd, void* from, int* socklen)
{
	int ret;
	struct sockaddr_fixup* fixed;

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_ACCEPT, ((long[6]) { fd, from, socklen }));
#else
	ret = LINUX_SYSCALL(__NR_accept, fd, from, socklen);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else
	{
		fixed = (struct sockaddr_fixup*) from;
		fixed->bsd_family = sfamily_linux_to_bsd(fixed->linux_family);
		fixed->bsd_length = *socklen;
	}

	return ret;
}

