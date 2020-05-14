#include "connect.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include "duct.h"
#include "../bsdthread/cancelable.h"

extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);

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

	fixed = __builtin_alloca(socklen+100);
	memcpy(fixed, name, socklen);

	fixed->linux_family = sfamily_bsd_to_linux(fixed->bsd_family);

	if (fixed->linux_family == LINUX_PF_LOCAL)
	{
		struct vchroot_expand_args vc;
		vc.flags = VCHROOT_FOLLOW;
		vc.dfd = get_perthread_wd();

		strcpy(vc.path, fixed->sun_path);

		ret = vchroot_expand(&vc);
		if (ret < 0)
			return errno_linux_to_bsd(ret);

		strncpy(fixed->sun_path, vc.path, sizeof(fixed->sun_path) - 1);
		fixed->sun_path[sizeof(fixed->sun_path) - 1] = '\0';
	}

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_CONNECT, ((long[6]) { fd, fixed, socklen }));
#else
	ret = LINUX_SYSCALL(__NR_connect, fd, fixed, socklen);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

