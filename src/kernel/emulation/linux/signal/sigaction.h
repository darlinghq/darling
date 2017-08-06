#ifndef LINUX_SIGACTION_H
#define LINUX_SIGACTION_H
#include "duct_signals.h"
#include "sigaltstack.h"

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
	int si_pid;
	int si_uid;

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

struct linux_gregset
{
	long long r8, r9, r10, r11, r12, r13, r14, r15, rdi, rsi, rbp, rbx;
	long long rdx, rax, rcx, rsp, rip, efl;
	short cs, gs, fs, __pad0;
	long long err, trapno, oldmask, cr2;
};

#else // now the i386 version

typedef struct _fpstate {
        unsigned long cw, sw, tag, ipoff, cssel, dataoff, datasel;
        struct {
                unsigned short significand[4], exponent;
        } _st[8];
        unsigned long status;
} *linux_fpregset_t;

struct linux_gregset
{
	int gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax;
	int trapno, err, eip, cs, efl, uesp;
};
#endif

struct linux_mcontext
{
	struct linux_gregset gregs;
	linux_fpregset_t fpregs;
#ifdef __x86_64__
	unsigned long long __reserved[8];
#else
	unsigned long oldmask, cr2;
#endif
	// +reserved
};

struct linux_ucontext
{
	unsigned long uc_flags;
	struct linux_ucontext* uc_link;
	struct linux_stack uc_stack;
	struct linux_mcontext uc_mcontext;
	linux_sigset_t uc_sigmask;
	// linux_libc_fpstate fpregs_mem;
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
#else
	int eax, ebx, ecx, edx, edi, esi, ebp, esp, ss, eflags;
	int eip, cs, ds, es, fs, gs;
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

