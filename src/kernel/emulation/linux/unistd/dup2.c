#include "dup2.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../duct_errno.h"
#include "../mach/lkm.h"
#include <lkm/api.h>

long sys_dup2(int fd_from, int fd_to)
{
	int ret;

	if (fd_from != fd_to) {
		struct closing_descriptor_args args = {
			.fd = fd_to,
		};
		lkm_call(NR_closing_descriptor, &args);
	}

	#if defined(__NR_dup2)
		ret = LINUX_SYSCALL2(__NR_dup2, fd_from, fd_to);
	#else		
		// It's not clear if 0 is offically a valid flag argument.
		// But we don't really have a choice.
		ret = LINUX_SYSCALL(__NR_dup3, fd_from, fd_to, 0);

		if (ret == LINUX_EINVAL && fd_from == fd_to)
			ret = fd_to;
	#endif
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

