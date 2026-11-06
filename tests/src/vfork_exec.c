// vfork() suspends the parent until the child calls execve() or _exit(). The child
// may only do that; the parent then continues with the child's PID and can collect
// its exit status.
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define ROUNDS 20

int main(void)
{
	for (int i = 0; i < ROUNDS; i++)
	{
		int exec_child = i % 2;
		char* const argv[] = { "sh", "-c", "exit 9", NULL };

		pid_t pid = vfork();
		if (pid < 0)
		{
			perror("vfork");
			return 1;
		}
		if (pid == 0)
		{
			if (exec_child)
				execv("/bin/sh", argv);
			_exit(5);
		}

		int status;
		if (waitpid(pid, &status, 0) != pid)
		{
			perror("waitpid");
			return 1;
		}
		int expected = exec_child ? 9 : 5;
		if (!WIFEXITED(status) || WEXITSTATUS(status) != expected)
		{
			printf("FAIL: round %d: child status 0x%x, expected exit %d\n", i, status, expected);
			return 1;
		}
	}

	printf("PASS\n");
	return 0;
}
