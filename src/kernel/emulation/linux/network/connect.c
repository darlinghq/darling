#include "connect.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include "duct.h"
#include "../bsdthread/cancelable.h"

extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);
extern __SIZE_TYPE__ strlen(const char* src);
extern char* strcpy(char* dest, const char* src);
extern char *strncpy(char *dest, const char *src, __SIZE_TYPE__ n);

#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"

long sys_connect(int fd, const void* name, int socklen)
{
	CANCELATION_POINT();
	return sys_connect_nocancel(fd, name, socklen);
}

long sys_connect_nocancel(int fd, const void* name, int socklen)
{
	int ret;
	int linux_domain;
	struct sockaddr_fixup* fixed;

	if (socklen > 512)
		return -EINVAL;

	fixed = __builtin_alloca(sockaddr_fixup_size_from_bsd(name, socklen));
	ret = socklen = sockaddr_fixup_from_bsd(fixed, name, socklen);
	if (ret < 0)
		return ret;

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_CONNECT, ((long[6]) { fd, fixed, socklen }));
#else
	ret = LINUX_SYSCALL(__NR_connect, fd, fixed, socklen);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

