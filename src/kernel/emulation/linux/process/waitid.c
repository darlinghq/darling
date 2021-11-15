#include "waitid.h"
#include "../base.h"
#include "../errno.h"
#include "../signal/duct_signals.h"
#include "wait4.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>

static void siginfo_linux_to_bsd(const linux_siginfo_t* linux_si, siginfo_t* si);

long sys_waitid(int idtype, int id, siginfo_t* si, int options)
{
	int ret, linux_options;

	linux_options = waitopts_bsd_to_linux(options);

	linux_siginfo_t linux_si;
	ret = LINUX_SYSCALL(__NR_waitid, idtype, id, &linux_si, linux_options);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	if (si != NULL)
	{
		siginfo_linux_to_bsd(&linux_si, si);
	}

	return ret;
}

static void siginfo_linux_to_bsd(const linux_siginfo_t* linux_si, siginfo_t* si)
{
	si->si_signo = signum_linux_to_bsd(linux_si->si_signo);
	si->si_errno = errno_linux_to_bsd(linux_si->si_errno);
	si->si_code = linux_si->si_code;
	si->si_pid = linux_si->linux_si_pid;
	si->si_uid = linux_si->linux_si_uid;
	si->si_status = linux_si->linux_si_status;
	si->si_addr = linux_si->linux_si_addr;
	si->si_value = linux_si->linux_si_value;
	si->si_band = linux_si->linux_si_band;
}

