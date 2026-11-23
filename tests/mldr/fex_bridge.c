#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <ucontext.h>
#include <unistd.h>
#include "fex/fex_bridge.h"
#include "fex/fex_callback_frames.h"

static int failures = 0;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// The fake guest functions of these tests lie in [0x4000000, 0x7000000). A host function that is handed
// one instead of a thunk must report it rather than call it natively, which would crash the test.
static bool is_fake_guest_fn(uint64_t fn)
{
	return fn >= 0x4000000 && fn < 0x7000000;
}

// Runs fn in a child process; true if the child died of SIGABRT after writing `message` to stderr.
static bool aborts_with(void (*fn)(void), const char* message)
{
	int err[2];
	if (pipe(err) != 0)
		return false;
	fflush(stdout);
	pid_t pid = fork();
	if (pid == 0) {
		dup2(err[1], 2);
		fn();
		_exit(0);
	}
	close(err[1]);
	char msg[512] = "";
	size_t len = 0;
	ssize_t n;
	while (len < sizeof(msg) - 1 && (n = read(err[0], msg + len, sizeof(msg) - 1 - len)) > 0)
		len += (size_t)n;
	msg[len] = '\0';
	close(err[0]);
	int status = 0;
	waitpid(pid, &status, 0);
	return WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT && strstr(msg, message) != NULL;
}

// Each bridge stub is "mov r10, rcx" (3 bytes), then "mov eax, <number>".
static uint64_t stub_syscall_nr(void* stub)
{
	uint32_t nr = 0;
	memcpy(&nr, (const uint8_t*)stub + 3 + 1, sizeof(nr));
	return nr;
}

static uintptr_t last_cb_fn = 0;
static uint64_t last_cb_arg0 = 0;
static uint64_t last_cb_arg1 = 0;
// What a test's fake guest function does while it runs.
static void (*guest_callback_body)(uint64_t arg0, uint64_t arg1) = NULL;

uint64_t fex_invoke_guest_callback(uintptr_t guest_fn, uint64_t arg0, uint64_t arg1)
{
	last_cb_fn = guest_fn;
	last_cb_arg0 = arg0;
	last_cb_arg1 = arg1;
	if (guest_callback_body)
		guest_callback_body(arg0, arg1);
	return 0x77;
}

static uintptr_t last_cb_args_fn = 0;
static uint64_t last_cb_args[6];

uint64_t fex_invoke_guest_callback_args(uintptr_t guest_fn, const uint64_t args[6])
{
	last_cb_args_fn = guest_fn;
	memcpy(last_cb_args, args, sizeof(last_cb_args));
	return 0x88;
}

static sigjmp_buf jb;
static void on_segv(int sig)
{
	(void)sig;
	siglongjmp(jb, 1);
}

static bool write_faults(volatile uint8_t* p)
{
	struct sigaction sa, old;
	bool faulted = false;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_segv;
	sigaction(SIGSEGV, &sa, &old);
	if (sigsetjmp(jb, 1) == 0)
		*p = *p ^ 0xff;
	else
		faulted = true;
	sigaction(SIGSEGV, &old, NULL);
	return faulted;
}

static unsigned long recorded_thread_args[7];
static darling_thread_create_callbacks_t recorded_thread_cbs;
static void* recorded_thread_dthread;

static void* mock_thread_create(unsigned long stack_size, unsigned long pthobj_size,
	void* entry_point, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
	darling_thread_create_callbacks_t callbacks, void* dthread)
{
	recorded_thread_args[0] = stack_size;
	recorded_thread_args[1] = pthobj_size;
	recorded_thread_args[2] = (unsigned long)(uintptr_t)entry_point;
	recorded_thread_args[3] = arg3;
	recorded_thread_args[4] = arg4;
	recorded_thread_args[5] = arg5;
	recorded_thread_args[6] = arg6;
	recorded_thread_cbs = callbacks;
	recorded_thread_dthread = dthread;
	return (void*)0xdeadbeefUL;
}

static void* recorded_term_stackaddr = NULL;
static unsigned long recorded_term_freesize = 0;
static unsigned long recorded_term_pthobj_size = 0;
static int recorded_term_call_count = 0;

static int mock_thread_terminate(void* stackaddr, unsigned long freesize, unsigned long pthobj_size)
{
	recorded_term_stackaddr = stackaddr;
	recorded_term_freesize = freesize;
	recorded_term_pthobj_size = pthobj_size;
	recorded_term_call_count++;
	return 0;
}

static void* mock_thread_get_stack(void)
{
	return (void*)0x7fff00002000ULL;
}

static char g_fork_seq[128] = "";
static bool g_fork_last_is_child = false;
static int g_fork_stub_ret = 4242;

static void test_fork_lock_hook(void)
{
	strcat(g_fork_seq, "lock;");
}

static void test_fork_unlock_hook(bool is_child)
{
	g_fork_last_is_child = is_child;
	strcat(g_fork_seq, is_child ? "unlock(child);" : "unlock(parent);");
}

static int mock_native_fork(void)
{
	strcat(g_fork_seq, "fork;");
	return g_fork_stub_ret;
}

static int mock_inherited_nofile_limit(unsigned int limits[2])
{
	limits[0] = 256;
	limits[1] = 4096;
	return 0;
}

static double host_math_Add8(
	uint64_t a, uint64_t b, uint64_t c, uint64_t d,
	uint64_t e, uint64_t f, uint64_t g, uint64_t h,
	double x, double y)
{
	uint64_t sum = a + b + c + d + e + f + g + h;
	return (double)sum + (x * y);
}

static uint64_t host_terminating_function(void)
{
	struct fex_x86_64_state term_st;
	memset(&term_st, 0, sizeof(term_st));
	term_st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 5;
	term_st.rdi = 0x7fff00003000ULL;
	term_st.rsi = 0x8000;
	term_st.rdx = 0x400;
	fex_linux_syscall_dispatch(&term_st);
	return 999;
}

static void* child_cb_worker(void* arg)
{
	const struct darling_thread_create_callbacks* cbs =
		(const struct darling_thread_create_callbacks*)arg;
	cbs->thread_set_tsd_base((void*)0x12345000UL, 9);
	return (void*)(uintptr_t)fex_get_current_thread_gs_base();
}

static void* mock_dlopen(const char* name)
{
	return dlopen(name, RTLD_LAZY);
}

static void test_elfcalls_trampolines(void)
{
	struct elf_calls host_calls;
	memset(&host_calls, 0, sizeof(host_calls));
	host_calls.dlopen = mock_dlopen;
	host_calls.dlsym = dlsym;
	host_calls.darling_thread_create = mock_thread_create;
	host_calls.darling_thread_terminate = mock_thread_terminate;
	host_calls.darling_thread_get_stack = mock_thread_get_stack;
	host_calls.native_fork = mock_native_fork;
	host_calls.inherited_nofile_limit = mock_inherited_nofile_limit;

	struct elf_calls* tramp = fex_elfcalls_get_trampoline_table(&host_calls);
	void** slots = (void**)tramp;

	bool all_valid = true;
	for (uint32_t i = 0; i < DARLING_FEX_ELFCALL_COUNT; i++) {
		const uint8_t* code = (const uint8_t*)slots[i];
		uint32_t imm = 0;
		memcpy(&imm, &code[4], sizeof(imm));
		if (code[0] != 0x49 || code[1] != 0x89 || code[2] != 0xca || code[3] != 0xb8 ||
		    imm != DARLING_FEX_ELFCALL_SYSCALL_BASE + i ||
		    code[8] != 0x0f || code[9] != 0x05 || code[10] != 0xc3) {
			all_valid = false;
			break;
		}
	}
	check(all_valid, "all 33 elf_calls stubs encode movq %rcx, %r10; movl $nr, %eax; syscall; ret");
	check(write_faults((volatile uint8_t*)slots[0]), "elf_calls stub page is W^X (writes fault)");

	struct darling_thread_create_callbacks guest_cbs = {
		.thread_self_trap = (unsigned int (*)(void))0x100100UL,
		.thread_set_tsd_base = (void (*)(void*, int))0x100200UL,
		.rpc_guard = (void (*)(int))0x100300UL,
		.rpc_unguard = (void (*)(int))0x100400UL,
	};
	uint64_t fake_stack[4] = {
		0, // return address at [rsp]
		77, // arg6 at [rsp+8]
		(uint64_t)(uintptr_t)&guest_cbs, // callbacks at [rsp+16]
		0x99887766UL, // dthread at [rsp+24]
	};
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 4;
	st.rdi = 11;
	st.rsi = 22;
	st.rdx = 33;
	st.r10 = 44;
	st.r8 = 55;
	st.r9 = 66;
	st.rsp = (uint64_t)(uintptr_t)fake_stack;

	long ret = fex_linux_syscall_dispatch(&st);
	check(ret == (long)0xdeadbeefUL && st.rax == 0xdeadbeefUL,
		"darling_thread_create trampoline dispatches and returns value in rax");
	check(recorded_thread_args[0] == 11 && recorded_thread_args[5] == 66 &&
	      recorded_thread_args[6] == 77 && recorded_thread_dthread == (void*)0x99887766UL,
		"darling_thread_create marshals 6 register args and 3 stack args");

	pthread_t tid;
	void* child_gs = NULL;
	pthread_create(&tid, NULL, child_cb_worker, (void*)recorded_thread_cbs);
	pthread_join(tid, &child_gs);
	check(child_gs == (void*)0x12345000UL &&
	      last_cb_fn == 0x100200UL && last_cb_arg0 == 0x12345000UL && last_cb_arg1 == 9,
		"wrapped thread_set_tsd_base works from a child pthread and updates thread gs_base");

	fex_elfcalls_set_fork_hooks(test_fork_lock_hook, test_fork_unlock_hook);

	// 1. native_fork in parent (ret = 4242)
	g_fork_seq[0] = '\0';
	g_fork_stub_ret = 4242;
	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 31;
	long r_fork1 = fex_linux_syscall_dispatch(&st);
	check(r_fork1 == 4242 && st.rax == 4242, "native_fork (slot 31) dispatches in parent");
	check(strcmp(g_fork_seq, "lock;fork;unlock(parent);") == 0 && !g_fork_last_is_child,
		"native_fork parent runs lock -> fork -> unlock(is_child=false)");

	// 2. native_fork in child (ret = 0)
	g_fork_seq[0] = '\0';
	g_fork_stub_ret = 0;
	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 31;
	long r_fork2 = fex_linux_syscall_dispatch(&st);
	check(r_fork2 == 0 && st.rax == 0, "native_fork (slot 31) returns 0 in child");
	check(strcmp(g_fork_seq, "lock;fork;unlock(child);") == 0 && g_fork_last_is_child,
		"native_fork child runs lock -> fork -> unlock(is_child=true)");

	// 3. native_fork on error (ret = -EAGAIN)
	g_fork_seq[0] = '\0';
	g_fork_stub_ret = -EAGAIN;
	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 31;
	long r_fork3 = fex_linux_syscall_dispatch(&st);
	check(r_fork3 == -EAGAIN && st.rax == (uint64_t)(int64_t)-EAGAIN, "native_fork (slot 31) returns -EAGAIN on error");
	check(strcmp(g_fork_seq, "lock;fork;unlock(parent);") == 0 && !g_fork_last_is_child,
		"native_fork error runs lock -> fork -> unlock(is_child=false)");

	// 4. native_fork with NULL hooks reset
	fex_elfcalls_set_fork_hooks(NULL, NULL);
	g_fork_seq[0] = '\0';
	g_fork_stub_ret = 777;
	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 31;
	long r_fork_nohooks = fex_linux_syscall_dispatch(&st);
	check(r_fork_nohooks == 777 && st.rax == 777,
		"native_fork (slot 31) dispatches cleanly with NULL fork hooks");
	check(strcmp(g_fork_seq, "fork;") == 0,
		"native_fork with NULL hooks invokes host fork without hook callbacks");

	unsigned int limits[2] = {0, 0};
	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 32;
	st.rdi = (uint64_t)(uintptr_t)limits;
	check(fex_linux_syscall_dispatch(&st) == 0 && limits[0] == 256 && limits[1] == 4096,
		"inherited_nofile_limit (slot 32) dispatches");

	// Task 0.1: darling_thread_terminate deferral
	check(!fex_elfcalls_has_pending_terminate(), "no pending terminate initially");
	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 5;
	st.rdi = 0x7fff00001000ULL;
	st.rsi = 0x4000;
	st.rdx = 0x200;
	check(fex_linux_syscall_dispatch(&st) == 0, "darling_thread_terminate (slot 5) dispatches successfully");
	check(st.unwind_thread == 1, "darling_thread_terminate sets state->unwind_thread = 1");
	check(recorded_term_call_count == 0, "host darling_thread_terminate was NOT called eagerly during dispatch");
	check(fex_elfcalls_has_pending_terminate(), "pending terminate flag is set");

	// Verify unwind_thread remains set if another elfcall returns while termination is pending
	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 6; // darling_thread_get_stack
	fex_linux_syscall_dispatch(&st);
	check(st.unwind_thread == 1, "state->unwind_thread remains 1 when subsequent call runs with pending terminate");

	// Flush deferred termination
	fex_elfcalls_flush_deferred_terminate();
	check(!fex_elfcalls_has_pending_terminate(), "pending terminate flag is cleared after flush");
	check(recorded_term_call_count == 1, "host darling_thread_terminate was called exactly once on flush");
	check(recorded_term_stackaddr == (void*)0x7fff00001000ULL &&
	      recorded_term_freesize == 0x4000 &&
	      recorded_term_pthobj_size == 0x200,
	      "host darling_thread_terminate received exact stackaddr, freesize, and pthobj_size");

	// Flush again and assert no duplicate termination call
	fex_elfcalls_flush_deferred_terminate();
	check(recorded_term_call_count == 1, "second flush is a no-op; terminate not called again");
}

static void test_dlsym_wrapping(void)
{
	void* optind_sym = dlsym(RTLD_DEFAULT, "optind");
	check(optind_sym != NULL, "dlsym found libc STT_OBJECT symbol optind");
	void* wrapped_obj = fex_dlsym_wrap_if_func(optind_sym, "optind");
	check(wrapped_obj == optind_sym, "STT_OBJECT data symbol is returned unwrapped");

	void* fn_stub1 = fex_dlsym_wrap_if_func((void*)host_math_Add8, "host_math_Add8");
	void* fn_stub2 = fex_dlsym_wrap_if_func((void*)host_math_Add8, "host_math_Add8");
	check(fn_stub1 != NULL && fn_stub1 != (void*)host_math_Add8, "STT_FUNC is wrapped in x86_64 stub");
	check(fn_stub1 == fn_stub2, "repeated dlsym on same function returns deduplicated stub");
	check(write_faults((volatile uint8_t*)fn_stub1), "dlsym stub page is W^X (writes fault)");

	uint32_t sc_nr = (uint32_t)stub_syscall_nr(fn_stub1);

	uint64_t stack[5] = { 0, 7, 8, 0, 0 };
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = sc_nr;
	st.rdi = 1;
	st.rsi = 2;
	st.rdx = 3;
	st.r10 = 4;
	st.r8 = 5;
	st.r9 = 6;
	st.rsp = (uint64_t)(uintptr_t)stack;
	double x = 2.5, y = 4.0;
	memcpy(&st.xmm[0][0], &x, sizeof(double));
	memcpy(&st.xmm[1][0], &y, sizeof(double));

	fex_linux_syscall_dispatch(&st);
	double out = 0.0;
	memcpy(&out, &st.xmm[0][0], sizeof(double));
	check(out == 46.0, "fex_dlsym_dispatch forwards integer, stack, and xmm float arguments (1..8 + 2.5*4.0 = 46)");

	void* fn_term_stub = fex_dlsym_wrap_if_func((void*)host_terminating_function, "host_terminating_function");
	uint32_t term_sc_nr = (uint32_t)stub_syscall_nr(fn_term_stub);
	memset(&st, 0, sizeof(st));
	st.rax = term_sc_nr;
	fex_linux_syscall_dispatch(&st);
	check(st.rax == 999, "fex_dlsym_dispatch returns function return value");
	check(st.unwind_thread == 1, "fex_dlsym_dispatch sets unwind_thread=1 when nested call triggers termination");
	fex_elfcalls_flush_deferred_terminate();
}

static uint64_t g_host_png_cb = 0;
static uint64_t host_png_set_read_fn(uint64_t png, uint64_t io, uint64_t cb)
{
	(void)png;
	(void)io;
	g_host_png_cb = cb;
	if (is_fake_guest_fn(cb))
		return 0xbad0bad0u;
	return ((uint64_t (*)(uint64_t, uint64_t))cb)(5, 6);
}

// A host library that is handed a guest function pointer would call x86_64 code natively, so the
// bridge hands it a host thunk that re-enters the guest instead.
static void test_host_callback_thunks(void)
{
	void* stub = fex_dlsym_wrap_if_func((void*)host_png_set_read_fn, "png_set_read_fn");
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = stub_syscall_nr(stub);
	st.rdx = 0x4000100;
	long ret = fex_linux_syscall_dispatch(&st);
	check(ret == 0x88 && st.rax == 0x88, "a guest callback passed to a host function returns through the guest");
	check(g_host_png_cb != 0 && g_host_png_cb != 0x4000100, "the host function receives a host thunk, not the guest pointer");
	check(last_cb_args_fn == 0x4000100 && last_cb_args[0] == 5 && last_cb_args[1] == 6,
		"the thunk calls the guest function with the host's arguments");

	void* other = fex_dlsym_wrap_if_func((void*)host_png_set_read_fn, "png_set_read_fn");
	check(other == stub, "the same host function keeps one stub");
}

static uint64_t g_host_kept_cb = 0;
static uint64_t host_png_set_error_fn(uint64_t png, uint64_t error_ptr, uint64_t error_fn, uint64_t warning_fn)
{
	(void)png;
	(void)error_ptr;
	(void)warning_fn;
	g_host_kept_cb = error_fn;
	return 0;
}

// The host thunk a fake host function receives for `guest_fn` (png_set_error_fn: arguments 2 and 3).
static uint64_t thunk_for(void* stub, uint64_t guest_fn)
{
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = stub_syscall_nr(stub);
	st.rdx = guest_fn;
	fex_linux_syscall_dispatch(&st);
	return g_host_kept_cb;
}

// A process may hand many distinct guest functions to host libraries over its life, and a library may
// call each of them at any later time, so every one keeps its own thunk.
static void test_many_callback_thunks(void)
{
	void* stub = fex_dlsym_wrap_if_func((void*)host_png_set_error_fn, "png_set_error_fn");
	enum { N = 600 }; // more than one page of thunks with 4 KB and 16 KB pages
	static uint64_t thunks[N];
	bool distinct = true;
	for (int i = 0; i < N; i++) {
		thunks[i] = thunk_for(stub, 0x6000000 + 0x40 * (uint64_t)i);
		for (int j = 0; j < i && distinct; j++)
			distinct = thunks[j] != thunks[i];
	}
	check(distinct && thunks[0] != 0, "600 distinct guest callbacks get 600 distinct host thunks");
	check(thunk_for(stub, 0x6000000 + 0x40 * 17) == thunks[17], "the same guest callback keeps its thunk");
	check(write_faults((volatile uint8_t*)(uintptr_t)thunks[N - 1]), "the thunk pages are W^X (writes fault)");

	bool routed = true;
	for (int i = 0; i < N && routed; i += 37) {
		if (is_fake_guest_fn(thunks[i])) {
			routed = false;
			break;
		}
		last_cb_args_fn = 0;
		uint64_t ret = ((uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(uintptr_t)thunks[i])(
			i, 2, 3, 4, 5, 6);
		routed = ret == 0x88 && last_cb_args_fn == 0x6000000 + 0x40 * (uint64_t)i &&
			last_cb_args[0] == (uint64_t)i && last_cb_args[5] == 6;
	}
	check(routed, "each thunk runs its own guest function with the six integer arguments it is called with");
}

static void exhaust_callback_thunks(void)
{
	void* stub = fex_dlsym_wrap_if_func((void*)host_png_set_error_fn, "png_set_error_fn");
	for (uint64_t i = 0; i < 0x10000; i++)
		thunk_for(stub, 0x6100000 + 0x10 * i);
}

static void test_callback_thunk_limit(void)
{
	check(aborts_with(exhaust_callback_thunks, "distinct guest functions were handed to host libraries"),
		"running out of host callback thunks stops the process and says why");
}

static uint64_t g_xlib_handler = 0xaaaa00;
static uint64_t g_xlib_handler_seen = 0;
static uint64_t host_XSetErrorHandler(uint64_t handler)
{
	uint64_t previous = g_xlib_handler;
	g_xlib_handler_seen = handler;
	g_xlib_handler = handler;
	return previous;
}

static uint64_t set_error_handler(void* stub, uint64_t handler)
{
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = stub_syscall_nr(stub);
	st.rdi = handler;
	fex_linux_syscall_dispatch(&st);
	return st.rax;
}

// XSetErrorHandler returns the handler Xlib had, which is host code. A guest that saves it and restores
// it later must not get it wrapped as if it were a guest function.
static void test_xlib_handler_restore(void)
{
	void* stub = fex_dlsym_wrap_if_func((void*)host_XSetErrorHandler, "XSetErrorHandler");
	uint64_t previous = set_error_handler(stub, 0x1111000);
	check(previous == 0xaaaa00, "setting an Xlib handler returns the host's original handler");
	check(g_xlib_handler_seen != 0x1111000 && g_xlib_handler_seen != 0,
		"Xlib is given a host handler in place of the guest one");

	uint64_t ours = set_error_handler(stub, 0xaaaa00);
	check(g_xlib_handler_seen == 0xaaaa00, "restoring the host's original handler passes it to Xlib unchanged");
	check(ours == 0x1111000, "replacing the guest's handler returns the guest's own pointer");
	check(set_error_handler(stub, 0) == 0xaaaa00, "clearing the handler returns the previous one");
}

static void test_signal_mask_filter(void)
{
	sigset_t before;
	pthread_sigmask(SIG_BLOCK, NULL, &before);

	uint64_t mask = (1ull << (SIGUSR1 - 1)) | (1ull << (SIGSEGV - 1)) | (1ull << (SIGBUS - 1)) | (1ull << (SIGILL - 1));
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = 14; // rt_sigprocmask
	st.rdi = SIG_SETMASK;
	st.rsi = (uint64_t)(uintptr_t)&mask;
	st.rdx = 0;
	st.r10 = 8;
	long ret = fex_linux_syscall_dispatch(&st);

	sigset_t now;
	pthread_sigmask(SIG_BLOCK, NULL, &now);
	check(ret == 0, "a guest rt_sigprocmask succeeds");
	check(sigismember(&now, SIGUSR1) == 1, "a signal the guest blocks is blocked on the host");
	check(sigismember(&now, SIGSEGV) == 0 && sigismember(&now, SIGBUS) == 0 && sigismember(&now, SIGILL) == 0,
		"the JIT's fault signals stay unblocked");

	uint64_t old = 0;
	memset(&st, 0, sizeof(st));
	st.rax = 14;
	st.rdi = SIG_SETMASK;
	st.rsi = (uint64_t)(uintptr_t)&mask;
	st.rdx = (uint64_t)(uintptr_t)&old;
	st.r10 = 8;
	fex_linux_syscall_dispatch(&st);
	check(old == mask, "the guest reads back the set it blocked, the JIT's signals included, in the x86_64 layout");

	memset(&st, 0, sizeof(st));
	st.rax = 14;
	st.rdi = SIG_SETMASK;
	st.rsi = 0;
	st.rdx = 0;
	st.r10 = 4;
	check(fex_linux_syscall_dispatch(&st) == -EINVAL, "a mask size other than 8 is refused");

	uint64_t none = 0;
	memset(&st, 0, sizeof(st));
	st.rax = 14;
	st.rdi = SIG_SETMASK;
	st.rsi = (uint64_t)(uintptr_t)&none;
	st.r10 = 8;
	fex_linux_syscall_dispatch(&st);
	pthread_sigmask(SIG_SETMASK, &before, NULL);
}

// The shim's side of signal delivery, faked: what mldr hands it and what it answers.
static int g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
static int g_fake_deliveries = 0;
static int g_fake_delivered_sig = 0;
static struct fex_guest_sigaction g_fake_delivered_action;
static struct fex_guest_stack g_fake_delivered_stack;
static uint64_t g_fake_delivered_old_mask;
static uint64_t g_fake_returned_mask;
static int g_fake_return_answer = 1;

static int fake_signal_preempt(int* sig, void* info, void* ucontext, uint64_t* guest_mask)
{
	(void)sig;
	(void)info;
	(void)ucontext;
	if (g_fake_preempt_answer == FEX_SIGNAL_RETURNED) {
		// As the shim does: the host context gets the frame's set, less the JIT's signals.
		*guest_mask = g_fake_returned_mask;
		uint64_t host = g_fake_returned_mask & ~FEX_JIT_SIGNALS;
		memcpy(&((ucontext_t*)ucontext)->uc_sigmask, &host, sizeof(host));
	}
	return g_fake_preempt_answer;
}

static int fake_signal_deliver(int sig, const void* info, void* ucontext,
	const struct fex_guest_sigaction* action, const struct fex_guest_stack* altstack, uint64_t old_mask)
{
	(void)info;
	(void)ucontext;
	g_fake_deliveries++;
	g_fake_delivered_sig = sig;
	g_fake_delivered_action = *action;
	g_fake_delivered_stack = *altstack;
	g_fake_delivered_old_mask = old_mask;
	return FEX_SIGNAL_DELIVERED;
}

static int fake_signal_return(uint64_t* guest_mask)
{
	*guest_mask = g_fake_returned_mask;
	return g_fake_return_answer;
}

static long guest_syscall(uint64_t nr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t rsp)
{
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = nr;
	st.rdi = a1;
	st.rsi = a2;
	st.rdx = a3;
	st.r10 = a4;
	st.rsp = rsp;
	return fex_linux_syscall_dispatch(&st);
}

static long guest_sigaction(int sig, uint64_t handler, uint64_t flags, uint64_t mask, struct fex_guest_sigaction* old)
{
	struct fex_guest_sigaction act = { .handler = handler, .flags = flags | FEX_GUEST_SA_RESTORER, .restorer = 0x5000, .mask = mask };
	return guest_syscall(13, sig, (uint64_t)(uintptr_t)&act, (uint64_t)(uintptr_t)old, 8, 0);
}

static uint64_t raw_blocked_mask(void)
{
	uint64_t mask = 0;
	syscall(SYS_rt_sigprocmask, SIG_BLOCK, NULL, &mask, 8);
	return mask;
}

// The signals pending on this thread alone, not on the process.
static uint64_t thread_pending_mask(void)
{
	char line[256];
	uint64_t mask = 0;
	FILE* f = fopen("/proc/thread-self/status", "r");
	while (f && fgets(line, sizeof(line), f)) {
		if (strncmp(line, "SigPnd:", 7) == 0)
			mask = strtoull(line + 7, NULL, 16);
	}
	if (f)
		fclose(f);
	return mask;
}

static void raise_on_this_thread(int sig)
{
	syscall(SYS_tgkill, getpid(), gettid(), sig);
}

// Sets the blocked set the way the guest does, so mldr's view of it follows.
static void set_guest_mask_to(uint64_t mask)
{
	guest_syscall(14, SIG_SETMASK, (uint64_t)(uintptr_t)&mask, 0, 8, 0);
}

static volatile int g_fresh_go;
static volatile pid_t g_fresh_tid;

static void* fresh_thread_main(void* arg)
{
	if (arg) {
		// As mldr starts a guest thread: all blocked while guest code already runs (a callback), then the
		// thread's own mask restored by mldr, not by the guest.
		sigset_t all, mine;
		sigfillset(&all);
		pthread_sigmask(SIG_SETMASK, &all, &mine);
		// The guest saves and puts back its blocked set meanwhile, as Darling's runtime does around a call.
		uint64_t block = 0x7fffffff, saved = 0;
		guest_syscall(14, SIG_BLOCK, (uint64_t)(uintptr_t)&block, (uint64_t)(uintptr_t)&saved, 8, 0);
		guest_syscall(14, SIG_SETMASK, (uint64_t)(uintptr_t)&saved, 0, 8, 0);
		pthread_sigmask(SIG_SETMASK, &mine, NULL);
	}
	g_fresh_tid = gettid();
	while (g_fresh_go)
		usleep(1000);
	return NULL;
}

// A thread that never made a guest syscall: its first signal arrives in the host handler, where the host's
// blocked set is the handler's own; the guest's set is the interrupted one.
static void test_fresh_thread(uint64_t guest_handler, uint64_t saved_mask)
{
	set_guest_mask_to(0);
	guest_sigaction(SIGUSR1, guest_handler, 0, 0, NULL);
	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
	for (int mldr_start = 0; mldr_start < 2; mldr_start++) {
		g_fresh_go = 1;
		g_fresh_tid = 0;
		pthread_t t;
		pthread_create(&t, NULL, fresh_thread_main, mldr_start ? (void*)&t : NULL);
		while (!g_fresh_tid)
			usleep(1000);
		int before = g_fake_deliveries;
		syscall(SYS_tgkill, getpid(), g_fresh_tid, SIGUSR1);
		for (int i = 0; i < 1000 && g_fake_deliveries == before; i++)
			usleep(1000);
		check(g_fake_deliveries == before + 1 && g_fake_delivered_sig == SIGUSR1 && !(g_fake_delivered_old_mask & FEX_JIT_SIGNALS),
			mldr_start ? "a thread whose mask mldr restored after guest code ran gets its signals"
				   : "a thread's first signal, before any guest syscall of it, is delivered");
		g_fresh_go = 0;
		pthread_join(t, NULL);
	}
	set_guest_mask_to(saved_mask);
}

static uint64_t guest_blocked(void)
{
	uint64_t old = 0;
	guest_syscall(14, SIG_BLOCK, 0, (uint64_t)(uintptr_t)&old, 8, 0);
	return old;
}

static struct fex_x86_64_state guest_syscall_state(uint64_t nr, uint64_t a1, uint64_t a2, uint64_t a3)
{
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = nr;
	st.rdi = a1;
	st.rsi = a2;
	st.rdx = a3;
	st.r10 = 8;
	fex_linux_syscall_dispatch(&st);
	return st;
}

// The blocked set a handler runs with is exactly Linux's: the interrupted set, sa_mask, and the signal
// itself unless SA_NODEFER; rt_sigreturn puts back the set in the frame, as the handler left it.
static void test_signal_masks(uint64_t guest_handler, uint64_t saved_mask)
{
	const uint64_t winch = FEX_SIGBIT(SIGWINCH), usr1 = FEX_SIGBIT(SIGUSR1), usr2 = FEX_SIGBIT(SIGUSR2);
	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
	guest_sigaction(SIGUSR1, guest_handler, 0, usr2, NULL);
	set_guest_mask_to(winch);
	int before = g_fake_deliveries;
	raise_on_this_thread(SIGUSR1);
	check(g_fake_deliveries == before + 1 && g_fake_delivered_old_mask == winch,
		"the frame keeps the blocked set the signal interrupted");
	check(guest_blocked() == (winch | usr2 | usr1) && raw_blocked_mask() == (winch | usr2 | usr1),
		"a handler runs with exactly the interrupted set, sa_mask and the signal blocked");

	set_guest_mask_to(winch);
	guest_sigaction(SIGUSR1, guest_handler, FEX_GUEST_SA_NODEFER, usr2, NULL);
	raise_on_this_thread(SIGUSR1);
	check(guest_blocked() == (winch | usr2), "SA_NODEFER: the interrupted set and sa_mask only");

	// A handler that returns through rt_sigreturn, after editing uc_sigmask in its frame.
	const uint64_t edited = FEX_SIGBIT(SIGHUP) | FEX_SIGBIT(SIGTRAP);
	g_fake_returned_mask = edited;
	g_fake_preempt_answer = FEX_SIGNAL_RETURNED;
	raise_on_this_thread(SIGUSR1);
	check(guest_blocked() == edited, "returning from a handler that interrupted emulated code restores the frame's set");
	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;

	set_guest_mask_to(0);
	g_fake_return_answer = 1;
	g_fake_returned_mask = winch;
	struct fex_x86_64_state st = guest_syscall_state(15, 0, 0, 0);
	check(st.frame_restored == 1 && guest_blocked() == winch && raw_blocked_mask() == winch,
		"returning from a handler run at a syscall return restores the frame's set and its registers");
	g_fake_return_answer = -EFAULT;
	st = guest_syscall_state(15, 0, 0, 0);
	check(st.frame_restored == 0 && st.signal_request && st.signal_request->sig == SIGSEGV,
		"an unreadable frame raises SIGSEGV for the guest");
	g_fake_return_answer = 1;

	// The JIT's own signals: the guest can block them (it reads them back), and one sent while blocked waits.
	set_guest_mask_to(FEX_SIGBIT(SIGTRAP));
	check(guest_blocked() == FEX_SIGBIT(SIGTRAP) && !(raw_blocked_mask() & FEX_SIGBIT(SIGTRAP)),
		"a blocked SIGTRAP reads back as blocked and stays open on the host");
	guest_sigaction(SIGTRAP, guest_handler, 0, 0, NULL);
	before = g_fake_deliveries;
	raise_on_this_thread(SIGTRAP);
	check(g_fake_deliveries == before, "a SIGTRAP the guest blocks does not run its handler");
	st = guest_syscall_state(39 /* getpid */, 0, 0, 0);
	check(st.signal_request == NULL, "nor at a syscall return while it stays blocked");
	uint64_t trap = FEX_SIGBIT(SIGTRAP);
	st = guest_syscall_state(14, SIG_UNBLOCK, (uint64_t)(uintptr_t)&trap, 0);
	check(st.signal_request && st.signal_request->sig == SIGTRAP, "unblocking it runs the handler as that syscall returns");
	guest_sigaction(SIGTRAP, 0, 0, 0, NULL);
	set_guest_mask_to(saved_mask);
}

static pthread_mutex_t g_host_lock = PTHREAD_MUTEX_INITIALIZER;

// A guest handler would need what host code holds (a lock, malloc): a signal landing in host code waits for
// the syscall that led there to return.
static void test_held_until_syscall_return(uint64_t guest_handler, uint64_t saved_mask)
{
	guest_sigaction(SIGUSR1, guest_handler, 0, FEX_SIGBIT(SIGUSR2), NULL);
	set_guest_mask_to(FEX_SIGBIT(SIGWINCH));
	g_fake_preempt_answer = FEX_SIGNAL_AT_SYSCALL_RETURN;
	int before = g_fake_deliveries;

	pthread_mutex_lock(&g_host_lock); // the host function the signal interrupts holds a lock
	raise_on_this_thread(SIGUSR1);
	check(g_fake_deliveries == before, "a signal in host code does not run the guest handler there");
	pthread_mutex_unlock(&g_host_lock);

	struct fex_x86_64_state st = guest_syscall_state(39 /* getpid */, 0, 0, 0);
	const struct fex_signal_request* req = st.signal_request;
	check(req && req->sig == SIGUSR1 && req->action.handler == guest_handler && req->old_mask == FEX_SIGBIT(SIGWINCH)
		&& !req->restart_syscall && (long)st.rax == getpid(), "the handler runs as the next syscall returns, after its result");
	check(guest_blocked() == (FEX_SIGBIT(SIGWINCH) | FEX_SIGBIT(SIGUSR2) | FEX_SIGBIT(SIGUSR1)),
		"and with the blocked set Linux gives it");
	st = guest_syscall_state(39, 0, 0, 0);
	check(st.signal_request == NULL, "it runs once");

	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
	set_guest_mask_to(saved_mask);
}

struct interrupter {
	pid_t tid;
	int sig;
};

static void* interrupt_later(void* arg)
{
	struct interrupter* it = arg;
	usleep(200 * 1000);
	syscall(SYS_tgkill, getpid(), it->tid, it->sig);
	return NULL;
}

// A guest syscall blocked in the host kernel: the signal interrupts it, and with SA_RESTART the handler
// returns to the syscall instruction (Linux restarts it), without it the guest sees EINTR.
static void test_syscall_restart(uint64_t guest_handler, uint64_t saved_mask)
{
	g_fake_preempt_answer = FEX_SIGNAL_AT_SYSCALL_RETURN;
	set_guest_mask_to(0);
	for (int restart = 1; restart >= 0; restart--) {
		guest_sigaction(SIGUSR1, guest_handler, restart ? FEX_GUEST_SA_RESTART : 0, 0, NULL);
		int fds[2];
		if (pipe(fds) != 0)
			abort();
		char byte;
		struct interrupter it = { gettid(), SIGUSR1 };
		pthread_t t;
		pthread_create(&t, NULL, interrupt_later, &it);
		struct fex_x86_64_state st = guest_syscall_state(0 /* read */, fds[0], (uint64_t)(uintptr_t)&byte, 1);
		pthread_join(t, NULL);
		const struct fex_signal_request* req = st.signal_request;
		check(req && req->sig == SIGUSR1 && (long)st.rax == -EINTR && req->restart_syscall == restart,
			restart ? "SA_RESTART: a blocked read is interrupted and made again after the handler"
				: "without SA_RESTART: a blocked read returns EINTR after the handler");
		close(fds[0]);
		close(fds[1]);
		set_guest_mask_to(0);
	}
	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
	set_guest_mask_to(saved_mask);
}

static void* write_later(void* arg)
{
	usleep(500 * 1000);
	if (write(*(int*)arg, "x", 1) != 1)
		abort();
	return NULL;
}

// Only the guest's own syscall is turned into EINTR at its kernel entry, and a signal the guest blocks does
// not interrupt it.
static void test_guest_syscall_entry(uint64_t guest_handler, uint64_t saved_mask)
{
	extern const char fex_guest_svc_insn[];
	static const uint32_t other_svc[2] = { 0xd2800008 /* mov x8, #0 */, 0xd4000001 /* svc #0 */ };
	check(fex_at_guest_syscall_entry(fex_guest_svc_insn) && fex_at_guest_syscall_entry((const char*)fex_guest_svc_insn - 4)
		&& !fex_at_guest_syscall_entry(&other_svc[1]) && !fex_at_guest_syscall_entry(&other_svc[0]),
		"only the guest's own syscall entry counts as one, not another svc (a helper syscall)");

	set_guest_mask_to(FEX_SIGBIT(SIGTRAP));
	guest_sigaction(SIGTRAP, guest_handler, 0, 0, NULL);
	g_fake_preempt_answer = FEX_SIGNAL_AT_SYSCALL_RETURN;
	int fds[2];
	if (pipe(fds) != 0)
		abort();
	struct interrupter it = { gettid(), SIGTRAP };
	pthread_t t;
	pthread_create(&t, NULL, interrupt_later, &it);
	pthread_t w;
	pthread_create(&w, NULL, write_later, &fds[1]);
	char byte = 0;
	struct fex_x86_64_state st = guest_syscall_state(0 /* read */, fds[0], (uint64_t)(uintptr_t)&byte, 1);
	pthread_join(t, NULL);
	pthread_join(w, NULL);
	check((long)st.rax == 1 && byte == 'x' && st.signal_request == NULL,
		"a signal the guest blocks does not interrupt its blocked syscall");
	close(fds[0]);
	close(fds[1]);
	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
	// It is still kept for the guest; unblocking takes it.
	uint64_t trap = FEX_SIGBIT(SIGTRAP);
	st = guest_syscall_state(14, SIG_UNBLOCK, (uint64_t)(uintptr_t)&trap, 0);
	check(st.signal_request && st.signal_request->sig == SIGTRAP, "and it runs once unblocked");
	guest_sigaction(SIGTRAP, 0, 0, 0, NULL);
	set_guest_mask_to(saved_mask);
}

// A signal kept for the guest is pending for it: sigtimedwait takes it, and a forked child, which starts with
// no pending signals, does not inherit it.
static void test_kept_signals(uint64_t guest_handler, uint64_t saved_mask)
{
	set_guest_mask_to(FEX_SIGBIT(SIGTRAP));
	guest_sigaction(SIGTRAP, guest_handler, 0, 0, NULL);
	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
	raise_on_this_thread(SIGTRAP); // a JIT signal the guest blocks: kept
	uint64_t trap = FEX_SIGBIT(SIGTRAP);
	siginfo_t info;
	memset(&info, 0, sizeof(info));
	struct timespec zero = {0, 0};
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = 128; // rt_sigtimedwait
	st.rdi = (uint64_t)(uintptr_t)&trap;
	st.rsi = (uint64_t)(uintptr_t)&info;
	st.rdx = (uint64_t)(uintptr_t)&zero;
	st.r10 = 8;
	long ret = fex_linux_syscall_dispatch(&st);
	check(ret == SIGTRAP && info.si_signo == SIGTRAP && info.si_code == SI_TKILL && st.signal_request == NULL,
		"sigtimedwait takes a signal kept for the guest, with its siginfo");
	memset(&st, 0, sizeof(st));
	st.rax = 128;
	st.rdi = (uint64_t)(uintptr_t)&trap;
	st.rdx = (uint64_t)(uintptr_t)&zero;
	st.r10 = 8;
	check(fex_linux_syscall_dispatch(&st) == -EAGAIN, "once");

	raise_on_this_thread(SIGTRAP);
	pid_t child = fork();
	if (child == 0) {
		fex_signals_after_fork_child();
		uint64_t pending = 0;
		guest_syscall(127, (uint64_t)(uintptr_t)&pending, 8, 0, 0, 0);
		_exit(pending & trap ? 1 : 0);
	}
	int status = 0;
	waitpid(child, &status, 0);
	check(WIFEXITED(status) && WEXITSTATUS(status) == 0, "a forked child starts with nothing kept");
	uint64_t pending = 0;
	guest_syscall(127, (uint64_t)(uintptr_t)&pending, 8, 0, 0, 0);
	check(pending & trap, "the parent still has it");
	memset(&st, 0, sizeof(st));
	st.rax = 128;
	st.rdi = (uint64_t)(uintptr_t)&trap;
	st.rdx = (uint64_t)(uintptr_t)&zero;
	st.r10 = 8;
	fex_linux_syscall_dispatch(&st);
	guest_sigaction(SIGTRAP, 0, 0, 0, NULL);
	set_guest_mask_to(saved_mask);
}

static struct fex_x86_64_state guest_syscall_state6(uint64_t nr, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
	uint64_t a5, uint64_t a6)
{
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = nr;
	st.rdi = a1;
	st.rsi = a2;
	st.rdx = a3;
	st.r10 = a4;
	st.r8 = a5;
	st.r9 = a6;
	fex_linux_syscall_dispatch(&st);
	return st;
}

// The signal a wait with a temporary blocked set let in runs its handler before the wait returns EINTR, with
// that set (plus sa_mask and the signal) blocked; the caller's set comes back with the handler's rt_sigreturn.
static void check_wait_delivery(struct fex_x86_64_state st, int sig, uint64_t caller, uint64_t wait, uint64_t sa_mask,
	const char* what)
{
	const struct fex_signal_request* req = st.signal_request;
	check(req && req->sig == sig && (long)st.rax == -EINTR && !req->restart_syscall && req->old_mask == caller
		&& guest_blocked() == (wait | sa_mask | FEX_SIGBIT(sig)), what);
	g_fake_return_answer = 1;
	g_fake_returned_mask = req ? req->old_mask : caller;
	guest_syscall_state(15, 0, 0, 0);
	check(guest_blocked() == caller, "and the handler's return puts the caller's set back");
}

static void test_masked_waits(uint64_t guest_handler, uint64_t saved_mask)
{
	const uint64_t usr1 = FEX_SIGBIT(SIGUSR1), usr2 = FEX_SIGBIT(SIGUSR2), winch = FEX_SIGBIT(SIGWINCH);
	const uint64_t none = 0;
	guest_sigaction(SIGUSR1, guest_handler, 0, usr2, NULL);
	g_fake_preempt_answer = FEX_SIGNAL_AT_SYSCALL_RETURN;

	// sigsuspend with the signal already pending: blocked by the caller, let in by the wait.
	set_guest_mask_to(usr1);
	raise_on_this_thread(SIGUSR1);
	struct fex_x86_64_state st = guest_syscall_state6(130, (uint64_t)(uintptr_t)&winch, 8, 0, 0, 0, 0);
	check_wait_delivery(st, SIGUSR1, usr1, winch, usr2, "sigsuspend takes a pending signal it lets in: handler first, then EINTR");

	// Woken by another thread.
	struct interrupter it = { gettid(), SIGUSR1 };
	pthread_t t;
	pthread_create(&t, NULL, interrupt_later, &it);
	st = guest_syscall_state6(130, (uint64_t)(uintptr_t)&none, 8, 0, 0, 0, 0);
	pthread_join(t, NULL);
	check_wait_delivery(st, SIGUSR1, usr1, 0, usr2, "sigsuspend woken by a signal runs its handler before it returns");

	// A JIT signal kept here while the guest blocks it ends the wait at once.
	guest_sigaction(SIGTRAP, guest_handler, 0, 0, NULL);
	set_guest_mask_to(FEX_SIGBIT(SIGTRAP) | usr1);
	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
	raise_on_this_thread(SIGTRAP);
	g_fake_preempt_answer = FEX_SIGNAL_AT_SYSCALL_RETURN;
	st = guest_syscall_state6(130, (uint64_t)(uintptr_t)&usr1, 8, 0, 0, 0, 0);
	check_wait_delivery(st, SIGTRAP, FEX_SIGBIT(SIGTRAP) | usr1, usr1, 0, "sigsuspend takes a signal kept for the guest");

	// Only an ignored one ended it: sigsuspend waits on, until a signal with a handler comes.
	guest_sigaction(SIGTRAP, 1 /* SIG_IGN */, 0, 0, NULL);
	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
	raise_on_this_thread(SIGTRAP);
	g_fake_preempt_answer = FEX_SIGNAL_AT_SYSCALL_RETURN;
	pthread_create(&t, NULL, interrupt_later, &it);
	st = guest_syscall_state6(130, (uint64_t)(uintptr_t)&none, 8, 0, 0, 0, 0);
	pthread_join(t, NULL);
	check_wait_delivery(st, SIGUSR1, FEX_SIGBIT(SIGTRAP) | usr1, 0, usr2, "an ignored signal does not end sigsuspend");
	guest_sigaction(SIGTRAP, 0, 0, 0, NULL);
	check(guest_syscall_state6(130, (uint64_t)(uintptr_t)&none, 4, 0, 0, 0, 0).rax == (uint64_t)-EINVAL,
		"sigsuspend checks the set's size");

	// ppoll and pselect6 with a set: the same.
	set_guest_mask_to(usr1);
	struct timespec second = { 1, 0 };
	raise_on_this_thread(SIGUSR1);
	st = guest_syscall_state6(271, 0, 0, (uint64_t)(uintptr_t)&second, (uint64_t)(uintptr_t)&none, 8, 0);
	check_wait_delivery(st, SIGUSR1, usr1, 0, usr2, "ppoll with a set lets the signal in and runs its handler");
	raise_on_this_thread(SIGUSR1);
	uint64_t data[2] = { (uint64_t)(uintptr_t)&none, 8 };
	st = guest_syscall_state6(270, 0, 0, 0, 0, (uint64_t)(uintptr_t)&second, (uint64_t)(uintptr_t)data);
	check_wait_delivery(st, SIGUSR1, usr1, 0, usr2, "pselect6 with a set lets the signal in and runs its handler");

	// epoll_pwait made again after an ignored signal ended it waits only for what is left of its timeout.
	guest_sigaction(SIGTRAP, 1 /* SIG_IGN */, 0, 0, NULL);
	int ep = epoll_create1(0);
	char events[64];
	struct interrupter trap = { gettid(), SIGTRAP };
	struct timespec t0, t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	pthread_create(&t, NULL, interrupt_later, &trap);
	st = guest_syscall_state6(281, ep, (uint64_t)(uintptr_t)events, 4, 400, (uint64_t)(uintptr_t)&none, 8);
	pthread_join(t, NULL);
	clock_gettime(CLOCK_MONOTONIC, &t1);
	long ms = (t1.tv_sec - t0.tv_sec) * 1000 + (t1.tv_nsec - t0.tv_nsec) / 1000000;
	check(st.rax == 0 && st.signal_request == NULL && ms >= 380 && ms < 520,
		"epoll_pwait made again after an ignored signal keeps its total timeout");
	close(ep);
	guest_sigaction(SIGTRAP, 0, 0, 0, NULL);

	// A wait that ends without a signal leaves one the caller blocks pending.
	guest_sigaction(SIGUSR2, guest_handler, 0, 0, NULL);
	set_guest_mask_to(usr2);
	raise_on_this_thread(SIGUSR2);
	struct timespec zero = { 0, 0 };
	st = guest_syscall_state6(271, 0, 0, (uint64_t)(uintptr_t)&zero, (uint64_t)(uintptr_t)&usr2, 8, 0);
	uint64_t pending = 0;
	guest_syscall(127, (uint64_t)(uintptr_t)&pending, 8, 0, 0, 0);
	check(st.rax == 0 && st.signal_request == NULL && (pending & usr2) && guest_blocked() == usr2,
		"a wait whose set blocks the signal leaves it pending");
	st = guest_syscall_state6(128, (uint64_t)(uintptr_t)&usr2, 0, (uint64_t)(uintptr_t)&zero, 8, 0, 0);
	check((long)st.rax == SIGUSR2, "(drained)");
	guest_sigaction(SIGUSR2, 0, 0, 0, NULL);

	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
	set_guest_mask_to(saved_mask);
}

static void test_guest_signal_delivery(void)
{
	const uint64_t guest_handler = 0x123400;
	uint64_t saved_mask = raw_blocked_mask();

	check(guest_sigaction(SIGUSR1, guest_handler, 0, 0, NULL) == -ENOSYS,
		"a guest handler cannot be installed before the shim's delivery hooks are known");
	check(guest_syscall(15, 0, 0, 0, 0, 0) == -ENOSYS, "rt_sigreturn without the shim fails");

	struct sigaction jit_saved[NSIG];
	for (int sig = 1; sig < NSIG; sig++) {
		if (FEX_JIT_SIGNALS & FEX_SIGBIT(sig))
			sigaction(sig, NULL, &jit_saved[sig]);
	}
	uint64_t s33_saved[4];
	syscall(SYS_rt_sigaction, 33, NULL, s33_saved, 8);

	fex_signals_set_hooks(fake_signal_preempt, fake_signal_deliver, fake_signal_return);
	struct sigaction host;
	sigaction(SIGSEGV, NULL, &host);
	check((host.sa_flags & SA_SIGINFO) && (host.sa_flags & SA_ONSTACK) && host.sa_sigaction != NULL,
		"the hooks put the host handler on the JIT's signals");
	struct sigaction jit_host = host;

	check(guest_sigaction(SIGSEGV, guest_handler, 0, 0, NULL) == 0, "a guest SIGSEGV handler is accepted");
	struct sigaction after;
	sigaction(SIGSEGV, NULL, &after);
	check(after.sa_sigaction == host.sa_sigaction, "a guest SIGSEGV handler leaves the host's in place");

	// A handler, with SIGUSR2 in its mask.
	check(guest_sigaction(SIGUSR1, guest_handler, FEX_GUEST_SA_RESTART, FEX_SIGBIT(SIGUSR2), NULL) == 0,
		"a guest SIGUSR1 handler is installed");
	sigaction(SIGUSR1, NULL, &host);
	check((host.sa_flags & SA_SIGINFO) && (host.sa_flags & SA_RESTART) && (host.sa_flags & SA_ONSTACK),
		"the host handler keeps the guest's SA_RESTART and runs on the signal stack");
	raise_on_this_thread(SIGUSR1);
	check(g_fake_deliveries == 1 && g_fake_delivered_sig == SIGUSR1 && g_fake_delivered_action.handler == guest_handler
		&& g_fake_delivered_action.restorer == 0x5000, "a signal on a guest thread is delivered with the guest's action");
	uint64_t during = raw_blocked_mask();
	check((during & FEX_SIGBIT(SIGUSR1)) && (during & FEX_SIGBIT(SIGUSR2)) && !(during & FEX_JIT_SIGNALS),
		"the handler runs with the signal and its sa_mask blocked, and the JIT's signals open");
	set_guest_mask_to(saved_mask);

	check(guest_sigaction(SIGUSR1, guest_handler, FEX_GUEST_SA_NODEFER, FEX_SIGBIT(SIGUSR2), NULL) == 0,
		"an SA_NODEFER handler is installed");
	raise_on_this_thread(SIGUSR1);
	during = raw_blocked_mask();
	check(g_fake_deliveries == 2 && !(during & FEX_SIGBIT(SIGUSR1)) && (during & FEX_SIGBIT(SIGUSR2)),
		"SA_NODEFER leaves the signal itself unblocked");
	set_guest_mask_to(saved_mask);

	check(guest_sigaction(SIGUSR1, guest_handler, FEX_GUEST_SA_RESETHAND, 0, NULL) == 0,
		"an SA_RESETHAND handler is installed");
	raise_on_this_thread(SIGUSR1);
	struct fex_guest_sigaction old;
	memset(&old, 0xff, sizeof(old));
	guest_syscall(13, SIGUSR1, 0, (uint64_t)(uintptr_t)&old, 8, 0);
	sigaction(SIGUSR1, NULL, &host);
	check(g_fake_deliveries == 3 && old.handler == 0 && host.sa_handler == SIG_DFL,
		"SA_RESETHAND puts the default action back after one delivery");
	set_guest_mask_to(saved_mask);

	check(guest_sigaction(SIGUSR1, 1 /* SIG_IGN */, 0, 0, NULL) == 0, "the guest can ignore SIGUSR1");
	sigaction(SIGUSR1, NULL, &host);
	raise_on_this_thread(SIGUSR1);
	check(host.sa_handler == SIG_IGN && g_fake_deliveries == 3, "an ignored signal is ignored by the host kernel");

	// The signals glibc reserves (32, 33: darlingserver's interrupts) reach the guest too.
	check(guest_sigaction(33, guest_handler, FEX_GUEST_SA_RESTART, 0, NULL) == 0, "a guest handler for signal 33 is installed");
	raise_on_this_thread(33);
	check(g_fake_deliveries == 4 && g_fake_delivered_sig == 33, "signal 33 is delivered to the guest");
	set_guest_mask_to(saved_mask);
	uint64_t block33 = FEX_SIGBIT(33);
	check(guest_syscall(14, SIG_BLOCK, (uint64_t)(uintptr_t)&block33, 0, 8, 0) == 0 && (raw_blocked_mask() & block33),
		"the guest can block signal 33");
	set_guest_mask_to(saved_mask);

	g_fake_preempt_answer = FEX_SIGNAL_CONSUMED;
	raise_on_this_thread(33);
	check(g_fake_deliveries == 4 && !(raw_blocked_mask() & block33),
		"a signal the shim consumed is neither delivered nor passed on");

	// No guest code on this thread yet: the signal is held here, blocked, until the thread's first guest
	// syscall. One sent to the process goes back to the process; one sent to the thread stays on it.
	g_fake_preempt_answer = FEX_SIGNAL_NO_GUEST_THREAD;
	check(guest_sigaction(SIGUSR2, guest_handler, 0, 0, NULL) == 0, "a guest SIGUSR2 handler is installed");
	kill(getpid(), SIGUSR2);
	raise_on_this_thread(33);
	sigset_t pending;
	sigpending(&pending);
	check(g_fake_deliveries == 4 && sigismember(&pending, SIGUSR2) == 1 && (raw_blocked_mask() & FEX_SIGBIT(SIGUSR2)),
		"a signal sent to the process, on a thread without guest code, goes back to the process");
	check(thread_pending_mask() & FEX_SIGBIT(33) && (raw_blocked_mask() & FEX_SIGBIT(33)),
		"a signal sent to a thread without guest code stays pending on it");
	g_fake_preempt_answer = FEX_SIGNAL_FOR_GUEST;
	guest_syscall(39 /* getpid */, 0, 0, 0, 0, 0);
	check(g_fake_deliveries == 6, "the thread's first guest syscall lets the held signals in");
	set_guest_mask_to(saved_mask);

	// The alternate signal stack is the guest's own, per thread.
	static uint8_t alt[16384];
	struct fex_guest_stack ss = { .sp = (uint64_t)(uintptr_t)alt, .flags = 0, .size = sizeof(alt) };
	struct fex_guest_stack oss;
	memset(&oss, 0, sizeof(oss));
	uint64_t outside = (uint64_t)(uintptr_t)&oss;
	check(guest_syscall(131, (uint64_t)(uintptr_t)&ss, (uint64_t)(uintptr_t)&oss, 0, 0, outside) == 0
		&& oss.flags == FEX_GUEST_SS_DISABLE, "a thread starts without an alternate stack");
	stack_t host_stack;
	sigaltstack(NULL, &host_stack);
	check(host_stack.ss_sp != alt, "the guest's alternate stack is not given to the host");
	check(guest_syscall(131, 0, (uint64_t)(uintptr_t)&oss, 0, 0, outside) == 0 && oss.sp == ss.sp && oss.size == ss.size
		&& oss.flags == 0, "the alternate stack reads back");
	uint64_t inside = (uint64_t)(uintptr_t)alt + 100;
	check(guest_syscall(131, 0, (uint64_t)(uintptr_t)&oss, 0, 0, inside) == 0 && oss.flags == FEX_GUEST_SS_ONSTACK,
		"SS_ONSTACK is reported when running on it");
	check(guest_syscall(131, (uint64_t)(uintptr_t)&ss, 0, 0, 0, inside) == -EPERM, "it cannot be changed while in use");
	struct fex_guest_stack small = { .sp = (uint64_t)(uintptr_t)alt, .flags = 0, .size = 1024 };
	check(guest_syscall(131, (uint64_t)(uintptr_t)&small, 0, 0, 0, outside) == -ENOMEM, "a stack below MINSIGSTKSZ is refused");
	struct fex_guest_stack autodisarm = { .sp = (uint64_t)(uintptr_t)alt, .flags = (int32_t)(1U << 31), .size = sizeof(alt) };
	check(guest_syscall(131, (uint64_t)(uintptr_t)&autodisarm, 0, 0, 0, outside) == -EINVAL, "SS_AUTODISARM is refused");

	check(guest_sigaction(SIGUSR1, guest_handler, FEX_GUEST_SA_ONSTACK, 0, NULL) == 0, "an SA_ONSTACK handler is installed");
	raise_on_this_thread(SIGUSR1);
	check(g_fake_deliveries == 7 && g_fake_delivered_stack.sp == ss.sp && g_fake_delivered_stack.size == ss.size
		&& (g_fake_delivered_action.flags & FEX_GUEST_SA_ONSTACK), "delivery is given the thread's alternate stack");
	set_guest_mask_to(saved_mask);

	struct fex_guest_stack off = { .flags = FEX_GUEST_SS_DISABLE };
	guest_syscall(131, (uint64_t)(uintptr_t)&off, 0, 0, 0, outside);
	check(guest_syscall(131, 0, (uint64_t)(uintptr_t)&oss, 0, 0, outside) == 0 && oss.flags == FEX_GUEST_SS_DISABLE,
		"SS_DISABLE turns it off");

	test_fresh_thread(guest_handler, saved_mask);
	test_signal_masks(guest_handler, saved_mask);
	test_held_until_syscall_return(guest_handler, saved_mask);
	test_syscall_restart(guest_handler, saved_mask);
	test_guest_syscall_entry(guest_handler, saved_mask);
	test_kept_signals(guest_handler, saved_mask);
	test_masked_waits(guest_handler, saved_mask);

	guest_sigaction(SIGUSR1, 0, 0, 0, NULL);
	guest_sigaction(SIGUSR2, 0, 0, 0, NULL);
	guest_sigaction(SIGSEGV, 0, 0, 0, NULL);
	sigaction(SIGSEGV, NULL, &after);
	check(after.sa_sigaction == jit_host.sa_sigaction && (after.sa_flags & SA_SIGINFO),
		"a guest SIG_DFL for SIGSEGV leaves the host's handler in place");
	syscall(SYS_rt_sigaction, 33, s33_saved, NULL, 8);
	for (int sig = 1; sig < NSIG; sig++) {
		if (FEX_JIT_SIGNALS & FEX_SIGBIT(sig))
			sigaction(sig, &jit_saved[sig], NULL);
	}
}

// libwayland-client calls the dispatcher of a proxy for every event of it, as
// dispatcher(implementation, proxy, opcode, message, arguments). The dispatcher is a guest function.
static uint64_t g_wl_seen_dispatcher = 0;
static int host_wl_proxy_add_dispatcher(void* proxy, uint64_t dispatcher, const void* implementation, void* data)
{
	(void)proxy;
	(void)implementation;
	(void)data;
	g_wl_seen_dispatcher = dispatcher;
	return 0;
}

static void test_wayland_dispatcher(void)
{
	void* stub = fex_dlsym_wrap_if_func((void*)host_wl_proxy_add_dispatcher, "wl_proxy_add_dispatcher");
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = stub_syscall_nr(stub);
	st.rdi = 0x7f0000a000;
	st.rsi = 0x5000200;
	st.rdx = 0x1111;
	st.r10 = 0x2222;
	fex_linux_syscall_dispatch(&st);
	bool replaced = g_wl_seen_dispatcher != 0 && g_wl_seen_dispatcher != 0x5000200;
	check(replaced, "libwayland is given a host function in place of the guest's dispatcher");
	if (!replaced)
		return; // calling the guest's pointer natively would crash and lose the report

	// What libwayland does for an event: call the dispatcher with the implementation, proxy, opcode, message, arguments.
	last_cb_args_fn = 0;
	uint64_t ret = ((uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(uintptr_t)g_wl_seen_dispatcher)(
		0x1111, 0x7f0000a000, 3, 0xaaaa, 0xbbbb);
	check(last_cb_args_fn == 0x5000200 && ret == 0x88, "an event runs the guest dispatcher under FEX");
	check(last_cb_args[0] == 0x1111 && last_cb_args[1] == 0x7f0000a000 && last_cb_args[2] == 3 &&
	      last_cb_args[3] == 0xaaaa && last_cb_args[4] == 0xbbbb,
		"the guest dispatcher gets the implementation, proxy, opcode, message and arguments");

	// A dispatcher the guest removes stays removed.
	st.rax = stub_syscall_nr(stub);
	st.rsi = 0;
	fex_linux_syscall_dispatch(&st);
	check(g_wl_seen_dispatcher == 0, "a NULL dispatcher stays NULL");
}

// Calls the bridge stub of a fake host function with up to four integer arguments.
static uint64_t call_stub(void* stub, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3)
{
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = stub_syscall_nr(stub);
	st.rdi = a0;
	st.rsi = a1;
	st.rdx = a2;
	st.r10 = a3;
	fex_linux_syscall_dispatch(&st);
	return st.rax;
}

// What the fakes below return when they are handed a guest function instead of a thunk.
#define GOT_GUEST_FN 0xbad0bad0u

// Xlib calls the predicate of XIfEvent and its siblings as predicate(display, event, arg).
static uint64_t host_call_predicate(uint64_t display, uint64_t event, uint64_t predicate, uint64_t arg)
{
	if (is_fake_guest_fn(predicate))
		return GOT_GUEST_FN;
	return ((uint64_t (*)(uint64_t, uint64_t, uint64_t))(uintptr_t)predicate)(display, event, arg);
}
// One fake per name: the bridge keeps one stub, and one name, per host function.
static uint64_t host_XIfEvent(uint64_t d, uint64_t e, uint64_t p, uint64_t a) { return host_call_predicate(d, e, p, a); }
static uint64_t host_XCheckIfEvent(uint64_t d, uint64_t e, uint64_t p, uint64_t a) { return host_call_predicate(d, e, p, a); }
static uint64_t host_XPeekIfEvent(uint64_t d, uint64_t e, uint64_t p, uint64_t a) { return host_call_predicate(d, e, p, a); }

// giflib reads the file through read_fn(gif, buffer, length) for as long as it is open.
static uint64_t g_gif_read_fn = 0;
static uint64_t host_DGifOpen(uint64_t user_data, uint64_t read_fn, uint64_t error)
{
	(void)user_data;
	(void)error;
	g_gif_read_fn = read_fn;
	return 0x9100;
}

static void test_callback_arguments(void)
{
	const char* predicates[] = {"XIfEvent", "XCheckIfEvent", "XPeekIfEvent"};
	void* fakes[] = {(void*)host_XIfEvent, (void*)host_XCheckIfEvent, (void*)host_XPeekIfEvent};
	for (size_t i = 0; i < sizeof(predicates) / sizeof(predicates[0]); i++) {
		void* stub = fex_dlsym_wrap_if_func(fakes[i], predicates[i]);
		last_cb_args_fn = 0;
		uint64_t ret = call_stub(stub, 0x7f0000b000, 0x7f0000c000, 0x5100000 + i, 0x7f0000d000);
		char what[128];
		snprintf(what, sizeof(what), "%s runs the guest predicate with the display, event and argument", predicates[i]);
		check(ret == 0x88 && last_cb_args_fn == 0x5100000 + i && last_cb_args[0] == 0x7f0000b000 &&
		      last_cb_args[1] == 0x7f0000c000 && last_cb_args[2] == 0x7f0000d000, what);
	}

	void* stub = fex_dlsym_wrap_if_func((void*)host_DGifOpen, "DGifOpen");
	bool replaced = call_stub(stub, 0x7f0000e000, 0x5200000, 0, 0) == 0x9100 && g_gif_read_fn != 0 &&
		!is_fake_guest_fn(g_gif_read_fn);
	check(replaced, "DGifOpen keeps a host thunk in place of the guest read function");
	last_cb_args_fn = 0;
	if (replaced)
		((uint64_t (*)(uint64_t, uint64_t, uint64_t))(uintptr_t)g_gif_read_fn)(0x7f0000f000, 0x7f00010000, 13);
	check(last_cb_args_fn == 0x5200000 && last_cb_args[1] == 0x7f00010000 && last_cb_args[2] == 13,
		"a later read by giflib runs the guest read function");
}

// FT_Outline_Funcs, as FreeType declares it.
struct fake_outline_funcs {
	uint64_t move_to, line_to, conic_to, cubic_to;
	int shift;
	long delta;
};

static int g_outline_shift = -1;
static long g_outline_delta = -1;
static uint64_t g_outline_funcs_ptr = 0;
// FreeType walks the outline and calls move_to(to, user), ..., cubic_to(c1, c2, to, user) while it runs.
static uint64_t host_FT_Outline_Decompose(uint64_t outline, uint64_t funcs, uint64_t user)
{
	(void)outline;
	const struct fake_outline_funcs* f = (const struct fake_outline_funcs*)(uintptr_t)funcs;
	g_outline_funcs_ptr = funcs;
	if (!f)
		return 0;
	g_outline_shift = f->shift;
	g_outline_delta = f->delta;
	if (is_fake_guest_fn(f->move_to) || is_fake_guest_fn(f->cubic_to))
		return GOT_GUEST_FN;
	((uint64_t (*)(uint64_t, uint64_t))(uintptr_t)f->move_to)(0x7f00011000, user);
	return ((uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t))(uintptr_t)f->cubic_to)(1, 2, 3, user);
}

// A struct of guest callbacks passed by pointer reaches the library as a copy holding host thunks.
static void test_callback_struct(void)
{
	void* stub = fex_dlsym_wrap_if_func((void*)host_FT_Outline_Decompose, "FT_Outline_Decompose");
	struct fake_outline_funcs guest = {0x5400000, 0x5400100, 0x5400200, 0x5400300, 6, -42};
	last_cb_args_fn = 0;
	uint64_t ret = call_stub(stub, 0x7f00012000, (uint64_t)(uintptr_t)&guest, 0x7f00013000, 0);
	check(g_outline_funcs_ptr != 0 && g_outline_funcs_ptr != (uint64_t)(uintptr_t)&guest,
		"FT_Outline_Decompose gets a copy of the guest's FT_Outline_Funcs");
	check(g_outline_shift == 6 && g_outline_delta == -42, "the copy keeps the struct's other fields");
	check(ret == 0x88 && last_cb_args_fn == 0x5400300 && last_cb_args[0] == 1 && last_cb_args[2] == 3 &&
	      last_cb_args[3] == 0x7f00013000,
		"FreeType's calls through the copy run the guest's callbacks");
	check(guest.move_to == 0x5400000 && guest.cubic_to == 0x5400300, "the guest's own struct is left alone");

	g_outline_funcs_ptr = 1;
	call_stub(stub, 0x7f00012000, 0, 0, 0);
	check(g_outline_funcs_ptr == 0, "a NULL struct pointer reaches the library as NULL");
}

// pa_mainloop_api as libpulse declares it: userdata, then thirteen function pointers.
struct fake_pa_mainloop_api {
	uint64_t userdata;
	uint64_t io_new, io_enable, io_free, io_set_destroy;
	uint64_t time_new, time_restart, time_free, time_set_destroy;
	uint64_t defer_new, defer_enable, defer_free, defer_set_destroy;
	uint64_t quit;
};

static uint64_t g_pa_api_seen = 0;
// libpulse keeps the main loop it is given for the life of the context.
static uint64_t host_pa_context_new(uint64_t api, uint64_t name)
{
	(void)name;
	g_pa_api_seen = api;
	return 0x7f00020000;
}
static uint64_t host_pa_context_new_with_proplist(uint64_t api, uint64_t name, uint64_t proplist)
{
	(void)name;
	(void)proplist;
	g_pa_api_seen = api;
	return 0x7f00021000;
}

// A libpulse callback the guest's main loop calls: io(api, event, fd, events, userdata).
static uint64_t g_pa_io_cb_args[5];
static void host_pa_io_cb(uint64_t api, uint64_t e, uint64_t fd, uint64_t events, uint64_t userdata)
{
	const uint64_t a[5] = {api, e, fd, events, userdata};
	memcpy(g_pa_io_cb_args, a, sizeof(a));
}

static uint64_t g_pa_seen_cb = 0;
static uint64_t host_pa_context_set_state_callback(uint64_t c, uint64_t cb, uint64_t userdata)
{
	(void)c;
	(void)userdata;
	g_pa_seen_cb = cb;
	return 0;
}
static uint64_t host_pa_stream_write(uint64_t s, uint64_t data, uint64_t nbytes, uint64_t free_cb)
{
	(void)s;
	(void)data;
	(void)nbytes;
	g_pa_seen_cb = free_cb;
	return 0;
}

static uint64_t call_pa_api(uint64_t api, size_t word, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
	uint64_t fn = ((const uint64_t*)(uintptr_t)api)[word];
	return ((uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(uintptr_t)fn)(a0, a1, a2, a3, a4);
}

static void test_pa_mainloop_api(void)
{
	void* new_stub = fex_dlsym_wrap_if_func((void*)host_pa_context_new_with_proplist, "pa_context_new_with_proplist");
	struct fake_pa_mainloop_api guest = {
		0x7f00022000, 0x5500100, 0x5500200, 0x5500300, 0x5500400, 0x5500500, 0x5500600, 0x5500700,
		0x5500800, 0x5500900, 0x5500a00, 0x5500b00, 0x5500c00, 0,
	};
	const uint64_t guest_api = (uint64_t)(uintptr_t)&guest;
	call_stub(new_stub, guest_api, 0x7f00023000, 0x7f00024000, 0);
	const uint64_t copy = g_pa_api_seen;
	const struct fake_pa_mainloop_api* c = (const struct fake_pa_mainloop_api*)(uintptr_t)copy;
	bool thunks = copy != 0 && copy != guest_api;
	for (size_t w = 1; thunks && w < 13; w++)
		thunks = !is_fake_guest_fn(((const uint64_t*)c)[w]) && ((const uint64_t*)c)[w] != 0;
	check(thunks, "pa_context_new_with_proplist keeps a host copy of the guest's pa_mainloop_api with thunks");
	check(thunks && c->userdata == guest.userdata && c->quit == 0, "the copy keeps userdata and a NULL quit");
	check(guest.io_new == 0x5500100, "the guest's own pa_mainloop_api is left alone");

	void* plain_stub = fex_dlsym_wrap_if_func((void*)host_pa_context_new, "pa_context_new");
	g_pa_api_seen = 0;
	call_stub(plain_stub, guest_api, 0x7f00023000, 0, 0);
	check(g_pa_api_seen == copy, "pa_context_new with the same main loop gets the same copy");
	if (!thunks)
		return;

	// libpulse adds an io event: the guest's io_new gets its own struct and a stub it can call for the callback.
	last_cb_args_fn = 0;
	call_pa_api(copy, 1, copy, 7, 1, (uint64_t)(uintptr_t)host_pa_io_cb, 0x7f00025000);
	uint64_t cb_stub = last_cb_args[3];
	check(last_cb_args_fn == 0x5500100 && last_cb_args[0] == guest_api && last_cb_args[1] == 7 &&
	      last_cb_args[2] == 1 && last_cb_args[4] == 0x7f00025000,
		"io_new runs the guest function with the guest's pa_mainloop_api");
	bool is_stub = cb_stub != (uint64_t)(uintptr_t)host_pa_io_cb &&
		(stub_syscall_nr((void*)(uintptr_t)cb_stub) & 0xffff0000u) == DARLING_FEX_DLSYM_SYSCALL_BASE;
	check(is_stub, "the guest gets a bridge stub for libpulse's io callback");
	if (is_stub) {
		struct fex_x86_64_state st;
		memset(&st, 0, sizeof(st));
		st.rax = stub_syscall_nr((void*)(uintptr_t)cb_stub);
		st.rdi = guest_api;
		st.rsi = 0x7f00026000;
		st.rdx = 7;
		st.r10 = 4;
		st.r8 = 0x7f00025000;
		fex_linux_syscall_dispatch(&st);
		check(g_pa_io_cb_args[0] == copy && g_pa_io_cb_args[1] == 0x7f00026000 && g_pa_io_cb_args[2] == 7 &&
		      g_pa_io_cb_args[3] == 4 && g_pa_io_cb_args[4] == 0x7f00025000,
			"the guest's call of that stub reaches libpulse's callback with the host copy");
	}

	// A callback libpulse got from the guest (pa_context_rttime_new) reaches the guest's time_new as the guest's function.
	uint64_t guest_timer_cb_thunk = thunk_for(fex_dlsym_wrap_if_func((void*)host_png_set_error_fn, "png_set_error_fn"), 0x5500d00);
	last_cb_args_fn = 0;
	call_pa_api(copy, 5, copy, 0x7f00027000, guest_timer_cb_thunk, 0x7f00025000, 0);
	check(last_cb_args_fn == 0x5500500 && last_cb_args[0] == guest_api && last_cb_args[2] == 0x5500d00,
		"time_new gets the guest function behind a thunk");

	last_cb_args_fn = 0;
	call_pa_api(copy, 2, 0x7f00026000, 4, 0, 0, 0);
	check(last_cb_args_fn == 0x5500200 && last_cb_args[0] == 0x7f00026000 && last_cb_args[1] == 4,
		"io_enable passes its arguments as is");

	// A main loop freed and reallocated at the same address gets its new functions in the same copy.
	guest.io_free = 0x5500e00;
	call_stub(plain_stub, guest_api, 0x7f00023000, 0, 0);
	last_cb_args_fn = 0;
	call_pa_api(copy, 3, 0x7f00026000, 0, 0, 0, 0);
	check(g_pa_api_seen == copy && last_cb_args_fn == 0x5500e00, "handing the struct over again updates its copy");
}

// Plain libpulse callbacks become thunks, as listed from pulse/*.h.
static void test_pa_callback_args(void)
{
	void* stub = fex_dlsym_wrap_if_func((void*)host_pa_context_set_state_callback, "pa_context_set_state_callback");
	call_stub(stub, 0x7f00028000, 0x5500f00, 0x7f00029000, 0);
	check(g_pa_seen_cb != 0 && !is_fake_guest_fn(g_pa_seen_cb), "pa_context_set_state_callback gets a thunk");
	stub = fex_dlsym_wrap_if_func((void*)host_pa_stream_write, "pa_stream_write");
	call_stub(stub, 0x7f00028000, 0x7f00029000, 64, 0x5501000);
	check(g_pa_seen_cb != 0 && !is_fake_guest_fn(g_pa_seen_cb), "pa_stream_write gets a thunk for free_cb");
}

static uint64_t g_xim_values[4];
// What libX11 keeps of XVaCreateNestedList(0, name1, value1, name2, value2, NULL): the values.
static uint64_t host_XVaCreateNestedList(uint64_t unused, uint64_t n1, uint64_t v1, uint64_t n2, uint64_t v2, uint64_t end)
{
	(void)unused;
	(void)n1;
	(void)n2;
	(void)end;
	g_xim_values[0] = v1;
	g_xim_values[1] = v2;
	return 0x9200;
}

static uint64_t host_XSetICValues(uint64_t ic, uint64_t n1, uint64_t v1, uint64_t end)
{
	(void)ic;
	(void)n1;
	(void)end;
	g_xim_values[2] = v1;
	return 0;
}

// A (name, value) list with no terminator among the 22 words the bridge copies.
static void set_overlong_xim_list(void)
{
	static uint64_t stack[17];
	for (int i = 0; i < 17; i++)
		stack[i] = (uint64_t)(uintptr_t)"spotLocation";
	void* set = fex_dlsym_wrap_if_func((void*)host_XSetICValues, "XSetICValues");
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = stub_syscall_nr(set);
	st.rsi = st.rdx = st.r10 = st.r8 = st.r9 = (uint64_t)(uintptr_t)"spotLocation";
	st.rsp = (uint64_t)(uintptr_t)stack;
	fex_linux_syscall_dispatch(&st);
}

// The XIMCallback { client_data, callback } structs in Xlib's input method value lists hold guest
// callbacks; libX11 is given host copies holding thunks.
static void test_xim_callbacks(void)
{
	uint64_t start_cb[2] = {0x7f00014000, 0x5500000};
	uint64_t point = 0x00050006;
	void* nested = fex_dlsym_wrap_if_func((void*)host_XVaCreateNestedList, "XVaCreateNestedList");
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = stub_syscall_nr(nested);
	st.rsi = (uint64_t)(uintptr_t)"preeditStartCallback";
	st.rdx = (uint64_t)(uintptr_t)start_cb;
	st.r10 = (uint64_t)(uintptr_t)"spotLocation";
	st.r8 = (uint64_t)(uintptr_t)&point;
	fex_linux_syscall_dispatch(&st);

	const uint64_t* host_cb = (const uint64_t*)(uintptr_t)g_xim_values[0];
	bool copied = host_cb != NULL && host_cb != start_cb && !is_fake_guest_fn(host_cb[1]);
	check(copied && host_cb[0] == 0x7f00014000,
		"a preedit callback in a nested list reaches libX11 as a host XIMCallback with a thunk");
	check(g_xim_values[1] == (uint64_t)(uintptr_t)&point, "values of other names are passed on as they are");
	check(start_cb[1] == 0x5500000, "the guest's XIMCallback is left alone");
	last_cb_args_fn = 0;
	if (copied)
		((void (*)(uint64_t, uint64_t, uint64_t))(uintptr_t)host_cb[1])(0x7f00015000, host_cb[0], 0x7f00016000);
	check(last_cb_args_fn == 0x5500000 && last_cb_args[0] == 0x7f00015000 && last_cb_args[1] == 0x7f00014000 &&
	      last_cb_args[2] == 0x7f00016000,
		"the input method's call runs the guest callback with the IC, client data and call data");

	void* set = fex_dlsym_wrap_if_func((void*)host_XSetICValues, "XSetICValues");
	call_stub(set, 0x7f00017000, (uint64_t)(uintptr_t)"preeditStartCallback", (uint64_t)(uintptr_t)start_cb, 0);
	check(copied && g_xim_values[2] == g_xim_values[0], "the same guest XIMCallback keeps one host copy");

	check(aborts_with(set_overlong_xim_list, "an Xlib input method value list longer than 22 arguments is not supported"),
		"a value list longer than the bridge copies stops the process and says why");
}

static uint64_t g_debug_callback_seen = 1;
static uint64_t host_glDebugMessageCallback(uint64_t callback, uint64_t user_param)
{
	(void)user_param;
	g_debug_callback_seen = callback;
	return 0;
}

static void set_guest_debug_callback(void)
{
	void* stub = fex_dlsym_wrap_if_func((void*)host_glDebugMessageCallback, "glDebugMessageCallback");
	call_stub(stub, 0x5300000, 0, 0, 0);
}

// A callback with more arguments than a thunk forwards stops the process with a message instead of
// being called later with arguments the guest never sees.
static void test_unsupported_callback(void)
{
	void* stub = fex_dlsym_wrap_if_func((void*)host_glDebugMessageCallback, "glDebugMessageCallback");
	call_stub(stub, 0, 0x1234, 0, 0);
	check(g_debug_callback_seen == 0, "removing an unsupported callback (NULL) still reaches the host");
	check(aborts_with(set_guest_debug_callback, "glDebugMessageCallback was given a guest callback, which the FEX "
		"bridge cannot run: its callback takes seven arguments"),
		"a guest callback the bridge cannot run stops the process and says why");
}

static float g_clear_color[4];
static void host_glClearColor(float r, float g, float b, float a)
{
	g_clear_color[0] = r;
	g_clear_color[1] = g;
	g_clear_color[2] = b;
	g_clear_color[3] = a;
}

static double host_glHalf(double d)
{
	return d / 2;
}

static uint64_t host_glDebugMessageCallbackKHR(uint64_t callback, uint64_t user_param)
{
	(void)callback;
	(void)user_param;
	return 0;
}

static void* host_eglGetProcAddress(const char* name)
{
	if (strcmp(name, "glClearColor") == 0)
		return (void*)host_glClearColor;
	if (strcmp(name, "glHalf") == 0)
		return (void*)host_glHalf;
	if (strcmp(name, "glDebugMessageCallbackKHR") == 0)
		return (void*)host_glDebugMessageCallbackKHR;
	return NULL;
}

// Calls a stub with one integer argument and floating point arguments in xmm0..; returns xmm0's low 8 bytes.
static uint64_t call_stub_xmm(void* stub, uint64_t a0, const uint64_t* xmm, int nxmm)
{
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = stub_syscall_nr(stub);
	st.rdi = a0;
	for (int i = 0; i < nxmm; i++)
		st.xmm[i][0] = xmm[i];
	fex_linux_syscall_dispatch(&st);
	return st.xmm[0][0];
}

static uint64_t float_bits(float f)
{
	uint32_t u;
	memcpy(&u, &f, sizeof(u));
	return u;
}

static void set_guest_khr_debug_callback(void)
{
	void* lookup = fex_dlsym_wrap_if_func((void*)host_eglGetProcAddress, "eglGetProcAddress");
	void* fn = (void*)(uintptr_t)call_stub(lookup, (uint64_t)(uintptr_t)"glDebugMessageCallbackKHR", 0, 0, 0);
	call_stub(fn, 0x5300100, 0, 0, 0);
}

// eglGetProcAddress returns host functions, which the guest calls: it gets stubs, with the rules of the name.
static void test_proc_address_lookup(void)
{
	void* lookup = fex_dlsym_wrap_if_func((void*)host_eglGetProcAddress, "eglGetProcAddress");
	uint64_t clear = call_stub(lookup, (uint64_t)(uintptr_t)"glClearColor", 0, 0, 0);
	check(clear != 0 && clear != (uint64_t)(uintptr_t)host_glClearColor &&
	      stub_syscall_nr((void*)(uintptr_t)clear) == stub_syscall_nr(fex_dlsym_wrap_if_func((void*)host_glClearColor, "x")),
		"eglGetProcAddress gives the guest the stub of the host function");
	check(call_stub(lookup, (uint64_t)(uintptr_t)"glClearColor", 0, 0, 0) == clear,
		"looking a function up twice gives the same stub");
	check(call_stub(lookup, (uint64_t)(uintptr_t)"glNoSuchFunction", 0, 0, 0) == 0, "a failed lookup stays NULL");

	const uint64_t colors[4] = {float_bits(0.25f), float_bits(0.5f), float_bits(0.75f), float_bits(1.0f)};
	call_stub_xmm((void*)(uintptr_t)clear, 0, colors, 4);
	check(g_clear_color[0] == 0.25f && g_clear_color[1] == 0.5f && g_clear_color[2] == 0.75f && g_clear_color[3] == 1.0f,
		"a looked-up function gets its float arguments");
	void* half = (void*)(uintptr_t)call_stub(lookup, (uint64_t)(uintptr_t)"glHalf", 0, 0, 0);
	double in = 3.0, out = 0;
	uint64_t in_bits, out_bits;
	memcpy(&in_bits, &in, sizeof(in));
	out_bits = call_stub_xmm(half, 0, &in_bits, 1);
	memcpy(&out, &out_bits, sizeof(out));
	check(out == 1.5, "a looked-up function returns a double in xmm0");

	check(aborts_with(set_guest_khr_debug_callback, "glDebugMessageCallbackKHR was given a guest callback"),
		"a looked-up function keeps the callback rules of its name");
}

// libjpeg fakes. An error manager starts with five methods; a libjpeg object starts with its err pointer.
struct fake_jpeg_error_mgr {
	uint64_t method[5]; // error_exit, emit_message, output_message, format_message, reset_error_mgr
	int64_t msg_code;
	uint64_t guest_extension; // what a guest keeps after the struct (Onyx2D: its jmp_buf)
};
struct fake_jpeg_object {
	uint64_t err;
	uint64_t state;
};
enum { JPEG_ERROR_EXIT, JPEG_EMIT, JPEG_OUTPUT, JPEG_FORMAT, JPEG_RESET };

static bool is_guest_stub(uint64_t fn)
{
	static const uint8_t stub_start[3] = {0x49, 0x89, 0xca};
	return fn >= 0x10000 && memcmp((const void*)(uintptr_t)fn, stub_start, sizeof(stub_start)) == 0;
}

// A method host code may call natively: neither a fake guest function nor a guest stub.
static bool host_callable(uint64_t fn)
{
	return fn != 0 && !is_fake_guest_fn(fn) && !is_guest_stub(fn);
}

static const struct fake_jpeg_error_mgr* jpeg_err_of(uint64_t object)
{
	return (const struct fake_jpeg_error_mgr*)(uintptr_t)((const struct fake_jpeg_object*)(uintptr_t)object)->err;
}

static char g_jpeg_formatted[32];
static void host_jpeg_format_message(uint64_t object, char* buffer)
{
	(void)object;
	strcpy(buffer, "corrupt data");
}

static int g_jpeg_outputs = 0;
static void host_jpeg_output_message(uint64_t object)
{
	const struct fake_jpeg_error_mgr* err = jpeg_err_of(object);
	g_jpeg_outputs++;
	if (!host_callable(err->method[JPEG_FORMAT]))
		return;
	((void (*)(uint64_t, char*))(uintptr_t)err->method[JPEG_FORMAT])(object, g_jpeg_formatted);
}

static void host_jpeg_error_exit(uint64_t object) { host_jpeg_output_message(object); }
static void host_jpeg_emit_message(uint64_t object, int level) { (void)object; (void)level; }
static void host_jpeg_reset_error_mgr(uint64_t object) { (void)object; }

static uint64_t host_jpeg_std_error(uint64_t err)
{
	struct fake_jpeg_error_mgr* e = (struct fake_jpeg_error_mgr*)(uintptr_t)err;
	e->method[JPEG_ERROR_EXIT] = (uint64_t)(uintptr_t)host_jpeg_error_exit;
	e->method[JPEG_EMIT] = (uint64_t)(uintptr_t)host_jpeg_emit_message;
	e->method[JPEG_OUTPUT] = (uint64_t)(uintptr_t)host_jpeg_output_message;
	e->method[JPEG_FORMAT] = (uint64_t)(uintptr_t)host_jpeg_format_message;
	e->method[JPEG_RESET] = (uint64_t)(uintptr_t)host_jpeg_reset_error_mgr;
	e->msg_code = 0;
	return err;
}

// Reads a header that turns out corrupt: reports the error through err->error_exit, as libjpeg does.
static uint64_t g_jpeg_host_view_ok;
static uint64_t host_jpeg_read_header(uint64_t object, uint64_t require_image)
{
	(void)require_image;
	const struct fake_jpeg_error_mgr* err = jpeg_err_of(object);
	g_jpeg_host_view_ok = 1;
	for (int m = 0; m < 5; m++)
		g_jpeg_host_view_ok &= host_callable(err->method[m]);
	if (!g_jpeg_host_view_ok)
		return GOT_GUEST_FN;
	((struct fake_jpeg_error_mgr*)err)->msg_code = 42;
	((void (*)(uint64_t))(uintptr_t)err->method[JPEG_ERROR_EXIT])(object);
	for (int m = 0; m < 5; m++)
		g_jpeg_host_view_ok &= host_callable(err->method[m]);
	return 1;
}

static uint64_t host_jpeg_quality_scaling(uint64_t quality)
{
	return quality * 2;
}

#define GUEST_ERROR_EXIT 0x5500000u
static struct fake_jpeg_error_mgr g_guest_err;
static bool g_guest_view_in_error_exit;
static uint64_t g_guest_saw_msg_code;
static jmp_buf g_guest_longjmp_target;

// The guest's error_exit sees its own view of the manager: its function and stubs, and the fields libjpeg set.
static void guest_error_exit_returns(uint64_t object, uint64_t arg)
{
	(void)arg;
	const struct fake_jpeg_error_mgr* err = jpeg_err_of(object);
	g_guest_view_in_error_exit = err == &g_guest_err && err->method[JPEG_ERROR_EXIT] == GUEST_ERROR_EXIT &&
		is_guest_stub(err->method[JPEG_OUTPUT]) && is_guest_stub(err->method[JPEG_FORMAT]);
	g_guest_saw_msg_code = (uint64_t)err->msg_code;
}

static void guest_error_exit_longjmps(uint64_t object, uint64_t arg)
{
	guest_error_exit_returns(object, arg);
	longjmp(g_guest_longjmp_target, 1);
}

static bool guest_view_of(const struct fake_jpeg_error_mgr* err)
{
	return err->method[JPEG_ERROR_EXIT] == GUEST_ERROR_EXIT && is_guest_stub(err->method[JPEG_EMIT]) &&
		is_guest_stub(err->method[JPEG_OUTPUT]) && is_guest_stub(err->method[JPEG_FORMAT]) &&
		is_guest_stub(err->method[JPEG_RESET]);
}

static jmp_buf g_host_longjmp_target;
// Leaves libjpeg by a longjmp in host code (as a signal handler's siglongjmp would), not through error_exit.
static uint64_t host_jpeg_abort_decompress(uint64_t object)
{
	(void)object;
	longjmp(g_host_longjmp_target, 1);
}

static sem_t g_jpeg_inside, g_jpeg_release;
static uint64_t host_jpeg_start_decompress(uint64_t object)
{
	(void)object;
	sem_post(&g_jpeg_inside);
	sem_wait(&g_jpeg_release);
	return 1;
}

static uint64_t g_shared_object;
static void* decode_on_thread(void* stub)
{
	call_stub(stub, g_shared_object, 0, 0, 0);
	return NULL;
}

// Two threads decoding with one error manager at once.
static void share_error_manager_between_threads(void)
{
	void* std_error = fex_dlsym_wrap_if_func((void*)host_jpeg_std_error, "jpeg_std_error");
	void* start = fex_dlsym_wrap_if_func((void*)host_jpeg_start_decompress, "jpeg_start_decompress");
	void* read_header = fex_dlsym_wrap_if_func((void*)host_jpeg_read_header, "jpeg_read_header");
	static struct fake_jpeg_error_mgr shared;
	call_stub(std_error, (uint64_t)(uintptr_t)&shared, 0, 0, 0);
	static struct fake_jpeg_object a, b;
	a.err = b.err = (uint64_t)(uintptr_t)&shared;
	g_shared_object = (uint64_t)(uintptr_t)&a;
	sem_init(&g_jpeg_inside, 0, 0);
	sem_init(&g_jpeg_release, 0, 0);
	pthread_t t;
	pthread_create(&t, NULL, decode_on_thread, start);
	sem_wait(&g_jpeg_inside);
	call_stub(read_header, (uint64_t)(uintptr_t)&b, 1, 0, 0);
}

static void test_jpeg_error_manager(void)
{
	void* std_error = fex_dlsym_wrap_if_func((void*)host_jpeg_std_error, "jpeg_std_error");
	void* read_header = fex_dlsym_wrap_if_func((void*)host_jpeg_read_header, "jpeg_read_header");
	void* quality = fex_dlsym_wrap_if_func((void*)host_jpeg_quality_scaling, "jpeg_quality_scaling");

	memset(&g_guest_err, 0, sizeof(g_guest_err));
	call_stub(std_error, (uint64_t)(uintptr_t)&g_guest_err, 0, 0, 0);
	bool all_stubs = true;
	for (int m = 0; m < 5; m++)
		all_stubs &= is_guest_stub(g_guest_err.method[m]);
	check(all_stubs, "jpeg_std_error leaves guest-callable stubs for its host methods in the guest's struct");

	struct fake_jpeg_object cinfo = {(uint64_t)(uintptr_t)&g_guest_err, 0};
	g_jpeg_outputs = 0;
	g_jpeg_formatted[0] = '\0';
	call_stub((void*)(uintptr_t)g_guest_err.method[JPEG_OUTPUT], (uint64_t)(uintptr_t)&cinfo, 0, 0, 0);
	check(g_jpeg_outputs == 1 && strcmp(g_jpeg_formatted, "corrupt data") == 0,
		"a guest call of output_message runs the host method, which calls format_message natively");

	g_guest_err.method[JPEG_ERROR_EXIT] = GUEST_ERROR_EXIT;
	g_guest_err.guest_extension = 0x1234;
	guest_callback_body = guest_error_exit_returns;
	g_guest_view_in_error_exit = false;
	last_cb_fn = 0;
	uint64_t ret = call_stub(read_header, (uint64_t)(uintptr_t)&cinfo, 1, 0, 0);
	check(ret == 1 && g_jpeg_host_view_ok, "libjpeg finds host-callable methods in the error manager while it runs");
	check(last_cb_fn == GUEST_ERROR_EXIT && last_cb_arg0 == (uint64_t)(uintptr_t)&cinfo,
		"libjpeg's call of error_exit runs the guest's function with the object");
	check(g_guest_view_in_error_exit && g_guest_saw_msg_code == 42,
		"the guest's error_exit sees its own struct, with the guest's methods and libjpeg's fields");
	check(guest_view_of(&g_guest_err) && g_guest_err.guest_extension == 0x1234 && cinfo.err == (uint64_t)(uintptr_t)&g_guest_err,
		"after the call the guest's error manager holds the guest's methods again");

	// Onyx2D's error_exit longjmps out of libjpeg; the manager must be back in the guest view all the same.
	guest_callback_body = guest_error_exit_longjmps;
	g_guest_view_in_error_exit = false;
	if (setjmp(g_guest_longjmp_target) == 0)
		call_stub(read_header, (uint64_t)(uintptr_t)&cinfo, 1, 0, 0);
	check(g_guest_view_in_error_exit && guest_view_of(&g_guest_err),
		"a guest error_exit that longjmps out of libjpeg leaves the guest view in place");
	guest_callback_body = guest_error_exit_returns;
	g_jpeg_host_view_ok = 0;
	ret = call_stub(read_header, (uint64_t)(uintptr_t)&cinfo, 1, 0, 0);
	check(ret == 1 && g_jpeg_host_view_ok && guest_view_of(&g_guest_err),
		"the error manager works for later libjpeg calls after a longjmp out of error_exit");
	guest_callback_body = NULL;

	// A libjpeg call left without the bridge seeing it, then a new error manager at the same address.
	void* abort_decompress = fex_dlsym_wrap_if_func((void*)host_jpeg_abort_decompress, "jpeg_abort_decompress");
	if (setjmp(g_host_longjmp_target) == 0)
		call_stub(abort_decompress, (uint64_t)(uintptr_t)&cinfo, 0, 0, 0);
	memset(&g_guest_err, 0, sizeof(g_guest_err));
	call_stub(std_error, (uint64_t)(uintptr_t)&g_guest_err, 0, 0, 0);
	g_guest_err.method[JPEG_ERROR_EXIT] = GUEST_ERROR_EXIT;
	guest_callback_body = guest_error_exit_returns;
	g_jpeg_host_view_ok = 0;
	ret = call_stub(read_header, (uint64_t)(uintptr_t)&cinfo, 1, 0, 0);
	guest_callback_body = NULL;
	check(ret == 1 && g_jpeg_host_view_ok && guest_view_of(&g_guest_err),
		"an error manager at the address of one whose libjpeg call was left unseen gets the host view");
	check(aborts_with(share_error_manager_between_threads, "a libjpeg error manager is used by two threads at once"),
		"one error manager used by two threads at once stops the process and says why");

	check(call_stub(quality, 75, 0, 0, 0) == 150, "a jpeg_* function whose first argument is not an object is called as is");
	uint64_t not_a_manager[5] = {0x5500100, 0x5500200, 7, 8, 9};
	struct fake_jpeg_object foreign = {(uint64_t)(uintptr_t)not_a_manager, 0};
	check(call_stub(read_header, (uint64_t)(uintptr_t)&foreign, 1, 0, 0) == GOT_GUEST_FN && not_a_manager[0] == 0x5500100 &&
	      not_a_manager[4] == 9,
		"memory that jpeg_std_error did not fill is not treated as an error manager");
	struct fake_jpeg_object unset = {0, 0};
	call_stub(quality, (uint64_t)(uintptr_t)&unset, 0, 0, 0);
	check(unset.err == 0, "an object whose err pointer is NULL is left alone");
}

static void test_syscall_translations(void)
{
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));

	// 1. arch_prctl ARCH_SET_GS (0x1001) & ARCH_GET_GS (0x1004)
	st.rax = 158;
	st.rdi = 0x1001;
	st.rsi = 0xabcdef001122ULL;
	check(fex_linux_syscall_dispatch(&st) == 0 && st.gs_base == 0xabcdef001122ULL,
		"arch_prctl(ARCH_SET_GS) sets gs_base");

	uint64_t read_gs = 0;
	st.rax = 158;
	st.rdi = 0x1004;
	st.rsi = (uint64_t)(uintptr_t)&read_gs;
	check(fex_linux_syscall_dispatch(&st) == 0 && read_gs == 0xabcdef001122ULL,
		"arch_prctl(ARCH_GET_GS) reads back gs_base");

	// 2. pipe (22) + dup2(fd, fd) (33) + poll (7)
	int pfd[2] = {-1, -1};
	memset(&st, 0, sizeof(st));
	st.rax = 22;
	st.rdi = (uint64_t)(uintptr_t)pfd;
	check(fex_linux_syscall_dispatch(&st) == 0 && pfd[0] >= 0 && pfd[1] >= 0,
		"x86_64 pipe(22) creates pipe via pipe2");

	memset(&st, 0, sizeof(st));
	st.rax = 33;
	st.rdi = (uint64_t)pfd[0];
	st.rsi = (uint64_t)pfd[0];
	check(fex_linux_syscall_dispatch(&st) == pfd[0],
		"x86_64 dup2(fd, fd) succeeds with fd");

	memset(&st, 0, sizeof(st));
	st.rax = 33;
	st.rdi = (uint64_t)-1;
	st.rsi = (uint64_t)-1;
	check(fex_linux_syscall_dispatch(&st) == -EBADF,
		"x86_64 dup2(-1, -1) returns -EBADF");

	struct pollfd poll_entry = { .fd = pfd[0], .events = POLLIN, .revents = 0 };
	memset(&st, 0, sizeof(st));
	st.rax = 7;
	st.rdi = (uint64_t)(uintptr_t)&poll_entry;
	st.rsi = 1;
	st.rdx = 0; // timeout_ms = 0
	check(fex_linux_syscall_dispatch(&st) == 0,
		"x86_64 poll(timeout_ms=0) converts timeout to timespec for ppoll");

	// 3. epoll_create (213), epoll_ctl (233), epoll_wait (232) with 12-byte packed event
	memset(&st, 0, sizeof(st));
	st.rax = 213;
	st.rdi = 1;
	long epfd = fex_linux_syscall_dispatch(&st);
	check(epfd >= 0, "x86_64 epoll_create(213) creates epoll fd");

	struct __attribute__((packed)) {
		uint32_t events;
		uint64_t data;
	} ev_in = { .events = EPOLLOUT, .data = 0x1122334455667788ULL }, ev_out = {0, 0};

	memset(&st, 0, sizeof(st));
	st.rax = 233;
	st.rdi = (uint64_t)epfd;
	st.rsi = EPOLL_CTL_ADD;
	st.rdx = (uint64_t)pfd[1];
	st.r10 = (uint64_t)(uintptr_t)&ev_in;
	check(fex_linux_syscall_dispatch(&st) == 0, "x86_64 epoll_ctl(233) unpacks 12-byte epoll_event");

	memset(&st, 0, sizeof(st));
	st.rax = 232;
	st.rdi = (uint64_t)epfd;
	st.rsi = (uint64_t)(uintptr_t)&ev_out;
	st.rdx = 1;
	st.r10 = 0;
	check(fex_linux_syscall_dispatch(&st) == 1 && ev_out.data == 0x1122334455667788ULL,
		"x86_64 epoll_wait(232) packs 12-byte epoll_event preserving 64-bit data");

	close((int)epfd);
	close(pfd[0]);
	close(pfd[1]);

	// 4. memfd_create (319) + write (1) + fstat (5) layout check
	memset(&st, 0, sizeof(st));
	st.rax = 319;
	st.rdi = (uint64_t)(uintptr_t)"fex_stat_test";
	st.rsi = 0;
	long mfd = fex_linux_syscall_dispatch(&st);
	check(mfd >= 0, "x86_64 memfd_create(319) succeeds");

	const char payload[37] = "0123456789abcdefghijklmnopqrstuvwxyz";
	memset(&st, 0, sizeof(st));
	st.rax = 1;
	st.rdi = (uint64_t)mfd;
	st.rsi = (uint64_t)(uintptr_t)payload;
	st.rdx = sizeof(payload);
	check(fex_linux_syscall_dispatch(&st) == (long)sizeof(payload), "x86_64 write(1) writes 37 bytes");

	uint8_t raw_stat[144];
	memset(raw_stat, 0, sizeof(raw_stat));
	memset(&st, 0, sizeof(st));
	st.rax = 5;
	st.rdi = (uint64_t)mfd;
	st.rsi = (uint64_t)(uintptr_t)raw_stat;
	check(fex_linux_syscall_dispatch(&st) == 0, "x86_64 fstat(5) succeeds");

	struct stat host_st;
	fstat((int)mfd, &host_st);

	uint64_t st_nlink = 0;
	uint32_t st_mode = 0;
	int64_t st_size = 0;
	memcpy(&st_nlink, raw_stat + 16, sizeof(st_nlink));
	memcpy(&st_mode, raw_stat + 24, sizeof(st_mode));
	memcpy(&st_size, raw_stat + 48, sizeof(st_size));
	check(st_nlink == (uint64_t)host_st.st_nlink && S_ISREG(st_mode) && st_size == 37,
		"x86_64 fstat populates st_nlink@16, st_mode@24, and st_size@48==37");

	close((int)mfd);

	// 5. clone (56) and clone3 (435) return -ENOSYS
	// Thread creation must go through darling_thread_create ELF call, not raw clone.
	// For clone(56): we pass CLONE_VM | CLONE_THREAD with NULL stack, which would return
	// -EINVAL from an unfiltered host kernel, verifying our explicit -ENOSYS intercept
	// and register writeback (st.rax == (uint64_t)(long)-ENOSYS).
	// For clone3(435): on an unfiltered Linux kernel, invalid args (NULL, 0) return -EINVAL;
	// some seccomp sandboxes pre-filter raw clone3 to -ENOSYS. Our intercept guarantees
	// uniform -ENOSYS across all environments.
	memset(&st, 0, sizeof(st));
	st.rax = 56;
	st.rdi = 0x00010000 | 0x00000100; // CLONE_VM | CLONE_THREAD
	st.rsi = 0; // null stack
	check(fex_linux_syscall_dispatch(&st) == -ENOSYS && st.rax == (uint64_t)(long)-ENOSYS,
		"x86_64 clone(56) returns -ENOSYS and updates rax");

	memset(&st, 0, sizeof(st));
	st.rax = 435;
	st.rdi = 0;
	st.rsi = 0;
	check(fex_linux_syscall_dispatch(&st) == -ENOSYS && st.rax == (uint64_t)(long)-ENOSYS,
		"x86_64 clone3(435) returns -ENOSYS and updates rax");
}

// Fake guest stacks for fex_left_callback: one mapping per 16 MB.
static bool fake_same_stack(uint64_t a, uint64_t b)
{
	return a >> 24 == b >> 24;
}

// Which callbacks a guest left, from its stack pointer: two callbacks nested on a thread stack in
// [0x10000000, 0x11000000), an outer one at 0x10f00000 and an inner one at 0x10e00000.
static void test_left_callbacks(void)
{
	const struct fex_callback_frame outer = {0x10f00000, 0, 0, NULL};
	const struct fex_callback_frame inner = {0x10e00000, 0, 0, &outer};
	const struct fex_guest_stack no_altstack = {0, FEX_GUEST_SS_DISABLE, 0, 0};
	const struct fex_guest_stack altstack_high = {0x12000000, 0, 0, 0x10000};
	const struct fex_guest_stack altstack_on_thread = {0x10e80000, 0, 0, 0x10000};
	const struct {
		uint64_t rsp;
		const struct fex_guest_stack* altstack;
		const struct fex_callback_frame* left;
		const char* what;
	} cases[] = {
		{0x10dfff00, &no_altstack, NULL, "the guest runs inside the inner callback"},
		{0x10e00000 - 16, &no_altstack, NULL, "the inner callback's first instruction"},
		{0x10e00000, &no_altstack, &inner, "a longjmp to the inner callback's host call level"},
		{0x10e80000, &no_altstack, &inner, "a longjmp into the outer callback's body"},
		{0x10f00000, &no_altstack, &outer, "a longjmp to the outer callback's host call level"},
		{0x10f80000, &no_altstack, &outer, "a longjmp further out on the same stack"},
		{0x11800000, &no_altstack, NULL, "a higher stack the callback switched to"},
		{0x12008000, &altstack_high, NULL, "a handler on the alternate stack"},
		{0x10e88000, &altstack_on_thread, NULL, "a handler on an alternate stack inside the thread's stack"},
	};
	bool all = true;
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		const struct fex_callback_frame* got = fex_left_callback(&inner, cases[i].rsp, cases[i].altstack, fake_same_stack);
		if (got != cases[i].left) {
			printf("  wrong callback for %s\n", cases[i].what);
			all = false;
		}
	}
	check(all, "fex_left_callback picks the outermost callback left, and none for other stacks and handlers");

	// A host call made from the alternate stack: a longjmp back to it is still a longjmp.
	const struct fex_callback_frame on_alt = {0x12008000, 0, 0, NULL};
	check(fex_left_callback(&on_alt, 0x12008000, &altstack_high, fake_same_stack) == &on_alt,
		"a callback whose host call ran on the alternate stack is left by a longjmp to it");
	// A callback on its own known stack: only that range counts, whatever same_stack says.
	const struct fex_callback_frame detached = {0x20ff0000, 0x20f80000, 0x21000000, NULL};
	check(fex_left_callback(&detached, 0x20ff8000, &no_altstack, fake_same_stack) == &detached &&
	      fex_left_callback(&detached, 0x21000000, &no_altstack, fake_same_stack) == NULL,
		"a callback on the callback stack is left only for an address on that stack");
	check(fex_returned_callback(&inner, 0x10f00000) == &outer && fex_returned_callback(&inner, 0x10e00000) == &inner &&
	      fex_returned_callback(&inner, 0x10e80000) == NULL,
		"fex_returned_callback finds the callback whose host call has the stack pointer of a return");
}

static void test_safe_memory_access(void)
{
	// 1. Direct fex_copy_to_guest and fex_copy_from_guest tests
	uint64_t val = 0x1234567890abcdefULL, out = 0;
	check(fex_copy_to_guest(&out, &val, sizeof(val)) == 0 && out == val,
		"fex_copy_to_guest succeeds with valid pointer");
	uint64_t from_out = 0;
	check(fex_copy_from_guest(&from_out, &val, sizeof(val)) == 0 && from_out == val,
		"fex_copy_from_guest succeeds with valid pointer");
	check(fex_copy_to_guest(&out, &val, 0) == 0, "fex_copy_to_guest len=0 returns 0");
	check(fex_copy_from_guest(&from_out, &val, 0) == 0, "fex_copy_from_guest len=0 returns 0");
	check(fex_copy_to_guest(NULL, &val, sizeof(val)) == -EFAULT, "fex_copy_to_guest NULL returns -EFAULT");
	check(fex_copy_from_guest(&from_out, NULL, sizeof(val)) == -EFAULT, "fex_copy_from_guest NULL returns -EFAULT");
	check(fex_copy_to_guest((void*)1, &val, sizeof(val)) == -EFAULT, "fex_copy_to_guest bad pointer returns -EFAULT");
	check(fex_copy_from_guest(&from_out, (void*)1, sizeof(val)) == -EFAULT, "fex_copy_from_guest bad pointer returns -EFAULT");

	// 2. Straddling page test for partial transfers
	const size_t ps = sysconf(_SC_PAGESIZE);
	void* pages = mmap(NULL, 2 * ps, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	check(pages != MAP_FAILED, "allocate 2 pages for straddle test");
	if (pages != MAP_FAILED) {
		mprotect((char*)pages + ps, ps, PROT_NONE);
		void* straddle_ptr = (char*)pages + ps - 4;
		uint64_t straddle_buf = 0x55aa55aa55aa55aaULL;
		check(fex_copy_to_guest(straddle_ptr, &straddle_buf, 8) == -EFAULT,
			"fex_copy_to_guest across straddling PROT_NONE page returns -EFAULT");
		check(fex_copy_from_guest(&straddle_buf, straddle_ptr, 8) == -EFAULT,
			"fex_copy_from_guest across straddling PROT_NONE page returns -EFAULT");
		munmap(pages, 2 * ps);
	}

	// 3. PROT_READ page test: writing to read-only memory returns -EFAULT
	void* ro_page = mmap(NULL, ps, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	check(ro_page != MAP_FAILED, "allocate PROT_READ page for read-only write test");
	if (ro_page != MAP_FAILED) {
		uint64_t write_val = 0xdeadbeefULL;
		check(fex_copy_to_guest(ro_page, &write_val, sizeof(write_val)) == -EFAULT,
			"fex_copy_to_guest into PROT_READ page returns -EFAULT");
	}

	// 4. arch_prctl GET_GS and GET_FS on bad pointer return -EFAULT without crash
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = 158;
	st.rdi = 0x1004; // ARCH_GET_GS
	st.rsi = 1;      // bad pointer (void*)1
	check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
		"arch_prctl(ARCH_GET_GS) on bad pointer returns -EFAULT and updates rax");

	memset(&st, 0, sizeof(st));
	st.rax = 158;
	st.rdi = 0x1003; // ARCH_GET_FS
	st.rsi = 1;      // bad pointer (void*)1
	check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
		"arch_prctl(ARCH_GET_FS) on bad pointer returns -EFAULT and updates rax");

	// 5. stat / lstat / fstat / newfstatat with NULL, (void*)1, PROT_READ, and valid buffers
	int tmp_fd = memfd_create("efault_stat_test", 0);
	check(tmp_fd >= 0, "memfd_create for efault stat test");

	if (tmp_fd >= 0) {
		uint8_t raw_stat_buf[144];

		// fstat (5)
		memset(&st, 0, sizeof(st));
		st.rax = 5;
		st.rdi = (uint64_t)tmp_fd;
		st.rsi = 0; // NULL
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"fstat with NULL statbuf returns -EFAULT and updates rax");

		memset(&st, 0, sizeof(st));
		st.rax = 5;
		st.rdi = (uint64_t)tmp_fd;
		st.rsi = 1; // (void*)1
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"fstat with bad statbuf returns -EFAULT without crash");

		if (ro_page != MAP_FAILED) {
			memset(&st, 0, sizeof(st));
			st.rax = 5;
			st.rdi = (uint64_t)tmp_fd;
			st.rsi = (uint64_t)(uintptr_t)ro_page;
			check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
				"fstat with PROT_READ statbuf returns -EFAULT");
		}

		memset(&st, 0, sizeof(st));
		st.rax = 5;
		st.rdi = (uint64_t)tmp_fd;
		st.rsi = (uint64_t)(uintptr_t)raw_stat_buf;
		check(fex_linux_syscall_dispatch(&st) == 0 && st.rax == 0,
			"fstat with valid statbuf succeeds");

		// stat (4)
		memset(&st, 0, sizeof(st));
		st.rax = 4;
		st.rdi = (uint64_t)(uintptr_t)"/dev/null";
		st.rsi = 0; // NULL
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"stat with NULL statbuf returns -EFAULT and updates rax");

		memset(&st, 0, sizeof(st));
		st.rax = 4;
		st.rdi = (uint64_t)(uintptr_t)"/dev/null";
		st.rsi = 1; // (void*)1
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"stat with bad statbuf returns -EFAULT without crash");

		memset(&st, 0, sizeof(st));
		st.rax = 4;
		st.rdi = (uint64_t)(uintptr_t)"/dev/null";
		st.rsi = (uint64_t)(uintptr_t)raw_stat_buf;
		check(fex_linux_syscall_dispatch(&st) == 0 && st.rax == 0,
			"stat with valid statbuf succeeds");

		// lstat (6)
		memset(&st, 0, sizeof(st));
		st.rax = 6;
		st.rdi = (uint64_t)(uintptr_t)"/dev/null";
		st.rsi = 0; // NULL
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"lstat with NULL statbuf returns -EFAULT");

		memset(&st, 0, sizeof(st));
		st.rax = 6;
		st.rdi = (uint64_t)(uintptr_t)"/dev/null";
		st.rsi = 1; // (void*)1
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"lstat with bad statbuf returns -EFAULT without crash");

		memset(&st, 0, sizeof(st));
		st.rax = 6;
		st.rdi = (uint64_t)(uintptr_t)"/dev/null";
		st.rsi = (uint64_t)(uintptr_t)raw_stat_buf;
		check(fex_linux_syscall_dispatch(&st) == 0 && st.rax == 0,
			"lstat with valid statbuf succeeds");

		// newfstatat (262)
		memset(&st, 0, sizeof(st));
		st.rax = 262;
		st.rdi = (uint64_t)tmp_fd;
		st.rsi = (uint64_t)(uintptr_t)"";
		st.rdx = 0; // NULL
		st.r10 = AT_EMPTY_PATH;
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"newfstatat with NULL statbuf returns -EFAULT");

		memset(&st, 0, sizeof(st));
		st.rax = 262;
		st.rdi = (uint64_t)tmp_fd;
		st.rsi = (uint64_t)(uintptr_t)"";
		st.rdx = 1; // bad pointer
		st.r10 = AT_EMPTY_PATH;
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"newfstatat with bad statbuf returns -EFAULT");

		memset(&st, 0, sizeof(st));
		st.rax = 262;
		st.rdi = (uint64_t)tmp_fd;
		st.rsi = (uint64_t)(uintptr_t)"";
		st.rdx = (uint64_t)(uintptr_t)raw_stat_buf;
		st.r10 = AT_EMPTY_PATH;
		check(fex_linux_syscall_dispatch(&st) == 0 && st.rax == 0,
			"newfstatat with valid statbuf succeeds");

		close(tmp_fd);
	}

	if (ro_page != MAP_FAILED) {
		munmap(ro_page, ps);
	}

	// 6. epoll_ctl on NULL and bad pointer
	int epfd = epoll_create1(0);
	check(epfd >= 0, "epoll_create1 for efault test");
	int pipefds[2] = {-1, -1};
	check(pipe(pipefds) == 0, "pipe for efault test");

	if (epfd >= 0 && pipefds[0] >= 0 && pipefds[1] >= 0) {
		memset(&st, 0, sizeof(st));
		st.rax = 233; // epoll_ctl
		st.rdi = (uint64_t)epfd;
		st.rsi = EPOLL_CTL_ADD;
		st.rdx = (uint64_t)pipefds[0];
		st.r10 = 0; // NULL event pointer
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"epoll_ctl ADD with NULL event returns -EFAULT and updates rax");

		memset(&st, 0, sizeof(st));
		st.rax = 233; // epoll_ctl
		st.rdi = (uint64_t)epfd;
		st.rsi = EPOLL_CTL_ADD;
		st.rdx = (uint64_t)pipefds[0];
		st.r10 = 1; // bad pointer
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"epoll_ctl ADD with bad event pointer returns -EFAULT");

		// EPOLL_CTL_DEL does not read event pointer on Linux: NULL does not return -EFAULT
		memset(&st, 0, sizeof(st));
		st.rax = 233;
		st.rdi = (uint64_t)epfd;
		st.rsi = EPOLL_CTL_DEL;
		st.rdx = (uint64_t)pipefds[0];
		st.r10 = 0; // NULL event
		long del_rc = fex_linux_syscall_dispatch(&st);
		check(del_rc != -EFAULT, "epoll_ctl DEL with NULL event does not return -EFAULT");

		// 7. epoll_wait and epoll_pwait on NULL and bad pointer
		memset(&st, 0, sizeof(st));
		st.rax = 232;
		st.rdi = (uint64_t)epfd;
		st.rsi = (uint64_t)(uintptr_t)NULL;
		st.rdx = 0; // maxevents = 0
		st.r10 = 0;
		check(fex_linux_syscall_dispatch(&st) == -EINVAL, "epoll_wait with maxevents=0 returns -EINVAL");

		// NULL event pointer with 0 ready events (timeout=0) -> returns 0
		memset(&st, 0, sizeof(st));
		st.rax = 232;
		st.rdi = (uint64_t)epfd;
		st.rsi = 0; // NULL events
		st.rdx = 1; // maxevents = 1
		st.r10 = 0; // timeout = 0
		check(fex_linux_syscall_dispatch(&st) == 0 && st.rax == 0,
			"epoll_wait with 0 ready events on NULL returns 0");

		// epoll_pwait with 0 ready events on NULL -> returns 0
		memset(&st, 0, sizeof(st));
		st.rax = 281; // epoll_pwait
		st.rdi = (uint64_t)epfd;
		st.rsi = 0; // NULL events
		st.rdx = 1; // maxevents = 1
		st.r10 = 0; // timeout = 0
		st.r8 = 0;  // sigmask
		st.r9 = 0;  // sigsetsize
		check(fex_linux_syscall_dispatch(&st) == 0 && st.rax == 0,
			"epoll_pwait with 0 ready events on NULL returns 0");

		// Add pipefds[1] to epoll (ready for writing)
		struct __attribute__((packed)) {
			uint32_t events;
			uint64_t data;
		} ev_add = { .events = EPOLLOUT, .data = 42 };
		memset(&st, 0, sizeof(st));
		st.rax = 233;
		st.rdi = (uint64_t)epfd;
		st.rsi = EPOLL_CTL_ADD;
		st.rdx = (uint64_t)pipefds[1];
		st.r10 = (uint64_t)(uintptr_t)&ev_add;
		check(fex_linux_syscall_dispatch(&st) == 0, "epoll_ctl ADD pipefds[1] for wait test");

		// Now events ARE ready: epoll_wait with NULL events pointer must return -EFAULT
		memset(&st, 0, sizeof(st));
		st.rax = 232;
		st.rdi = (uint64_t)epfd;
		st.rsi = 0; // NULL events
		st.rdx = 1;
		st.r10 = 0;
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"epoll_wait with ready events on NULL returns -EFAULT and updates rax");

		// epoll_wait with bad pointer (void*)1 must return -EFAULT without crashing
		memset(&st, 0, sizeof(st));
		st.rax = 232;
		st.rdi = (uint64_t)epfd;
		st.rsi = 1; // (void*)1
		st.rdx = 1;
		st.r10 = 0;
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"epoll_wait with ready events on bad pointer returns -EFAULT");

		// epoll_pwait with ready events on NULL must return -EFAULT
		memset(&st, 0, sizeof(st));
		st.rax = 281;
		st.rdi = (uint64_t)epfd;
		st.rsi = 0; // NULL events
		st.rdx = 1;
		st.r10 = 0;
		st.r8 = 0;
		st.r9 = 0;
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"epoll_pwait with ready events on NULL returns -EFAULT and updates rax");

		// epoll_pwait with ready events on bad pointer (void*)1 must return -EFAULT without crashing
		memset(&st, 0, sizeof(st));
		st.rax = 281;
		st.rdi = (uint64_t)epfd;
		st.rsi = 1; // (void*)1
		st.rdx = 1;
		st.r10 = 0;
		st.r8 = 0;
		st.r9 = 0;
		check(fex_linux_syscall_dispatch(&st) == -EFAULT && st.rax == (uint64_t)(long)-EFAULT,
			"epoll_pwait with ready events on bad pointer returns -EFAULT");

		// epoll_pwait with valid event buffer succeeds
		struct __attribute__((packed)) {
			uint32_t events;
			uint64_t data;
		} ev_wait_out = {0, 0};
		memset(&st, 0, sizeof(st));
		st.rax = 281;
		st.rdi = (uint64_t)epfd;
		st.rsi = (uint64_t)(uintptr_t)&ev_wait_out;
		st.rdx = 1;
		st.r10 = 0;
		st.r8 = 0;
		st.r9 = 0;
		check(fex_linux_syscall_dispatch(&st) == 1 && ev_wait_out.data == 42,
			"epoll_pwait with valid buffer succeeds and unpacks event");

		close(epfd);
		close(pipefds[0]);
		close(pipefds[1]);
	}
}

int main(void)
{
	test_elfcalls_trampolines();
	test_dlsym_wrapping();
	test_host_callback_thunks();
	test_many_callback_thunks();
	test_callback_thunk_limit();
	test_xlib_handler_restore();
	test_signal_mask_filter();
	test_wayland_dispatcher();
	test_callback_arguments();
	test_unsupported_callback();
	test_callback_struct();
	test_proc_address_lookup();
	test_jpeg_error_manager();
	test_xim_callbacks();
	test_left_callbacks();
	test_pa_mainloop_api();
	test_pa_callback_args();
	test_syscall_translations();
	test_safe_memory_access();
	test_guest_signal_delivery();
	return failures ? 1 : 0;
}
