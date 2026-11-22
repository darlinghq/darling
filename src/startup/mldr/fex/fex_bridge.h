#ifndef _MLDR_FEX_BRIDGE_H_
#define _MLDR_FEX_BRIDGE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
struct load_results;
#include "../elfcalls/elfcalls.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DARLING_FEX_ELFCALL_COUNT            33U
#define DARLING_FEX_ELFCALL_SYSCALL_BASE     0x44415200U
#define DARLING_FEX_DLSYM_SYSCALL_BASE       0x44420000U
#define DARLING_FEX_DLSYM_MAX_SLOTS          2048U

struct fex_x86_64_state {
	uint64_t rax;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rbx;
	uint64_t rsp;
	uint64_t rbp;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rip;
	uint64_t rflags;
	uint64_t gs_base;
	uint64_t fs_base;
	uint64_t xmm[16][2];
	uint8_t unwind_thread;
	// Set by the syscall dispatch for the shim's syscall handler: the guest registers are already the ones a
	// returning signal handler's frame holds (frame_restored), or a guest handler is to run as the syscall
	// returns (signal_request).
	uint8_t frame_restored;
	const struct fex_signal_request* signal_request;
};

typedef long (*fex_syscall_cb_t)(struct fex_x86_64_state* state);

typedef void (*fex_fork_lock_fn_t)(void);
typedef void (*fex_fork_unlock_fn_t)(bool is_child);

void darling_fexcore_lock_before_fork(void);
void darling_fexcore_unlock_after_fork(bool is_child);

// Guest signal delivery. mldr keeps the guest's signal state (actions, alternate stack) and decides what a
// signal does; the shim builds the x86_64 signal frame and moves the interrupted thread into the guest handler.

// The x86_64 Linux struct sigaction the guest passes to rt_sigaction.
struct fex_guest_sigaction {
	uint64_t handler;
	uint64_t flags;
	uint64_t restorer;
	uint64_t mask;
};

// The x86_64 Linux stack_t of the guest's alternate signal stack.
struct fex_guest_stack {
	uint64_t sp;
	int32_t flags;
	uint32_t pad;
	uint64_t size;
};

#define FEX_GUEST_SA_NOCLDSTOP 0x00000001ULL
#define FEX_GUEST_SA_NOCLDWAIT 0x00000002ULL
#define FEX_GUEST_SA_RESTORER  0x04000000ULL
#define FEX_GUEST_SA_ONSTACK   0x08000000ULL
#define FEX_GUEST_SA_RESTART   0x10000000ULL
#define FEX_GUEST_SA_NODEFER   0x40000000ULL
#define FEX_GUEST_SA_RESETHAND 0x80000000ULL
#define FEX_GUEST_SS_ONSTACK   1
#define FEX_GUEST_SS_DISABLE   2

// Signals the generated code raises itself (SIGILL, SIGTRAP, SIGBUS, SIGFPE, SIGSEGV: faults, the traps it
// uses for guest exceptions and for a guest handler's return). The host keeps its handler for them and they
// are never blocked on the host, whatever the guest asks for.
#define FEX_SIGBIT(sig) (1ULL << ((sig) - 1))
#define FEX_JIT_SIGNALS (FEX_SIGBIT(4) | FEX_SIGBIT(5) | FEX_SIGBIT(7) | FEX_SIGBIT(8) | FEX_SIGBIT(11))

enum fex_signal_disposition {
	FEX_SIGNAL_CONSUMED,           // the shim handled it (a patched access, a deferred signal): nothing more to do
	FEX_SIGNAL_RETURNED,           // a guest handler returned and the interrupted context is restored
	FEX_SIGNAL_FOR_GUEST,          // the guest thread on this host thread can take it now (it is in emulated code)
	FEX_SIGNAL_AT_SYSCALL_RETURN,  // the guest thread is in host code (a syscall, a host library): take it later
	FEX_SIGNAL_NO_GUEST_THREAD,    // no guest code can run here (a host-only thread, or a fault in the handler)
	FEX_SIGNAL_DELIVERED,          // the thread resumes in the guest handler when the host handler returns
};

// A guest handler to run as the current syscall returns, built by the dispatch and read by the shim.
struct fex_signal_request {
	int32_t sig;
	uint8_t restart_syscall; // the handler returns to the syscall instruction, which runs again
	uint8_t pad[3];
	uint64_t old_mask;       // the guest's blocked set before the handler, restored by rt_sigreturn
	struct fex_guest_sigaction action;
	struct fex_guest_stack altstack;
	uint8_t info[128];       // siginfo_t
};

// `info` is the host siginfo_t and `ucontext` the host ucontext_t of the host signal handler. preempt sets
// `guest_mask` to the blocked set a returning handler's frame holds when it answers FEX_SIGNAL_RETURNED.
// deliver gets the guest's blocked set at the signal, which the frame keeps for rt_sigreturn. sigreturn
// restores the frame of a handler that ran at a syscall return and gives its blocked set (returns 1), does
// not return for a handler that interrupted emulated code, and returns -EFAULT for an unreadable frame.
typedef int (*fex_signal_preempt_fn_t)(int* sig, void* info, void* ucontext, uint64_t* guest_mask);
typedef int (*fex_signal_deliver_fn_t)(int sig, const void* info, void* ucontext,
	const struct fex_guest_sigaction* action, const struct fex_guest_stack* altstack, uint64_t old_mask);
typedef int (*fex_signal_return_fn_t)(uint64_t* guest_mask);

int darling_fexcore_signal_preempt(int* sig, void* info, void* ucontext, uint64_t* guest_mask);
int darling_fexcore_signal_deliver(int sig, const void* info, void* ucontext,
	const struct fex_guest_sigaction* action, const struct fex_guest_stack* altstack, uint64_t old_mask);
int darling_fexcore_signal_return(uint64_t* guest_mask);

void fex_signals_set_hooks(fex_signal_preempt_fn_t preempt, fex_signal_deliver_fn_t deliver,
	fex_signal_return_fn_t sigreturn);
// A forked child starts with no pending signals, as on Linux.
void fex_signals_after_fork_child(void);
// Whether a host handler interrupted a thread at the kernel entry of a guest's own syscall (about to make it,
// or rewound by the kernel to restart it), not of a helper syscall mldr makes around it.
bool fex_at_guest_syscall_entry(const void* pc);

struct elf_calls* fex_elfcalls_get_trampoline_table(const struct elf_calls* host_calls);
long fex_elfcalls_dispatch(uint32_t index, struct fex_x86_64_state* state);
bool fex_elfcalls_has_pending_terminate(void);
int fex_elfcalls_flush_deferred_terminate(void);
void fex_elfcalls_set_fork_hooks(fex_fork_lock_fn_t lock_hook, fex_fork_unlock_fn_t unlock_hook);

void* fex_dlsym_wrap_if_func(void* host_sym, const char* symbol_name);
long fex_dlsym_dispatch(uint32_t slot, struct fex_x86_64_state* state);

darling_thread_create_callbacks_t fex_wrap_guest_thread_callbacks(darling_thread_create_callbacks_t guest_cbs);

long fex_linux_syscall_dispatch(struct fex_x86_64_state* state);

int fex_copy_to_guest(void* guest, const void* src, size_t len);
int fex_copy_from_guest(void* dst, const void* guest, size_t len);

void fex_set_current_thread_gs_base(uint64_t gs_base);
uint64_t fex_get_current_thread_gs_base(void);
uint64_t fex_invoke_guest_callback(uintptr_t guest_fn, uint64_t arg0, uint64_t arg1);
uint64_t fex_invoke_guest_callback_args(uintptr_t guest_fn, const uint64_t args[6]);

void fex_start_main_thread(struct load_results* lr) __attribute__((noreturn));
void fex_start_darling_thread(
	void* pth,
	int port,
	uintptr_t real_entry,
	uintptr_t arg1,
	uintptr_t arg2,
	uintptr_t arg3,
	uintptr_t entry_point,
	uintptr_t stack_ptr,
	uintptr_t gs_base) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif // _MLDR_FEX_BRIDGE_H_
