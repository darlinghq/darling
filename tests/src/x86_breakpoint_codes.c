// Copyright (C) 2026 Lubos Dolezel
//
// x86_64: the Mach exception codes of the two kinds of breakpoint trap.
// int3 raises EXC_BREAKPOINT with codes [EXC_I386_BPT, 0] (observed on macOS 27,
// x86_64 under Rosetta). A single step (the trap flag in RFLAGS) raises
// EXC_BREAKPOINT with code EXC_I386_SGL, as XNU's published trap handler does
// for every debug exception. The handler moves the PC past the int3 or clears
// the trap flag and replies KERN_SUCCESS, so no SIGTRAP follows.
#include <mach/mach.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if !defined(__x86_64__)
int main(void)
{
	printf("SKIP x86_64 only\n");
	return 0;
}
#else

static int failures;
#define CHECK(cond, what) do { if (!(cond)) { printf("FAIL %s: %s\n", what, #cond); failures++; } else printf("PASS %s\n", what); } while (0)

#pragma pack(push, 4)
typedef struct {
	mach_msg_header_t head;
	mach_msg_body_t body;
	mach_msg_port_descriptor_t thread;
	mach_msg_port_descriptor_t task;
	NDR_record_t ndr;
	exception_type_t exception;
	mach_msg_type_number_t code_count;
	int64_t code[2];
	char trailer[128];
} exc_request_t;

typedef struct {
	mach_msg_header_t head;
	NDR_record_t ndr;
	kern_return_t ret;
} exc_reply_t;
#pragma pack(pop)

static mach_port_t exc_port;
static exc_request_t last;
static uintptr_t resume_at; // nonzero: move the PC there; zero: clear the trap flag
static volatile sig_atomic_t sigtraps;

static void on_sigtrap(int sig)
{
	(void)sig;
	sigtraps++;
}

static void* exc_thread(void* arg)
{
	(void)arg;
	exc_request_t req;
	memset(&req, 0, sizeof(req));
	if (mach_msg(&req.head, MACH_RCV_MSG, 0, sizeof(req), exc_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL) != KERN_SUCCESS)
		return NULL;
	last = req;

	thread_t th = req.thread.name;
	x86_thread_state64_t st;
	mach_msg_type_number_t cnt = x86_THREAD_STATE64_COUNT;
	thread_get_state(th, x86_THREAD_STATE64, (thread_state_t)&st, &cnt);
	if (resume_at)
		st.__rip = resume_at;
	else
		st.__rflags &= ~0x100ULL;
	thread_set_state(th, x86_THREAD_STATE64, (thread_state_t)&st, cnt);

	exc_reply_t rep;
	memset(&rep, 0, sizeof(rep));
	rep.head.msgh_bits = MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(req.head.msgh_bits), 0);
	rep.head.msgh_remote_port = req.head.msgh_remote_port;
	rep.head.msgh_size = sizeof(rep);
	rep.head.msgh_id = req.head.msgh_id + 100;
	rep.ndr = NDR_record;
	rep.ret = KERN_SUCCESS;
	mach_msg(&rep.head, MACH_SEND_MSG, sizeof(rep), 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	return NULL;
}

static void with_handler(void (*trigger)(void))
{
	pthread_t t;
	memset(&last, 0, sizeof(last));
	mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &exc_port);
	mach_port_insert_right(mach_task_self(), exc_port, exc_port, MACH_MSG_TYPE_MAKE_SEND);
	thread_set_exception_ports(mach_thread_self(), EXC_MASK_BREAKPOINT, exc_port,
		EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES, THREAD_STATE_NONE);
	pthread_create(&t, NULL, exc_thread, NULL);
	trigger();
	pthread_join(t, NULL);
	thread_set_exception_ports(mach_thread_self(), EXC_MASK_BREAKPOINT, MACH_PORT_NULL,
		EXCEPTION_DEFAULT, THREAD_STATE_NONE);
	mach_port_destroy(mach_task_self(), exc_port);
}

static void hit_int3(void)
{
	uintptr_t after;
	__asm__ volatile("lea 1f(%%rip), %0\n\tmovq %0, %1\n\tint3\n1:\n" : "=&r"(after), "=m"(resume_at) :: "memory");
}

static void single_step(void)
{
	resume_at = 0;
	__asm__ volatile("pushfq\n\torq $0x100, (%%rsp)\n\tpopfq\n\tnop\n\tnop\n" ::: "memory", "cc");
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	signal(SIGTRAP, on_sigtrap);

	with_handler(hit_int3);
	printf("int3: exception %d, %u codes [%#llx, %#llx]\n", last.exception, last.code_count,
		(unsigned long long)last.code[0], (unsigned long long)last.code[1]);
	CHECK(last.exception == EXC_BREAKPOINT && last.code_count == 2, "int3 raises EXC_BREAKPOINT");
	CHECK(last.code[0] == EXC_I386_BPT && last.code[1] == 0, "int3 has codes [EXC_I386_BPT, 0]");

	with_handler(single_step);
	printf("single step: exception %d, %u codes [%#llx, %#llx]\n", last.exception, last.code_count,
		(unsigned long long)last.code[0], (unsigned long long)last.code[1]);
	CHECK(last.exception == EXC_BREAKPOINT, "a single step raises EXC_BREAKPOINT");
	CHECK(last.code[0] == EXC_I386_SGL, "a single step has code EXC_I386_SGL");

	CHECK(sigtraps == 0, "handled exceptions raise no SIGTRAP");

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
#endif
