#include "fchmodat.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../common_at.h"
#include <mach/lkm.h>
#include <lkm/api.h>

extern char* strcpy(char* dst, const char* src);

long sys_fchmodat(int fd, const char* path, int mode, int flag)
{
	int ret;
	struct vchroot_expand_args vc;

	vc.flags = 0;
	vc.dfd = atfd(fd);
	
	strcpy(vc.path, path);

	ret = lkm_call(NR_vchroot_expand, &vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = LINUX_SYSCALL(__NR_fchmodat, vc.dfd, vc.path, mode, atflags_bsd_to_linux(flag));
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
