// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of how the launcher names the host's PulseAudio files for the guest
// (src/startup/launcher-pulse.c). The argument is a new private directory to work in.
#define _XOPEN_SOURCE 700
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include "launcher-pulse.h"

static int g_failures = 0;
static const char* base;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

static void path(char* out, size_t size, const char* rel)
{
	snprintf(out, size, "%s/%s", base, rel);
}

static void makeDir(const char* rel)
{
	char p[4096];

	path(p, sizeof(p), rel);
	if (mkdir(p, 0700) != 0)
	{
		perror(p);
		exit(2);
	}
}

static void makeSocket(const char* rel)
{
	char p[4096];
	struct sockaddr_un addr = { .sun_family = AF_UNIX };
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);

	// sun_path is short; bind through a relative name from inside the work directory
	path(p, sizeof(p), rel);
	strncpy(addr.sun_path, rel, sizeof(addr.sun_path) - 1);
	if (fd < 0 || chdir(base) != 0 || bind(fd, (struct sockaddr*) &addr, sizeof(addr)) != 0)
	{
		perror(p);
		exit(2);
	}
	// the socket file stays after the descriptor is closed
	close(fd);
}

static void makeLink(const char* target, const char* rel)
{
	char p[4096];

	path(p, sizeof(p), rel);
	if (symlink(target, p) != 0)
	{
		perror(p);
		exit(2);
	}
}

static void makeFile(const char* rel)
{
	char p[4096];
	FILE* f;

	path(p, sizeof(p), rel);
	f = fopen(p, "w");
	if (!f)
	{
		perror(p);
		exit(2);
	}
	fputs("cookie", f);
	fclose(f);
}

static int removeEntry(const char* path, const struct stat* st, int flag, struct FTW* ftw)
{
	(void) st;
	(void) flag;
	(void) ftw;
	return remove(path);
}

int main(int argc, char** argv)
{
	struct launcher_pulse_env env;
	char home[4096], tmp[4096], expectRuntime[4400], expectCookie[4400], xdg[4096];

	char workDir[] = "/tmp/darling-pulse-test.XXXXXX";

	// the work directory is made here unless given (a socket path is short, so it is kept short)
	base = argc >= 2 ? argv[1] : mkdtemp(workDir);
	if (base == NULL)
		return 2;

	// a host like Termux: ~/.config/pulse/<id>-runtime links to a private directory holding the socket
	makeDir("home");
	makeDir("home/.config");
	makeDir("home/.config/pulse");
	makeDir("tmp");
	makeDir("tmp/pulse-abc");
	makeSocket("tmp/pulse-abc/native");
	path(home, sizeof(home), "home");
	path(tmp, sizeof(tmp), "tmp/pulse-abc");
	makeLink(tmp, "home/.config/pulse/machine-runtime");
	makeFile("home/.config/pulse/cookie");
	snprintf(expectRuntime, sizeof(expectRuntime), "PULSE_RUNTIME_PATH=%s", tmp);
	snprintf(expectCookie, sizeof(expectCookie), "PULSE_COOKIE=%s/.config/pulse/cookie", home);

	CHECK(launcherFindPulse(home, NULL, NULL, NULL, NULL, &env));
	CHECK(strcmp(env.runtimePath, expectRuntime) == 0);
	CHECK(strcmp(env.cookie, expectCookie) == 0);

	// the user's own settings are kept
	CHECK(launcherFindPulse(home, NULL, "unix:/x", NULL, NULL, &env) && env.runtimePath[0] == '\0');
	CHECK(launcherFindPulse(home, NULL, NULL, "/other", NULL, &env) && env.runtimePath[0] == '\0');
	CHECK(launcherFindPulse(home, NULL, NULL, NULL, "/mine", &env) && env.cookie[0] == '\0' && strcmp(env.runtimePath, expectRuntime) == 0);
	CHECK(!launcherFindPulse(home, NULL, "unix:/x", NULL, "/mine", &env));
	CHECK(env.runtimePath[0] == '\0' && env.cookie[0] == '\0');

	// an XDG_RUNTIME_DIR that already leads to a server is enough
	makeDir("xdg");
	makeDir("xdg/pulse");
	makeSocket("xdg/pulse/native");
	path(xdg, sizeof(xdg), "xdg");
	CHECK(launcherFindPulse(home, xdg, NULL, NULL, "/mine", &env) == false);
	// one without a server does not hide the host's link
	CHECK(launcherFindPulse(home, "/nonexistent", NULL, NULL, "/mine", &env) && strcmp(env.runtimePath, expectRuntime) == 0);

	// a link to a directory without a socket, a relative link, and a plain file are not servers
	makeDir("home2");
	makeDir("home2/.config");
	makeDir("home2/.config/pulse");
	makeDir("tmp/pulse-dead");
	{
		char dead[4096];

		path(dead, sizeof(dead), "tmp/pulse-dead");
		makeLink(dead, "home2/.config/pulse/a-runtime");
	}
	makeLink("../../../tmp/pulse-abc", "home2/.config/pulse/b-runtime");
	makeFile("home2/.config/pulse/c-runtime");
	makeFile("home2/.config/pulse/not-a-runtime-link");
	path(home, sizeof(home), "home2");
	CHECK(!launcherFindPulse(home, NULL, NULL, NULL, NULL, &env));

	// a cookie that is a directory is not used; missing home, relative home
	makeDir("home2/.config/pulse/cookie");
	CHECK(!launcherFindPulse(home, NULL, NULL, NULL, NULL, &env));
	CHECK(!launcherFindPulse(NULL, NULL, NULL, NULL, NULL, &env));
	CHECK(!launcherFindPulse("relative", NULL, NULL, NULL, NULL, &env));
	CHECK(!launcherFindPulse("/nonexistent-home", NULL, NULL, NULL, NULL, &env));

	chdir("/");
	if (argc < 2)
		nftw(base, removeEntry, 16, FTW_DEPTH | FTW_PHYS);
	if (g_failures == 0)
		printf("all passed\n");
	return g_failures != 0;
}
