#include "getxattr.h"

extern char* strcpy(char* dst, const char* src);

#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"
#include <linux-syscalls/linux.h>

long sys_getxattr(const char* path, const char* name, char* value,
		unsigned long size, unsigned int pos, int options)
{
	int ret;

	if (pos != 0)
		return -ERANGE;

	struct vchroot_expand_args vc;
	vc.flags = (options & XATTR_NOFOLLOW) ? 0 : VCHROOT_FOLLOW;
	vc.dfd = get_perthread_wd();

	strcpy(vc.path, path);
	ret = vchroot_expand(&vc);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = LINUX_SYSCALL(__NR_getxattr, vc.path, name, value, size);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}
