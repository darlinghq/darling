// Copyright (C) 2026 Lubos Dolezel
//
// A child created by fork() inherits its parent's task special ports, and
// libSystem sets the child's bootstrap_port from TASK_BOOTSTRAP_PORT. What the
// child gets is the parent's bootstrap port at the time of fork(): a parent
// that changes it right after fork() returns (launchd does this around every
// job it starts) must not change what the child got. That must hold however
// the parent's other children behave and whatever reaches the parent during
// fork(), so the test forks in three ways:
//   - with nothing else going on,
//   - while other children of the parent execute a program, half of them
//     with posix_spawn(POSIX_SPAWN_SETEXEC) as launchd starts its jobs and
//     half with execv(), and
//   - while a helper process keeps signaling the parent.
//
// Each child sends a message with its index to its bootstrap_port; the parent
// set its bootstrap port to a port it receives on before each fork(), so it
// must receive one message per child.
#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ROUNDS 200
#define TIMEOUT 150
#define MSG_ID_BASE 0x4a00

// child exit codes
#define CHILD_OK 0
#define CHILD_NULL_PORT 10
#define CHILD_SEND_FAILED 11
#define CHILD_EXEC_FAILED 12

static pid_t g_helper = -1, g_plain = -1;
static pid_t g_senders[ROUNDS], g_execers[ROUNDS];
static const char* volatile g_stage = "start";
static volatile int g_round = -1;
static char g_self[PATH_MAX];

extern char** environ;

static void on_signal(int sig)
{
	(void) sig;
}

static void kill_children(void)
{
	if (g_helper > 0)
		kill(g_helper, SIGKILL);
	if (g_plain > 0)
		kill(g_plain, SIGKILL);
	for (int i = 0; i < ROUNDS; i++)
	{
		if (g_senders[i] > 0)
			kill(g_senders[i], SIGKILL);
		if (g_execers[i] > 0)
			kill(g_execers[i], SIGKILL);
	}
}

// Kills everything this test started, so a hang leaves nothing behind.
static void on_alarm(int sig)
{
	(void) sig;
	char line[160];
	int n = snprintf(line, sizeof(line), "FAIL: timed out after %d s (%s, round %d)\n", TIMEOUT, g_stage, g_round);
	write(STDOUT_FILENO, line, n);
	kill_children();
	_exit(1);
}

static void stage(const char* name)
{
	g_stage = name;
	printf("%s\n", name);
}

static pid_t wait_child(pid_t pid, int* status)
{
	pid_t r;
	while ((r = waitpid(pid, status, 0)) < 0 && errno == EINTR)
		;
	return r;
}

static void child_send(int index)
{
	if (bootstrap_port == MACH_PORT_NULL)
		_exit(CHILD_NULL_PORT);

	mach_msg_header_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
	msg.msgh_size = sizeof(msg);
	msg.msgh_remote_port = bootstrap_port;
	msg.msgh_id = MSG_ID_BASE + index;
	kern_return_t kr = mach_msg(&msg, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof(msg), 0, MACH_PORT_NULL, 5000, MACH_PORT_NULL);
	_exit(kr == KERN_SUCCESS ? CHILD_OK : CHILD_SEND_FAILED);
}

// Receives the children's messages; returns how many distinct children sent one.
static int receive_all(mach_port_t port, int timeout_ms)
{
	static bool seen[ROUNDS];
	memset(seen, 0, sizeof(seen));
	int received = 0;
	for (;;)
	{
		struct
		{
			mach_msg_header_t header;
			mach_msg_trailer_t trailer;
			char slack[64];
		} msg;
		kern_return_t kr = mach_msg(&msg.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(msg), port, timeout_ms, MACH_PORT_NULL);
		if (kr != KERN_SUCCESS)
			break;
		int index = msg.header.msgh_id - MSG_ID_BASE;
		if (index >= 0 && index < ROUNDS && !seen[index])
		{
			seen[index] = true;
			received++;
		}
	}
	return received;
}

// Forks ROUNDS children that check their bootstrap port; returns the number of failures (0 or 1).
static int run_phase(const char* name, mach_port_t port, mach_port_t original, bool with_execs, bool with_signals)
{
	mach_port_t self = mach_task_self();
	int status;

	stage(name);
	receive_all(port, 0);

	if (with_signals)
	{
		pid_t parent = getpid();
		pid_t helper = fork();
		if (helper == 0)
		{
			// stops when the parent is gone, and in any case before the parent's timeout
			time_t end = time(NULL) + TIMEOUT - 10;
			while (time(NULL) < end && kill(parent, SIGUSR1) == 0)
				usleep(50);
			_exit(0);
		}
		g_helper = helper;
	}

	for (int i = 0; i < ROUNDS; i++)
	{
		g_round = i;
		if (with_execs)
		{
			// a child that runs a program while the parent goes on forking
			// it gets the parent's own bootstrap port, and has to have one after the exec too
			task_set_bootstrap_port(self, original);
			pid_t execer = fork();
			if (execer == 0)
			{
				char* const args[] = { g_self, "exec-child", NULL };
				if (i % 2 == 0)
				{
					posix_spawnattr_t attr;
					posix_spawnattr_init(&attr);
					posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETEXEC);
					posix_spawn(NULL, g_self, NULL, &attr, args, environ);
				}
				else
					execv(g_self, args);
				_exit(CHILD_EXEC_FAILED);
			}
			g_execers[i] = execer;
		}

		task_set_bootstrap_port(self, port);
		pid_t child = fork();
		if (child == 0)
			child_send(i);
		// what launchd does after starting a job
		task_set_bootstrap_port(self, MACH_PORT_NULL);
		g_senders[i] = child;
		if (i % 50 == 49)
			printf("%d forks\n", i + 1);
	}
	task_set_bootstrap_port(self, original);

	int null_port = 0, send_failed = 0, other = 0, fork_failed = 0, exec_failed = 0;
	for (int i = 0; i < ROUNDS; i++)
	{
		g_round = i;
		if (g_senders[i] < 0)
			fork_failed++;
		else if (wait_child(g_senders[i], &status) != g_senders[i] || !WIFEXITED(status))
			other++;
		else if (WEXITSTATUS(status) == CHILD_NULL_PORT)
			null_port++;
		else if (WEXITSTATUS(status) == CHILD_SEND_FAILED)
			send_failed++;
		else if (WEXITSTATUS(status) != CHILD_OK)
			other++;
		g_senders[i] = 0;

		if (with_execs)
		{
			if (g_execers[i] < 0)
				fork_failed++;
			else if (wait_child(g_execers[i], &status) != g_execers[i] || !WIFEXITED(status) || WEXITSTATUS(status) != CHILD_OK)
				exec_failed++;
			g_execers[i] = 0;
		}
	}
	g_round = -1;

	if (g_helper > 0)
	{
		kill(g_helper, SIGKILL);
		wait_child(g_helper, &status);
		g_helper = -1;
	}

	int received = receive_all(port, 1000);
	if (fork_failed || null_port || send_failed || other || exec_failed || received != ROUNDS)
	{
		printf("FAIL: %d forks: %d children had no bootstrap port, %d could not send to it, "
			"%d ended otherwise, %d forks failed, %d exec children failed; %d of the messages arrived at the parent's port\n",
			ROUNDS, null_port, send_failed, other, fork_failed, exec_failed, received);
		return 1;
	}
	printf("ok: %d children got the bootstrap port the parent had at fork()\n", ROUNDS);
	return 0;
}

int main(int argc, char** argv)
{
	if (argc == 2 && strcmp(argv[1], "exec-child") == 0)
		return bootstrap_port == MACH_PORT_NULL ? CHILD_NULL_PORT : CHILD_OK;

	int failures = 0;
	mach_port_t self = mach_task_self();

	setvbuf(stdout, NULL, _IONBF, 0);

	// In case the alarm cannot be handled: a watchdog kills the test's
	// process group (this process and everything it started) a bit later.
	setpgid(0, 0);
	pid_t watchdog = fork();
	if (watchdog == 0)
	{
		sleep(TIMEOUT + 15);
		const char msg[] = "FAIL: the test did not handle its alarm; killing its processes\n";
		write(STDOUT_FILENO, msg, sizeof(msg) - 1);
		kill(0, SIGKILL);
		_exit(1);
	}

	signal(SIGALRM, on_alarm);
	alarm(TIMEOUT);

	char exe[PATH_MAX];
	uint32_t size = sizeof(exe);
	if (_NSGetExecutablePath(exe, &size) != 0 || realpath(exe, g_self) == NULL)
	{
		printf("FAIL: cannot find this program's path\n");
		kill(watchdog, SIGKILL);
		return 1;
	}

	// A plain child has a bootstrap port.
	stage("forking a plain child");
	pid_t pid = fork();
	if (pid == 0)
		_exit(bootstrap_port == MACH_PORT_NULL ? CHILD_NULL_PORT : CHILD_OK);
	g_plain = pid;
	int status;
	if (pid < 0 || wait_child(pid, &status) != pid || !WIFEXITED(status) || WEXITSTATUS(status) != CHILD_OK)
	{
		printf("FAIL: a forked child has no bootstrap port (status 0x%x)\n", pid < 0 ? -1 : status);
		failures++;
	}
	else
		printf("ok: a forked child has a bootstrap port\n");
	g_plain = -1;

	mach_port_t original;
	if (task_get_bootstrap_port(self, &original) != KERN_SUCCESS)
	{
		printf("FAIL: task_get_bootstrap_port\n");
		kill(watchdog, SIGKILL);
		return 1;
	}

	mach_port_t port;
	if (mach_port_allocate(self, MACH_PORT_RIGHT_RECEIVE, &port) != KERN_SUCCESS
		|| mach_port_insert_right(self, port, port, MACH_MSG_TYPE_MAKE_SEND) != KERN_SUCCESS)
	{
		printf("FAIL: cannot create a port\n");
		kill(watchdog, SIGKILL);
		return 1;
	}
	mach_port_limits_t limits = { .mpl_qlimit = MACH_PORT_QLIMIT_MAX };
	mach_port_set_attributes(self, port, MACH_PORT_LIMITS_INFO, (mach_port_info_t) &limits, MACH_PORT_LIMITS_INFO_COUNT);

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sa, NULL);

	failures += run_phase("forking children", port, original, false, false);
	failures += run_phase("forking children while other children execute a program", port, original, true, false);
	failures += run_phase("forking children while being signaled", port, original, false, true);

	alarm(0);
	kill(watchdog, SIGKILL);
	wait_child(watchdog, &status);
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures ? 1 : 0;
}
