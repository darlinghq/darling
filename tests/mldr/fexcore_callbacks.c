// Runs x86_64 code through the darling-fexcore shim: guest callbacks made from host code, both while the
// host thread runs a guest thread (nested in it) and while it does not, and the per-thread FEX state that
// such callbacks leave behind when the host thread exits.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "fex/fex_bridge.h"

int darling_fexcore_run_thread(struct fex_x86_64_state* state, fex_syscall_cb_t syscall_cb);
int darling_fexcore_run_callback(struct fex_x86_64_state* state, fex_syscall_cb_t syscall_cb);

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

static uint8_t* g_code;
static uintptr_t g_fn_add, g_fn_nested, g_fn_unwinding, g_fn_alignment, g_fn_gs;

static uint8_t* emit(uint8_t* p, const uint8_t* bytes, size_t n)
{
	memcpy(p, bytes, n);
	return p + n;
}

static uint8_t* emit_mov_eax(uint8_t* p, uint32_t imm)
{
	*p++ = 0xb8;
	memcpy(p, &imm, 4);
	return p + 4;
}

static uint8_t* emit_syscall(uint8_t* p, uint32_t nr)
{
	p = emit_mov_eax(p, nr);
	*p++ = 0x0f;
	*p++ = 0x05;
	return p;
}

static uint64_t g_saved_rbx = 0x1111111111111111ULL;
static uint64_t g_saved_r12 = 0x2222222222222222ULL;
static uintptr_t g_outer_entry;
static uintptr_t g_outer_stack_top;

static void build_guest_code(void)
{
	g_code = map_low(4096, 0x20000000, PROT_READ | PROT_WRITE);
	uint8_t* p = g_code;

	// add: rax = rdi + rsi
	g_fn_add = (uintptr_t)p;
	p = emit(p, (const uint8_t[]){0x48, 0x8d, 0x04, 0x37, 0xc3}, 5);

	// nested: push rbx; mov rbx, rdi; (rax = add(rdi, 100) through the host); add rax, rbx; pop rbx; ret
	g_fn_nested = (uintptr_t)p;
	p = emit(p, (const uint8_t[]){0x53, 0x48, 0x89, 0xfb}, 4);
	p = emit_syscall(p, NR_ADD_100);
	p = emit(p, (const uint8_t[]){0x48, 0x01, 0xd8, 0x5b, 0xc3}, 5);

	// unwinding: the host unwinds the guest thread from inside the callback
	g_fn_unwinding = (uintptr_t)p;
	p = emit_syscall(p, NR_UNWIND);
	p = emit_mov_eax(p, 0x99);
	*p++ = 0xc3;

	// alignment: rax = rsp & 15 on entry
	g_fn_alignment = (uintptr_t)p;
	p = emit(p, (const uint8_t[]){0x48, 0x89, 0xe0, 0x48, 0x83, 0xe0, 0x0f, 0xc3}, 8);

	// gs: rax = gs:[8]
	g_fn_gs = (uintptr_t)p;
	p = emit(p, (const uint8_t[]){0x65, 0x48, 0x8b, 0x04, 0x25, 0x08, 0x00, 0x00, 0x00, 0xc3}, 10);

	// outer thread: rbx and r12 hold values the callbacks must not disturb; each host call's result is
	// pushed, then the thread exits.
	g_outer_entry = (uintptr_t)p;
	*p++ = 0x48; *p++ = 0xbb; memcpy(p, &g_saved_rbx, 8); p += 8;      // mov rbx, imm64
	*p++ = 0x49; *p++ = 0xbc; memcpy(p, &g_saved_r12, 8); p += 8;      // mov r12, imm64
	p = emit(p, (const uint8_t[]){0xbf, 0x07, 0x00, 0x00, 0x00}, 5);  // mov edi, 7
	p = emit_syscall(p, NR_CALL_ADD);
	*p++ = 0x50;                                                       // push rax
	p = emit(p, (const uint8_t[]){0xbf, 0x05, 0x00, 0x00, 0x00}, 5);  // mov edi, 5
	p = emit_syscall(p, NR_CALL_NESTED);
	*p++ = 0x50;
	p = emit(p, (const uint8_t[]){0xbf, 0x03, 0x00, 0x00, 0x00}, 5);  // mov edi, 3
	p = emit_syscall(p, NR_CALL_UNWINDING);
	*p++ = 0x50;
	p = emit(p, (const uint8_t[]){0xbf, 0xe8, 0x03, 0x00, 0x00}, 5);  // mov edi, 1000
	p = emit_syscall(p, NR_CALL_MANY);
	*p++ = 0x50;
	p = emit_syscall(p, NR_EXIT);

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
	if (darling_fexcore_run_callback(&cb, guest_syscall) != 0) {
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
		default:
			state->rax = (uint64_t)-38;
			break;
	}
	return (long)state->rax;
}

static void test_callbacks_nested_in_guest_thread(void)
{
	uint8_t* stack = map_low(1 << 20, 0x30000000, PROT_READ | PROT_WRITE);
	g_outer_stack_top = (uintptr_t)stack + (1 << 20) - 8;

	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rip = g_outer_entry;
	st.rsp = g_outer_stack_top;
	st.rflags = 0x202;
	darling_fexcore_run_thread(&st, guest_syscall);

	const uint64_t* pushed = (const uint64_t*)(uintptr_t)st.rsp;
	check(pushed[3] == 1007, "a callback made from a host call returns its result to the guest thread");
	check(pushed[2] == 5 + 100 + 5, "a callback can make a host call that runs another callback");
	check(pushed[1] == 4, "a callback unwound by thread termination leaves later callbacks working");
	check(pushed[0] == 1000 * 1001 / 2, "a thousand callbacks from one host call all return");
	check(st.rbx == g_saved_rbx && st.r12 == g_saved_r12,
		"callbacks leave the guest thread's callee-saved registers intact");
	check(st.rsp == g_outer_stack_top - 4 * 8, "callbacks leave the guest thread's stack pointer intact");
}

static void test_callbacks_without_guest_thread(void)
{
	check(callback(g_fn_add, 2, 3, 0) == 5, "a callback runs on a host thread that runs no guest code");
	check(callback(g_fn_nested, 7, 0, 0) == 7 + 100 + 7,
		"a callback on a host thread without guest code can run a nested callback");
	check(callback(g_fn_alignment, 0, 0, 0) == 8, "a callback starts with the stack alignment of a call");

	static uint64_t tsd[2] = {0, 0x5eed};
	uint64_t* low_tsd = (uint64_t*)map_low(4096, 0x40000000, PROT_READ | PROT_WRITE);
	memcpy(low_tsd, tsd, sizeof(tsd));
	check(callback(g_fn_gs, 0, 0, (uintptr_t)low_tsd) == 0x5eed, "a callback runs with the gs base it is given");
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

int main(void)
{
	build_guest_code();
	test_callbacks_without_guest_thread();
	test_callbacks_nested_in_guest_thread();
	test_thread_exit_releases_emulator();
	// mldr never returns from main with FEX loaded (it ends with _exit), and the shim's static
	// destructors fail when run at exit, so leave the same way.
	fflush(stdout);
	_exit(failures ? 1 : 0);
}
