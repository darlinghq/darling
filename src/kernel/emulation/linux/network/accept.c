#include "accept.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "socket.h"
#include "duct.h"
#include "../bsdthread/cancelable.h"
#include "../fcntl/fcntl.h"
#include "../fcntl/open.h"

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

		// on macOS (and BSD), accept-ed sockets inherit O_NONBLOCK
		int parent_flags = sys_fcntl_nocancel(fd, F_GETFL, 0);
		int child_flags = sys_fcntl_nocancel(ret, F_GETFL, 0);
		// we silently ignore errors we get from fcntl (it's probably fine; fnctl shouldn't fail here)
		if (parent_flags >= 0 && child_flags >= 0 && (parent_flags & BSD_O_NONBLOCK) != 0) {
			sys_fcntl_nocancel(ret, F_SETFL, child_flags | BSD_O_NONBLOCK);
		}
	}

	return ret;
}

