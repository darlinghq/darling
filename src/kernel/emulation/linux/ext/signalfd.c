#include "../fcntl/fcntl.h"
#include "./sys/signalfd.h"
#include "../errno.h"
#include "../base.h"
#include "../signal/duct_signals.h"
#include <linux-syscalls/linux.h>

extern long cerror(int __err);

VISIBLE
int signalfd (int __fd, const sigset_t *__mask, int __flags)
{
	int rv, linux_flags = 0;
	linux_sigset_t linux_mask;

	sigset_bsd_to_linux(__mask, &linux_mask);

	if (__flags & O_CLOEXEC)
		linux_flags |= 02000000;
	if (__flags & O_NONBLOCK)
		linux_flags |= 04000;

#ifdef __NR_signalfd4
	rv = LINUX_SYSCALL(__NR_signalfd4, __fd, &linux_mask, 8, linux_flags);
#else
	rv = LINUX_SYSCALL(__NR_signalfd, __fd, &linux_mask, 8);
	// handle flags
	if (rv >= 0)
	{
		if (__flags & O_CLOEXEC)
			sys_fcntl(rv, F_SETFD, BSD_FD_CLOEXEC);
		if (__flags & O_NONBLOCK)
			sys_fcntl(rv, F_SETFL, BSD_O_NONBLOCK);
	}
#endif
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

