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

#define DARWIN_SIGHUP  1
#define DARWIN_SIGINT  2
#define DARWIN_SIGQUIT 3
#define DARWIN_SIGILL  4
#define DARWIN_SIGTRAP 5
#define DARWIN_SIGABRT 6
#define DARWIN_SIGPOLL 7
#define DARWIN_SIGFPE  8
#define DARWIN_SIGKILL 9
#define DARWIN_SIGBUS  10
#define DARWIN_SIGSEGV 11
#define DARWIN_SIGSYS  12
#define DARWIN_SIGPIPE 13
#define DARWIN_SIGALRM 14
#define DARWIN_SIGTERM 15
#define DARWIN_SIGURG  16
#define DARWIN_SIGSTOP 17
#define DARWIN_SIGTSTP 18
#define DARWIN_SIGCONT 19
#define DARWIN_SIGCHLD 20
#define DARWIN_SIGTTIN 21
#define DARWIN_SIGTTOU 22
#define DARWIN_SIGIO   23
#define DARWIN_SIGXCPU 24
#define DARWIN_SIGXFSZ 25
#define DARWIN_SIGVTALRM 26
#define DARWIN_SIGPROF 27
#define DARWIN_SIGWINCH 28
#define DARWIN_SIGINFO 29
#define DARWIN_SIGUSR1 30
#define DARWIN_SIGUSR2 31

typedef int __darwin_sigset_t;
struct __darwin_sigaction
{
	union
	{
		void    (*xsa_handler)(int); // sa_handler is often broken due to macro pollution
		void    (*xsa_sigaction)(int, siginfo_t *, void*);
	};
	__darwin_sigset_t sa_mask;
	int     sa_flags;
};

extern "C"
{

sighandler_t __darwin_signal(int signum, sighandler_t handler);
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

namespace Darling
{
	int signalLinuxToDarwin(int sig);
	int signalDarwinToLinux(int sig);
}

#endif

