#include "close.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../bsdthread/cancelable.h"
#include "../mach/lkm.h"
#include <lkm/api.h>

long sys_close(int fd)
{
	CANCELATION_POINT();
	return sys_close_nocancel(fd);
}

long sys_close_nocancel(int fd)
{
	int ret;

	struct closing_descriptor_args args = {
		.fd = fd,
	};
	lkm_call(NR_closing_descriptor, &args);

	ret = LINUX_SYSCALL1(__NR_close, fd);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

long close_internal(int fd)
{
	int ret;

	ret = LINUX_SYSCALL1(__NR_close, fd);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}
