// Runs x86_64 code through the darling-fexcore shim: guest callbacks made from host code, both while the
// host thread runs a guest thread (nested in it) and while it does not, through the shim directly and
// through mldr's fex_invoke_guest_callback_args, and the per-thread FEX state that such callbacks leave
// behind when the host thread exits. Usage: mldr_fexcore_callbacks_test <path to libdarling-fexcore.so>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include "fex/fex_bridge.h"

// FEX emulates a 4 KB-page process; on other hosts the test runs inside a 4 KB-page VM (muvm).
#define SKIP_RETURN_CODE 77

typedef int (*run_fn)(struct fex_x86_64_state* state, fex_syscall_cb_t syscall_cb);
static run_fn run_thread, run_callback;

static int failures = 0;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

enum {
	NR_EXIT = 0x500,      // ends the guest thread
	NR_CALL_ADD,          // rax = add(rdi, 1000), run as a callback
	NR_CALL_NESTED,       // rax = nested(rdi), run as a callback
	NR_ADD_100,           // rax = add(rdi, 100), run as a callback: the nested callback's host call
	NR_UNWIND,            // unwinds the guest thread
	NR_CALL_UNWINDING,    // runs a callback that unwinds, then add(rdi, 1) as a callback
	NR_CALL_MANY,         // rax = sum of add(i, 1) for i < rdi, each a callback
	NR_CALL_ALIGNMENT,    // rax = alignment(), run as a callback
	NR_CALL_CLOBBER,      // runs clobber() as a callback
	NR_CALL_DEEP,         // rax = deep(rdi), run as a callback
	NR_ARM_TIMER,         // arms a 200 ms SIGALRM
};

// Guest memory must be below 47 bits, where an x86_64 process can address it.
static uint8_t* map_low(size_t size, uintptr_t hint, int prot)
{
	void* p = mmap((void*)hint, size, prot, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
	if (p == MAP_FAILED) {
		perror("mmap");
		exit(2);
	}
	return p;
}

#define CODE_ADDR 0x20000000
#define SAVED_ADDR 0x50000000
// Register values a frame sets before a host call whose callback clobbers them, then stores to memory.
enum { SAVED_RBX, SAVED_RBP, SAVED_R12, SAVED_R13, SAVED_R14, SAVED_R15, SAVED_XMM6, SAVED_FLAGS, SAVED_COUNT };
static uint64_t* g_saved_outer;     // stored by the guest thread
static uint64_t* g_saved_detached;  // stored by a callback on a host thread without guest code
static uint64_t* g_tsd;             // a gs base: gs:[8] holds 0x5eed

static uint8_t* g_code;
static uintptr_t g_fn_add, g_fn_nested, g_fn_unwinding, g_fn_alignment, g_fn_gs, g_fn_clobber, g_fn_deep;
static uintptr_t g_fn_keeps_registers, g_outer_entry, g_many_entry, g_spin_entry;

static uint8_t* emit(uint8_t* p, const void* bytes, size_t n)
{
	memcpy(p, bytes, n);
	return p + n;
}

#define EMIT(p, ...) emit(p, (const uint8_t[]){__VA_ARGS__}, sizeof((const uint8_t[]){__VA_ARGS__}))

static uint8_t* emit_imm32(uint8_t* p, uint32_t v)
{
	return emit(p, &v, 4);
}

static uint8_t* emit_syscall(uint8_t* p, uint32_t nr)
{
	p = EMIT(p, 0xb8);         // mov eax, nr
	p = emit_imm32(p, nr);
	return EMIT(p, 0x0f, 0x05);
}

static uint64_t saved_value(int i)
{
	return 0x1000000000000001ULL * (uint64_t)(i + 1);
}

// Sets rbx, rbp, r12-r15, xmm6 and CF, makes a host call that runs clobber() as a callback, then stores
// what the registers and flags hold afterwards to `out`.
static uint8_t* emit_clobber_check(uint8_t* p, uint64_t* out)
{
	static const uint8_t mov_imm64[][2] = {
		{0x48, 0xbb}, {0x48, 0xbd}, {0x49, 0xbc}, {0x49, 0xbd}, {0x49, 0xbe}, {0x49, 0xbf}};
	static const uint8_t store[][3] = {
		{0x48, 0x89, 0x1c}, {0x48, 0x89, 0x2c}, {0x4c, 0x89, 0x24}, {0x4c, 0x89, 0x2c}, {0x4c, 0x89, 0x34},
		{0x4c, 0x89, 0x3c}};
	for (int i = 0; i < 6; i++) {
		uint64_t v = saved_value(i);
		p = emit(p, mov_imm64[i], 2);
		p = emit(p, &v, 8);
	}
	uint64_t x = saved_value(SAVED_XMM6);
	p = EMIT(p, 0x48, 0xb8);                          // mov rax, imm64
	p = emit(p, &x, 8);
	p = EMIT(p, 0x66, 0x48, 0x0f, 0x6e, 0xf0);       // movq xmm6, rax
	p = EMIT(p, 0x48, 0x85, 0xe4);                    // test rsp, rsp (ZF=0)
	p = EMIT(p, 0xf9);                                // stc
	p = emit_syscall(p, NR_CALL_CLOBBER);
	p = EMIT(p, 0x9c);                                // pushfq
	for (int i = 0; i < 6; i++) {
		p = emit(p, store[i], 3);                     // mov [disp32], reg
		p = EMIT(p, 0x25);
		p = emit_imm32(p, (uint32_t)(uintptr_t)&out[i]);
	}
	p = EMIT(p, 0x58);                                // pop rax (the flags)
	p = EMIT(p, 0x48, 0x89, 0x04, 0x25);              // mov [disp32], rax
	p = emit_imm32(p, (uint32_t)(uintptr_t)&out[SAVED_FLAGS]);
	p = EMIT(p, 0x66, 0x48, 0x0f, 0x7e, 0xf0);       // movq rax, xmm6
	p = EMIT(p, 0x48, 0x89, 0x04, 0x25);
	return emit_imm32(p, (uint32_t)(uintptr_t)&out[SAVED_XMM6]);
}

static void build_guest_code(void)
{
	g_code = map_low(4096, CODE_ADDR, PROT_READ | PROT_WRITE);
	uint64_t* saved = (uint64_t*)map_low(4096, SAVED_ADDR, PROT_READ | PROT_WRITE);
	g_saved_outer = saved;
	g_saved_detached = saved + 16;
	g_tsd = (uint64_t*)map_low(4096, 0x40000000, PROT_READ | PROT_WRITE);
	g_tsd[1] = 0x5eed;
	uint8_t* p = g_code;

	g_fn_add = (uintptr_t)p;                          // rax = rdi + rsi
	p = EMIT(p, 0x48, 0x8d, 0x04, 0x37, 0xc3);

	// nested: push rbx; mov rbx, rdi; (rax = add(rdi, 100) through the host); add rax, rbx; pop rbx; ret
	g_fn_nested = (uintptr_t)p;
	p = EMIT(p, 0x53, 0x48, 0x89, 0xfb);
	p = emit_syscall(p, NR_ADD_100);
	p = EMIT(p, 0x48, 0x01, 0xd8, 0x5b, 0xc3);

	g_fn_unwinding = (uintptr_t)p;                    // the host unwinds the guest thread from inside it
	p = emit_syscall(p, NR_UNWIND);
	p = EMIT(p, 0xb8, 0x99, 0x00, 0x00, 0x00, 0xc3);

	g_fn_alignment = (uintptr_t)p;                    // rax = rsp & 15 on entry
	p = EMIT(p, 0x48, 0x89, 0xe0, 0x48, 0x83, 0xe0, 0x0f, 0xc3);

	g_fn_gs = (uintptr_t)p;                           // rax = gs:[8]
	p = EMIT(p, 0x65, 0x48, 0x8b, 0x04, 0x25, 0x08, 0x00, 0x00, 0x00, 0xc3);

	// clobber: breaks the calling convention on purpose: overwrites rbx, rbp, r12-r15, xmm6 and the flags
	g_fn_clobber = (uintptr_t)p;
	static const uint8_t mov_minus1[][3] = {
		{0x48, 0xc7, 0xc3}, {0x48, 0xc7, 0xc5}, {0x49, 0xc7, 0xc4}, {0x49, 0xc7, 0xc5}, {0x49, 0xc7, 0xc6},
		{0x49, 0xc7, 0xc7}};
	for (int i = 0; i < 6; i++) {
		p = emit(p, mov_minus1[i], 3);
		p = emit_imm32(p, 0xffffffff);
	}
	p = EMIT(p, 0x66, 0x0f, 0x76, 0xf6);             // pcmpeqd xmm6, xmm6
	p = EMIT(p, 0x31, 0xc0, 0xc3);                    // xor eax, eax (CF=0, ZF=1); ret

	// deep(n): n == 0 ? 1000 : n + deep(n - 1), each level a callback run from the previous one's host call
	g_fn_deep = (uintptr_t)p;
	p = EMIT(p, 0x48, 0x85, 0xff, 0x75, 0x06, 0xb8, 0xe8, 0x03, 0x00, 0x00, 0xc3);
	p = EMIT(p, 0x53, 0x48, 0x89, 0xfb, 0x48, 0xff, 0xcf);
	p = emit_syscall(p, NR_CALL_DEEP);
	p = EMIT(p, 0x48, 0x01, 0xd8, 0x5b, 0xc3);

	// A callback that runs the clobber check itself, on a host thread with no guest code.
	g_fn_keeps_registers = (uintptr_t)p;
	p = EMIT(p, 0x48, 0x83, 0xec, 0x08);             // sub rsp, 8: the call alignment of a host call
	p = emit_clobber_check(p, g_saved_detached);
	p = EMIT(p, 0x48, 0x83, 0xc4, 0x08, 0xc3);       // add rsp, 8; ret

	// Guest thread: each host call's result is pushed, then the thread exits. Its first host call is made
	// with rsp 8 below a 16-byte boundary, as inside a function a guest called.
	g_outer_entry = (uintptr_t)p;
	p = emit_syscall(p, NR_CALL_ALIGNMENT);
	p = EMIT(p, 0x50);                                // push rax
	p = EMIT(p, 0xbf, 0x07, 0x00, 0x00, 0x00);        // mov edi, 7
	p = emit_syscall(p, NR_CALL_ADD);
	p = EMIT(p, 0x50);
	p = EMIT(p, 0xbf, 0x05, 0x00, 0x00, 0x00);
	p = emit_syscall(p, NR_CALL_NESTED);
	p = EMIT(p, 0x50);
	p = EMIT(p, 0xbf, 0x06, 0x00, 0x00, 0x00);
	p = emit_syscall(p, NR_CALL_DEEP);
	p = EMIT(p, 0x50);
	p = EMIT(p, 0xbf, 0x03, 0x00, 0x00, 0x00);
	p = emit_syscall(p, NR_CALL_UNWINDING);
	p = EMIT(p, 0x50);
	p = EMIT(p, 0xbf, 0xe8, 0x03, 0x00, 0x00);        // mov edi, 1000
	p = emit_syscall(p, NR_CALL_MANY);
	p = EMIT(p, 0x50);
	p = emit_clobber_check(p, g_saved_outer);
	p = emit_syscall(p, NR_EXIT);

	// Guest thread for the concurrency test: 500 callbacks, push the sum, exit.
	g_many_entry = (uintptr_t)p;
	p = EMIT(p, 0xbf, 0xf4, 0x01, 0x00, 0x00);        // mov edi, 500
	p = emit_syscall(p, NR_CALL_MANY);
	p = EMIT(p, 0x50);
	p = emit_syscall(p, NR_EXIT);

	// Arms the timer once guest code runs, so it fires in the loop, then jmp . (no further host calls)
	g_spin_entry = (uintptr_t)p;
	p = emit_syscall(p, NR_ARM_TIMER);
	p = EMIT(p, 0xeb, 0xfe);

	mprotect(g_code, 4096, PROT_READ | PROT_EXEC);
	__builtin___clear_cache((char*)g_code, (char*)g_code + 4096);
}

static long guest_syscall(struct fex_x86_64_state* state);

static uint64_t callback(uintptr_t fn, uint64_t a0, uint64_t a1, uint64_t gs_base)
{
	struct fex_x86_64_state cb;
	memset(&cb, 0, sizeof(cb));
	cb.rip = fn;
	cb.rdi = a0;
	cb.rsi = a1;
	cb.gs_base = gs_base;
	if (run_callback(&cb, guest_syscall) != 0) {
		fprintf(stderr, "darling_fexcore_run_callback failed\n");
		exit(2);
	}
	return cb.rax;
}

static long guest_syscall(struct fex_x86_64_state* state)
{
	switch (state->rax) {
		case NR_EXIT:
		case NR_UNWIND:
			state->unwind_thread = 1;
			break;
		case NR_CALL_ADD:
			state->rax = callback(g_fn_add, state->rdi, 1000, state->gs_base);
			break;
		case NR_CALL_NESTED:
			state->rax = callback(g_fn_nested, state->rdi, 0, state->gs_base);
			break;
		case NR_ADD_100:
			state->rax = callback(g_fn_add, state->rdi, 100, state->gs_base);
			break;
		case NR_CALL_UNWINDING:
			callback(g_fn_unwinding, 0, 0, state->gs_base);
			state->rax = callback(g_fn_add, state->rdi, 1, state->gs_base);
			break;
		case NR_CALL_MANY: {
			uint64_t sum = 0;
			for (uint64_t i = 0; i < state->rdi; i++)
				sum += callback(g_fn_add, i, 1, state->gs_base);
			state->rax = sum;
			break;
		}
		case NR_CALL_ALIGNMENT:
			state->rax = callback(g_fn_alignment, 0, 0, state->gs_base);
			break;
		case NR_CALL_CLOBBER:
			state->rax = callback(g_fn_clobber, 0, 0, state->gs_base);
			break;
		case NR_CALL_DEEP:
			state->rax = callback(g_fn_deep, state->rdi, 0, state->gs_base);
			break;
		case NR_ARM_TIMER: {
			struct itimerval it = {{0, 0}, {0, 200000}};
			state->rax = (uint64_t)setitimer(ITIMER_REAL, &it, NULL);
			break;
		}
		default:
			state->rax = (uint64_t)-38;
			break;
	}
	return (long)state->rax;
}

static int registers_kept(const uint64_t* saved)
{
	for (int i = 0; i < 6; i++)
		if (saved[i] != saved_value(i))
			return 0;
	return saved[SAVED_XMM6] == saved_value(SAVED_XMM6) && (saved[SAVED_FLAGS] & 0x41) == 0x01; // CF=1, ZF=0
}

static void test_callbacks_nested_in_guest_thread(void)
{
	uint8_t* stack = map_low(1 << 20, 0x30000000, PROT_READ | PROT_WRITE);
	const uintptr_t top = (uintptr_t)stack + (1 << 20) - 8;

	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rip = g_outer_entry;
	st.rsp = top;
	st.rflags = 0x202;
	run_thread(&st, guest_syscall);

	const uint64_t* pushed = (const uint64_t*)(uintptr_t)st.rsp;
	check(pushed[5] == 8, "a callback nested in a guest thread starts with the stack alignment of a call");
	check(pushed[4] == 1007, "a callback made from a host call returns its result to the guest thread");
	check(pushed[3] == 5 + 100 + 5, "a callback can make a host call that runs another callback");
	check(pushed[2] == 1000 + 6 + 5 + 4 + 3 + 2 + 1, "callbacks nest seven deep");
	check(pushed[1] == 4, "a callback unwound by thread termination leaves later callbacks working");
	check(pushed[0] == 1000 * 1001 / 2, "a thousand callbacks from one host call all return");
	check(registers_kept(g_saved_outer),
		"a callback that clobbers registers and flags leaves the guest thread's values intact");
	check(st.rsp == top - 6 * 8, "callbacks leave the guest thread's stack pointer intact");
}

static void test_callbacks_without_guest_thread(void)
{
	check(callback(g_fn_add, 2, 3, 0) == 5, "a callback runs on a host thread that runs no guest code");
	check(callback(g_fn_nested, 7, 0, 0) == 7 + 100 + 7,
		"a callback on a host thread without guest code can run a nested callback");
	check(callback(g_fn_deep, 5, 0, 0) == 1000 + 15, "callbacks without a guest thread nest six deep");
	check(callback(g_fn_alignment, 0, 0, 0) == 8, "a callback starts with the stack alignment of a call");
	callback(g_fn_keeps_registers, 0, 0, 0);
	check(registers_kept(g_saved_detached),
		"a nested callback that clobbers registers and flags leaves its caller's values intact");

	check(callback(g_fn_gs, 0, 0, (uintptr_t)g_tsd) == 0x5eed, "a callback runs with the gs base it is given");
}

static void test_engine_invokes_callbacks(void)
{
	const uint64_t args[6] = {40, 2, 0, 0, 0, 0};
	check(fex_invoke_guest_callback_args(g_fn_add, args) == 42, "fex_invoke_guest_callback_args runs the callback");
	fex_set_current_thread_gs_base((uintptr_t)g_tsd);
	check(fex_invoke_guest_callback(g_fn_gs, 0, 0) == 0x5eed,
		"fex_invoke_guest_callback runs the callback with the thread's gs base");
}

static size_t count_mappings(void)
{
	FILE* f = fopen("/proc/self/maps", "r");
	if (!f)
		return 0;
	size_t n = 0;
	int c;
	while ((c = fgetc(f)) != EOF)
		if (c == '\n')
			n++;
	fclose(f);
	return n;
}

static void* thread_running_callbacks(void* arg)
{
	(void)arg;
	uint64_t sum = 0;
	for (int i = 0; i < 50; i++)
		sum += callback(g_fn_add, (uint64_t)i, 1, 0);
	return (void*)(uintptr_t)sum;
}

static void test_thread_exit_releases_emulator(void)
{
	enum { THREADS = 32 };
	// One thread first, so that mappings FEX makes once per process are not counted.
	pthread_t warm;
	pthread_create(&warm, NULL, thread_running_callbacks, NULL);
	pthread_join(warm, NULL);

	size_t before = count_mappings();
	int all_ok = 1;
	for (int i = 0; i < THREADS; i++) {
		pthread_t t;
		void* ret = NULL;
		pthread_create(&t, NULL, thread_running_callbacks, NULL);
		pthread_join(t, &ret);
		all_ok &= (uintptr_t)ret == 50 * 51 / 2;
	}
	size_t after = count_mappings();
	check(all_ok, "callbacks run on many short-lived host threads");
	check(after < before + THREADS, "a host thread's FEX state is released when the thread exits");
}

enum { CONCURRENT = 4 };
static pthread_barrier_t g_start;

static void* concurrent_guest_thread(void* arg)
{
	uintptr_t index = (uintptr_t)arg;
	uint8_t* stack = map_low(1 << 20, 0x60000000 + index * 0x200000, PROT_READ | PROT_WRITE);
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rip = g_many_entry;
	st.rsp = (uintptr_t)stack + (1 << 20) - 8;
	st.rflags = 0x202;
	pthread_barrier_wait(&g_start);
	uint64_t detached = callback(g_fn_deep, 4, 0, 0);
	run_thread(&st, guest_syscall);
	uint64_t nested = *(const uint64_t*)(uintptr_t)st.rsp;
	return (void*)(uintptr_t)(detached == 1010 && nested == 500 * 501 / 2);
}

static void test_concurrent_threads(void)
{
	pthread_t t[CONCURRENT];
	pthread_barrier_init(&g_start, NULL, CONCURRENT);
	for (uintptr_t i = 0; i < CONCURRENT; i++)
		pthread_create(&t[i], NULL, concurrent_guest_thread, (void*)i);
	int all_ok = 1;
	for (int i = 0; i < CONCURRENT; i++) {
		void* ret = NULL;
		pthread_join(t[i], &ret);
		all_ok &= ret != NULL;
	}
	check(all_ok, "guest threads on several host threads run callbacks at the same time");
}

static void call_back_from_signal(int sig)
{
	(void)sig;
	callback(g_fn_add, 1, 2, 0);
	_exit(0);
}

// A callback while the guest runs outside a host call (here from a signal handler) has no current guest
// stack pointer to build its frame on, and must stop the process instead of corrupting the guest stack.
static void test_callback_outside_host_call_aborts(void)
{
	pid_t pid = fork();
	if (pid == 0) {
		signal(SIGALRM, call_back_from_signal);
		uint8_t* stack = map_low(1 << 16, 0x70000000, PROT_READ | PROT_WRITE);
		struct fex_x86_64_state st;
		memset(&st, 0, sizeof(st));
		st.rip = g_spin_entry;
		st.rsp = (uintptr_t)stack + (1 << 16) - 8;
		run_thread(&st, guest_syscall);
		_exit(3);
	}
	int status = 0;
	for (int i = 0; i < 100 && waitpid(pid, &status, WNOHANG) == 0; i++)
		usleep(50000);
	if (waitpid(pid, &status, WNOHANG) == 0) {
		kill(pid, SIGKILL);
		waitpid(pid, &status, 0);
	}
	check(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT,
		"a callback on a guest thread that is not in a host call aborts");
}

int main(int argc, char** argv)
{
	if (sysconf(_SC_PAGESIZE) != 4096) {
		printf("SKIP: FEX needs 4 KB pages\n");
		return SKIP_RETURN_CODE;
	}
	if (argc < 2) {
		fprintf(stderr, "usage: %s <libdarling-fexcore.so>\n", argv[0]);
		return 2;
	}
	build_guest_code();
	if (fork() == 0) {
		// FEX is set up per process, so the abort probe runs in a child that has not loaded it yet.
		setenv("DARLING_FEXCORE_LIB", argv[1], 1);
		void* h = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
		run_thread = h ? (run_fn)dlsym(h, "darling_fexcore_run_thread") : NULL;
		run_callback = h ? (run_fn)dlsym(h, "darling_fexcore_run_callback") : NULL;
		if (!run_thread || !run_callback)
			_exit(2);
		test_callback_outside_host_call_aborts();
		fflush(stdout);
		_exit(failures ? 1 : 0);
	}
	int child_status = 0;
	wait(&child_status);
	if (!WIFEXITED(child_status) || WEXITSTATUS(child_status) != 0)
		failures++;

	// mldr's own loader finds the shim, as in a real run; the test then uses the same loaded copy.
	setenv("DARLING_FEXCORE_LIB", argv[1], 1);
	test_engine_invokes_callbacks();
	void* h = dlopen(argv[1], RTLD_NOW | RTLD_NOLOAD);
	run_thread = h ? (run_fn)dlsym(h, "darling_fexcore_run_thread") : NULL;
	run_callback = h ? (run_fn)dlsym(h, "darling_fexcore_run_callback") : NULL;
	if (!run_thread || !run_callback) {
		fprintf(stderr, "cannot find the shim's entry points: %s\n", dlerror());
		return 2;
	}
	test_callbacks_without_guest_thread();
	test_callbacks_nested_in_guest_thread();
	test_concurrent_threads();
	test_thread_exit_releases_emulator();
	// mldr never returns from main with FEX loaded (it ends with _exit), and the shim's static
	// destructors fail when run at exit, so leave the same way.
	fflush(stdout);
	_exit(failures ? 1 : 0);
}
