#include "sigaltstack.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>

long sys_sigaltstack(const struct bsd_stack* ss, struct bsd_stack* oss)
{
	int ret;
	struct linux_stack lss, loss;

	if (ss != NULL)
	{
		lss.ss_sp = ss->ss_sp;
		lss.ss_flags = ss->ss_flags;
		lss.ss_size = ss->ss_size;
	}

	ret = LINUX_SYSCALL(__NR_sigaltstack, (ss != NULL) ? &lss : NULL, &loss);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	if (oss != NULL)
	{
		oss->ss_sp = loss.ss_sp;
		oss->ss_flags = loss.ss_flags;
		oss->ss_size = loss.ss_size;
	}
	return 0;
}

