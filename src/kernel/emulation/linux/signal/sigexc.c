#include "sigaction.h"
#include "sigexc.h"
#include "../base.h"
#include "../unistd/exit.h"
#include <stddef.h>
#include <sys/signal.h>
#include <linux-syscalls/linux.h>
#include <pthread/tsd_private.h>
#include "signal/mach_exc.h"
#include "signal/exc.h"
#include "sigaltstack.h"
#include "../mach/lkm.h"
#include "../../../../external/lkm/api.h"
#include "../../../libsyscall/wrappers/_libkernel_init.h"
#include "kill.h"
#include "../simple.h"

// Support for Darwin debugging.
// Unlike other Unix-like systems, macOS doesn't use wait() to handle events in the debugged process.
// wait() only receives termination events.

static volatile bool am_i_ptraced = false;
static void handle_rt_signal(int signum);
static void sigexc_fake_suspend(struct linux_ucontext* ctxt);
extern void sig_restorer(void);
extern int getpid(void);

extern _libkernel_functions_t _libkernel_functions;

void darling_sigexc_uninstall(void);
void sigrt_handler(int signum, struct linux_siginfo* info, void* ctxt);

#define SIGEXC_TSD_KEY	102
#define SIGEXC_CONTEXT_TSD_KEY	103
static char sigexc_altstack[16*1024];
static struct bsd_stack orig_stack;

#if defined(__x86_64__)
static void mcontext_to_thread_state(const struct linux_gregset* regs, x86_thread_state64_t* s);
static void mcontext_to_float_state(const linux_fpregset_t fx, x86_float_state64_t* s);
static void thread_state_to_mcontext(const x86_thread_state64_t* s, struct linux_gregset* regs);
static void float_state_to_mcontext(const x86_float_state64_t* s, linux_fpregset_t fx);
#elif defined(__i386__)
static void mcontext_to_thread_state(const struct linux_gregset* regs, x86_thread_state32_t* s);
static void mcontext_to_float_state(const linux_fpregset_t fx, x86_float_state32_t* s);
static void thread_state_to_mcontext(const x86_thread_state32_t* s, struct linux_gregset* regs);
static void float_state_to_mcontext(const x86_float_state32_t* s, linux_fpregset_t fx);
#endif

static void state_to_kernel(struct linux_ucontext* ctxt, thread_t thread);

#define DEBUG_SIGEXC
#ifdef DEBUG_SIGEXC
#define kern_printf(...) { char buf[128]; __simple_sprintf(buf, __VA_ARGS__); lkm_call(0x1028, buf); }
#else
#define kern_printf(...)
#endif

void sigexc_setup1(void)
{
	
}

void sigexc_setup2(void)
{
	
}

void sigexc_setup(void)
{
	darling_sigexc_self();
#ifdef VARIANT_DYLD

#else
	
#endif
}


bool darling_am_i_ptraced(void)
{
	return am_i_ptraced;
}


void darling_sigexc_self(void)
{
	am_i_ptraced = true;

	lkm_call(0x1028, "darling_sigexc_self()\n");
	// Make sigexc_handler the handler for all signals in the process
	for (int i = 1; i <= 31; i++)
	{
		if (i == LINUX_SIGSTOP || i == LINUX_SIGKILL)
			continue;

		struct linux_sigaction sa;
		sa.sa_sigaction = (linux_sig_handler*) sigexc_handler;
		sa.sa_mask = 0x7fffffff; // all other standard Unix signals should be blocked while the handler is run
		sa.sa_flags = LINUX_SA_RESTORER | LINUX_SA_SIGINFO | LINUX_SA_RESTART | LINUX_SA_ONSTACK;
		sa.sa_restorer = sig_restorer;

		LINUX_SYSCALL(__NR_rt_sigaction, i,
				&sa, NULL,
				sizeof(sa.sa_mask));
	}

	struct bsd_stack newstack = {
		.ss_sp = sigexc_altstack,
		.ss_size = sizeof(sigexc_altstack),
		.ss_flags = 0
	};
	sys_sigaltstack(&newstack, &orig_stack);
}

// syscall tracking
#define LINUX_TRAP_BRKPT	1
// single-stepping
#define LINUX_TRAP_TRACE	2
// ???
#define LINUX_TRAP_BRANCH	3
// memory watchpoint
#define LINUX_TRAP_HWBKPT	4
// int3 (on x86, other platforms probably use TRAP_BRKPT)
#define LINUX_SI_KERNEL		0x80

void sigexc_handler(int linux_signum, struct linux_siginfo* info, struct linux_ucontext* ctxt)
{
	kern_printf("sigexc_handler(%d, %p, %p)\n", linux_signum, info, ctxt);

	
	if (linux_signum == LINUX_SIGCONT)
		return;

	int bsd_signum = signum_linux_to_bsd(linux_signum);
	if (bsd_signum <= 0)
	{
		kern_printf("sigexc: Unmapped signal!\n");
		return;
	}

	struct sigprocess_args sigprocess;
	sigprocess.signum = bsd_signum;

	memcpy(&sigprocess.linux_siginfo, info, sizeof(*info));
	memcpy(&sigprocess.linux_ucontext, ctxt, sizeof(*ctxt));

	lkm_call(NR_sigprocess, &sigprocess);

	if (!sigprocess.signum)
	{
		kern_printf("sigexc: drop signal\n");
		return;
	}

	memcpy(info, &sigprocess.linux_siginfo, sizeof(*info));
	memcpy(ctxt, &sigprocess.linux_ucontext, sizeof(*ctxt));

	linux_signum = signum_bsd_to_linux(sigprocess.signum);

	if (sig_handlers[linux_signum] != SIG_IGN)
	{
		if (sig_handlers[linux_signum])
		{
			kern_printf("sigexc: will forward signal to app handler (%p)\n", sig_handlers[linux_signum]);
			handler_linux_to_bsd(linux_signum, info, ctxt);
		}
		else
		{
			if (sigprocess.signum == SIGTSTP || sigprocess.signum == SIGSTOP)
			{
				kern_printf("sigexc: emulating SIGTSTP/SIGSTOP\n");
				LINUX_SYSCALL(__NR_kill, 0, LINUX_SIGSTOP);
			}
			else
			{
				kern_printf("sigexc: emulating default signal effects\n");
				// Set handler to SIG_DFL
				struct linux_sigaction sa;
				sa.sa_sigaction = (linux_sig_handler*) NULL; // SIG_DFL
				sa.sa_mask = 0x7fffffff; // all other standard Unix signals should be blocked while the handler is run
				sa.sa_flags = LINUX_SA_RESTORER | LINUX_SA_SIGINFO | LINUX_SA_RESTART | LINUX_SA_ONSTACK;
				sa.sa_restorer = sig_restorer;

				LINUX_SYSCALL(__NR_rt_sigaction, linux_signum,
						&sa, NULL,
						sizeof(sa.sa_mask));

				// Resend signal to self
				LINUX_SYSCALL(__NR_kill, 0, linux_signum);
			}
		}
	}
	
	kern_printf("sigexc: handler (%d) returning\n", linux_signum);
}

