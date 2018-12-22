#include "mkfifo.h"
#include "common.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../bsdthread/per_thread_wd.h"
#include <lkm/api.h>
#include <mach/lkm.h>

#define LINUX_S_IFIFO 0010000

long sys_mkfifo(const char* path, unsigned int mode)
{
	int ret;
	struct vchroot_expand_args vc;

	vc.flags = 0;
	vc.dfd = get_perthread_wd();
	
	strcpy(vc.path, path);

	ret = lkm_call(NR_vchroot_expand, &vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = LINUX_SYSCALL(__NR_mknod, vc.path, mode | LINUX_S_IFIFO, 0);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	return 0;
}
