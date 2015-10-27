#define __sigset_t_defined
#include "../../../../../platform-include/sys/signal.h"
#include "duct_signals.h"

int signum_linux_to_bsd(int signum)
{
	switch (signum)
	{
		case LINUX_SIGHUP:
			return SIGHUP;
		case LINUX_SIGINT:
			return SIGINT;
		case LINUX_SIGQUIT:
			return SIGQUIT;
		case LINUX_SIGILL:
			return SIGILL;
		case LINUX_SIGTRAP:
			return SIGTRAP;
		case LINUX_SIGABRT:
			return SIGABRT;
		case LINUX_SIGFPE:
			return SIGFPE;
		case LINUX_SIGKILL:
			return SIGKILL;
		case LINUX_SIGBUS:
			return SIGBUS;
		case LINUX_SIGSEGV:
			return SIGSEGV;
		case LINUX_SIGSYS:
			return SIGSYS;
		case LINUX_SIGPIPE:
			return SIGPIPE;
		case LINUX_SIGALRM:
			return SIGALRM;
		case LINUX_SIGTERM:
			return SIGTERM;
		case LINUX_SIGURG:
			return SIGURG;
		case LINUX_SIGSTOP:
			return SIGSTOP;
		case LINUX_SIGTSTP:
			return SIGTSTP;
		case LINUX_SIGCONT:
			return SIGCONT;
		case LINUX_SIGCHLD:
			return SIGCHLD;
		case LINUX_SIGTTIN:
			return SIGTTIN;
		case LINUX_SIGTTOU:
			return SIGTTOU;
		case LINUX_SIGIO:
			return SIGIO;
		case LINUX_SIGXCPU:
			return SIGXCPU;
		case LINUX_SIGXFSZ:
			return SIGXFSZ;
		case LINUX_SIGVTALRM:
			return SIGVTALRM;
		case LINUX_SIGPROF:
			return SIGPROF;
		case LINUX_SIGWINCH:
			return SIGWINCH;
		case LINUX_SIGUSR1:
			return SIGUSR1;
		case LINUX_SIGUSR2:
			return SIGUSR2;
		default:
			return 0;
	}
}

int signum_bsd_to_linux(int signum)
{
	switch (signum)
	{
		case SIGHUP:
			return LINUX_SIGHUP;
		case SIGINT:
			return LINUX_SIGINT;
		case SIGQUIT:
			return LINUX_SIGQUIT;
		case SIGILL:
			return LINUX_SIGILL;
		case SIGTRAP:
			return LINUX_SIGTRAP;
		case SIGABRT:
			return LINUX_SIGABRT;
		case SIGFPE:
			return LINUX_SIGFPE;
		case SIGKILL:
			return LINUX_SIGKILL;
		case SIGBUS:
			return LINUX_SIGBUS;
		case SIGSEGV:
			return LINUX_SIGSEGV;
		case SIGSYS:
			return LINUX_SIGSYS;
		case SIGPIPE:
			return LINUX_SIGPIPE;
		case SIGALRM:
			return LINUX_SIGALRM;
		case SIGTERM:
			return LINUX_SIGTERM;
		case SIGURG:
			return LINUX_SIGURG;
		case SIGSTOP:
			return LINUX_SIGSTOP;
		case SIGTSTP:
			return LINUX_SIGTSTP;
		case SIGCONT:
			return LINUX_SIGCONT;
		case SIGCHLD:
			return LINUX_SIGCHLD;
		case SIGTTIN:
			return LINUX_SIGTTIN;
		case SIGTTOU:
			return LINUX_SIGTTOU;
		case SIGIO:
			return LINUX_SIGIO;
		case SIGXCPU:
			return LINUX_SIGXCPU;
		case SIGXFSZ:
			return LINUX_SIGXFSZ;
		case SIGVTALRM:
			return LINUX_SIGVTALRM;
		case SIGPROF:
			return LINUX_SIGPROF;
		case SIGWINCH:
			return LINUX_SIGWINCH;
		case SIGUSR1:
			return LINUX_SIGUSR1;
		case SIGUSR2:
			return LINUX_SIGUSR2;
		default:
			return 0;
	}
}

void sigset_linux_to_bsd(const linux_sigset_t* linux, sigset_t* bsd)
{
	int i;

	*bsd = 0;

	for (i = 0; i < 64; i++)
	{
		if (*linux & (1ull << i))
			*bsd |= signum_linux_to_bsd(i+1);
	}
}

void sigset_bsd_to_linux(const sigset_t* bsd, linux_sigset_t* linux)
{
	int i;

	*linux = 0;
	for (i = 0; i < 64; i++)
	{
		if (*bsd & (1ull << i))
			*linux |= signum_bsd_to_linux(i+1);
	}
}

