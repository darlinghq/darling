// CFLAGS: -std=c99
#include <mach/task.h>
#include <mach/lock_set.h>
#include <mach/sync_policy.h>
#include <mach/mach_init.h>
#include <pthread.h>
#include <assert.h>

#define NUM_THREADS 30
#define NUM_CYCLES 10000

void* contender(void* p);

lock_set_t g_locks;
int g_variable = 0;
long long g_nlocks = 0;

int main()
{
	int rv;
	pthread_t threads[NUM_THREADS];

	rv = lock_set_create(mach_task_self(), &g_locks, 1, SYNC_POLICY_FIFO);

	assert(rv == KERN_SUCCESS);

	for (int i = 0; i < NUM_THREADS; i++)
		pthread_create(&threads[i], NULL, contender, NULL);

	for (int i = 0; i < NUM_THREADS; i++)
		pthread_join(threads[i], NULL);

	lock_set_destroy(mach_task_self(), g_locks);
	printf("Done, %ld locks done\n", g_nlocks);
	return 0;
}

void* contender(void* p)
{
	for (int i = 0; i < NUM_CYCLES; i++)
	{
		int rv;
		rv = lock_acquire(g_locks, 0);
		assert(rv == KERN_SUCCESS);

		rv = __sync_add_and_fetch(&g_variable, 1);
		assert(rv == 1);

		rv = __sync_sub_and_fetch(&g_variable, 1);
		assert(rv == 0);
		g_nlocks++;

		rv = lock_release(g_locks, 0);
		assert(rv == KERN_SUCCESS);
	}

	return NULL;
}

