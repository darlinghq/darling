// POSIX only lets a child of a multithreaded process call async-signal-safe functions,
// but macOS programs commonly do more than that, and fork() on macOS prepares malloc
// and dyld for it. Fork repeatedly while other threads keep allocating memory, loading
// libraries and creating threads, so that they hold the allocator, dyld and loader locks
// at the moment of the fork. Every child must still be able to do all of these.
#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define FORKS 100
#define CHILD_TIMEOUT 20

static const char* const libraries[] = {
	"/usr/lib/libz.1.dylib",
	"/usr/lib/libsqlite3.dylib",
	"/usr/lib/libxml2.2.dylib",
};

static const char* library;
static volatile int stop;

static void* allocate(void* arg)
{
	(void) arg;
	while (!stop)
	{
		void* blocks[64];
		for (int i = 0; i < 64; i++)
			blocks[i] = malloc(16 << (i % 12));
		for (int i = 0; i < 64; i++)
			free(blocks[i]);
	}
	return NULL;
}

static void* load(void* arg)
{
	(void) arg;
	while (!stop)
	{
		void* handle = dlopen(library, RTLD_NOW);
		if (handle)
		{
			dlsym(handle, "zlibVersion");
			dlclose(handle);
		}
		dladdr((void*) &load, &(Dl_info) { 0 });
	}
	return NULL;
}

static void* nothing(void* arg)
{
	return arg;
}

static void* spawn_threads(void* arg)
{
	(void) arg;
	while (!stop)
	{
		pthread_t thread;
		if (pthread_create(&thread, NULL, nothing, NULL) == 0)
			pthread_join(thread, NULL);
	}
	return NULL;
}

static int child(void)
{
	alarm(CHILD_TIMEOUT);

	char* block = malloc(100000);
	if (!block)
		return 2;
	memset(block, 1, 100000);
	free(block);

	void* handle = dlopen(library, RTLD_NOW);
	if (!handle)
		return 3;
	dlclose(handle);

	pthread_t thread;
	if (pthread_create(&thread, NULL, nothing, NULL) != 0)
		return 4;
	pthread_join(thread, NULL);
	return 0;
}

int main(void)
{
	for (size_t i = 0; i < sizeof(libraries) / sizeof(libraries[0]) && !library; i++)
	{
		void* handle = dlopen(libraries[i], RTLD_NOW);
		if (handle)
		{
			library = libraries[i];
			dlclose(handle);
		}
	}
	if (!library)
	{
		printf("FAIL: none of the test libraries could be loaded\n");
		return 1;
	}

	void* (*workers[])(void*) = { allocate, allocate, load, spawn_threads, spawn_threads };
	pthread_t threads[sizeof(workers) / sizeof(workers[0])];
	for (size_t i = 0; i < sizeof(workers) / sizeof(workers[0]); i++)
	{
		if (pthread_create(&threads[i], NULL, workers[i], NULL) != 0)
		{
			perror("pthread_create");
			return 1;
		}
	}

	int failed = 0;
	for (int i = 0; i < FORKS && !failed; i++)
	{
		pid_t pid = fork();
		if (pid < 0)
		{
			perror("fork");
			failed = 1;
			break;
		}
		if (pid == 0)
			_exit(child());

		int status;
		if (waitpid(pid, &status, 0) != pid)
		{
			perror("waitpid");
			failed = 1;
		}
		else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM)
		{
			printf("FAIL: fork %d: the child hung\n", i);
			failed = 1;
		}
		else if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		{
			printf("FAIL: fork %d: child status 0x%x\n", i, status);
			failed = 1;
		}
	}

	stop = 1;
	for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); i++)
		pthread_join(threads[i], NULL);

	if (failed)
		return 1;
	printf("PASS\n");
	return 0;
}
