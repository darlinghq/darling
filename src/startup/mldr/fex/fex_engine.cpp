#include "fex_bridge.h"
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

extern "C" {

typedef int (*darling_fexcore_run_thread_fn)(
	struct fex_x86_64_state* state,
	fex_syscall_cb_t syscall_cb);

static darling_fexcore_run_thread_fn g_run_thread_fn = nullptr;
static pthread_once_t g_fexcore_init_once = PTHREAD_ONCE_INIT;
static uint8_t* g_initial_gs_zero_page = nullptr;
static uint8_t* g_callback_return_stub = nullptr;

static void init_fexcore_runtime(void)
{
	const char* override_path = getenv("DARLING_FEXCORE_LIB");
	const char* lib_path = (override_path && override_path[0]) ? override_path : "libFEXCore.so";

	void* handle = dlopen(lib_path, RTLD_NOW | RTLD_LOCAL);
	if (!handle) {
		fprintf(stderr,
			"[mldr/fex] fatal: %s not found (%s); cannot execute x86_64 Mach-O binary on aarch64\n",
			lib_path, dlerror());
		exit(1);
	}

	void* sym = dlsym(handle, "darling_fexcore_run_thread");
	if (!sym) {
		fprintf(stderr,
			"[mldr/fex] fatal: darling_fexcore_run_thread symbol missing in %s (%s)\n",
			lib_path, dlerror());
		exit(1);
	}

	g_run_thread_fn = reinterpret_cast<darling_fexcore_run_thread_fn>(sym);

	size_t page_sz = static_cast<size_t>(sysconf(_SC_PAGESIZE));
	void* zero_page = mmap(nullptr, page_sz, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (zero_page == MAP_FAILED) {
		fprintf(stderr, "[mldr/fex] fatal: failed to mmap initial gs zero page: %s\n", strerror(errno));
		exit(1);
	}
	g_initial_gs_zero_page = static_cast<uint8_t*>(zero_page);

	uint8_t* ret_stub = static_cast<uint8_t*>(mmap(nullptr, page_sz, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
	if (ret_stub == MAP_FAILED) {
		fprintf(stderr, "[mldr/fex] fatal: failed to mmap callback return stub: %s\n", strerror(errno));
		exit(1);
	}
	uint32_t ret_nr = DARLING_FEX_CALLBACK_RETURN_SYSCALL;
	ret_stub[0] = 0x48; // movq %rax, %rdi
	ret_stub[1] = 0x89;
	ret_stub[2] = 0xc7;
	ret_stub[3] = 0xb8; // movl $DARLING_FEX_CALLBACK_RETURN_SYSCALL, %eax
	memcpy(&ret_stub[4], &ret_nr, sizeof(ret_nr));
	ret_stub[8] = 0x0f; // syscall
	ret_stub[9] = 0x05;
	if (mprotect(ret_stub, page_sz, PROT_READ | PROT_EXEC) != 0) {
		fprintf(stderr, "[mldr/fex] fatal: failed to mprotect callback return stub RX: %s\n", strerror(errno));
		exit(1);
	}
	__builtin___clear_cache(reinterpret_cast<char*>(ret_stub), reinterpret_cast<char*>(ret_stub) + page_sz);
	g_callback_return_stub = ret_stub;
}

static darling_fexcore_run_thread_fn ensure_fexcore_loaded(void)
{
	pthread_once(&g_fexcore_init_once, init_fexcore_runtime);
	return g_run_thread_fn;
}

uint64_t fex_invoke_guest_callback(uintptr_t guest_fn, uint64_t arg0, uint64_t arg1)
{
	darling_fexcore_run_thread_fn run_fn = ensure_fexcore_loaded();

	alignas(16) uint8_t cb_stack[16384];
	uintptr_t sp = reinterpret_cast<uintptr_t>(cb_stack + sizeof(cb_stack));
	sp &= ~uintptr_t(15);
	sp -= sizeof(uint64_t);
	*reinterpret_cast<uint64_t*>(sp) = reinterpret_cast<uintptr_t>(g_callback_return_stub);

	struct fex_x86_64_state state;
	memset(&state, 0, sizeof(state));
	state.rip = static_cast<uint64_t>(guest_fn);
	state.rsp = static_cast<uint64_t>(sp);
	state.rdi = arg0;
	state.rsi = arg1;
	state.rflags = 0x202;
	state.gs_base = fex_get_current_thread_gs_base();

	run_fn(&state, fex_linux_syscall_dispatch);
	fex_set_current_thread_gs_base(state.gs_base);
	return state.rax;
}

void fex_start_main_thread(struct load_results* lr)
{
	darling_fexcore_run_thread_fn run_fn = ensure_fexcore_loaded();

	uint64_t gs_base = reinterpret_cast<uint64_t>(g_initial_gs_zero_page);
	fex_set_current_thread_gs_base(gs_base);

	struct fex_x86_64_state state;
	memset(&state, 0, sizeof(state));
	state.rip = static_cast<uint64_t>(lr->entry_point);
	state.rsp = static_cast<uint64_t>(lr->stack_top);
	state.rflags = 0x202;
	state.gs_base = gs_base;

	int rc = run_fn(&state, fex_linux_syscall_dispatch);
	_exit(rc);
}

void fex_start_darling_thread(
	void* pth,
	int port,
	uintptr_t real_entry,
	uintptr_t arg1,
	uintptr_t arg2,
	uintptr_t arg3,
	uintptr_t entry_point,
	uintptr_t stack_ptr,
	uintptr_t gs_base)
{
	darling_fexcore_run_thread_fn run_fn = ensure_fexcore_loaded();

	uintptr_t rsp = (stack_ptr & ~uintptr_t(15)) - sizeof(uint64_t);
	*reinterpret_cast<uint64_t*>(rsp) = 0;

	fex_set_current_thread_gs_base(static_cast<uint64_t>(gs_base));

	struct fex_x86_64_state state;
	memset(&state, 0, sizeof(state));
	state.rdi = reinterpret_cast<uint64_t>(pth);
	state.rsi = static_cast<uint64_t>(static_cast<uint32_t>(port));
	state.rdx = static_cast<uint64_t>(real_entry);
	state.rcx = static_cast<uint64_t>(arg1);
	state.r8  = static_cast<uint64_t>(arg2);
	state.r9  = static_cast<uint64_t>(arg3);
	state.rip = static_cast<uint64_t>(entry_point);
	state.rsp = static_cast<uint64_t>(rsp);
	state.rflags = 0x202;
	state.gs_base = static_cast<uint64_t>(gs_base);

	int rc = run_fn(&state, fex_linux_syscall_dispatch);
	_exit(rc);
}

} // extern "C"
