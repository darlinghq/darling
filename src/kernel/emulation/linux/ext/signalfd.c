#include "./sys/eventfd.h"
#include "../errno.h"
#include "../base.h"
#include "../signal/duct_signals.h"
#include <asm/unistd.h>

extern long cerror(int __err);

VISIBLE
int signalfd (int __fd, const sigset_t *__mask, int __flags)
{
	int rv;
	linux_sigset_t linux_mask;

	sigset_bsd_to_linux(__mask, &linux_mask);

	rv = LINUX_SYSCALL(__NR_signalfd, linux_mask, __flags);
	if (rv < 0)
	{
		cerror(errno_linux_to_bsd(rv));
		return -1;
	}

	return rv;
}

