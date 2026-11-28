// Copyright (C) 2026 Lubos Dolezel
//
// A signal can be delivered at any instruction of a Mach trap, and the handler's frame is built below the
// stack pointer the interrupted code has at that moment. The arguments of the trap, which the dispatcher
// of 32-bit programs copies for the routine that implements the trap, must therefore never sit below the
// stack pointer, not even for the few instructions between copying them and reserving their space: a
// signal that arrives there overwrites them, and the trap runs with arguments that no caller gave
// (a garbage message address made mach_msg() fail with MACH_SEND_INVALID_DATA, or crash).
//
// The test sends a small message to a port of its own and receives it again, over and over, with helper
// processes signaling it all the time. Every send and every receive has to succeed and the message that
// arrives has to be the one that was sent. A trap whose arguments were overwritten returns another code,
// delivers another message, or crashes the program.
#include <mach/mach.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SIGNALERS 2
#define TIMEOUT_SECONDS 100
#define RUN_SECONDS 20
#define MSG_ID_BASE 0x5100

static pid_t g_signalers[SIGNALERS];

static void on_signal(int sig)
{
	(void) sig;
}

static void stop_signalers(void)
{
	for (int i = 0; i < SIGNALERS; i++)
	{
		if (g_signalers[i] > 0)
		{
			kill(g_signalers[i], SIGKILL);
			waitpid(g_signalers[i], NULL, 0);
			g_signalers[i] = 0;
		}
	}
}

static void on_alarm(int sig)
{
	(void) sig;
	const char msg[] = "FAIL: timed out\n";
	write(STDOUT_FILENO, msg, sizeof(msg) - 1);
	stop_signalers();
	_exit(1);
}

int main(void)
{
	mach_port_t port;
	struct sigaction sa;

	setvbuf(stdout, NULL, _IONBF, 0);

	if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port) != KERN_SUCCESS
		|| mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND) != KERN_SUCCESS)
	{
		printf("FAIL: cannot create a port\n");
		return 1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sa, NULL);
	signal(SIGALRM, on_alarm);
	alarm(TIMEOUT_SECONDS);

	pid_t self = getpid();
	for (int i = 0; i < SIGNALERS; i++)
	{
		g_signalers[i] = fork();
		if (g_signalers[i] == 0)
		{
			// stops when the program is gone
			while (getppid() == self && kill(self, SIGUSR1) == 0)
				usleep(100);
			_exit(0);
		}
	}

	int wrong = 0;
	long rounds = 0;
	mach_msg_return_t first_wrong = 0;
	time_t end = time(NULL) + RUN_SECONDS;
	for (int i = 0; wrong < 10; i++)
	{
		if ((i & 0xff) == 0 && time(NULL) >= end)
			break;
		rounds++;
		mach_msg_header_t out;
		struct
		{
			mach_msg_header_t header;
			mach_msg_trailer_t trailer;
			char slack[64];
		} in;

		memset(&out, 0, sizeof(out));
		out.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
		out.msgh_size = sizeof(out);
		out.msgh_remote_port = port;
		out.msgh_id = MSG_ID_BASE + (i & 0xff);

		mach_msg_return_t kr = mach_msg(&out, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof(out), 0, MACH_PORT_NULL, 1000, MACH_PORT_NULL);
		if (kr == MACH_MSG_SUCCESS)
			kr = mach_msg(&in.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(in), port, 1000, MACH_PORT_NULL);
		if (kr != MACH_MSG_SUCCESS || in.header.msgh_id != MSG_ID_BASE + (i & 0xff))
		{
			if (!wrong)
				first_wrong = kr;
			wrong++;
		}
	}

	alarm(0);
	stop_signalers();

	if (wrong)
	{
		printf("FAIL: %d of %ld messages did not arrive intact while the program was being signaled (first return code: 0x%x)\n", wrong, rounds, first_wrong);
		return 1;
	}
	printf("PASS: %ld messages to the program's own port arrived intact while it was being signaled\n", rounds);
	return 0;
}
