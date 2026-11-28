// Copyright (C) 2026 Lubos Dolezel
//
// A process that is started by launchd must find its bootstrap port in place when it starts: launchd sets the
// bootstrap port it hands out around each fork() and clears it again as soon as fork() returns in the parent,
// so the child depends on fork() not returning in the parent before the child has been created far enough to
// have inherited the port (the child's check-in with darlingserver).
//
// The test provokes the situation in which launchd's fork() returned too early. Children whose forking
// process is killed during fork() become children of launchd, the container's reaper, and then check in
// after their real parent is gone. A check-in of such a child must not end a wait of the reaper; if it did,
// the reaper would, from then on, return from each of its own fork() calls one child too early, and about
// every other job it started would find no bootstrap port, fail its post-fork handshake and exit with
// status 1 before it ever ran.
//
// The test creates many such orphans, then has launchd start jobs through `launchctl submit` and requires that
// every one of them runs.
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define STORM_ROUNDS 400
#define JOBS 24
#define JOB_TIMEOUT_MS 10000

// Forks children that exit at once until killed, so that the kill is likely to arrive during one of the fork() calls.
static void forker(void)
{
	for (;;)
	{
		pid_t pid = fork();
		if (pid == 0)
			_exit(0);
		if (pid < 0)
			usleep(1000);
	}
}

static void orphan_storm(void)
{
	srandom(getpid());
	for (int i = 0; i < STORM_ROUNDS; i++)
	{
		pid_t pid = fork();
		if (pid == 0)
		{
			forker();
			_exit(0);
		}
		if (pid < 0)
			continue;
		usleep(random() % 3000);
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
	}
}

static int run(const char* command)
{
	int status = system(command);
	return status == -1 ? -1 : WEXITSTATUS(status);
}

static int file_has_content(const char* path)
{
	struct stat st;
	return stat(path, &st) == 0 && st.st_size > 0;
}

int main(void)
{
	char dir[64];
	snprintf(dir, sizeof(dir), "/tmp/fork-reaper-checkin.%d", getpid());
	mkdir(dir, 0700);

	// without launchctl there is nothing to test
	if (run("launchctl list >/dev/null 2>&1") != 0)
	{
		printf("SKIP: launchd is not reachable\n");
		return 0;
	}

	orphan_storm();
	// let the orphans finish checking in and exiting
	sleep(2);

	int started = 0;
	for (int i = 0; i < JOBS; i++)
	{
		char out[128], cmd[512], label[96];
		snprintf(label, sizeof(label), "org.darlinghq.test.fork-reaper-checkin.%d.%d", getpid(), i);
		snprintf(out, sizeof(out), "%s/job%d", dir, i);
		snprintf(cmd, sizeof(cmd), "launchctl submit -l %s -- /bin/sh -c 'echo ran > %s'", label, out);
		if (run(cmd) != 0)
		{
			printf("FAIL: launchctl submit failed for job %d\n", i);
			return 1;
		}

		for (int waited = 0; waited < JOB_TIMEOUT_MS && !file_has_content(out); waited += 50)
		{
			struct timespec ts = { 0, 50 * 1000 * 1000 };
			nanosleep(&ts, NULL);
		}
		if (file_has_content(out))
			started++;

		snprintf(cmd, sizeof(cmd), "launchctl remove %s >/dev/null 2>&1", label);
		run(cmd);
		unlink(out);
	}
	rmdir(dir);

	if (started != JOBS)
	{
		printf("FAIL: launchd started %d of %d jobs after the orphans checked in\n", started, JOBS);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
