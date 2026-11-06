// fork() returns the child's PID in the parent and 0 in the child, the child is a new
// process whose parent is the caller, and it can keep using memory allocation, threads
// and stdio before it exits with a status the parent collects with waitpid().
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ROUNDS 50

static void* thread_main(void* arg)
{
	return arg;
}

static int child(pid_t parent, int out)
{
	if (getppid() != parent || getpid() == parent)
		return 2;

	char* buffer = malloc(4096);
	if (!buffer)
		return 3;
	memset(buffer, 'x', 4096);
	free(buffer);

	pthread_t thread;
	void* result = NULL;
	if (pthread_create(&thread, NULL, thread_main, (void*) 0x1234) != 0)
		return 4;
	if (pthread_join(thread, &result) != 0 || result != (void*) 0x1234)
		return 5;

	if (write(out, "ok", 2) != 2)
		return 6;
	return 42;
}

int main(void)
{
	pid_t self = getpid();

	for (int i = 0; i < ROUNDS; i++)
	{
		int fds[2];
		if (pipe(fds) != 0)
		{
			perror("pipe");
			return 1;
		}

		pid_t pid = fork();
		if (pid < 0)
		{
			perror("fork");
			return 1;
		}
		if (pid == 0)
		{
			close(fds[0]);
			_exit(child(self, fds[1]));
		}

		close(fds[1]);
		char reply[3] = { 0 };
		ssize_t got = read(fds[0], reply, 2);
		close(fds[0]);

		int status;
		if (waitpid(pid, &status, 0) != pid)
		{
			perror("waitpid");
			return 1;
		}
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 42)
		{
			printf("FAIL: round %d: child status 0x%x\n", i, status);
			return 1;
		}
		if (got != 2 || strcmp(reply, "ok") != 0)
		{
			printf("FAIL: round %d: no reply from the child\n", i);
			return 1;
		}
		if (getpid() != self)
		{
			printf("FAIL: the parent's PID changed\n");
			return 1;
		}
	}

	printf("PASS\n");
	return 0;
}
