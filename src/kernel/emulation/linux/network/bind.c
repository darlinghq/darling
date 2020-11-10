#include "bind.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include "duct.h"

extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);

#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"

long sys_bind(int fd, const void* name, int socklen)
{
	int ret;
	struct sockaddr_fixup* fixed;

	if (socklen > 512)
		return -EINVAL;

	// if we're going to be fixing up the path for vchroot,
	// we need to make sure we allocate the full structure so we have max path length
	fixed = __builtin_alloca((((struct sockaddr_fixup*)name)->bsd_family == PF_LOCAL) ? sizeof(struct sockaddr_fixup) : socklen);
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
		socklen = sizeof(*fixed) - sizeof(fixed->sun_path) + strlen(fixed->sun_path);
	}

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_BIND, ((long[6]) { fd, fixed, socklen }));
#else
	ret = LINUX_SYSCALL(__NR_bind, fd, fixed, socklen);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

