#include "chown.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../common_at.h"
#include <lkm/api.h>
#include <mach/lkm.h>

extern char* strcpy(char* dst, const char* src);

long sys_fchownat(int fd, const char* path, int uid, int gid, int flag)
{
	int ret;
	struct vchroot_expand_args vc;

	vc.flags = VCHROOT_FOLLOW;
	vc.dfd = atfd(fd);
	
	strcpy(vc.path, path);

	ret = lkm_call(NR_vchroot_expand, &vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = LINUX_SYSCALL(__NR_fchownat, vc.dfd, vc.path, uid, gid, atflags_bsd_to_linux(flag));

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
