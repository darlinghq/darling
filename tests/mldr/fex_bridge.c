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
#include <unistd.h>
#include "fex/fex_bridge.h"

static int failures = 0;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
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

static int mock_native_fork(void)
{
	return 4242;
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
	host_calls.native_fork = mock_native_fork;
	host_calls.inherited_nofile_limit = mock_inherited_nofile_limit;

	struct elf_calls* tramp = fex_elfcalls_get_trampoline_table(&host_calls);
	void** slots = (void**)tramp;

	bool all_valid = true;
	for (uint32_t i = 0; i < DARLING_FEX_ELFCALL_COUNT; i++) {
		const uint8_t* code = (const uint8_t*)slots[i];
		uint32_t imm = 0;
		memcpy(&imm, &code[1], sizeof(imm));
		if (code[0] != 0xb8 || imm != DARLING_FEX_ELFCALL_SYSCALL_BASE + i ||
		    code[5] != 0x0f || code[6] != 0x05 || code[7] != 0xc3) {
			all_valid = false;
			break;
		}
	}
	check(all_valid, "all 33 elf_calls stubs encode movl $nr, %eax; syscall; ret");
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
	st.rcx = 44;
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

	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 31;
	check(fex_linux_syscall_dispatch(&st) == 4242, "native_fork (slot 31) dispatches");

	unsigned int limits[2] = {0, 0};
	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_ELFCALL_SYSCALL_BASE + 32;
	st.rdi = (uint64_t)(uintptr_t)limits;
	check(fex_linux_syscall_dispatch(&st) == 0 && limits[0] == 256 && limits[1] == 4096,
		"inherited_nofile_limit (slot 32) dispatches");

	memset(&st, 0, sizeof(st));
	st.rax = DARLING_FEX_CALLBACK_RETURN_SYSCALL;
	st.rdi = 0x123456789abcdef0ULL;
	check(fex_linux_syscall_dispatch(&st) == 0 && st.rax == 0x123456789abcdef0ULL,
		"DARLING_FEX_CALLBACK_RETURN_SYSCALL restores callback return value in rax");
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

	const uint8_t* code = (const uint8_t*)fn_stub1;
	uint32_t sc_nr = 0;
	memcpy(&sc_nr, &code[1], sizeof(sc_nr));

	uint64_t stack[5] = { 0, 7, 8, 0, 0 };
	struct fex_x86_64_state st;
	memset(&st, 0, sizeof(st));
	st.rax = sc_nr;
	st.rdi = 1;
	st.rsi = 2;
	st.rdx = 3;
	st.rcx = 4;
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
}

int main(void)
{
	test_elfcalls_trampolines();
	test_dlsym_wrapping();
	test_syscall_translations();
	return failures ? 1 : 0;
}
