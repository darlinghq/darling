#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

static void handler(int);
void test(int handleIt);

int main()
{
	printf("First pass...\n");
	test(1);
	printf("Second pass...\n");
	test(0);

	return 0;
}

void test(int handleIt)
{
	pid_t child = fork();
	if (child)
	{
		sleep(1);
		kill(child, SIGSYS);

		int status;
		pid_t p = wait(&status);

		if (WIFSIGNALED(status) == handleIt)
		{
			printf("WIFSIGNALED...\n");
			exit(1);
		}
		if (!WIFEXITED(status) == handleIt)
		{
			printf("!WIFEXITED...\n");
			exit(1);
		}
	}
	else
	{
		signal(SIGSYS, handler);

		if (!handleIt)
		{
			struct sigaction act;

			memset(&act, 0, sizeof(act));
			act.sa_handler = SIG_DFL;
			sigaction(SIGSYS, &act, NULL);
		}


		sleep(10);
		exit(1);
	}
}

static void handler(int sig)
{
	if (sig != SIGSYS)
	{
		printf("Bad signal code: %d\n", sig);
		exit(1);
	}
	else
	{
		printf("Handler!\n");
		exit(0);
	}
}

