// A forked child can replace itself with another program with execv(), which keeps
// the descriptors it was given, and the parent gets that program's exit status.
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ROUNDS 20

int main(void)
{
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
			char* const argv[] = { "sh", "-c", "echo exec-ok; exit 7", NULL };

			close(fds[0]);
			if (dup2(fds[1], STDOUT_FILENO) < 0)
				_exit(2);
			close(fds[1]);
			execv("/bin/sh", argv);
			_exit(3);
		}

		close(fds[1]);
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
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 7)
		{
			printf("FAIL: round %d: child status 0x%x\n", i, status);
			return 1;
		}
		if (strcmp(output, "exec-ok\n") != 0)
		{
			printf("FAIL: round %d: unexpected output '%s'\n", i, output);
			return 1;
		}
	}

	printf("PASS\n");
	return 0;
}
