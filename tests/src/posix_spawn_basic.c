// posix_spawn() starts a program with the file actions and attributes it is given and
// stores the child's PID. Spawn a shell whose standard output goes to a pipe, once from
// a single-threaded process and then while another thread keeps creating threads.
#include <pthread.h>
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ROUNDS 20

extern char** environ;

static volatile int stop;

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

static int run_once(int round)
{
	int fds[2];
	if (pipe(fds) != 0)
	{
		perror("pipe");
		return 1;
	}

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_addclose(&actions, fds[0]);
	posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);
	posix_spawn_file_actions_addclose(&actions, fds[1]);

	char* const argv[] = { "sh", "-c", "echo spawn-ok; exit 3", NULL };
	pid_t pid = -1;
	int error = posix_spawn(&pid, "/bin/sh", &actions, NULL, argv, environ);
	posix_spawn_file_actions_destroy(&actions);
	close(fds[1]);
	if (error != 0)
	{
		printf("FAIL: round %d: posix_spawn: %s\n", round, strerror(error));
		close(fds[0]);
		return 1;
	}

	char output[32] = { 0 };
	size_t total = 0;
	ssize_t got;
	while (total < sizeof(output) - 1 && (got = read(fds[0], output + total, sizeof(output) - 1 - total)) > 0)
		total += got;
	close(fds[0]);

	int status;
	if (waitpid(pid, &status, 0) != pid)
	{
		perror("waitpid");
		return 1;
	}
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 3)
	{
		printf("FAIL: round %d: child status 0x%x\n", round, status);
		return 1;
	}
	if (strcmp(output, "spawn-ok\n") != 0)
	{
		printf("FAIL: round %d: unexpected output '%s'\n", round, output);
		return 1;
	}
	return 0;
}

int main(void)
{
	for (int i = 0; i < ROUNDS; i++)
		if (run_once(i))
			return 1;

	pthread_t thread;
	if (pthread_create(&thread, NULL, spawn_threads, NULL) != 0)
	{
		perror("pthread_create");
		return 1;
	}

	int failed = 0;
	for (int i = 0; i < ROUNDS && !failed; i++)
		failed = run_once(ROUNDS + i);

	stop = 1;
	pthread_join(thread, NULL);

	if (failed)
		return 1;
	printf("PASS\n");
	return 0;
}
