#include "mkdirat.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../common_at.h"
#include <lkm/api.h>
#include <mach/lkm.h>

extern char* strcpy(char* dst, const char* src);

long sys_mkdirat(int fd, const char* path, unsigned int mode)
{
	int ret;
	struct vchroot_expand_args vc;

	vc.flags = 0;
	vc.dfd = atfd(fd);
	
	strcpy(vc.path, path);

	ret = lkm_call(NR_vchroot_expand, &vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = LINUX_SYSCALL(__NR_mkdirat, vc.dfd, vc.path, mode);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}
