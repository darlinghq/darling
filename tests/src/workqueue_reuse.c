// Runs work on a dispatch queue often enough for its worker thread to be parked and reused
// hundreds of times. A reused worker must start again on an empty stack, however often it's reused.
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define CYCLES 1000
#define MAX_DEPTH (64 * 1024)

int main(void)
{
	dispatch_queue_t queue = dispatch_queue_create("org.darlinghq.tests.workqueue-reuse", NULL);
	dispatch_semaphore_t done = dispatch_semaphore_create(0);
	static size_t deepest;

	for (int i = 0; i < CYCLES; i++)
	{
		dispatch_async(queue, ^{
			char marker;
			const size_t depth = (uintptr_t) pthread_get_stackaddr_np(pthread_self()) - (uintptr_t) &marker;

			if (depth > deepest)
				deepest = depth;
			dispatch_semaphore_signal(done);
		});

		dispatch_semaphore_wait(done, DISPATCH_TIME_FOREVER);

		// Long enough for the worker to run out of work and park.
		usleep(2000);
	}

	printf("deepest stack use: %zu bytes\n", deepest);
	if (deepest > MAX_DEPTH)
	{
		printf("FAIL: workers keep stack frames across reuse\n");
		return 1;
	}
	printf("workqueue_reuse: OK\n");
	return 0;
}
