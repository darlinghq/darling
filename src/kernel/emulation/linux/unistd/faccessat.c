#include "faccessat.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../common_at.h"
#include "../vchroot_expand.h"
#include <lkm/api.h>
#include <mach/lkm.h>

extern char* strcpy(char* dst, const char* src);

long sys_faccessat(int fd, const char* filename, int amode, int flag)
{
	int ret;

	struct vchroot_expand_args vc;

	vc.flags = 0;
	vc.dfd = atfd(fd);

	strcpy(vc.path, filename);

	ret = vchroot_expand(&vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = LINUX_SYSCALL(__NR_faccessat, vc.dfd, vc.path, amode, atflags_bsd_to_linux(flag));

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
