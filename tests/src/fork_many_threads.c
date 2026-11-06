// Fork after the process has created and finished many threads, and while many others
// are still alive. The child has only the thread that called fork(); it must be able to
// create threads of its own, and the threads left in the parent must keep working.
#include <pthread.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define FINISHED_THREADS 1000
#define LIVE_THREADS 64
#define CHILD_THREADS 64

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int released;

static void* nothing(void* arg)
{
	return arg;
}

static void* wait_for_release(void* arg)
{
	pthread_mutex_lock(&mutex);
	while (!released)
		pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
	return arg;
}

static int create_and_join(int count)
{
	for (int i = 0; i < count; i++)
	{
		pthread_t thread;
		void* result = NULL;

		if (pthread_create(&thread, NULL, nothing, (void*) (long) (i + 1)) != 0)
			return 1;
		if (pthread_join(thread, &result) != 0 || result != (void*) (long) (i + 1))
			return 1;
	}
	return 0;
}

static int fork_and_check(const char* stage)
{
	pid_t pid = fork();
	if (pid < 0)
	{
		perror("fork");
		return 1;
	}
	if (pid == 0)
	{
		alarm(30);
		_exit(create_and_join(CHILD_THREADS) ? 2 : 0);
	}

	int status;
	if (waitpid(pid, &status, 0) != pid)
	{
		perror("waitpid");
		return 1;
	}
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	{
		printf("FAIL: %s: child status 0x%x\n", stage, status);
		return 1;
	}
	return 0;
}

int main(void)
{
	if (create_and_join(FINISHED_THREADS))
	{
		printf("FAIL: could not create threads\n");
		return 1;
	}
	if (fork_and_check("after finished threads"))
		return 1;

	pthread_t live[LIVE_THREADS];
	for (int i = 0; i < LIVE_THREADS; i++)
	{
		if (pthread_create(&live[i], NULL, wait_for_release, (void*) (long) (i + 1)) != 0)
		{
			printf("FAIL: could not create live threads\n");
			return 1;
		}
	}
	for (int i = 0; i < 5; i++)
		if (fork_and_check("with live threads"))
			return 1;

	pthread_mutex_lock(&mutex);
	released = 1;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);

	for (int i = 0; i < LIVE_THREADS; i++)
	{
		void* result = NULL;
		if (pthread_join(live[i], &result) != 0 || result != (void*) (long) (i + 1))
		{
			printf("FAIL: live thread %d did not finish after the forks\n", i);
			return 1;
		}
	}

	if (create_and_join(LIVE_THREADS))
	{
		printf("FAIL: the parent could not create threads after the forks\n");
		return 1;
	}

	printf("PASS\n");
	return 0;
}
