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
#define DARLING_FEX_CALLBACK_RETURN_SYSCALL  0x44415300U
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
};

typedef long (*fex_syscall_cb_t)(struct fex_x86_64_state* state);

typedef void (*fex_fork_lock_fn_t)(void);
typedef void (*fex_fork_unlock_fn_t)(bool is_child);

void darling_fexcore_lock_before_fork(void);
void darling_fexcore_unlock_after_fork(bool is_child);

struct elf_calls* fex_elfcalls_get_trampoline_table(const struct elf_calls* host_calls);
long fex_elfcalls_dispatch(uint32_t index, struct fex_x86_64_state* state);
bool fex_elfcalls_has_pending_terminate(void);
int fex_elfcalls_flush_deferred_terminate(void);
void fex_elfcalls_set_fork_hooks(fex_fork_lock_fn_t lock_hook, fex_fork_unlock_fn_t unlock_hook);

void* fex_dlsym_wrap_if_func(void* host_sym, const char* symbol_name);
long fex_dlsym_dispatch(uint32_t slot, struct fex_x86_64_state* state);

darling_thread_create_callbacks_t fex_wrap_guest_thread_callbacks(darling_thread_create_callbacks_t guest_cbs);

long fex_linux_syscall_dispatch(struct fex_x86_64_state* state);

void fex_set_current_thread_gs_base(uint64_t gs_base);
uint64_t fex_get_current_thread_gs_base(void);
uint64_t fex_invoke_guest_callback(uintptr_t guest_fn, uint64_t arg0, uint64_t arg1);

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
