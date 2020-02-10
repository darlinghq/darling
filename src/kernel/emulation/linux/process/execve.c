#include "execve.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../fcntl/open.h"
#include "../unistd/read.h"
#include "../unistd/close.h"
#include "../unistd/readlink.h"
#include "../signal/sigexc.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>
#include <lkm/api.h>
#include <mach/lkm.h>
#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"

long sys_execve(char* fname, char** argvp, char** envp)
{
	int ret;
	struct vchroot_expand_args vc;

	vc.flags = VCHROOT_FOLLOW;
	vc.dfd = get_perthread_wd();
	
	strcpy(vc.path, fname);

	ret = vchroot_expand(&vc);
	__simple_kprintf("execve expand %s -> %s, ret %d", fname, vc.path, ret);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	linux_sigset_t set;
	set = (1ull << (SIGNAL_SIGEXC_TOGGLE-1));
	set |= (1ull << (SIGNAL_SIGEXC_THUPDATE-1));

	LINUX_SYSCALL(__NR_rt_sigprocmask, 0 /* LINUX_SIG_BLOCK */,
			&set, NULL, sizeof(linux_sigset_t));

	ret = LINUX_SYSCALL(__NR_execve, vc.path, argvp, envp);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	LINUX_SYSCALL(__NR_rt_sigprocmask, 1 /* LINUX_SIG_UNBLOCK */,
			&set, NULL, sizeof(linux_sigset_t));

	return ret;
}
