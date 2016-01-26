#include "recvfrom.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <stddef.h>
#include "socket.h"
#include "duct.h"

extern void *memcpy(void *dest, const void *src, unsigned long n);

long sys_recvfrom(int fd, void* buf, unsigned long len,
		int flags, const void* from, int* socklen)
{
	return sys_recvfrom_nocancel(fd, buf, len, flags, from, socklen);
}

long sys_recvfrom_nocancel(int fd, void* buf, unsigned long len,
		int flags, const void* from, int* socklen)
{
	int ret, linux_flags;
	struct sockaddr_fixup* fixed;

	linux_flags = msgflags_bsd_to_linux(flags);

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_RECVFROM, ((long[6]) { fd, buf, len,
			flags, from, socklen }));
#else
	ret = LINUX_SYSCALL(__NR_recvfrom, fd, buf, len, flags, from, socklen);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else if (from != NULL)
	{
		fixed = (struct sockaddr_fixup*) from;
		fixed->bsd_family = sfamily_linux_to_bsd(fixed->linux_family);
		fixed->bsd_length = *socklen;
	}

	return ret;
}

