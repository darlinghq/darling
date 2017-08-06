#include "sigexc.h"
#include "../base.h"
#include <stddef.h>
#include <linux-syscalls/linux.h>

// Support for Darwin debugging.
// Unlike other Unix-like systems, macOS doesn't use wait() to handle events in the debugged process.

static bool am_i_ptraced = false;
static void handle_rt_signal(int signum);
extern void sig_restorer(void);
void darling_sigexc_uninstall(void);
void sigrt_handler(int signum, struct linux_siginfo* info, void* ctxt);

void sigexc_setup(void)
{
	// Setup handler for SIGNAL_SIGEXC_TOGGLE and SIGNAL_SIGEXC_THUPDATE
	handle_rt_signal(SIGNAL_SIGEXC_TOGGLE);
	handle_rt_signal(SIGNAL_SIGEXC_THUPDATE);
}

static void handle_rt_signal(int signum)
{
  	struct linux_sigaction sa;
	sa.sa_sigaction = sigrt_handler;
	sa.sa_mask = 0;
	sa.sa_flags = LINUX_SA_RESTORER | LINUX_SA_SIGINFO | LINUX_SA_RESTART;
	sa.sa_restorer = sig_restorer;

	LINUX_SYSCALL(__NR_rt_sigaction, signum,
			&sa, NULL,
			sizeof(sa.sa_mask));
}

bool darling_am_i_ptraced(void)
{
	return am_i_ptraced;
}

void sigrt_handler(int signum, struct linux_siginfo* info, void* ctxt)
{
	if (signum == SIGNAL_SIGEXC_TOGGLE)
	{
		if (info->si_value == SIGRT_MAGIC_ENABLE_SIGEXC)
		{
			am_i_ptraced = true;
			darling_sigexc_self();
		}
		else if (info->si_value == SIGRT_MAGIC_DISABLE_SIGEXC)
		{
			am_i_ptraced = false;
			darling_sigexc_uninstall();
		}
	}
	else if (signum == SIGNAL_SIGEXC_THUPDATE)
	{
		// TODO: Change how a pending signal is handled
		// Use TLS?
		if (!am_i_ptraced)
			return;
	}
}

void darling_sigexc_self(void)
{
	// Make sigexc_handler the handler for all signals in the process
	for (int i = 1; i <= 31; i++)
	{
		struct linux_sigaction sa;
		sa.sa_sigaction = sigrt_handler;
		sa.sa_mask = sig_masks[i];
		sa.sa_flags = LINUX_SA_RESTORER | LINUX_SA_SIGINFO | LINUX_SA_RESTART;
		sa.sa_restorer = sig_restorer;

		LINUX_SYSCALL(__NR_rt_sigaction, i,
				&sa, NULL,
				sizeof(sa.sa_mask));
	}
}
void darling_sigexc_uninstall(void)
{
	for (int i = 1; i <= 31; i++)
	{
		struct linux_sigaction sa;

		if (sig_handlers[i] == SIG_DFL || sig_handlers[i] != SIG_IGN
				|| sig_handlers[i] != SIG_ERR)
		{
			sa.sa_sigaction = (linux_sig_handler*) sig_handlers[i];
		}
		else
			sa.sa_sigaction = &handler_linux_to_bsd;

		sa.sa_mask = sig_masks[i];
		sa.sa_flags = sig_flags[i];
		sa.sa_restorer = sig_restorer;

		LINUX_SYSCALL(__NR_rt_sigaction, i,
				&sa, NULL,
				sizeof(sa.sa_mask));
	}
}

void sigexc_handler(int linux_signum, struct linux_siginfo* info, void* ctxt)
{
	// SIGSEGV + SIGBUS -> EXC_BAD_ACCESS
	// SIGTRAP -> EXC_BREAKPOINT
	// * -> EXC_SOFTWARE with EXC_SOFT_SIGNAL (e.g. SIGSTOP
	if (darling_am_i_ptraced())
	{
	}
}

