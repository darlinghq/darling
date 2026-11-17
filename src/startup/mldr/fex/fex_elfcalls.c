#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "fex_bridge.h"
#include <dlfcn.h>
#include <elf.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

_Static_assert(sizeof(struct elf_calls) == DARLING_FEX_ELFCALL_COUNT * sizeof(void*),
	"DARLING_FEX_ELFCALL_COUNT must match struct elf_calls field count");

static struct elf_calls g_host_elfcalls;
static struct elf_calls g_trampoline_elfcalls;
static uint8_t* g_elfcall_stub_page = NULL;
static pthread_once_t g_elfcall_init_once = PTHREAD_ONCE_INIT;

static pthread_mutex_t g_dlsym_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint8_t* g_dlsym_stub_area = NULL;
static pthread_once_t g_dlsym_init_once = PTHREAD_ONCE_INIT;
static void* g_dlsym_targets[DARLING_FEX_DLSYM_MAX_SLOTS];
static uint32_t g_dlsym_count = 0;

static darling_thread_create_callbacks_t g_guest_thread_cbs = NULL;
static fex_fork_lock_fn_t g_fork_lock_hook = NULL;
static fex_fork_unlock_fn_t g_fork_unlock_hook = NULL;

struct fex_deferred_terminate {
	bool pending;
	void* stackaddr;
	unsigned long freesize;
	unsigned long pthobj_size;
};
static __thread struct fex_deferred_terminate t_deferred_terminate = { false, NULL, 0, 0 };

bool fex_elfcalls_has_pending_terminate(void)
{
	return t_deferred_terminate.pending;
}

int fex_elfcalls_flush_deferred_terminate(void)
{
	if (!t_deferred_terminate.pending)
		return 0;

	void* stackaddr = t_deferred_terminate.stackaddr;
	unsigned long freesize = t_deferred_terminate.freesize;
	unsigned long pthobj_size = t_deferred_terminate.pthobj_size;
	t_deferred_terminate.pending = false;

	if (g_host_elfcalls.darling_thread_terminate) {
		return g_host_elfcalls.darling_thread_terminate(stackaddr, freesize, pthobj_size);
	}
	return 0;
}

void fex_elfcalls_set_fork_hooks(fex_fork_lock_fn_t lock_hook, fex_fork_unlock_fn_t unlock_hook)
{
	g_fork_lock_hook = lock_hook;
	g_fork_unlock_hook = unlock_hook;
}

static void write_x86_64_syscall_stub(uint8_t* dst, uint32_t syscall_nr)
{
	// movl $syscall_nr, %eax (0xb8 + imm32); syscall (0x0f 0x05); ret (0xc3); nop (0x90)
	dst[0] = 0xb8;
	memcpy(&dst[1], &syscall_nr, sizeof(syscall_nr));
	dst[5] = 0x0f;
	dst[6] = 0x05;
	dst[7] = 0xc3;
}

static void init_elfcall_stubs(void)
{
	size_t page_sz = (size_t)sysconf(_SC_PAGESIZE);
	g_elfcall_stub_page = mmap(NULL, page_sz, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (g_elfcall_stub_page == MAP_FAILED) {
		fprintf(stderr, "mldr/fex: failed to mmap elf_calls stub page: %s\n", strerror(errno));
		abort();
	}

	void** table_slots = (void**)&g_trampoline_elfcalls;
	for (uint32_t i = 0; i < DARLING_FEX_ELFCALL_COUNT; i++) {
		uint8_t* stub = g_elfcall_stub_page + (i * 8);
		write_x86_64_syscall_stub(stub, DARLING_FEX_ELFCALL_SYSCALL_BASE + i);
		table_slots[i] = stub;
	}

	if (mprotect(g_elfcall_stub_page, page_sz, PROT_READ | PROT_EXEC) != 0) {
		fprintf(stderr, "mldr/fex: failed to mprotect elf_calls stub page RX: %s\n", strerror(errno));
		abort();
	}
	__builtin___clear_cache((char*)g_elfcall_stub_page, (char*)g_elfcall_stub_page + page_sz);
}

static void init_dlsym_stubs(void)
{
	size_t page_sz = (size_t)sysconf(_SC_PAGESIZE);
	size_t needed = DARLING_FEX_DLSYM_MAX_SLOTS * 8;
	size_t area_size = (needed + page_sz - 1) & ~(page_sz - 1);

	g_dlsym_stub_area = mmap(NULL, area_size, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (g_dlsym_stub_area == MAP_FAILED) {
		g_dlsym_stub_area = NULL;
		fprintf(stderr, "mldr/fex: failed to mmap dlsym stub area: %s\n", strerror(errno));
		abort();
	}

	for (uint32_t i = 0; i < DARLING_FEX_DLSYM_MAX_SLOTS; i++)
		write_x86_64_syscall_stub(g_dlsym_stub_area + (i * 8), DARLING_FEX_DLSYM_SYSCALL_BASE + i);

	if (mprotect(g_dlsym_stub_area, area_size, PROT_READ | PROT_EXEC) != 0) {
		fprintf(stderr, "mldr/fex: failed to mprotect dlsym stub area RX: %s\n", strerror(errno));
		abort();
	}
	__builtin___clear_cache((char*)g_dlsym_stub_area, (char*)g_dlsym_stub_area + area_size);
}

struct elf_calls* fex_elfcalls_get_trampoline_table(const struct elf_calls* host_calls)
{
	if (host_calls)
		g_host_elfcalls = *host_calls;
	pthread_once(&g_elfcall_init_once, init_elfcall_stubs);
	return &g_trampoline_elfcalls;
}

static unsigned int host_cb_thread_self_trap(void)
{
	darling_thread_create_callbacks_t cbs = __atomic_load_n(&g_guest_thread_cbs, __ATOMIC_ACQUIRE);
	if (!cbs || !cbs->thread_self_trap)
		return 0;
	return (unsigned int)fex_invoke_guest_callback(
		(uintptr_t)cbs->thread_self_trap, 0, 0);
}

static void host_cb_thread_set_tsd_base(void* tsd_base, int unk)
{
	fex_set_current_thread_gs_base((uint64_t)(uintptr_t)tsd_base);
	darling_thread_create_callbacks_t cbs = __atomic_load_n(&g_guest_thread_cbs, __ATOMIC_ACQUIRE);
	if (cbs && cbs->thread_set_tsd_base) {
		fex_invoke_guest_callback(
			(uintptr_t)cbs->thread_set_tsd_base,
			(uint64_t)(uintptr_t)tsd_base,
			(uint64_t)(uint32_t)unk);
	}
}

static void host_cb_rpc_guard(int fd)
{
	darling_thread_create_callbacks_t cbs = __atomic_load_n(&g_guest_thread_cbs, __ATOMIC_ACQUIRE);
	if (cbs && cbs->rpc_guard) {
		fex_invoke_guest_callback(
			(uintptr_t)cbs->rpc_guard,
			(uint64_t)(uint32_t)fd, 0);
	}
}

static void host_cb_rpc_unguard(int fd)
{
	darling_thread_create_callbacks_t cbs = __atomic_load_n(&g_guest_thread_cbs, __ATOMIC_ACQUIRE);
	if (cbs && cbs->rpc_unguard) {
		fex_invoke_guest_callback(
			(uintptr_t)cbs->rpc_unguard,
			(uint64_t)(uint32_t)fd, 0);
	}
}

static const struct darling_thread_create_callbacks g_wrapped_thread_cbs = {
	.thread_self_trap = host_cb_thread_self_trap,
	.thread_set_tsd_base = host_cb_thread_set_tsd_base,
	.rpc_guard = host_cb_rpc_guard,
	.rpc_unguard = host_cb_rpc_unguard,
};

darling_thread_create_callbacks_t fex_wrap_guest_thread_callbacks(darling_thread_create_callbacks_t guest_cbs)
{
	__atomic_store_n(&g_guest_thread_cbs, guest_cbs, __ATOMIC_RELEASE);
	return &g_wrapped_thread_cbs;
}

void* fex_dlsym_wrap_if_func(void* host_sym, const char* symbol_name)
{
	(void)symbol_name;
	if (!host_sym)
		return NULL;

	Dl_info info;
	const Elf64_Sym* elf_sym = NULL;
	if (dladdr1(host_sym, &info, (void**)&elf_sym, RTLD_DL_SYMENT) != 0 && elf_sym != NULL) {
		if (ELF64_ST_TYPE(elf_sym->st_info) == STT_OBJECT)
			return host_sym;
	}

	pthread_once(&g_dlsym_init_once, init_dlsym_stubs);

	pthread_mutex_lock(&g_dlsym_mutex);

	for (uint32_t i = 0; i < g_dlsym_count; i++) {
		if (g_dlsym_targets[i] == host_sym) {
			void* existing = g_dlsym_stub_area + (i * 8);
			pthread_mutex_unlock(&g_dlsym_mutex);
			return existing;
		}
	}

	if (g_dlsym_count >= DARLING_FEX_DLSYM_MAX_SLOTS) {
		pthread_mutex_unlock(&g_dlsym_mutex);
		fprintf(stderr, "mldr/fex: dlsym stub table exhausted (%u slots)\n", DARLING_FEX_DLSYM_MAX_SLOTS);
		abort();
	}

	uint32_t slot = g_dlsym_count++;
	g_dlsym_targets[slot] = host_sym;
	void* stub = g_dlsym_stub_area + (slot * 8);

	pthread_mutex_unlock(&g_dlsym_mutex);
	return stub;
}

long fex_dlsym_dispatch(uint32_t slot, struct fex_x86_64_state* state)
{
	if (!state)
		return -EINVAL;

	pthread_mutex_lock(&g_dlsym_mutex);
	void* target = (slot < g_dlsym_count) ? g_dlsym_targets[slot] : NULL;
	pthread_mutex_unlock(&g_dlsym_mutex);

	if (!target)
		return -ENOSYS;

	uint64_t s0 = 0, s1 = 0, s2 = 0, s3 = 0;
	if (state->rsp != 0) {
		const uint64_t* sp = (const uint64_t*)(uintptr_t)state->rsp;
		s0 = sp[1];
		s1 = sp[2];
		s2 = sp[3];
		s3 = sp[4];
	}

#if defined(__aarch64__)
	double v0, v1, v2, v3, v4, v5, v6, v7;
	memcpy(&v0, &state->xmm[0][0], sizeof(double));
	memcpy(&v1, &state->xmm[1][0], sizeof(double));
	memcpy(&v2, &state->xmm[2][0], sizeof(double));
	memcpy(&v3, &state->xmm[3][0], sizeof(double));
	memcpy(&v4, &state->xmm[4][0], sizeof(double));
	memcpy(&v5, &state->xmm[5][0], sizeof(double));
	memcpy(&v6, &state->xmm[6][0], sizeof(double));
	memcpy(&v7, &state->xmm[7][0], sizeof(double));

	register uint64_t x0 asm("x0") = state->rdi;
	register uint64_t x1 asm("x1") = state->rsi;
	register uint64_t x2 asm("x2") = state->rdx;
	register uint64_t x3 asm("x3") = state->rcx;
	register uint64_t x4 asm("x4") = state->r8;
	register uint64_t x5 asm("x5") = state->r9;
	register uint64_t x6 asm("x6") = s0;
	register uint64_t x7 asm("x7") = s1;
	register double d0 asm("d0") = v0;
	register double d1 asm("d1") = v1;
	register double d2 asm("d2") = v2;
	register double d3 asm("d3") = v3;
	register double d4 asm("d4") = v4;
	register double d5 asm("d5") = v5;
	register double d6 asm("d6") = v6;
	register double d7 asm("d7") = v7;

	__asm__ volatile(
		"sub sp, sp, #16\n\t"
		"stp %[s2], %[s3], [sp]\n\t"
		"blr %[fn]\n\t"
		"add sp, sp, #16\n\t"
		: "+r"(x0), "+r"(x1), "+w"(d0), "+w"(d1), "+w"(d2), "+w"(d3),
		  "+w"(d4), "+w"(d5), "+w"(d6), "+w"(d7)
		: "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x6), "r"(x7),
		  [s2] "r"(s2), [s3] "r"(s3), [fn] "r"(target)
		: "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
		  "x16", "x17", "x18", "x30", "cc", "memory"
	);

	state->rax = x0;
	state->rdx = x1;
	v0 = d0;
	memcpy(&state->xmm[0][0], &v0, sizeof(double));
	long ret = (long)x0;
#else
	typedef uint64_t (*host_fn_t)(uint64_t, uint64_t, uint64_t, uint64_t,
		uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
	uint64_t ret_val = ((host_fn_t)target)(state->rdi, state->rsi, state->rdx, state->rcx,
		state->r8, state->r9, s0, s1, s2, s3);
	state->rax = ret_val;
	long ret = (long)ret_val;
#endif

	if (t_deferred_terminate.pending) {
		state->unwind_thread = 1;
	}
	return ret;
}

long fex_elfcalls_dispatch(uint32_t index, struct fex_x86_64_state* state)
{
	if (!state || index >= DARLING_FEX_ELFCALL_COUNT)
		return -ENOSYS;

	uint64_t ret = 0;
	switch (index) {
		case 0: // dlopen
			ret = (uintptr_t)g_host_elfcalls.dlopen((const char*)(uintptr_t)state->rdi);
			break;
		case 1: // dlclose
			ret = (uint64_t)(int64_t)g_host_elfcalls.dlclose((void*)(uintptr_t)state->rdi);
			break;
		case 2: { // dlsym
			void* sym = g_host_elfcalls.dlsym((void*)(uintptr_t)state->rdi, (const char*)(uintptr_t)state->rsi);
			ret = (uintptr_t)fex_dlsym_wrap_if_func(sym, (const char*)(uintptr_t)state->rsi);
			break;
		}
		case 3: // dlerror
			ret = (uintptr_t)g_host_elfcalls.dlerror();
			break;
		case 4: { // darling_thread_create (9 arguments: 6 in rdi..r9, 3 at rsp+8..rsp+24)
			const uint64_t* sp = (const uint64_t*)(uintptr_t)state->rsp;
			uintptr_t arg6 = sp ? (uintptr_t)sp[1] : 0;
			darling_thread_create_callbacks_t guest_cbs =
				sp ? (darling_thread_create_callbacks_t)(uintptr_t)sp[2] : NULL;
			void* dthread = sp ? (void*)(uintptr_t)sp[3] : NULL;
			darling_thread_create_callbacks_t wrapped_cbs =
				guest_cbs ? fex_wrap_guest_thread_callbacks(guest_cbs) : NULL;
			ret = (uintptr_t)g_host_elfcalls.darling_thread_create(
				(unsigned long)state->rdi,
				(unsigned long)state->rsi,
				(void*)(uintptr_t)state->rdx,
				(uintptr_t)state->rcx,
				(uintptr_t)state->r8,
				(uintptr_t)state->r9,
				arg6,
				wrapped_cbs,
				dthread);
			break;
		}
		case 5: // darling_thread_terminate (deferred until FEX thread cleanup runs)
			t_deferred_terminate.pending = true;
			t_deferred_terminate.stackaddr = (void*)(uintptr_t)state->rdi;
			t_deferred_terminate.freesize = (unsigned long)state->rsi;
			t_deferred_terminate.pthobj_size = (unsigned long)state->rdx;
			state->unwind_thread = 1;
			ret = 0;
			break;
		case 6: // darling_thread_get_stack
			ret = (uintptr_t)g_host_elfcalls.darling_thread_get_stack();
			break;
		case 7: // dlopen_fatal
			ret = (uintptr_t)g_host_elfcalls.dlopen_fatal((const char*)(uintptr_t)state->rdi);
			break;
		case 8: // dlclose_fatal
			ret = (uint64_t)(int64_t)g_host_elfcalls.dlclose_fatal((void*)(uintptr_t)state->rdi);
			break;
		case 9: { // dlsym_fatal
			void* sym = g_host_elfcalls.dlsym_fatal((void*)(uintptr_t)state->rdi, (const char*)(uintptr_t)state->rsi);
			ret = (uintptr_t)fex_dlsym_wrap_if_func(sym, (const char*)(uintptr_t)state->rsi);
			break;
		}
		case 10: // get_errno
			ret = (uint64_t)(int64_t)g_host_elfcalls.get_errno();
			break;
		case 11: // sem_open
			ret = (uintptr_t)g_host_elfcalls.sem_open(
				(const char*)(uintptr_t)state->rdi,
				(int)state->rsi,
				(unsigned short)state->rdx,
				(unsigned int)state->rcx);
			break;
		case 12: // sem_wait
			ret = (uint64_t)(int64_t)g_host_elfcalls.sem_wait((int*)(uintptr_t)state->rdi);
			break;
		case 13: // sem_trywait
			ret = (uint64_t)(int64_t)g_host_elfcalls.sem_trywait((int*)(uintptr_t)state->rdi);
			break;
		case 14: // sem_post
			ret = (uint64_t)(int64_t)g_host_elfcalls.sem_post((int*)(uintptr_t)state->rdi);
			break;
		case 15: // sem_close
			ret = (uint64_t)(int64_t)g_host_elfcalls.sem_close((int*)(uintptr_t)state->rdi);
			break;
		case 16: // sem_unlink
			ret = (uint64_t)(int64_t)g_host_elfcalls.sem_unlink((const char*)(uintptr_t)state->rdi);
			break;
		case 17: // shm_open
			ret = (uint64_t)(int64_t)g_host_elfcalls.shm_open(
				(const char*)(uintptr_t)state->rdi,
				(int)state->rsi,
				(unsigned short)state->rdx);
			break;
		case 18: // shm_unlink
			ret = (uint64_t)(int64_t)g_host_elfcalls.shm_unlink((const char*)(uintptr_t)state->rdi);
			break;
		case 19: // exit
			g_host_elfcalls.exit((int)state->rdi);
			break;
		case 20: // malloc
			ret = (uintptr_t)g_host_elfcalls.malloc((size_t)state->rdi);
			break;
		case 21: // free
			g_host_elfcalls.free((void*)(uintptr_t)state->rdi);
			break;
		case 22: // realloc
			ret = (uintptr_t)g_host_elfcalls.realloc((void*)(uintptr_t)state->rdi, (size_t)state->rsi);
			break;
		case 23: // sysconf
			ret = (uint64_t)g_host_elfcalls.sysconf((int)state->rdi);
			break;
		case 24: // dserver_socket_address
			ret = (uintptr_t)g_host_elfcalls.dserver_socket_address();
			break;
		case 25: // dserver_per_thread_socket
			ret = (uint64_t)(int64_t)g_host_elfcalls.dserver_per_thread_socket();
			break;
		case 26: // dserver_per_thread_socket_refresh
			g_host_elfcalls.dserver_per_thread_socket_refresh();
			break;
		case 27: // dserver_close_socket
			g_host_elfcalls.dserver_close_socket((int)state->rdi);
			break;
		case 28: // dserver_get_process_lifetime_pipe
			ret = (uint64_t)(int64_t)g_host_elfcalls.dserver_get_process_lifetime_pipe();
			break;
		case 29: // dserver_process_lifetime_pipe_refresh
			ret = (uint64_t)(int64_t)g_host_elfcalls.dserver_process_lifetime_pipe_refresh();
			break;
		case 30: // dserver_close_process_lifetime_pipe
			g_host_elfcalls.dserver_close_process_lifetime_pipe((int)state->rdi);
			break;
		case 31: { // native_fork
			pthread_mutex_lock(&g_dlsym_mutex);
			if (g_fork_lock_hook) {
				g_fork_lock_hook();
			}
			int fork_ret = g_host_elfcalls.native_fork ? g_host_elfcalls.native_fork() : -ENOSYS;
			bool is_child = (fork_ret == 0);
			if (g_fork_unlock_hook) {
				g_fork_unlock_hook(is_child);
			}
			pthread_mutex_unlock(&g_dlsym_mutex);
			ret = (uint64_t)(int64_t)fork_ret;
			break;
		}
		case 32: // inherited_nofile_limit
			ret = (uint64_t)(int64_t)g_host_elfcalls.inherited_nofile_limit(
				(unsigned int*)(uintptr_t)state->rdi);
			break;
		default:
			return -ENOSYS;
	}

	state->rax = ret;
	if (t_deferred_terminate.pending) {
		state->unwind_thread = 1;
	}
	return (long)ret;
}
