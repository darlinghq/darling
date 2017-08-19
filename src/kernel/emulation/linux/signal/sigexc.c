#include "sigexc.h"
#include "../base.h"
#include "../unistd/exit.h"
#include <stddef.h>
#include <sys/signal.h>
#include <linux-syscalls/linux.h>
#include <pthread/tsd_private.h>
#include "signal/mach_exc.h"
#include "sigaltstack.h"
#include "../mach/lkm.h"
#include "../../../../lkm/api.h"
#include "kill.h"

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
static char sigexc_altstack[8192];
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

void sigexc_setup(void)
{
	// Setup handler for SIGNAL_SIGEXC_TOGGLE and SIGNAL_SIGEXC_THUPDATE
	handle_rt_signal(SIGNAL_SIGEXC_TOGGLE);
	handle_rt_signal(SIGNAL_SIGEXC_THUPDATE);

	// If we have a tracer (i.e. we are a new process after execve),
	// enable sigexc handling and send SIGTRAP to self to allow
	// the debugger to handle this situation.
	if (lkm_call(NR_get_tracer, NULL) != 0)
	{
		darling_sigexc_self();
		sys_kill(getpid(), SIGTRAP, 1);
	}
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

			// Stop on attach
			sigexc_handler(LINUX_SIGSTOP, NULL, NULL);
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

	sys_sigaltstack(&orig_stack, NULL);
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

void sigexc_handler(int linux_signum, struct linux_siginfo* info, struct linux_ucontext* ctxt)
{
	if (!darling_am_i_ptraced())
		return;

	if (linux_signum == LINUX_SIGCONT)
		return;

	// Send a Mach message to the debugger.
	// The debugger may use ptrace(PT_THUPDATE) to change how the signal is processed.

	int mach_exception;
	long long codes[EXCEPTION_CODE_MAX] = { 0 };
	mach_port_t port;
	thread_t thread = mach_thread_self();

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

	// Pass register states to LKM
#if defined(__x86_64__)
	x86_thread_state64_t tstate;
	x86_float_state64_t fstate;

	mcontext_to_thread_state(&ctxt->uc_mcontext.gregs, &tstate);
	mcontext_to_float_state(ctxt->uc_mcontext.fpregs, &fstate);

	thread_set_state(thread, x86_THREAD_STATE64, &tstate, x86_THREAD_STATE64_COUNT);
	thread_set_state(thread, x86_FLOAT_STATE64, &fstate, x86_THREAD_FLOAT64_COUNT);
#elif defined(__i386__)
	x86_thread_state32_t tstate;
	x86_float_state32_t fstate;

	mcontext_to_thread_state(&ctxt->uc_mcontext.gregs, &tstate);
	mcontext_to_float_state(ctxt->uc_mcontext.fpregs, &fstate);

	thread_set_state(thread, x86_THREAD_STATE32, &tstate, x86_THREAD_STATE32_COUNT);
	thread_set_state(thread, x86_FLOAT_STATE32, &fstate, x86_THREAD_FLOAT32_COUNT);
#endif

	if (port != 0)
	{
		_pthread_setspecific_direct(SIGEXC_TSD_KEY, bsd_signum);

		mach_exception_raise(port, mach_thread_self(), thread, mach_exception, codes, sizeof(codes) / sizeof(codes[0]));

		bsd_signum = _pthread_getspecific_direct(SIGEXC_TSD_KEY);
	}

	// Get (possibly updated by GDB) register states from LKM
#if defined(__x86_64__)
	mach_msg_type_number_t count;

	count = x86_THREAD_STATE64_COUNT;
	thread_get_state(thread, x86_THREAD_STATE64, &tstate, &count);
	count = x86_THREAD_FLOAT64_COUNT;
	thread_get_state(thread, x86_FLOAT_STATE64, &fstate, &count);

	thread_state_to_mcontext(&tstate, &ctxt->uc_mcontext.gregs);
	float_state_to_mcontext(&fstate, ctxt->uc_mcontext.fpregs);
#elif defined(__i386__)
	mach_msg_type_number_t count;

	count = x86_THREAD_STATE32_COUNT;
	thread_get_state(thread, x86_THREAD_STATE32, &tstate, &count);
	count = x86_THREAD_FLOAT32_COUNT;
	thread_get_state(thread, x86_FLOAT_STATE32, &fstate, &count);

	thread_state_to_mcontext(&tstate, &ctxt->uc_mcontext.gregs);
	float_state_to_mcontext(&fstate, ctxt->uc_mcontext.fpregs);
#endif

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

#if defined(__x86_64__)
void mcontext_to_thread_state(const struct linux_gregset* regs, x86_thread_state64_t* s)
{
	s->rax = regs->rax;
	s->rbx = regs->rbx;
	s->rcx = regs->rcx;
	s->rdx = regs->rdx;
	s->rdi = regs->rdi;
	s->rsi = regs->rsi;
	s->rbp = regs->rbp;
	s->rsp = regs->rsp;
	s->r8 = regs->r8;
	s->r9 = regs->r9;
	s->r10 = regs->r10;
	s->r11 = regs->r11;
	s->r12 = regs->r12;
	s->r13 = regs->r13;
	s->r14 = regs->r14;
	s->r15 = regs->r15;
	s->rip = regs->rip;
	s->rflags = regs->efl;
	s->cs = regs->cs;
	s->fs = regs->fs;
	s->gs = regs->gs;
}

void mcontext_to_float_state(const linux_fpregset_t fx, x86_float_state64_t* s)
{
	*((uint32_t*)&s->fpu_fcw) = fx->cwd;
	*((uint32_t*)&s->fpu_fsw) = fx->swd;
	s->fpu_ftw = fx->ftw;
	s->fpu_fop = fx->fop;
	s->fpu_ip = fx->rip;
	s->fpu_cs = 0;
	s->fpu_dp = fx->rdp;
	s->fpu_ds = 0;
	s->fpu_mxcsr = fx->mxcsr;
	s->fpu_mxcsrmask = fx->mxcsr_mask;

	memcpy(&s->fpu_stmm0, fx->_st, 128);
	memcpy(&s->fpu_xmm0, fx->_xmm, 256);
}

void thread_state_to_mcontext(const x86_thread_state64_t* s, struct linux_gregset* regs)
{
	regs->rax = s->rax;
	regs->rbx = s->rbx;
	regs->rcx = s->rcx;
	regs->rdx = s->rdx;
	regs->rdi = s->rdi;
	regs->rsi = s->rsi;
	regs->rbp = s->rbp;
	regs->rsp = s->rsp;
	regs->r8 = s->r8;
	regs->r9 = s->r9;
	regs->r10 = s->r10;
	regs->r11 = s->r11;
	regs->r12 = s->r12;
	regs->r13 = s->r13;
	regs->r14 = s->r14;
	regs->r15 = s->r15;
	regs->rip = s->rip;
	regs->efl = s->rflags;
	regs->cs = s->cs;
	regs->fs = s->fs;
	regs->gs = s->gs;
}

void float_state_to_mcontext(const x86_float_state64_t* s, linux_fpregset_t fx)
{
	fx->cwd = *((uint32_t*)&s->fpu_fcw);
	fx->swd = *((uint32_t*)&s->fpu_fsw);
	fx->ftw = s->fpu_ftw;
	fx->fop = s->fpu_fop;
	fx->rip = s->fpu_ip;
	fx->rdp = s->fpu_dp;
	fx->mxcsr = s->fpu_mxcsr;
	fx->mxcsr_mask = s->fpu_mxcsrmask;

	memcpy(fx->_st, &s->fpu_stmm0, 128);
	memcpy(fx->_xmm, &s->fpu_xmm0, 256);
}

#elif defined(__i386__)
void mcontext_to_thread_state(const struct linux_gregset* regs, x86_thread_state32_t* s)
{
	s->eax = regs->eax;
	s->ebx = regs->ebx;
	s->ecx = regs->ecx;
	s->edx = regs->edx;
	s->edi = regs->edi;
	s->esi = regs->esi;
	s->ebp = regs->ebp;
	s->esp = regs->esp;
	s->ss = regs->ss;
	s->eflags = regs->efl;
	s->eip = regs->eip;
	s->cs = regs->cs;
	s->ds = regs->ds;
	s->es = regs->es;
	s->fs = regs->fs;
	s->gs = regs->gs;
}

void mcontext_to_float_state(const linux_fpregset_t fx, x86_float_state32_t* s)
{
	*((uint32_t*)&s->fpu_fcw) = fx->cw;
	*((uint32_t*)&s->fpu_fsw) = fx->sw;
	s->fpu_ftw = fx->tag;
	s->fpu_fop = 0;
	s->fpu_ip = fx->ipoff;
	s->fpu_cs = fx->cssel;
	s->fpu_dp = fx->dataoff;
	s->fpu_ds = fx->datasel;
	s->fpu_mxcsr = fx->mxcsr;
	s->fpu_mxcsrmask = fx->mxcsr_mask;

	memcpy(&s->fpu_stmm0, fx->_st, 128);
	memcpy(&s->fpu_xmm0, fx->_xmm, 128);
	memset(((char*) &s->fpu_xmm0) + 128, 0, 128);
}

void thread_state_to_mcontext(const x86_thread_state32_t* s, struct linux_gregset* regs)
{
	regs->eax = s->eax;
	regs->ebx = s->ebx;
	regs->ecx = s->ecx;
	regs->edx = s->edx;
	regs->edi = s->edi;
	regs->esi = s->esi;
	regs->ebp = s->ebp;
	regs->esp = s->esp;
	regs->ss = s->ss;
	regs->efl = s->eflags;
	regs->eip = s->eip;
	regs->cs = s->cs;
	regs->ds = s->ds;
	regs->es = s->es;
	regs->fs = s->fs;
	regs->gs = s->gs;
}

void float_state_to_mcontext(const x86_float_state32_t* s, linux_fpregset_t fx)
{
	fx->cw = *((uint32_t*)&s->fpu_fcw);
	fx->sw = *((uint32_t*)&s->fpu_fsw);
	fx->tag = s->fpu_ftw;
	fx->ipoff = s->fpu_ip;
	fx->cssel = s->fpu_cs;
	fx->dataoff = s->fpu_dp;
	fx->datasel = s->fpu_ds;
	fx->mxcsr = s->fpu_mxcsr;
	fx->mxcsr_mask = s->fpu_mxcsrmask;

	memcpy(fx->_st, &s->fpu_stmm0, 128);
	memcpy(fx->_xmm, &s->fpu_xmm0, 128);
}
#endif

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

