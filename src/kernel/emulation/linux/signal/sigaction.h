#ifndef LINUX_SIGACTION_H
#define LINUX_SIGACTION_H
#include "duct_signals.h"

#define BSD_SA_ONSTACK      0x0001
#define BSD_SA_RESTART      0x0002
#define BSD_SA_RESETHAND    0x0004
#define BSD_SA_NOCLDSTOP    0x0008
#define BSD_SA_NODEFER      0x0010
#define BSD_SA_NOCLDWAIT    0x0020
#define BSD_SA_SIGINFO      0x0040

#define LINUX_SA_NOCLDSTOP    0x00000001u
#define LINUX_SA_NOCLDWAIT    0x00000002u
#define LINUX_SA_SIGINFO      0x00000004u
#define LINUX_SA_ONSTACK      0x08000000u
#define LINUX_SA_RESTART      0x10000000u
#define LINUX_SA_NODEFER      0x40000000u
#define LINUX_SA_RESETHAND    0x80000000u

struct bsd_siginfo
{
	int si_signo;
	int si_errno;
	int si_code;
	unsigned int si_pid;
	unsigned int si_uid;
	int si_status;
	void* si_addr;
	void* si_val_ptr;
	long si_band;
	unsigned long __pad[7];
};

# define __SI_MAX_SIZE     128
# if defined (__x86_64__)
#  define __SI_PAD_SIZE     ((__SI_MAX_SIZE / sizeof (int)) - 4)
# else
#  define __SI_PAD_SIZE     ((__SI_MAX_SIZE / sizeof (int)) - 3)
# endif

struct linux_siginfo
{
	int si_signo;
	int si_errno;
	int si_code;

	union
	{
		int _pad[__SI_PAD_SIZE];
	};
};

typedef void (bsd_sig_handler)(int, struct bsd_siginfo*, void*);
struct bsd_sigaction
{
	bsd_sig_handler* sa_sigaction;
	unsigned int sa_mask;
	int sa_flags;
};

struct bsd___sigaction
{
	bsd_sig_handler* sa_sigaction;
	void (*sa_tramp)(void*, int, int, struct bsd_siginfo*, void*);
	unsigned int sa_mask;
	int sa_flags;
};

struct linux_sigaction
{
	void (*sa_sigaction)(int, struct linux_siginfo*, void*);
	int sa_flags;
	void (*sa_restorer)(void);
	linux_sigset_t sa_mask;
};

long sys_sigaction(int signum, const struct bsd___sigaction* nsa, struct bsd_sigaction* osa);

#endif

