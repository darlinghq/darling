#include "link.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"
#include "../common_at.h"
#include <lkm/api.h>
#include <mach/lkm.h>

extern char* strcpy(char* dst, const char* src);

long sys_linkat(int fd, const char* path, int fdlink, const char* link, int flag)
{
	int ret;
	struct vchroot_expand_args vc, vc2;

	vc.flags = 0;
	vc.dfd = atfd(fd);
	
	strcpy(vc.path, path);

	vc2.flags = 0;
	vc2.dfd = atfd(fdlink);

	strcpy(vc2.path, link);

	ret = lkm_call(NR_vchroot_expand, &vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = lkm_call(NR_vchroot_expand, &vc2);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = LINUX_SYSCALL(__NR_linkat, vc.dfd, vc.path, vc2.dfd, vc2.path, atflags_bsd_to_linux(flag));
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
