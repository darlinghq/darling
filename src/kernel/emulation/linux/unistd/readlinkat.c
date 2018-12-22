#include "readlink.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../common_at.h"
#include <lkm/api.h>
#include <mach/lkm.h>

extern __SIZE_TYPE__ strlen(const char* str);
extern char* strcpy(char* dst, const char* src);

long sys_readlinkat(int fd, const char* path, char* buf, int count)
{
	int ret;
	struct vchroot_expand_args vc;

	vc.flags = 0;
	vc.dfd = atfd(fd);
	
	strcpy(vc.path, path);

	ret = lkm_call(NR_vchroot_expand, &vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ret = LINUX_SYSCALL(__NR_readlinkat, vc.dfd, vc.path, buf, count);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
