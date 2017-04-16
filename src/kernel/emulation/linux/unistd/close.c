#include "close.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>

__attribute__((weak))
__attribute__((visibility("default")))
int kqueue_close(int kq) { return -1; }

long sys_close(int fd)
{
	return sys_close_nocancel(fd);
}

long sys_close_nocancel(int fd)
{
	int ret;

	ret = LINUX_SYSCALL1(__NR_close, fd);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else
		kqueue_close(fd);

	return ret;
}

// Special variant for libkqueue to avoid recursion into kqueue_close()
__attribute__((visibility("default")))
long __close_for_kqueue(int fd)
{
	int ret;

	ret = LINUX_SYSCALL1(__NR_close, fd);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

