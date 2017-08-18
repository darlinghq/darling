#include "wait4.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include "../signal/sigexc.h"
#include "../misc/ptrace.h"
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"
#include <sys/signal.h>
#include <stddef.h>
#include <linux-syscalls/linux.h>

extern int getpid(void);

long sys_wait4(int pid, int* status, int options, void* rusage)
{
	int ret, linux_options;
	int mystatus;

	// We always need status even if the caller doesn't
	if (status == NULL)
		status = &mystatus;

	linux_options = waitopts_bsd_to_linux(options);

restart:
	ret = LINUX_SYSCALL(__NR_wait4, pid, status, linux_options, rusage);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	if ((*status & 0x7f) != 0x7f) // process terminated
	{
		int signum;

		signum = *status & 0x7f;
		*status &= ~0x7f;
		*status |= signum_linux_to_bsd(signum);
	}
	else if (*status == 0xffff) // Linux: __W_CONTINUED
		*status = 0x7f | (0x13 << 8);
	else if ((*status & 0x7f) == 0x7f) // process stopped
	{
		int signal = *status >> 8;
		signal = signum_linux_to_bsd(signal);
		*status = (*status & 0x7f) | (signal << 8);
	}

	return ret;
}

int waitopts_bsd_to_linux(int options)
{
	int out = 0;

	if (options & BSD_WNOHANG)
		out |= LINUX_WNOHANG;
	if (options & BSD_WUNTRACED)
		out |= LINUX_WUNTRACED;
	if (options & BSD_WEXITED)
		out |= LINUX_WEXITED;
	if (options & BSD_WSTOPPED)
		out |= LINUX_WSTOPPED;
	if (options & BSD_WCONTINUED)
		out |= LINUX_WCONTINUED;
	if (options & BSD_WNOWAIT)
		out |= LINUX_WNOWAIT;

	return out;
}

