#include "bind.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include "duct.h"

extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);
extern __SIZE_TYPE__ strlen(const char* src);
extern char* strcpy(char* dest, const char* src);
extern char *strncpy(char *dest, const char *src, __SIZE_TYPE__ n);

#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"

long sys_bind(int fd, const void* name, int socklen)
{
	int ret;
	struct sockaddr_fixup* fixed;

	if (socklen > 512)
		return -EINVAL;

	fixed = __builtin_alloca(sockaddr_fixup_size_from_bsd(name, socklen));
	ret = socklen = sockaddr_fixup_from_bsd(fixed, name, socklen);
	if (ret < 0)
		return ret;

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_BIND, ((long[6]) { fd, fixed, socklen }));
#else
	ret = LINUX_SYSCALL(__NR_bind, fd, fixed, socklen);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

