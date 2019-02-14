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
#include "../../../../lkm/api.h"
#include "../../../libsyscall/wrappers/_libkernel_init.h"
#include "kill.h"
#include "../simple.h"

// Support for Darwin debugging.
// Unlike other Unix-like systems, macOS doesn't use wait() to handle events in the debugged process.
// wait() only receives termination events.

static volatile bool am_i_ptraced = false;
static void handle_rt_signal(int signum);
extern void sig_restorer(void);
extern int getpid(void);

extern _libkernel_functions_t _libkernel_functions;

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

#ifdef DEBUG_SIGEXC
#define kern_printf(...) { char buf[128]; __simple_sprintf(buf, __VA_ARGS__); lkm_call(0x1028, buf); }
#else
#define kern_printf(...)
#endif

void sigexc_setup1(void)
{
	kern_printf("sigexc_setup1()");
	// Setup handler for SIGNAL_SIGEXC_TOGGLE and SIGNAL_SIGEXC_THUPDATE
	handle_rt_signal(SIGNAL_SIGEXC_TOGGLE);
	handle_rt_signal(SIGNAL_SIGEXC_THUPDATE);
}

void sigexc_setup2(void)
{
	kern_printf("sigexc_setup2()\n");

	linux_sigset_t set;
	set = (1ull << (SIGNAL_SIGEXC_TOGGLE-1));
	set |= (1ull << (SIGNAL_SIGEXC_THUPDATE-1));

	LINUX_SYSCALL(__NR_rt_sigprocmask, 1 /* LINUX_SIG_UNBLOCK */,
		&set, NULL, sizeof(linux_sigset_t));

	// If we have a tracer (i.e. we are a new process after execve),
	// enable sigexc handling and send SIGTRAP to self to allow
	// the debugger to handle this situation.
	if (!am_i_ptraced && lkm_call(NR_get_tracer, NULL) != 0)
	{
		kern_printf("sigexc: the parent is traced\n");
		darling_sigexc_self();
		sigexc_handler(LINUX_SIGTRAP, NULL, NULL);
	}
}

void sigexc_setup(void)
{
#ifdef VARIANT_DYLD
	sigexc_setup1();
	if (lkm_call(NR_started_suspended, 0))
	{
		kern_printf("sigexc: waiting for signal\n");

		// sigsuspend until resumed or debugger attached
		linux_sigset_t set = -1ll;
		set &= ~(1ull << (LINUX_SIGCONT-1));
		set &= ~(1ull << (SIGNAL_SIGEXC_TOGGLE-1));
		set &= ~(1ull << (SIGNAL_SIGEXC_THUPDATE-1));

		LINUX_SYSCALL(__NR_rt_sigsuspend, &set, 8);

		kern_printf("sigexc: done waiting for signal\n");
	}
	sigexc_setup2();
#else
	sigexc_setup1();

	// get am_i_ptraced value from dyld
	bool (*is_traced)(void);
	_libkernel_functions->dyld_func_lookup("__dyld_am_i_ptraced", (void**) &is_traced);
	am_i_ptraced = is_traced();

	if (am_i_ptraced)
	{
		// We have to take over from dyld's build of this file, because
		// we rely on having accurate signal handler information of the running application.
		kern_printf("sigexc: taking over sigexc handling\n");
		darling_sigexc_self();
	}
#endif
}

static void handle_rt_signal(int signum)
{
	int rv;
	struct linux_sigaction sa;

	sa.sa_sigaction = sigrt_handler;
	sa.sa_mask = 0;
	sa.sa_flags = LINUX_SA_RESTORER | LINUX_SA_SIGINFO | LINUX_SA_RESTART;
	sa.sa_restorer = sig_restorer;

	rv = LINUX_SYSCALL(__NR_rt_sigaction, signum,
			&sa, NULL,
			sizeof(sa.sa_mask));

	//char buf[128];
	//__simple_sprintf(xyzbuf, "Setting handler for RT signal %d: %d", signum, rv);
	//lkm_call(0x1028, buf);
}

bool darling_am_i_ptraced(void)
{
	return am_i_ptraced;
}

void sigrt_handler(int signum, struct linux_siginfo* info, void* ctxt)
{
	kern_printf("sigexc: sigrt_handler signum=%d, si_value=%x\n", signum, info->si_value);

	if (signum == SIGNAL_SIGEXC_TOGGLE)
	{
		if (((uint32_t) info->si_value) == SIGRT_MAGIC_ENABLE_SIGEXC)
		{
			darling_sigexc_self();

			// Stop on attach
			sigexc_handler(LINUX_SIGSTOP, NULL, (struct linux_ucontext*) ctxt);
		}
		else if (((uint32_t) info->si_value) == SIGRT_MAGIC_DISABLE_SIGEXC)
		{
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
			sigexc_handler(-sig, NULL, (struct linux_ucontext*) ctxt);
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
void darling_sigexc_uninstall(void)
{
	am_i_ptraced = false;

	// __simple_printf("darling_sigexc_uninstall()\n");
	for (int i = 1; i <= 31; i++)
	{
		if (i == LINUX_SIGSTOP || i == LINUX_SIGKILL)
			continue;

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

static mach_port_t get_exc_port(int type, int* behavior)
{
	mach_msg_type_number_t count = 0;
	exception_mask_t masks[EXC_TYPES_COUNT];
	mach_port_t ports[EXC_TYPES_COUNT];
	exception_behavior_t behaviors[EXC_TYPES_COUNT];
	thread_state_flavor_t flavors[EXC_TYPES_COUNT];

	kern_return_t result = task_get_exception_ports(mach_task_self(), 1 << type,
			masks, &count, ports, behaviors, flavors);

	if (result != KERN_SUCCESS)
		return 0;

	for (int i = 0; i < count; i++)
	{
		if (masks[i] & (1 << type))
		{
			if (behavior != NULL)
				*behavior = behaviors[i];
			return ports[i];
		}
	}

	return 0;
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

	if (!darling_am_i_ptraced())
	{
		kern_printf("sigexc: NOT TRACED!\n");
		return;
	}

	if (linux_signum == LINUX_SIGCONT)
		return;

	// Send a Mach message to the debugger.
	// The debugger may use ptrace(PT_THUPDATE) to change how the signal is processed.

	int mach_exception, behavior;
	long long codes[EXCEPTION_CODE_MAX] = { 0 };
	mach_port_t port;
	thread_t thread = mach_thread_self();

	int bsd_signum = signum_linux_to_bsd(linux_signum);
	if (bsd_signum <= 0)
	{
		kern_printf("sigexc: Unmapped signal!\n");
		return;
	}

	// SIGSEGV + SIGBUS -> EXC_BAD_ACCESS
	// SIGTRAP -> EXC_BREAKPOINT
	// SIGILL -> EXC_BAD_INSTRUCTION
	// SIGFPE -> EXC_ARITHMETIC
	// * -> EXC_SOFTWARE with EXC_SOFT_SIGNAL (e.g. SIGSTOP)

	switch (bsd_signum)
	{
		// Only real exceptions produced by the CPU get translated to these
		// Mach exceptions. The rest comes as EXC_SOFTWARE.
		if (info != NULL && info->si_code != 0)
		{
			case SIGSEGV:
				mach_exception = EXC_BAD_ACCESS;
				codes[0] = EXC_I386_GPFLT;
				break;
			case SIGBUS:
				mach_exception = EXC_BAD_ACCESS;
				codes[0] = EXC_I386_ALIGNFLT;
				break;
			case SIGILL:
				mach_exception = EXC_BAD_INSTRUCTION;
				codes[0] = EXC_I386_INVOP;
				break;
			case SIGFPE:
				mach_exception = EXC_ARITHMETIC;
				codes[0] = info->si_code;
				break;
			case SIGTRAP:
				mach_exception = EXC_BREAKPOINT;
				codes[0] = (info->si_code == LINUX_SI_KERNEL) ? EXC_I386_BPT : EXC_I386_SGL;

				if (info->si_code == LINUX_TRAP_HWBKPT)
				{
					// Memory watchpoint triggered
					struct last_triggered_watchpoint_args args;
					if (lkm_call(NR_last_triggered_watchpoint, &args) == 0)
					{
						codes[1] = args.address;
						// codes[2] = args.flags;
					}
				}
				break;
		}
		default:
			mach_exception = EXC_SOFTWARE;
			codes[0] = EXC_SOFT_SIGNAL;
			codes[1] = bsd_signum;
	}

	port = get_exc_port(mach_exception, &behavior);

	// Pass register states to LKM
#if defined(__x86_64__)
	x86_thread_state64_t tstate;
	x86_float_state64_t fstate;

	if (ctxt != NULL)
	{
		mcontext_to_thread_state(&ctxt->uc_mcontext.gregs, &tstate);
		mcontext_to_float_state(ctxt->uc_mcontext.fpregs, &fstate);
		/*
#ifdef __x86_64__
		if (bsd_signum == SIGTRAP)
		{
			uint8_t* rip = (uint8_t*) ctxt->uc_mcontext.gregs.rip;
			kern_printf("rip is %p\n", rip);
			kern_printf("Value at rip-1 on suspend: 0x%x\n", *(rip-1));
		}
#endif
		*/
	}
	else
	{
		memset(&tstate, 0, sizeof(tstate));
		memset(&fstate, 0, sizeof(fstate));
	}

	thread_set_state(thread, x86_THREAD_STATE64, (thread_state_t) &tstate, x86_THREAD_STATE64_COUNT);
	thread_set_state(thread, x86_FLOAT_STATE64, (thread_state_t) &fstate, x86_FLOAT_STATE64_COUNT);
#elif defined(__i386__)
	x86_thread_state32_t tstate;
	x86_float_state32_t fstate;

	if (ctxt != NULL)
	{
		mcontext_to_thread_state(&ctxt->uc_mcontext.gregs, &tstate);
		mcontext_to_float_state(ctxt->uc_mcontext.fpregs, &fstate);
	}
	else
	{
		memset(&tstate, 0, sizeof(tstate));
		memset(&fstate, 0, sizeof(fstate));
	}

	thread_set_state(thread, x86_THREAD_STATE32, (thread_state_t) &tstate, x86_THREAD_STATE32_COUNT);
	thread_set_state(thread, x86_FLOAT_STATE32, (thread_state_t) &fstate, x86_FLOAT_STATE32_COUNT);
#endif

	// __simple_printf("Passing Mach exception to port %d\n", port);
	if (port != 0)
	{
		_pthread_setspecific_direct(SIGEXC_TSD_KEY, bsd_signum);

		if (behavior & MACH_EXCEPTION_CODES)
		{
			mach_exception_raise(port, thread, mach_task_self(), mach_exception, codes, sizeof(codes) / sizeof(codes[0]));
		}
		else
		{
			exception_data_type_t small_codes[2] = { (exception_data_type_t) codes[0], (exception_data_type_t) codes[1] };
			exception_raise(port, thread, mach_task_self(), mach_exception, small_codes, sizeof(small_codes) / sizeof(small_codes[0]));
		}

		bsd_signum = _pthread_getspecific_direct(SIGEXC_TSD_KEY);
	}

	// Get (possibly updated by GDB) register states from LKM
	if (ctxt != NULL)
	{
#if defined(__x86_64__)
		mach_msg_type_number_t count;

		count = x86_THREAD_STATE64_COUNT;
		thread_get_state(thread, x86_THREAD_STATE64, (thread_state_t) &tstate, &count);
		count = x86_FLOAT_STATE64_COUNT;
		thread_get_state(thread, x86_FLOAT_STATE64, (thread_state_t) &fstate, &count);

		thread_state_to_mcontext(&tstate, &ctxt->uc_mcontext.gregs);
		float_state_to_mcontext(&fstate, ctxt->uc_mcontext.fpregs);
		
#elif defined(__i386__)
		mach_msg_type_number_t count;

		count = x86_THREAD_STATE32_COUNT;
		thread_get_state(thread, x86_THREAD_STATE32, (thread_state_t) &tstate, &count);
		count = x86_FLOAT_STATE32_COUNT;
		thread_get_state(thread, x86_FLOAT_STATE32, (thread_state_t) &fstate, &count);

		thread_state_to_mcontext(&tstate, &ctxt->uc_mcontext.gregs);
		float_state_to_mcontext(&fstate, ctxt->uc_mcontext.fpregs);
#endif
	}

	// Pass the signal to the application handler or emulate the effects of the signal if SIG_DFL is set.
	if (bsd_signum)
	{
		const int linux_signum = signum_bsd_to_linux(bsd_signum);

		bsd_sig_handler* handler = sig_handlers[linux_signum];
		if (handler == SIG_DFL || handler == SIG_ERR)
		{
			/*
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
				case SIGTRAP:
					if (ctxt == NULL)
						break; // This trap wasn't caused by int3, carry on
				
				// Other signals cause termination or core dump.
				default:
				{
					int linux_signum = signum_bsd_to_linux(bsd_signum);
					sys_exit(linux_signum);
				}
			}
			*/
		}
		else if (handler != SIG_IGN)
		{
			handler_linux_to_bsd(linux_signum, info, ctxt);
		}
	}
}

#if defined(__x86_64__)
void mcontext_to_thread_state(const struct linux_gregset* regs, x86_thread_state64_t* s)
{
	s->__rax = regs->rax;
	s->__rbx = regs->rbx;
	s->__rcx = regs->rcx;
	s->__rdx = regs->rdx;
	s->__rdi = regs->rdi;
	s->__rsi = regs->rsi;
	s->__rbp = regs->rbp;
	s->__rsp = regs->rsp;
	s->__r8 = regs->r8;
	s->__r9 = regs->r9;
	s->__r10 = regs->r10;
	s->__r11 = regs->r11;
	s->__r12 = regs->r12;
	s->__r13 = regs->r13;
	s->__r14 = regs->r14;
	s->__r15 = regs->r15;
	s->__rip = regs->rip;
	s->__rflags = regs->efl;
	s->__cs = regs->cs;
	s->__fs = regs->fs;
	s->__gs = regs->gs;
}

void mcontext_to_float_state(const linux_fpregset_t fx, x86_float_state64_t* s)
{
	*((uint16_t*)&s->__fpu_fcw) = fx->cwd;
	*((uint16_t*)&s->__fpu_fsw) = fx->swd;
	s->__fpu_ftw = fx->ftw;
	s->__fpu_fop = fx->fop;
	s->__fpu_ip = fx->rip;
	s->__fpu_cs = 0;
	s->__fpu_dp = fx->rdp;
	s->__fpu_ds = 0;
	s->__fpu_mxcsr = fx->mxcsr;
	s->__fpu_mxcsrmask = fx->mxcr_mask;

	memcpy(&s->__fpu_stmm0, fx->_st, 128);
	memcpy(&s->__fpu_xmm0, fx->_xmm, 256);
}

void thread_state_to_mcontext(const x86_thread_state64_t* s, struct linux_gregset* regs)
{
	regs->rax = s->__rax;
	regs->rbx = s->__rbx;
	regs->rcx = s->__rcx;
	regs->rdx = s->__rdx;
	regs->rdi = s->__rdi;
	regs->rsi = s->__rsi;
	regs->rbp = s->__rbp;
	regs->rsp = s->__rsp;
	regs->r8 = s->__r8;
	regs->r9 = s->__r9;
	regs->r10 = s->__r10;
	regs->r11 = s->__r11;
	regs->r12 = s->__r12;
	regs->r13 = s->__r13;
	regs->r14 = s->__r14;
	regs->r15 = s->__r15;
	regs->rip = s->__rip;
	regs->efl = s->__rflags;
	regs->cs = s->__cs;
	regs->fs = s->__fs;
	regs->gs = s->__gs;
}

void float_state_to_mcontext(const x86_float_state64_t* s, linux_fpregset_t fx)
{
	fx->cwd = *((uint16_t*)&s->__fpu_fcw);
	fx->swd = *((uint16_t*)&s->__fpu_fsw);
	fx->ftw = s->__fpu_ftw;
	fx->fop = s->__fpu_fop;
	fx->rip = s->__fpu_ip;
	fx->rdp = s->__fpu_dp;
	fx->mxcsr = s->__fpu_mxcsr;
	fx->mxcr_mask = s->__fpu_mxcsrmask;

	memcpy(fx->_st, &s->__fpu_stmm0, 128);
	memcpy(fx->_xmm, &s->__fpu_xmm0, 256);
}

#elif defined(__i386__)
void mcontext_to_thread_state(const struct linux_gregset* regs, x86_thread_state32_t* s)
{
	s->__eax = regs->eax;
	s->__ebx = regs->ebx;
	s->__ecx = regs->ecx;
	s->__edx = regs->edx;
	s->__edi = regs->edi;
	s->__esi = regs->esi;
	s->__ebp = regs->ebp;
	s->__esp = regs->esp;
	s->__ss = regs->ss;
	s->__eflags = regs->efl;
	s->__eip = regs->eip;
	s->__cs = regs->cs;
	s->__ds = regs->ds;
	s->__es = regs->es;
	s->__fs = regs->fs;
	s->__gs = regs->gs;
}

void mcontext_to_float_state(const linux_fpregset_t fx, x86_float_state32_t* s)
{
	*((uint16_t*)&s->__fpu_fcw) = fx->cw;
	*((uint16_t*)&s->__fpu_fsw) = fx->sw;
	s->__fpu_ftw = fx->tag;
	s->__fpu_fop = 0;
	s->__fpu_ip = fx->ipoff;
	s->__fpu_cs = fx->cssel;
	s->__fpu_dp = fx->dataoff;
	s->__fpu_ds = fx->datasel;
	s->__fpu_mxcsr = fx->mxcsr;
	s->__fpu_mxcsrmask = 0;

	memcpy(&s->__fpu_stmm0, fx->_st, 128);
	memcpy(&s->__fpu_xmm0, fx->_xmm, 128);
	memset(((char*) &s->__fpu_xmm0) + 128, 0, 128);
}

void thread_state_to_mcontext(const x86_thread_state32_t* s, struct linux_gregset* regs)
{
	regs->eax = s->__eax;
	regs->ebx = s->__ebx;
	regs->ecx = s->__ecx;
	regs->edx = s->__edx;
	regs->edi = s->__edi;
	regs->esi = s->__esi;
	regs->ebp = s->__ebp;
	regs->esp = s->__esp;
	regs->ss = s->__ss;
	regs->efl = s->__eflags;
	regs->eip = s->__eip;
	regs->cs = s->__cs;
	regs->ds = s->__ds;
	regs->es = s->__es;
	regs->fs = s->__fs;
	regs->gs = s->__gs;
}

void float_state_to_mcontext(const x86_float_state32_t* s, linux_fpregset_t fx)
{
	fx->cw = *((uint16_t*)&s->__fpu_fcw);
	fx->sw = *((uint16_t*)&s->__fpu_fsw);
	fx->tag = s->__fpu_ftw;
	fx->ipoff = s->__fpu_ip;
	fx->cssel = s->__fpu_cs;
	fx->dataoff = s->__fpu_dp;
	fx->datasel = s->__fpu_ds;
	fx->mxcsr = s->__fpu_mxcsr;

	memcpy(fx->_st, &s->__fpu_stmm0, 128);
	memcpy(fx->_xmm, &s->__fpu_xmm0, 128);
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

