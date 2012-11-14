// CFLAGS: -std=c99
#include <mach/task.h>
#include <mach/lock_set.h>
#include <mach/sync_policy.h>
#include <mach/mach_init.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>

void* contender1(void* p);
void* contender2(void* p);
void setstate(int n);

lock_set_t g_locks;
int g_state = 0;

int main()
{
	int rv;
	pthread_t threads[2];

	rv = lock_set_create(mach_task_self(), &g_locks, 1, SYNC_POLICY_FIFO);

	assert(rv == KERN_SUCCESS);

	pthread_create(&threads[0], NULL, contender1, NULL);
	pthread_create(&threads[1], NULL, contender2, NULL);

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	lock_set_destroy(mach_task_self(), g_locks);
	return 0;
}

void setstate(int n)
{
	g_state = n;
	printf("state=%d\n", n);
}

void* contender1(void* p)
{
	setstate(1);
	sleep(1);
	lock_acquire(g_locks, 0);
	setstate(2);
	lock_handoff(g_locks, 0);
	setstate(3);
	sleep(3);
	setstate(4);
	lock_handoff_accept(g_locks, 0);
	lock_release(g_locks, 0);

	return NULL;
}

void* contender2(void* p)
{
	lock_handoff_accept(g_locks, 0);
	sleep(1);
	assert(g_state == 3);

	lock_handoff(g_locks, 0);
	assert(g_state == 4);
	return NULL;
}

