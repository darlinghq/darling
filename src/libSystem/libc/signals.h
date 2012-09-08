#ifndef SIGNALS_H
#define SIGNALS_H
#include <signal.h>

#define DARWIN_SA_ONSTACK      0x0001
#define DARWIN_SA_RESTART      0x0002
#define DARWIN_SA_RESETHAND    0x0004
#define DARWIN_SA_NOCLDSTOP    0x0008
#define DARWIN_SA_NODEFER      0x0010
#define DARWIN_SA_NOCLDWAIT    0x0020
#define DARWIN_SA_SIGINFO      0x0040

typedef int __darwin_sigset_t;
struct __darwin_sigaction
{
	union
	{
		void    (*xsa_handler)(int); // sa_handler is often broken due to macro pollution
		void    (*xsa_sigaction)(int, struct siginfo *, void*);
	};
	__darwin_sigset_t sa_mask;
	int     sa_flags;
};

extern "C"
{

int __darwin_sigaction(int signum, const struct __darwin_sigaction* act, struct __darwin_sigaction* oldact);
int __darwin_sigprocmask(int how, const __darwin_sigset_t *set, __darwin_sigset_t *oldset);

int __darwin_sigemptyset(__darwin_sigset_t *set);
int __darwin_sigfillset(__darwin_sigset_t *set);
int __darwin_sigaddset(__darwin_sigset_t *set, int signum);
int __darwin_sigdelset(__darwin_sigset_t *set, int signum);
int __darwin_sigismember(__darwin_sigset_t *set, int signum);

int __darwin_sigsuspend(const __darwin_sigset_t *mask);
int __darwin_sigwait(const __darwin_sigset_t *set, int *sig);
int __darwin_sigpending(__darwin_sigset_t *set);

}

#endif

