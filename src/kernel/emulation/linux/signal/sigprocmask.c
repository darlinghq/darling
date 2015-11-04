#include "sigprocmask.h"
#include "../base.h"
#include "../errno.h"
#include "duct_signals.h"
#include <stddef.h>
#include <asm/unistd.h>

static inline int spchow_bsd_to_linux(int how) { return how-1; }

long sys_sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
	int ret, linux_how;
	linux_sigset_t in, out;

	linux_how = spchow_bsd_to_linux(how);

	if (set != NULL)
		sigset_bsd_to_linux(set, &in);

	ret = LINUX_SYSCALL(__NR_rt_sigprocmask, linux_how,
			(set != NULL) ? &in : NULL, &out);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else if (oldset != NULL)
		sigset_linux_to_bsd(&out, oldset);

	return ret;
}

