#ifndef LINUX_SIGACTION_H
#define LINUX_SIGACTION_H
#include "duct_signals.h"
#include "sigaltstack.h"

#undef sa_sigaction

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
# ifdef __LP64__
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
		struct
		{
			int si_pid;
			int si_uid;
		};
		void* si_addr;
	};

	union
	{
		int _pad[__SI_PAD_SIZE];
		unsigned long si_value;
	};
};

typedef void (bsd_sig_handler)(int, struct bsd_siginfo*, void*);
typedef void (linux_sig_handler)(int, struct linux_siginfo*, void*);

#ifndef SIG_DFL
#define SIG_DFL (bsd_sig_handler*)0
#endif
#ifndef SIG_IGN
#define SIG_IGN (bsd_sig_handler*)1
#endif
#ifndef SIG_ERR
#define SIG_ERR ((bsd_sig_handler*)-1l)
#endif

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
	linux_sig_handler* sa_sigaction;
	int sa_flags;
	void (*sa_restorer)(void);
	linux_sigset_t sa_mask;
};

long sys_sigaction(int signum, const struct bsd___sigaction* nsa, struct bsd_sigaction* osa);

#ifdef __x86_64__
	// #include <asm/sigcontext.h>
	// struct _fpstate_64 { ... };
	typedef struct _fpstate {
		unsigned short cwd, swd, ftw, fop;
		unsigned long long rip, rdp;
		unsigned mxcsr, mxcr_mask;
		struct {
				unsigned short significand[4], exponent, padding[3];
		} _st[8];
		struct {
				unsigned element[4];
		} _xmm[16];
		unsigned padding[24];
	} *linux_fpregset_t;

	// #include <sys/user.h>
	// struct user_regs_struct { ... };
	struct linux_gregset
	{
		long long r8, r9, r10, r11, r12, r13, r14, r15, rdi, rsi, rbp, rbx;
		long long rdx, rax, rcx, rsp, rip, efl;
		short cs, gs, fs, __pad0;
		long long err, trapno, oldmask, cr2;
	};

#elif defined(__i386__)
	// #include <asm/sigcontext.h>
	// struct _fpstate_32 { ... };
	typedef struct _fpstate {
		unsigned long cw, sw, tag, ipoff, cssel, dataoff, datasel;
		struct {
				unsigned short significand[4], exponent;
		} _st[8];
		unsigned short status, magic;
		unsigned int _fxsr_env[6];
		unsigned int mxcsr;
		unsigned int reserved;
		struct _fpxreg {
			unsigned short significand[4];
			unsigned short exponent;
			unsigned short padding[3];
		} _fxsr_st[8];
		struct {
				unsigned element[4];
		} _xmm[8];
	} *linux_fpregset_t;

	// #include <asm/sigcontext.h>
	// struct sigcontext_32 { ... }; ?
	struct linux_gregset
	{
		int gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax;
		int trapno, err, eip, cs, efl, uesp;
		int ss;
	};

#elif defined(__arm64__)
	// #include <asm/sigcontext.h>
	// struct _aarch64_ctx { ... };
	struct linux_aarch64_ctx {
		__uint32_t magic;
		__uint32_t size;
	};

	#define FPSIMD_MAGIC    0x46508001
	
	// #include <asm/sigcontext.h>
	// struct fpsimd_context { ... }
	struct linux_fpsimd_context {
		struct linux_aarch64_ctx head;
		__uint32_t fpsr;
		__uint32_t fpcr;
		__uint128_t vregs[32];
	};
#else
	#error "Floating point registers not defined"
#endif

// #include <sys/ucontext.h>
// typedef struct { ... } mcontext_t;
struct linux_mcontext 
{
	#if defined(__x86_64__) || defined(__i386__)
		struct linux_gregset gregs;
		linux_fpregset_t fpregs;
		#ifdef __x86_64__
			unsigned long long __reserved[8];
		#else
			unsigned long oldmask, cr2;
		#endif
	#elif defined(__arm64__)
		unsigned long long int fault_address;
		unsigned long long int regs[31];
		unsigned long long int sp;
		unsigned long long int pc;
		unsigned long long int pstate;
		unsigned char __reserved[4096] __attribute__ ((__aligned__ (16)));
	#else
		#error "linux_mcontext not defined"
	#endif
	// +reserved
};

// #include <sys/ucontext.h>
// typedef struct ucontext_t { ... } ucontext_t;
struct linux_ucontext
{
	unsigned long uc_flags;
	struct linux_ucontext* uc_link;
	struct linux_stack uc_stack;
	#if defined(__x64_64__) || defined(__i386__)
		struct linux_mcontext uc_mcontext;
		linux_sigset_t uc_sigmask;
		// linux_libc_fpstate fpregs_mem;
	#else //defined(__arm64__)
		linux_sigset_t uc_sigmask;
		struct linux_mcontext uc_mcontext;
	#endif
};

struct bsd_exception_state
{
	unsigned short trapno;
	unsigned short cpu;
	unsigned int err;
	unsigned long faultvaddr;
};

struct bsd_thread_state
{
#ifdef __x86_64__
	long long rax, rbx, rcx, rdx, rdi, rsi, rbp, rsp, r8, r9, r10;
	long long r11, r12, r13, r14, r15, rip, rflags, cs, fs, gs;
#elif defined(__i386__)
	int eax, ebx, ecx, edx, edi, esi, ebp, esp, ss, eflags;
	int eip, cs, ds, es, fs, gs;
#elif defined(__arm64__)
	__uint64_t x[29];
	__uint64_t fp, lr, sp, pc;
	__uint32_t cpsr, __pad;
#else
	#error "Architecture registers are missing in bsd_thread_state"
#endif
};

struct bsd_float_state
{
	// TODO
};

struct bsd_mcontext
{
	struct bsd_exception_state es;
	struct bsd_thread_state ss;
	struct bsd_float_state fs;
};

struct bsd_ucontext
{
	int uc_onstack;
	sigset_t uc_sigmask;
	struct bsd_stack uc_stack;
	struct bsd_ucontext* uc_link;
	unsigned long uc_mcsize;
	struct bsd_mcontext* uc_mcontext;
};

void handler_linux_to_bsd(int linux_signum, struct linux_siginfo* info, void* ctxt);
extern bsd_sig_handler* sig_handlers[32];
extern int sig_flags[32];
extern unsigned int sig_masks[32];

#endif

