#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
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
#include <sys/wait.h>
#include <unistd.h>
#include "fex/fex_bridge.h"

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

uint64_t fex_invoke_guest_callback(uintptr_t guest_fn, uint64_t arg0, uint64_t arg1)
{
	last_cb_fn = guest_fn;
	last_cb_arg0 = arg0;
	last_cb_arg1 = arg1;
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
	test_xim_callbacks();
	test_syscall_translations();
	test_safe_memory_access();
	return failures ? 1 : 0;
}
