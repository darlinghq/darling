#include "fex_bridge.h"
#include "../loader.h"
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

#define FEXCORE_MAX_CANDIDATES 5
#define FEXCORE_PATH_MAX 4096

static darling_fexcore_run_thread_fn g_run_thread_fn = nullptr;
static pthread_once_t g_fexcore_init_once = PTHREAD_ONCE_INIT;
static uint8_t* g_initial_gs_zero_page = nullptr;
static uint8_t* g_callback_return_stub = nullptr;

static void add_candidate(const char* path, char* buf, size_t buf_size, size_t* used,
	size_t* count, size_t max_paths, const char** out)
{
	if (*count >= max_paths) {
		return;
	}
	size_t len = strlen(path);
	if (*used + len + 1 > buf_size) {
		return;
	}
	char* dst = buf + *used;
	memcpy(dst, path, len + 1);
	out[*count] = dst;
	*used += len + 1;
	(*count)++;
}

// Fills `out` with the paths to try, in order, writing their text into `buf`; the
// entries alias `buf` and are only valid until the next call. An explicit
// DARLING_FEXCORE_LIB overrides discovery entirely: falling back to an installed
// copy would hide a broken override.
extern "C" size_t fexcore_candidate_paths(char* buf, size_t buf_size, size_t max_paths,
	const char** out, const char* self_exe)
{
	if (!buf || !out || max_paths == 0) {
		return 0;
	}
	size_t used = 0;
	size_t count = 0;

	const char* override_path = getenv("DARLING_FEXCORE_LIB");
	if (override_path && override_path[0]) {
		add_candidate(override_path, buf, buf_size, &used, &count, max_paths, out);
		return count;
	}

	if (self_exe && self_exe[0]) {
		char dir[FEXCORE_PATH_MAX];
		size_t len = strlen(self_exe);
		if (len < sizeof(dir)) {
			memcpy(dir, self_exe, len + 1);
			char* slash = strrchr(dir, '/');
			if (slash) {
				*slash = '\0';
			} else {
				memcpy(dir, ".", 2);
			}

			// mldr is installed both as <prefix>/libexec/darling/bin/mldr and as
			// <prefix>/libexec/darling/usr/libexec/darling/mldr, so the shim sits one and
			// three levels up from those directories respectively.
			char candidate[FEXCORE_PATH_MAX];
			int written = snprintf(candidate, sizeof(candidate), "%s/../lib/libdarling-fexcore.so", dir);
			if (written >= 0 && static_cast<size_t>(written) < sizeof(candidate)) {
				add_candidate(candidate, buf, buf_size, &used, &count, max_paths, out);
			}
			written = snprintf(candidate, sizeof(candidate),
				"%s/../../../lib/libdarling-fexcore.so", dir);
			if (written >= 0 && static_cast<size_t>(written) < sizeof(candidate)) {
				add_candidate(candidate, buf, buf_size, &used, &count, max_paths, out);
			}
		}
	}

	add_candidate(INSTALL_PREFIX "/libexec/darling/lib/libdarling-fexcore.so",
		buf, buf_size, &used, &count, max_paths, out);
	add_candidate("libFEXCore.so", buf, buf_size, &used, &count, max_paths, out);
	return count;
}

enum candidate_state {
	CANDIDATE_UNUSED,
	CANDIDATE_LOAD_FAILED,
	CANDIDATE_SYMBOL_MISSING,
};

struct candidate_result {
	enum candidate_state state;
	char reason[256];
};

static void init_fexcore_runtime(void)
{
	char self_exe[FEXCORE_PATH_MAX] = "";
	ssize_t self_len = readlink("/proc/self/exe", self_exe, sizeof(self_exe) - 1);
	if (self_len > 0) {
		self_exe[self_len] = '\0';
	}

	char buf[FEXCORE_MAX_CANDIDATES * FEXCORE_PATH_MAX];
	const char* paths[FEXCORE_MAX_CANDIDATES];
	size_t count = fexcore_candidate_paths(buf, sizeof(buf), FEXCORE_MAX_CANDIDATES, paths,
		self_len > 0 ? self_exe : nullptr);

	// A missing candidate is routine: only one of the discovery paths matches an
	// install layout. Diagnostics are held back and printed only when the search
	// fails or when a library that exists cannot be used.
	struct candidate_result results[FEXCORE_MAX_CANDIDATES] = {};
	size_t first_broken = 0;

	for (size_t i = 0; i < count; i++) {
		void* handle = dlopen(paths[i], RTLD_NOW | RTLD_LOCAL);
		if (!handle) {
			results[i].state = CANDIDATE_LOAD_FAILED;
			snprintf(results[i].reason, sizeof(results[i].reason), "%s", dlerror());
			continue;
		}
		void* sym = dlsym(handle, "darling_fexcore_run_thread");
		void* lock_sym = dlsym(handle, "darling_fexcore_lock_before_fork");
		void* unlock_sym = dlsym(handle, "darling_fexcore_unlock_after_fork");
		if (!sym || !lock_sym || !unlock_sym) {
			results[i].state = CANDIDATE_SYMBOL_MISSING;
			snprintf(results[i].reason, sizeof(results[i].reason), "%s", dlerror());
			dlclose(handle);
			continue;
		}
		g_run_thread_fn = reinterpret_cast<darling_fexcore_run_thread_fn>(sym);
		fex_elfcalls_set_fork_hooks(
			reinterpret_cast<fex_fork_lock_fn_t>(lock_sym),
			reinterpret_cast<fex_fork_unlock_fn_t>(unlock_sym));
		break;
	}

	if (!g_run_thread_fn) {
		for (size_t i = 0; i < count; i++) {
			if (results[i].state == CANDIDATE_LOAD_FAILED) {
				fprintf(stderr, "[mldr/fex] cannot load %s: %s\n", paths[i], results[i].reason);
			} else if (results[i].state == CANDIDATE_SYMBOL_MISSING) {
				fprintf(stderr, "[mldr/fex] cannot use %s: %s\n", paths[i], results[i].reason);
			}
		}
		fprintf(stderr,
			"[mldr/fex] fatal: no FEXCore library found; cannot execute an x86_64 Mach-O binary on aarch64\n");
		exit(1);
	}

	while (first_broken < count && results[first_broken].state != CANDIDATE_SYMBOL_MISSING) {
		first_broken++;
	}
	if (first_broken < count) {
		fprintf(stderr, "[mldr/fex] skipped %s: %s\n", paths[first_broken],
			results[first_broken].reason);
	}

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

uint64_t fex_invoke_guest_callback_args(uintptr_t guest_fn, const uint64_t args[6])
{
	darling_fexcore_run_thread_fn run_fn = ensure_fexcore_loaded();

	// A guest callback can be as heavy as an error handler that logs, so it gets a real stack, not a few
	// kilobytes of the host thread's.
	constexpr size_t kCallbackStackSize = 512 * 1024;
	void* cb_stack = mmap(nullptr, kCallbackStackSize, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	if (cb_stack == MAP_FAILED) {
		fprintf(stderr, "[mldr/fex] fatal: failed to mmap a callback stack: %s\n", strerror(errno));
		abort();
	}
	uintptr_t sp = reinterpret_cast<uintptr_t>(cb_stack) + kCallbackStackSize;
	sp &= ~uintptr_t(15);
	sp -= sizeof(uint64_t);
	*reinterpret_cast<uint64_t*>(sp) = reinterpret_cast<uintptr_t>(g_callback_return_stub);

	struct fex_x86_64_state state;
	memset(&state, 0, sizeof(state));
	state.rip = static_cast<uint64_t>(guest_fn);
	state.rsp = static_cast<uint64_t>(sp);
	state.rdi = args[0];
	state.rsi = args[1];
	state.rdx = args[2];
	state.rcx = args[3];
	state.r8 = args[4];
	state.r9 = args[5];
	state.rflags = 0x202;
	state.gs_base = fex_get_current_thread_gs_base();

	run_fn(&state, fex_linux_syscall_dispatch);
	fex_set_current_thread_gs_base(state.gs_base);
	munmap(cb_stack, kCallbackStackSize);
	return state.rax;
}

uint64_t fex_invoke_guest_callback(uintptr_t guest_fn, uint64_t arg0, uint64_t arg1)
{
	const uint64_t args[6] = {arg0, arg1, 0, 0, 0, 0};
	return fex_invoke_guest_callback_args(guest_fn, args);
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
	fex_elfcalls_flush_deferred_terminate();
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
	fex_elfcalls_flush_deferred_terminate();
	_exit(rc);
}

} // extern "C"
