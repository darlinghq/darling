#include "config.h"
#include "signals.h"
#include "errno.h"
#include "common/auto.h"
#include "Bidimap.h"
#include <signal.h>
#include <iostream>
#include <memory>
#include "darwin_errno_codes.h"

static Darling::MappedFlag g_sigactionFlags[] = {
	{ DARWIN_SA_ONSTACK, SA_ONSTACK },
	{ DARWIN_SA_RESTART, SA_RESTART },
	{ DARWIN_SA_RESETHAND, int(SA_RESETHAND) },
	{ DARWIN_SA_NOCLDSTOP, SA_NOCLDSTOP },
	{ DARWIN_SA_NODEFER, SA_NODEFER },
	{ DARWIN_SA_NOCLDWAIT, SA_NOCLDWAIT },
	{ DARWIN_SA_SIGINFO, SA_SIGINFO }
};

typedef void(*HandlerType)(int,siginfo_t*,void*);
static int g_sigLinuxToDarwin[33], g_sigDarwinToLinux[33];
static const int SIGNAL_MAX = 31;
static HandlerType g_darwinHandlers[32];

__attribute__((constructor))
static void initializeSignalMaps()
{
	Darling::Bidimap<int,33> map(g_sigLinuxToDarwin, g_sigDarwinToLinux);
	map.put(SIGHUP, DARWIN_SIGHUP);
	map.put(SIGINT, DARWIN_SIGINT);
	map.put(SIGQUIT, DARWIN_SIGQUIT);
	map.put(SIGILL, DARWIN_SIGILL);
	map.put(SIGTRAP, DARWIN_SIGTRAP);
	map.put(SIGABRT, DARWIN_SIGABRT);
	map.put(SIGPOLL, DARWIN_SIGPOLL);
	map.put(SIGFPE, DARWIN_SIGFPE);
	map.put(SIGKILL, DARWIN_SIGKILL);
	map.put(SIGBUS, DARWIN_SIGBUS);
	map.put(SIGSEGV, DARWIN_SIGSEGV);
	map.put(SIGSYS, DARWIN_SIGSYS);
	map.put(SIGPIPE, DARWIN_SIGPIPE);
	map.put(SIGALRM, DARWIN_SIGALRM);
	map.put(SIGTERM, DARWIN_SIGTERM);
	map.put(SIGURG, DARWIN_SIGURG);
	map.put(SIGSTOP, DARWIN_SIGSTOP);
	map.put(SIGTSTP, DARWIN_SIGTSTP);
	map.put(SIGCONT, DARWIN_SIGCONT);
	map.put(SIGCHLD, DARWIN_SIGCHLD);
	map.put(SIGTTIN, DARWIN_SIGTTIN);
	map.put(SIGTTOU, DARWIN_SIGTTOU);
	map.put(SIGIO, DARWIN_SIGIO);
	map.put(SIGXCPU, DARWIN_SIGXCPU);
	map.put(SIGXFSZ, DARWIN_SIGXFSZ);
	map.put(SIGVTALRM, DARWIN_SIGVTALRM);
	map.put(SIGPROF, DARWIN_SIGPROF);
	map.put(SIGWINCH, DARWIN_SIGWINCH);
	//map.put(SIGINFO, DARWIN_SIGINFO); // N/A on Linux
	map.put(SIGUSR1, DARWIN_SIGUSR1);
	map.put(SIGUSR2, DARWIN_SIGUSR2);

	std::fill(g_darwinHandlers, g_darwinHandlers+32, reinterpret_cast<HandlerType>(SIG_DFL));
}

int Darling::signalLinuxToDarwin(int sig)
{
	return g_sigLinuxToDarwin[sig];
}

int Darling::signalDarwinToLinux(int sig)
{
	return g_sigDarwinToLinux[sig];
}

static void GenericHandler(int sig, siginfo_t* p, void* p2)
{
	int dsig = g_sigLinuxToDarwin[sig];
	g_darwinHandlers[sig](dsig, p, p2);
}

sighandler_t __darwin_signal(int signum, sighandler_t handler)
{
	signum = g_sigDarwinToLinux[signum];
	g_darwinHandlers[signum] = reinterpret_cast<HandlerType>(handler);

	if (handler != SIG_DFL && handler != SIG_IGN && handler)
		handler = reinterpret_cast<sighandler_t>(GenericHandler);

	return signal(signum, handler);
}

sigset_t Darling::sigsetDarwinToLinux(const __darwin_sigset_t* set)
{
	sigset_t rv;

	sigemptyset(&rv);

	for (int i = 0; i <= SIGNAL_MAX; i++)
	{
		if (*set & (1 << i))
			sigaddset(&rv, g_sigDarwinToLinux[i+1]);
	}

	return rv;
}

__darwin_sigset_t Darling::sigsetLinuxToDarwin(const sigset_t* set)
{
	__darwin_sigset_t rv = 0;

	for (int i = 0; i <= SIGNAL_MAX; i++)
	{
		if (sigismember(set, i+1) > 0)
			rv |= (1 << g_sigLinuxToDarwin[i+1]);
	}

	return rv;
}

// TODO: check siginfo compatibility
// TODO: map signal numbers for 
int __darwin_sigaction(int signum, const struct __darwin_sigaction* act, struct __darwin_sigaction* oldact)
{
	std::unique_ptr<struct sigaction> nact;
	std::unique_ptr<struct sigaction> noldact;
	HandlerType oldhdl = 0;

	signum = g_sigDarwinToLinux[signum];

	if (oldact)
		noldact.reset(new struct sigaction);

	if (act)
	{
		nact.reset(new struct sigaction);
		nact->sa_flags = Darling::flagsDarwinToNative(g_sigactionFlags, sizeof(g_sigactionFlags)/sizeof(g_sigactionFlags[0]), act->sa_flags);
		nact->sa_handler = act->xsa_handler;
		nact->sa_sigaction = act->xsa_sigaction;
		nact->sa_mask = Darling::sigsetDarwinToLinux(&act->sa_mask);
		oldhdl = g_darwinHandlers[signum];

		// defer a user-supplied function to a wrapper that will translate the signal number
		if (act->xsa_handler != 0 && act->xsa_handler != SIG_IGN && act->xsa_handler != SIG_DFL)
		{
			//std::cout << "Setting GenericHandler for " << signum << std::endl;
			nact->sa_sigaction = GenericHandler;
		}
		g_darwinHandlers[signum] = act->xsa_sigaction;
	}

	int rv = sigaction(signum, nact.get(), noldact.get());

	if (rv != -1 && noldact)
	{
		oldact->sa_flags = Darling::flagsNativeToDarwin(g_sigactionFlags, sizeof(g_sigactionFlags)/sizeof(g_sigactionFlags[0]), noldact->sa_flags);

		if (noldact->sa_flags & SA_SIGINFO)
			oldact->xsa_sigaction = noldact->sa_sigaction;
		else
			oldact->xsa_handler = noldact->sa_handler;
		oldact->xsa_sigaction = oldhdl;
		oldact->sa_mask = Darling::sigsetLinuxToDarwin(&noldact->sa_mask);
	}
	if (rv == -1)
		errnoOut();

	return rv;
}

int __darwin_sigemptyset(__darwin_sigset_t *set)
{
	*set = 0;
	return 0;
}

int __darwin_sigfillset(__darwin_sigset_t *set)
{
	*set = 0xffffffff;
	return 0;
}

int __darwin_sigaddset(__darwin_sigset_t *set, int signum)
{
	if (signum >= SIGNAL_MAX)
	{
		errno = DARWIN_EINVAL;
		return -1;
	}
	else
	{
		*set |= (1 << (signum-1));
		return 0;
	}
}

int __darwin_sigdelset(__darwin_sigset_t *set, int signum)
{
	if (signum >= SIGNAL_MAX)
	{
		errno = DARWIN_EINVAL;
		return -1;
	}
	else
	{
		*set &= ~(1 << (signum-1));
		return 0;
	}
}

int __darwin_sigismember(__darwin_sigset_t *set, int signum)
{
	if (signum >= SIGNAL_MAX)
	{
		errno = DARWIN_EINVAL;
		return -1;
	}
	else
	{
		return (*set & (1 << (signum-1))) ? 1 : 0;
	}
}

int __darwin_sigprocmask(int how, const __darwin_sigset_t *set, __darwin_sigset_t *oldset)
{
	std::unique_ptr<sigset_t> nset;
	std::unique_ptr<sigset_t> noldset;

	if (set)
	{
		nset.reset(new sigset_t);
		*nset = Darling::sigsetDarwinToLinux(set);
	}
	if (oldset)
		noldset.reset(new sigset_t);

	// -1 -> conversion from Darwin to Linux
	int rv = sigprocmask(how - 1, nset.get(), noldset.get());

	if (rv == -1)
		errnoOut();
	else if (oldset)
		*oldset = Darling::sigsetLinuxToDarwin(noldset.get());
	
	return rv;
}

int __darwin_sigsuspend(const __darwin_sigset_t *mask)
{
	sigset_t set = Darling::sigsetDarwinToLinux(mask);
	int rv = sigsuspend(&set);
	if (rv == -1)
		errnoOut();
	return rv;
}

int __darwin_sigwait(const __darwin_sigset_t *set, int *sig)
{
	sigset_t nset = Darling::sigsetDarwinToLinux(set);
	int rv = sigwait(&nset, sig);
	if (rv == -1)
		errnoOut();
	*sig = g_sigLinuxToDarwin[*sig];
	return rv;
}

int __darwin_sigpending(__darwin_sigset_t *set)
{
	sigset_t nset;
	int rv = sigpending(&nset);

	if (rv == -1)
		errnoOut();
	else
		*set = Darling::sigsetLinuxToDarwin(&nset);
	return rv;
}


