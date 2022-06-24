#include "sigaction.h"
#include "sigexc.h"
#include "../base.h"
#include "../unistd/exit.h"
#include <stddef.h>
#include <sys/signal.h>
#include <linux-syscalls/linux.h>
#include "sigaltstack.h"
#include "../mach/lkm.h"
#include "../../../libsyscall/wrappers/_libkernel_init.h"
#include <sys/mman.h>
#include "../mman/mman.h"
#include "kill.h"
#include "../simple.h"

#include <darlingserver/rpc.h>

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
void sigrt_handler(int signum, struct linux_siginfo* info, struct linux_ucontext* ctxt);

#define SIGEXC_STACK_SIZE (16ULL * 1024ULL)

#ifndef SIGALTSTACK_GUARD
	#define SIGALTSTACK_GUARD 1
#endif

#if SIGALTSTACK_GUARD
// align it on a page boundary so mprotect works properly
static char sigexc_altstack[SIGEXC_STACK_SIZE + 4096ULL] __attribute__((aligned(4096)));
#else
static char sigexc_altstack[SIGEXC_STACK_SIZE];
#endif
size_t default_sigaltstack_size = SIGEXC_STACK_SIZE;

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

static void state_from_kernel(struct linux_ucontext* ctxt, const void* tstate, const void* fstate);
static void state_to_kernel(struct linux_ucontext* ctxt, void* tstate, void* fstate);

#define DEBUG_SIGEXC
#ifdef DEBUG_SIGEXC
#define kern_printf(...) __simple_kprintf(__VA_ARGS__)
#else
#define kern_printf(...)
#endif

void sigexc_setup1(void)
{
	handle_rt_signal(SIGNAL_SIGEXC_SUSPEND);
	handle_rt_signal(SIGNAL_S2C);
}

void sigexc_setup2(void)
{
	linux_sigset_t set;
	set = (1ull << (SIGNAL_SIGEXC_SUSPEND-1));
	set |= (1ull << (SIGNAL_S2C-1));

	LINUX_SYSCALL(__NR_rt_sigprocmask, 1 /* LINUX_SIG_UNBLOCK */,
		&set, NULL, sizeof(linux_sigset_t));
}

static void dump_gregs(const struct linux_gregset* regs)
{
	unsigned long long* p = (unsigned long long*) regs;
	for (int i = 0; i < 23; i++)
	{
		kern_printf("sigexc:   gregs 0x%llx\n", p[i]);
	}
}

static void handle_rt_signal(int signum)
{
	int rv;
	struct linux_sigaction sa;

	sa.sa_sigaction = (linux_sig_handler*)sigrt_handler;
	sa.sa_mask = (1ull << (SIGNAL_SIGEXC_SUSPEND-1)) | (1ull << (signum-1));
	sa.sa_flags = LINUX_SA_RESTORER | LINUX_SA_SIGINFO | LINUX_SA_RESTART | LINUX_SA_ONSTACK;
	sa.sa_restorer = sig_restorer;

	rv = LINUX_SYSCALL(__NR_rt_sigaction, signum,
			&sa, NULL,
			sizeof(sa.sa_mask));

	//char buf[128];
	//__simple_sprintf(xyzbuf, "Setting handler for RT signal %d: %d", signum, rv);
	//external/lkm_call(0x1028, buf);
}

void sigexc_setup(void)
{
	darling_sigexc_self();
	sigexc_setup1();
	sigexc_setup2();

#ifdef VARIANT_DYLD
	bool started_suspended;
	int code = dserver_rpc_started_suspended(&started_suspended);
	if (code < 0) {
		__simple_printf("Failed to get started_suspended status: %d\n", code);
		__simple_abort();
	}
	if (started_suspended) {
		kern_printf("sigexc: start_suspended -> suspending (ret to %p)\n", __builtin_return_address(0));
		task_suspend(mach_task_self());
		kern_printf("sigexc: start_suspended -> wokenup (ret to %p)\n", __builtin_return_address(0));
	} else {
		int32_t tracer;
		code = dserver_rpc_get_tracer(&tracer);
		if (code < 0) {
			__simple_printf("Failed to get tracer status: %d\n", code);
			__simple_abort();
		}
		if (tracer != 0) {
			kern_printf("sigexc: already traced -> SIGTRAP\n");
			sys_kill(0, SIGTRAP, 0);
		}
	}
#else
	
#endif
}

void sigrt_handler(int signum, struct linux_siginfo* info, struct linux_ucontext* ctxt)
{
	int status = dserver_rpc_interrupt_enter();

	if (status != 0) {
		__simple_printf("*** dserver_rpc_interrupt_enter failed with code %d ***\n", status);
		__simple_abort();
	}

	if (signum == SIGNAL_SIGEXC_SUSPEND) {
#if defined(__x86_64__)
	x86_thread_state64_t tstate;
	x86_float_state64_t fstate;
#elif defined(__i386__)
	x86_thread_state32_t tstate;
	x86_float_state32_t fstate;
#endif

	kern_printf("sigexc: sigrt_handler SUSPEND\n");
	
	thread_t thread = mach_thread_self();
	state_to_kernel(ctxt, &tstate, &fstate);

	int ret = dserver_rpc_thread_suspended(&tstate, &fstate);
	if (ret < 0) {
		__simple_printf("dserver_rpc_thread_suspended failed internally: %d", ret);
		__simple_abort();
	}

	state_from_kernel(ctxt, &tstate, &fstate);
	} else if (signum == SIGNAL_S2C) {
		__simple_kprintf("sigexc: sigrt_handler S2C");

		int ret = dserver_rpc_s2c_perform();
		if (ret < 0) {
			__simple_printf("dserver_rpc_s2c_perform failed internally: %d", ret);
			__simple_abort();
		}
	} else {
		__simple_printf("Unknown/unrecognized real-time signal: %d", signum);
	}

	status = dserver_rpc_interrupt_exit();

	if (status != 0) {
		__simple_printf("*** dserver_rpc_interrupt_exit failed with code %d ***\n", status);
		__simple_abort();
	}
}


void darling_sigexc_self(void)
{
	am_i_ptraced = true;

	__simple_kprintf("darling_sigexc_self()\n");
	// Make sigexc_handler the handler for all signals in the process
	for (int i = 1; i <= 31; i++)
	{
		if (i == LINUX_SIGSTOP || i == LINUX_SIGKILL || i == LINUX_SIGCHLD)
			continue;

		struct linux_sigaction sa;
		sa.sa_sigaction = (linux_sig_handler*) sigexc_handler;
		sa.sa_mask = 0x7fffffff; // all other standard Unix signals should be blocked while the handler is run
		sa.sa_mask |= (1ull << (SIGNAL_SIGEXC_SUSPEND-1));
		sa.sa_flags = LINUX_SA_RESTORER | LINUX_SA_SIGINFO | LINUX_SA_RESTART | LINUX_SA_ONSTACK;
		sa.sa_restorer = sig_restorer;

		LINUX_SYSCALL(__NR_rt_sigaction, i,
				&sa, NULL,
				sizeof(sa.sa_mask));
	}

#if SIGALTSTACK_GUARD
	sys_mprotect(sigexc_altstack, 4096, PROT_NONE);
#endif

	struct bsd_stack newstack = {
#if SIGALTSTACK_GUARD
		.ss_sp = sigexc_altstack + 4096,
#else
		.ss_sp = sigexc_altstack,
#endif
		.ss_size = SIGEXC_STACK_SIZE,
		.ss_flags = 0
	};
	sys_sigaltstack(&newstack, NULL);
}


static void state_to_kernel(struct linux_ucontext* ctxt, void* tstate, void* fstate)
{
#if defined(__x86_64__)

	dump_gregs(&ctxt->uc_mcontext.gregs);
	mcontext_to_thread_state(&ctxt->uc_mcontext.gregs, (x86_thread_state64_t*) tstate);
	mcontext_to_float_state(ctxt->uc_mcontext.fpregs, (x86_float_state64_t*) fstate);

#elif defined(__i386__)
	mcontext_to_thread_state(&ctxt->uc_mcontext.gregs, (x86_thread_state32_t*) tstate);
	mcontext_to_float_state(ctxt->uc_mcontext.fpregs, (x86_float_state32_t*) fstate);
#endif

}

static void state_from_kernel(struct linux_ucontext* ctxt, const void* tstate, const void* fstate)
{
#if defined(__x86_64__)

	thread_state_to_mcontext((const x86_thread_state64_t*) tstate, &ctxt->uc_mcontext.gregs);
	float_state_to_mcontext((const x86_float_state64_t*) fstate, ctxt->uc_mcontext.fpregs);

	dump_gregs(&ctxt->uc_mcontext.gregs);
	
#elif defined(__i386__)
	thread_state_to_mcontext((const x86_thread_state32_t*) tstate, &ctxt->uc_mcontext.gregs);
	float_state_to_mcontext((const x86_float_state32_t*) fstate, ctxt->uc_mcontext.fpregs);
#endif
}

void sigexc_handler(int linux_signum, struct linux_siginfo* info, struct linux_ucontext* ctxt)
{
	int status = dserver_rpc_interrupt_enter();

	if (status != 0) {
		__simple_printf("*** dserver_rpc_interrupt_enter failed with code %d ***\n", status);
		__simple_abort();
	}

	kern_printf("sigexc_handler(%d, %p, %p)\n", linux_signum, info, ctxt);

	
	if (linux_signum == LINUX_SIGCONT)
		goto out;

	int bsd_signum = signum_linux_to_bsd(linux_signum);
	if (bsd_signum <= 0)
	{
		kern_printf("sigexc: Unmapped signal!\n");
		goto out;
	}

#ifdef __x86_64__
	kern_printf("sigexc: have RIP 0x%llx\n", ctxt->uc_mcontext.gregs.rip);
#endif

	thread_t thread = mach_thread_self();

#if defined(__x86_64__)
	x86_thread_state64_t tstate;
	x86_float_state64_t fstate;
#elif defined(__i386__)
	x86_thread_state32_t tstate;
	x86_float_state32_t fstate;
#endif

	state_to_kernel(ctxt, &tstate, &fstate);
	int ret = dserver_rpc_sigprocess(bsd_signum, linux_signum, info->si_pid, info->si_code, info->si_addr, &tstate, &fstate, &bsd_signum);
	if (ret < 0) {
		__simple_printf("sigprocess failed internally while processing Linux signal %d: %d", linux_signum, ret);
		__simple_abort();
	}
	state_from_kernel(ctxt, &tstate, &fstate);

	if (!bsd_signum)
	{
		kern_printf("sigexc: drop signal\n");
		goto out;
	}

	linux_signum = signum_bsd_to_linux(bsd_signum);

	if (sig_handlers[linux_signum] != SIG_IGN)
	{
		if (sig_handlers[linux_signum])
		{
			kern_printf("sigexc: will forward signal to app handler (%p)\n", sig_handlers[linux_signum]);

			// Update the signal mask to what the application actually wanted
			linux_sigset_t set = sig_masks[linux_signum];

			// Keep the current signal blocked, which is the default behavior
			if (!(sig_flags[linux_signum] & LINUX_SA_NODEFER))
				set |= (1ull << (linux_signum-1));

			LINUX_SYSCALL(__NR_rt_sigprocmask, 2 /* LINUX_SIG_SETMASK */,
				&set, NULL, sizeof(linux_sigset_t));

			handler_linux_to_bsd(linux_signum, info, ctxt);
		}
		else
		{
			if (bsd_signum == SIGTSTP || bsd_signum == SIGSTOP)
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

out:
	status = dserver_rpc_interrupt_exit();

	if (status != 0) {
		__simple_printf("*** dserver_rpc_interrupt_exit failed with code %d ***\n", status);
		__simple_abort();
	}
}

#define DUMPREG(regname) kern_printf("sigexc:   " #regname ": 0x%llx\n", regs->regname);

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

	kern_printf("sigexc: saving to kernel:\n");
	DUMPREG(rip);
	DUMPREG(efl);
	DUMPREG(rax);
	DUMPREG(rbx);
	DUMPREG(rcx);
	DUMPREG(rdx);
	DUMPREG(rdi);
	DUMPREG(rsi);
	DUMPREG(rbp);
	DUMPREG(rsp);
}

void mcontext_to_float_state(const linux_fpregset_t fx, x86_float_state64_t* s)
{
	kern_printf("sigexc: fcw out: 0x%x", fx->cwd);
	kern_printf("sigexc: xmm0 out: 0x%x", fx->_xmm[0].element[0]);
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

	kern_printf("sigexc: restored from kernel:\n");
	DUMPREG(rip);
	DUMPREG(efl);
	DUMPREG(rax);
	DUMPREG(rbx);
	DUMPREG(rcx);
	DUMPREG(rdx);
	DUMPREG(rdi);
	DUMPREG(rsi);
	DUMPREG(rbp);
	DUMPREG(rsp);
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
	kern_printf("sigexc: fcw in: 0x%x", fx->cwd);
	kern_printf("sigexc: xmm0 in: 0x%x", fx->_xmm[0].element[0]);
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

void sigexc_thread_setup(void)
{
	struct bsd_stack newstack = {
		.ss_size = default_sigaltstack_size,
		.ss_flags = 0
	};

#if SIGALTSTACK_GUARD
	newstack.ss_sp = (void*) sys_mmap(NULL, newstack.ss_size + 4096, PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_PRIVATE, -1, 0);
	sys_mprotect(newstack.ss_sp, 4096, PROT_NONE);
	newstack.ss_sp = (char*)newstack.ss_sp + 4096;
#else
	newstack.ss_sp = (void*) sys_mmap(NULL, newstack.ss_size, PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_PRIVATE, -1, 0);
#endif
	sys_sigaltstack(&newstack, NULL);
}

void sigexc_thread_exit(void)
{
	struct bsd_stack oldstack;
	sys_sigaltstack(NULL, &oldstack);

#if SIGALTSTACK_GUARD
	sys_munmap((char*)oldstack.ss_sp - 4096, oldstack.ss_size + 4096);
#else
	sys_munmap(oldstack.ss_sp, oldstack.ss_size);
#endif
}

