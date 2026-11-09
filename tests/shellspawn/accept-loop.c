// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of shellspawn's accept loop. shellspawn.c is linked in as it
// is (with its main renamed), and accept, usleep, perror and fork are replaced
// with the linker's --wrap, so each case can script what accept returns.
//
// EINTR, EAGAIN and ECONNABORTED are retried at once. EMFILE, ENFILE, ENOBUFS
// and ENOMEM are retried after a 100 ms pause and logged once per run of
// failures; an accepted connection ends the run. Any other error is logged and
// makes listenForConnections return -1.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int listenForConnections(void);
extern int g_serverSocket;

// Only called from shellspawn's main, which the test does not run
void waitForSystemConfig(void)
{
}

// A positive entry is a descriptor accept returns, a negative one an errno
#define MAX_SCRIPT 16
static int script[MAX_SCRIPT];
static int script_len, script_pos;
static int accepts, pauses, logs, forks;
static long paused_us;

// Not open in the test, so the loop's close() of it in the "parent" is harmless
#define CONNECTION_FD 1000

int __wrap_accept(int fd, struct sockaddr* addr, socklen_t* len)
{
	(void)fd;
	(void)addr;
	(void)len;

	accepts++;
	if (script_pos >= script_len)
	{
		// A loop that keeps retrying past the end of the script would never
		// return; end the test instead of hanging
		if (accepts > script_len + 4)
		{
			printf("FAIL: accept called %d times for a script of %d\n", accepts, script_len);
			exit(1);
		}
		errno = EBADF;
		return -1;
	}

	int next = script[script_pos++];
	if (next > 0)
		return next;
	errno = -next;
	return -1;
}

int __wrap_usleep(useconds_t usec)
{
	pauses++;
	paused_us += usec;
	return 0;
}

void __wrap_perror(const char* s)
{
	(void)s;
	logs++;
}

pid_t __wrap_fork(void)
{
	// Pretend to be the parent; the child would spawn a shell
	forks++;
	return 12345;
}

struct expect
{
	int accepts, pauses, logs, forks;
};

static int failures;

static void run(const char* name, const int* steps, int count, struct expect want)
{
	memcpy(script, steps, count * sizeof(*steps));
	script_len = count;
	script_pos = accepts = pauses = logs = forks = 0;
	paused_us = 0;

	int rc = listenForConnections();
	int ok = rc == -1
		&& accepts == want.accepts
		&& pauses == want.pauses
		&& paused_us == 100000L * want.pauses
		&& logs == want.logs
		&& forks == want.forks;

	printf("%s: %s: rc=%d accepts=%d pauses=%d (%ld us) logs=%d forks=%d\n",
		ok ? "PASS" : "FAIL", name, rc, accepts, pauses, paused_us, logs, forks);
	if (!ok)
		failures++;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	g_serverSocket = 3;

	// Each script ends with EBADF from the wrapper (one more accept and log)
	{
		int s[] = { -EINTR, -EAGAIN, -ECONNABORTED, CONNECTION_FD };
		run("interrupted accept is retried at once", s, 4, (struct expect){ 5, 0, 1, 1 });
	}
	{
		int s[] = { -EMFILE, -EMFILE, -EMFILE, CONNECTION_FD };
		run("EMFILE pauses, logged once per run", s, 4, (struct expect){ 5, 3, 2, 1 });
	}
	{
		int s[] = { -ENFILE, -ENOBUFS, -ENOMEM, CONNECTION_FD, -EMFILE, CONNECTION_FD };
		run("a connection starts a new run", s, 6, (struct expect){ 7, 4, 3, 2 });
	}
	{
		int s[] = { -EINVAL };
		run("EINVAL ends the loop", s, 1, (struct expect){ 1, 0, 1, 0 });
	}
	{
		int s[] = { -ENOTSOCK };
		run("ENOTSOCK ends the loop", s, 1, (struct expect){ 1, 0, 1, 0 });
	}

	if (failures)
	{
		printf("FAILED %d\n", failures);
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}
