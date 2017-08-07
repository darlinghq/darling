#include "sigexc.h"
#include "../base.h"
#include "../unistd/exit.h"
#include <stddef.h>
#include <sys/signal.h>
#include <linux-syscalls/linux.h>
#include <pthread/tsd_private.h>
#include "signal/mach_exc.h"

// Support for Darwin debugging.
// Unlike other Unix-like systems, macOS doesn't use wait() to handle events in the debugged process.
// wait() only receives termination events.

static volatile bool am_i_ptraced = false;
static void handle_rt_signal(int signum);
extern void sig_restorer(void);
extern int getpid(void);

void darling_sigexc_uninstall(void);
void sigrt_handler(int signum, struct linux_siginfo* info, void* ctxt);

#define SIGEXC_TSD_KEY	102

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
		if (!am_i_ptraced)
			return;

		// Examine info->si_value
		// If 0, drop the signal
		// If >0, process the signal
		// If <0, introduce a new signal
		int sig = info->si_value;
		if (sig < 0)
		{
			// This is only used to pass a SIGSTOP to the traced process (from the debugger)
			// and have it passed back through the sigexc mechanism.
			// See sys_wait4().
			sigexc_handler(-sig, NULL, NULL);
		}
		else
		{
			// This is the debugger telling us how to deal with the signal.
			_pthread_setspecific_direct(SIGEXC_TSD_KEY, sig);
		}
	}
}

void darling_sigexc_self(void)
{
	// Make sigexc_handler the handler for all signals in the process
	for (int i = 1; i <= 31; i++)
	{
		struct linux_sigaction sa;
		sa.sa_sigaction = sigrt_handler;
		sa.sa_mask = 0xffffffff; // all other standard Unix signals should be blocked while the handler is run
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

static mach_port_t get_exc_port(int type)
{
	mach_msg_type_number_t count = 0;
	exception_mask_t masks[EXC_TYPES_COUNT];
	mach_port_t ports[EXC_TYPES_COUNT];
	exception_behavior_t behaviors[EXC_TYPES_COUNT];
	thread_state_flavor_t flavors[EXC_TYPES_COUNT];

	kern_return_t result = task_get_exception_ports(mach_task_self(), 1 << type,
			masks, &count, ports, behaviors, flavors);

	if (result == KERN_SUCCESS)
		return ports[type];

	return 0;
}

void sigexc_handler(int linux_signum, struct linux_siginfo* info, void* ctxt)
{
	if (!darling_am_i_ptraced())
		return;

	// Send a Mach message to the debugger.
	// The debugger may use ptrace(PT_THUPDATE) to change how the signal is processed.

	int mach_exception;
	long long codes[EXCEPTION_CODE_MAX] = { 0 };
	mach_port_t port;

	int bsd_signum = signum_linux_to_bsd(linux_signum);
	if (bsd_signum <= 0)
		return;

	// SIGSEGV + SIGBUS -> EXC_BAD_ACCESS
	// SIGTRAP -> EXC_BREAKPOINT
	// SIGILL -> EXC_BAD_INSTRUCTION
	// SIGFPE -> EXC_ARITHMETIC
	// * -> EXC_SOFTWARE with EXC_SOFT_SIGNAL (e.g. SIGSTOP)

	switch (bsd_signum)
	{
		case SIGSEGV:
			mach_exception = EXC_BAD_ACCESS;
			codes[0] = EXC_I386_GPFLT;
			break;
		case SIGBUS:
			mach_exception = EXC_BAD_ACCESS;
			codes[0] = EXC_I386_ALIGNFLT;
			break;
		case SIGTRAP:
			mach_exception = EXC_BREAKPOINT;
			codes[0] = EXC_I386_BPT;
			break;
		case SIGILL:
			mach_exception = EXC_BAD_INSTRUCTION;
			codes[0] = EXC_I386_INVOP;
			break;
		case SIGFPE:
			mach_exception = EXC_ARITHMETIC;
			codes[0] = info->si_code;
			break;
		default:
			mach_exception = EXC_SOFTWARE;
			codes[0] = EXC_SOFT_SIGNAL;
			codes[1] = bsd_signum;
	}

	port = get_exc_port(mach_exception);

	if (port != 0)
	{
		_pthread_setspecific_direct(SIGEXC_TSD_KEY, bsd_signum);

		mach_exception_raise(port, mach_thread_self(), mach_task_self(), mach_exception, codes, sizeof(codes) / sizeof(codes[0]));

		bsd_signum = _pthread_getspecific_direct(SIGEXC_TSD_KEY);
	}

	// Pass the signal to the application handler or emulate the effects of the signal if SIG_DFL is set.
	if (bsd_signum)
	{
		bsd_sig_handler* handler = sig_handlers[bsd_signum];
		if (handler == SIG_DFL || handler == SIG_ERR)
		{
			switch (bsd_signum)
			{
				// We have to stop the process manually
				case SIGSTOP:
				case SIGTSTP:
					task_suspend(mach_task_self());
					break;

				// These signals do nothing by default
				case SIGCHLD:
				case SIGWINCH:
				case SIGURG:
					break;
				
				// Other signals cause termination or core dump.
				default:
				{
					int linux_signum = signum_bsd_to_linux(bsd_signum);
					sys_exit(linux_signum);
				}
			}
		}
		else if (handler != SIG_IGN)
		{
			handler_linux_to_bsd(signum_bsd_to_linux(bsd_signum), info, ctxt);
		}
	}
}

#define LINUX_SI_QUEUE (-1)
int linux_sigqueue(int pid, int rtsig, int value)
{
	struct linux_siginfo si;

	memset(&si, 0, sizeof(si));
	si.si_signo = rtsig;
	si.si_code = LINUX_SI_QUEUE;
	si.si_pid = getpid();
	si.si_uid = LINUX_SYSCALL(__NR_getuid);
	si.si_value = value;

	return LINUX_SYSCALL(__NR_rt_sigqueueinfo, pid, rtsig, &si);
}

int linux_sigqueue_thread(int pid, int tid, int rtsig, int value)
{
	struct linux_siginfo si;

	memset(&si, 0, sizeof(si));
	si.si_signo = rtsig;
	si.si_code = LINUX_SI_QUEUE;
	si.si_pid = getpid();
	si.si_uid = LINUX_SYSCALL(__NR_getuid);
	si.si_value = value;

	return LINUX_SYSCALL(__NR_rt_tgsigqueueinfo, pid, tid, rtsig, &si);
}

