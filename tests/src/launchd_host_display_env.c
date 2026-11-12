// Copyright (C) 2026 Lubos Dolezel
//
// The container's launchd gets the host session's display variables (DISPLAY,
// WAYLAND_DISPLAY, XAUTHORITY, XDG_RUNTIME_DIR) from the environment darling
// was started in, so jobs it starts (daemons, applications started through
// launchd) reach the host display server without `launchctl setenv`. A
// variable that is unset on the host must stay unset in the job.
//
// The test runs in the same environment darling was started in (darling shell
// passes the caller's environment on); it needs the container to have been
// started from that environment too (darling shutdown first).
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static const char* const names[] = { "DISPLAY", "WAYLAND_DISPLAY", "XAUTHORITY", "XDG_RUNTIME_DIR" };

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

// Value of NAME in the job's `env` output, or NULL.
static char* find(char* out, const char* name)
{
	size_t len = strlen(name);
	for (char* line = strtok(out, "\n"); line != NULL; line = strtok(NULL, "\n"))
	{
		if (strncmp(line, name, len) == 0 && line[len] == '=')
			return line + len + 1;
	}
	return NULL;
}

int main(void)
{
	char dir[] = "/tmp/hostdisplayenv.XXXXXX";
	char outPath[64], cmd[256], label[64];
	char buf[8192];
	struct stat st;

	signal(SIGALRM, on_alarm);
	alarm(60);

	if (mkdtemp(dir) == NULL)
	{
		printf("FAIL: mkdtemp: %s\n", strerror(errno));
		return 1;
	}
	snprintf(outPath, sizeof(outPath), "%s/env.out", dir);
	snprintf(label, sizeof(label), "hostdisplayenv.%d", (int) getpid());
	snprintf(cmd, sizeof(cmd), "launchctl submit -l %s -o %s -e %s/env.err -- /usr/bin/env", label, outPath, dir);
	CHECK(system(cmd) == 0);

	for (int i = 0; i < 100 && (stat(outPath, &st) != 0 || st.st_size == 0); i++)
		usleep(100000);
	usleep(200000);

	FILE* f = fopen(outPath, "r");
	CHECK(f != NULL);
	size_t n = f ? fread(buf, 1, sizeof(buf) - 1, f) : 0;
	buf[n] = 0;
	if (f)
		fclose(f);
	CHECK(n > 0);

	for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
	{
		const char* mine = getenv(names[i]);
		char copy[sizeof(buf)];
		memcpy(copy, buf, sizeof(buf));
		char* theirs = find(copy, names[i]);

		if (mine == NULL)
		{
			if (theirs != NULL)
				printf("%s unset here but the job has %s\n", names[i], theirs);
			CHECK(theirs == NULL);
		}
		else
		{
			if (theirs == NULL || strcmp(theirs, mine) != 0)
				printf("%s is %s here but the job has %s\n", names[i], mine, theirs ? theirs : "(unset)");
			CHECK(theirs != NULL && strcmp(theirs, mine) == 0);
		}
	}

	snprintf(cmd, sizeof(cmd), "launchctl remove %s >/dev/null 2>&1", label);
	system(cmd);
	unlink(outPath);
	snprintf(cmd, sizeof(cmd), "%s/env.err", dir);
	unlink(cmd);
	rmdir(dir);

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("PASSED\n");
	return 0;
}
