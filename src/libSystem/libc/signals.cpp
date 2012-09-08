#include "config.h"
#include "signals.h"
#include "errno.h"
#include "common/auto.h"
#include <signal.h>
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

static sigset_t sigsetDarwinToLinux(const __darwin_sigset_t* set)
{
	sigset_t rv;

	sigemptyset(&rv);

	for (int i = 0; i < 19; i++)
	{
		if (*set & (1 << i))
			sigaddset(&rv, i+1);
	}

	return rv;
}

static __darwin_sigset_t sigsetLinuxToDarwin(const sigset_t* set)
{
	__darwin_sigset_t rv = 0;

	for (int i = 0; i < 19; i++)
	{
		if (sigismember(set, i+1) > 0)
			rv |= (1 << (i+1));
	}

	return rv;
}

// TODO: check siginfo compatibility
int __darwin_sigaction(int signum, const struct __darwin_sigaction* act, struct __darwin_sigaction* oldact)
{
	std::unique_ptr<struct sigaction> nact;
	std::unique_ptr<struct sigaction> noldact;

	if (oldact)
		noldact.reset(new struct sigaction);

	if (act)
	{
		nact.reset(new struct sigaction);
		nact->sa_flags = Darling::flagsDarwinToNative(g_sigactionFlags, sizeof(g_sigactionFlags)/sizeof(g_sigactionFlags[0]), act->sa_flags);
		nact->sa_handler = act->xsa_handler;
		nact->sa_sigaction = act->xsa_sigaction;
		nact->sa_mask = sigsetDarwinToLinux(&act->sa_mask);
	}

	int rv = sigaction(signum, nact.get(), noldact.get());

	if (rv != -1 && noldact)
	{
		oldact->sa_flags = Darling::flagsNativeToDarwin(g_sigactionFlags, sizeof(g_sigactionFlags)/sizeof(g_sigactionFlags[0]), noldact->sa_flags);

		if (noldact->sa_flags & SA_SIGINFO)
			oldact->xsa_sigaction = noldact->sa_sigaction;
		else
			oldact->xsa_handler = noldact->sa_handler;
		oldact->sa_mask = sigsetLinuxToDarwin(&noldact->sa_mask);
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
	*set = 0x7ffff; // 19 signals on Linux
	return 0;
}

int __darwin_sigaddset(__darwin_sigset_t *set, int signum)
{
	if (signum > 19)
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
	if (signum > 19)
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
	if (signum > 19)
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
		*nset = sigsetDarwinToLinux(set);
	}
	if (oldset)
		noldset.reset(new sigset_t);

	// -1 -> conversion from Darwin to Linux
	int rv = sigprocmask(how - 1, nset.get(), noldset.get());

	if (rv == -1)
		errnoOut();
	else if (oldset)
		*oldset = sigsetLinuxToDarwin(noldset.get());
	
	return rv;
}

int __darwin_sigsuspend(const __darwin_sigset_t *mask)
{
	sigset_t set = sigsetDarwinToLinux(mask);
	int rv = sigsuspend(&set);
	if (rv == -1)
		errnoOut();
	return rv;
}

int __darwin_sigwait(const __darwin_sigset_t *set, int *sig)
{
	sigset_t nset = sigsetDarwinToLinux(set);
	int rv = sigwait(&nset, sig);
	if (rv == -1)
		errnoOut();
	return rv;
}

int __darwin_sigpending(__darwin_sigset_t *set)
{
	sigset_t nset;
	int rv = sigpending(&nset);

	if (rv == -1)
		errnoOut();
	else
		*set = sigsetLinuxToDarwin(&nset);
	return rv;
}


