// Copyright (C) 2026 Lubos Dolezel
//
// A dispatch signal source counts every SIGUSR1 sent to the process while the
// signal is ignored (SIG_IGN), whichever thread the signal is delivered to,
// also after the main thread has left through dispatch_main(). Idle threads
// that do not block the signal are present, as in a typical daemon.
// Build for x86_64 and i386; prints PASS or FAIL.
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 20

static void* idle_thread(void* arg)
{
	(void) arg;
	for (;;)
		pause();
	return NULL;
}

int main(void)
{
	__block int got = 0;

	signal(SIGUSR1, SIG_IGN);
	dispatch_source_t src = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGUSR1, 0, dispatch_get_main_queue());
	dispatch_source_set_event_handler(src, ^{ got += (int) dispatch_source_get_data(src); });
	dispatch_resume(src);

	for (int i = 0; i < 4; i++)
	{
		pthread_t t;
		pthread_create(&t, NULL, idle_thread, NULL);
	}

	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 500 * NSEC_PER_MSEC), dispatch_get_global_queue(0, 0), ^{
		pid_t parent = getpid();
		pid_t child = fork();
		if (child == 0)
		{
			for (int i = 0; i < N; i++)
			{
				kill(parent, SIGUSR1);
				usleep(100000);
			}
			_exit(0);
		}
		waitpid(child, NULL, 0);
		sleep(1);
		dispatch_async(dispatch_get_main_queue(), ^{
			printf("sent=%d counted=%d\n%s\n", N, got, got == N ? "PASS" : "FAIL");
			fflush(stdout);
			_exit(got == N ? 0 : 1);
		});
	});
	dispatch_main();
}
