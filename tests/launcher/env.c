// Host-side test of the environment the setuid launcher passes to darlingserver
// (src/startup/launcher-env.c).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "launcher-env.h"

static int g_failures = 0;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

static size_t countOf(char** env)
{
	size_t n = 0;

	while (env[n] != NULL)
		n++;
	return n;
}

static int indexOf(char** env, const char* entry)
{
	for (size_t i = 0; env[i] != NULL; i++)
	{
		if (strcmp(env[i], entry) == 0)
			return (int) i;
	}
	return -1;
}

static bool hasName(char** env, const char* name)
{
	for (size_t i = 0; env[i] != NULL; i++)
	{
		if (launcherEnvEntryIs(env[i], name))
			return true;
	}
	return false;
}

static char* g_userEnv[] = {
	"HOME=/home/user",
	"PATH=/home/user/bin:/usr/bin",
	"XDG_CONFIG_HOME=/home/user/.config",
	"DARLING_NOOVERLAYFS=1",
	"DSERVER_INIT=/sbin/launchd",
	"DSERVER_LOG_LEVEL=debug",
	"DSERVER_LOG_STDERR=1",
	"DTAPE_LOG_SAFE_STUBS=1",
	"WSLENV=",
	"WSL_INTEROP=/run/WSL/1_interop",
	"DSERVER_WAIT4DEBUGGER=1",
	"SHELL=/bin/bash",
	"IFS= ",
	"ENV=/home/user/.evil",
	"BASH_ENV=/home/user/.evil",
	"LD_AUDIT=/home/user/audit.so",
	"GCONV_PATH=/home/user/gconv",
	"HOMEDIR=/x",
	"HOME",
	"=HOME",
	"XDG_CONFIG_HOMEX=/x",
	"LD_LIBRARY_PATH=/home/user/lib",
	"LD_PRELOAD=/home/user/preload.so",
	"",
	"LD_LIBRARY_PATHS=/kept",
	NULL
};

static void test_entry_names(void)
{
	CHECK(launcherEnvEntryIs("HOME=/x", "HOME"));
	CHECK(launcherEnvEntryIs("HOME=", "HOME"));
	CHECK(!launcherEnvEntryIs("HOMEDIR=/x", "HOME"));
	CHECK(!launcherEnvEntryIs("HOM=/x", "HOME"));
	CHECK(!launcherEnvEntryIs("HOME", "HOME"));
	CHECK(!launcherEnvEntryIs("=HOME", "HOME"));
	CHECK(!launcherEnvEntryIs("", "HOME"));
}

static void test_restricted(void)
{
	char** env = launcherServerEnvironment(g_userEnv, true);

	CHECK(env != NULL);
	if (!env)
		return;

	// Exactly the allowlisted variables and the fixed PATH, in their order.
	CHECK(indexOf(env, LAUNCHER_SERVER_PATH) == 0);
	CHECK(indexOf(env, "HOME=/home/user") == 1);
	CHECK(indexOf(env, "XDG_CONFIG_HOME=/home/user/.config") == 2);
	CHECK(indexOf(env, "DARLING_NOOVERLAYFS=1") == 3);
	CHECK(indexOf(env, "DSERVER_INIT=/sbin/launchd") == 4);
	CHECK(indexOf(env, "DSERVER_LOG_LEVEL=debug") == 5);
	CHECK(indexOf(env, "DSERVER_LOG_STDERR=1") == 6);
	CHECK(indexOf(env, "DTAPE_LOG_SAFE_STUBS=1") == 7);
	CHECK(indexOf(env, "WSLENV=") == 8);
	CHECK(indexOf(env, "WSL_INTEROP=/run/WSL/1_interop") == 9);
	CHECK(countOf(env) == 10);

	// In particular, none of these.
	CHECK(indexOf(env, "PATH=/home/user/bin:/usr/bin") < 0);
	CHECK(!hasName(env, "DSERVER_WAIT4DEBUGGER"));
	CHECK(!hasName(env, "SHELL"));
	CHECK(!hasName(env, "IFS"));
	CHECK(!hasName(env, "ENV"));
	CHECK(!hasName(env, "BASH_ENV"));
	CHECK(!hasName(env, "LD_AUDIT"));
	CHECK(!hasName(env, "GCONV_PATH"));
	CHECK(!hasName(env, "HOMEDIR"));
	CHECK(!hasName(env, "XDG_CONFIG_HOMEX"));
	CHECK(indexOf(env, "HOME") < 0);
	CHECK(indexOf(env, "=HOME") < 0);
	CHECK(!hasName(env, "LD_LIBRARY_PATH"));
	CHECK(!hasName(env, "LD_PRELOAD"));

	free(env);
}

static void test_restricted_empty(void)
{
	char* empty[] = { NULL };
	char** env = launcherServerEnvironment(empty, true);

	CHECK(env != NULL);
	if (!env)
		return;
	CHECK(countOf(env) == 1);
	CHECK(strcmp(env[0], LAUNCHER_SERVER_PATH) == 0);
	free(env);
}

static void test_guest(void)
{
	CHECK(launcherGuestMayGet("HOME=/home/user"));
	CHECK(launcherGuestMayGet("EMPTY="));
	CHECK(launcherGuestMayGet("DYLD_LIBRARY_PATH=/usr/local/lib"));
	CHECK(launcherGuestMayGet("LD_LIBRARY_PATHS=/x"));
	CHECK(launcherGuestMayGet("MY_LD_PRELOAD=/x"));

	CHECK(!launcherGuestMayGet("LD_LIBRARY_PATH=/home/user/lib"));
	CHECK(!launcherGuestMayGet("LD_LIBRARY_PATH="));
	CHECK(!launcherGuestMayGet("LD_PRELOAD=/home/user/preload.so"));
	CHECK(!launcherGuestMayGet(""));
	CHECK(!launcherGuestMayGet("NOEQUALS"));
	CHECK(!launcherGuestMayGet("=value"));
	CHECK(!launcherGuestMayGet("="));
}

// Started by root itself: the environment is root's own and is kept, except
// for malformed entries and the host loader's variables.
static void test_unrestricted(void)
{
	char** env = launcherServerEnvironment(g_userEnv, false);
	size_t n = 0;

	CHECK(env != NULL);
	if (!env)
		return;
	for (size_t i = 0; g_userEnv[i] != NULL; i++)
	{
		if (launcherGuestMayGet(g_userEnv[i]))
		{
			CHECK(env[n] == g_userEnv[i]);
			n++;
		}
	}
	CHECK(countOf(env) == n);
	CHECK(n == sizeof(g_userEnv) / sizeof(g_userEnv[0]) - 1 - 5);
	CHECK(indexOf(env, "PATH=/home/user/bin:/usr/bin") >= 0);
	CHECK(indexOf(env, "LD_LIBRARY_PATHS=/kept") >= 0);
	CHECK(!hasName(env, "LD_LIBRARY_PATH"));
	CHECK(!hasName(env, "LD_PRELOAD"));
	CHECK(indexOf(env, "") < 0);
	CHECK(indexOf(env, "HOME") < 0);
	CHECK(indexOf(env, "=HOME") < 0);
	free(env);
}

int main(void)
{
	test_entry_names();
	test_restricted();
	test_restricted_empty();
	test_guest();
	test_unrestricted();

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("launcher environment: all checks passed\n");
	return 0;
}
