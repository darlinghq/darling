// Copyright (C) 2026 Lubos Dolezel
//
// Work items on dispatch worker threads run on a 16-byte aligned stack, both on
// a newly started worker and on one that was parked and reused. The macOS ABI
// requires the stack to be 16-byte aligned at every function call on i386 and
// x86_64, and compilers keep 16-byte aligned locals (SSE spills included) at
// fixed offsets from the stack pointer, relying on it.
//
// A serial queue is fed one item at a time with pauses in between, so its worker
// runs out of work, parks, and is reused for the next item. dispatch_apply() then
// runs items on several workers at once.
#include <dispatch/dispatch.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define CYCLES 200
#define APPLY_ITERATIONS 256

static volatile int g_misaligned = 0;

__attribute__((noinline))
static void check_aligned(const void* p)
{
	if (((uintptr_t) p & 15) != 0)
		__atomic_fetch_add(&g_misaligned, 1, __ATOMIC_RELAXED);
}

__attribute__((noinline))
static void work_item(void)
{
	_Alignas(16) volatile char local[16];

	local[0] = 1;
	check_aligned((const void*) local);
}

int main(void)
{
	dispatch_queue_t queue = dispatch_queue_create("org.darlinghq.tests.workqueue-stack-alignment", NULL);
	dispatch_semaphore_t done = dispatch_semaphore_create(0);

	for (int i = 0; i < CYCLES; i++)
	{
		dispatch_async(queue, ^{
			work_item();
			dispatch_semaphore_signal(done);
		});

		dispatch_semaphore_wait(done, DISPATCH_TIME_FOREVER);

		// Long enough for the worker to run out of work and park.
		usleep(2000);
	}

	dispatch_apply(APPLY_ITERATIONS, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^(size_t i) {
		work_item();
	});

	if (g_misaligned != 0)
	{
		printf("FAIL: %d work items ran on a misaligned stack\n", g_misaligned);
		return 1;
	}

	printf("workqueue_stack_alignment: OK\n");
	return 0;
}
