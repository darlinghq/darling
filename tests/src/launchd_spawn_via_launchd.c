// Copyright (C) 2026 Lubos Dolezel
//
// spawn_via_launchd() (launch_priv.h) is how LaunchServices starts an
// application: launchd creates a job for the program and returns its pid
// once the program has been exec'd. Darling has no loginwindow and so no
// Aqua session, which launchd used to require for this call; it failed with
// EIO.
#include <launch_priv.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;
static char g_dir[] = "/tmp/spawnvialaunchd.XXXXXX";

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void cleanup(void)
{
	char path[sizeof(g_dir) + 16];
	snprintf(path, sizeof(path), "%s/pid", g_dir);
	unlink(path);
	rmdir(g_dir);
}

static void finish(int status)
{
	cleanup();
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(1);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	if (mkdtemp(g_dir) == NULL)
	{
		printf("FAIL: mkdtemp: %s\n", strerror(errno));
		finish(1);
	}

	char label[128], script[256], path[sizeof(g_dir) + 16];
	snprintf(label, sizeof(label), "org.darlinghq.tests.spawn-via-launchd.%d", (int) getpid());
	snprintf(path, sizeof(path), "%s/pid", g_dir);
	// The shell writes its own pid, which must be the pid launchd returns.
	snprintf(script, sizeof(script), "echo $$ > %s.tmp && mv %s.tmp %s", path, path, path);
	const char* argv[] = { "/bin/sh", "-c", script, NULL };

	errno = 0;
	pid_t pid = spawn_via_launchd(label, argv, NULL);
	printf("spawn_via_launchd: pid %d, errno %d\n", (int) pid, pid < 0 ? errno : 0);
	CHECK(pid > 0);
	if (pid <= 0)
		finish(1);

	// The job runs whether or not the caller waits for it.
	long reported = -1;
	for (int i = 0; i < 200 && reported < 0; i++)
	{
		FILE* f = fopen(path, "r");
		if (f)
		{
			if (fscanf(f, "%ld", &reported) != 1)
				reported = -1;
			fclose(f);
		}
		if (reported < 0)
			usleep(50 * 1000);
	}
	printf("the job reported pid %ld\n", reported);
	CHECK(reported == pid);

	if (g_failures == 0)
		printf("PASS\n");
	finish(g_failures == 0 ? 0 : 1);
}
