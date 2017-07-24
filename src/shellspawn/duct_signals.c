#include <sys/signal.h>
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


