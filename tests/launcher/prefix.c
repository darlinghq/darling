// Host-side test of the setuid launcher's prefix path checks
// (src/startup/launcher-prefix.c).
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/un.h>
#include "../../src/shellspawn/shellspawn.h"
#include "launcher-prefix.h"

static int g_failures = 0;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

#define SUN_PATH_SIZE sizeof(((struct sockaddr_un*) NULL)->sun_path)

// An absolute path of exactly len bytes.
static const char* pathOfLength(size_t len)
{
	static char buf[1024];

	memset(buf, 'a', len);
	buf[0] = '/';
	buf[len] = '\0';
	return buf;
}

static void test_max_length(void)
{
	size_t max = launcherMaxPrefixLength();

	// The shellspawn socket has the longer name; with the longest accepted
	// prefix it exactly fills sun_path.
	CHECK(max + strlen(SHELLSPAWN_SOCKPATH) + 1 == SUN_PATH_SIZE);
	CHECK(max + strlen("/.darlingserver.sock") + 1 <= SUN_PATH_SIZE);
}

static void test_check(void)
{
	size_t max = launcherMaxPrefixLength();

	CHECK(launcherCheckPrefix("/home/user/.darling") == LAUNCHER_PREFIX_OK);
	CHECK(launcherCheckPrefix("/") == LAUNCHER_PREFIX_OK);
	CHECK(launcherCheckPrefix("/home/user/.darling/") == LAUNCHER_PREFIX_OK);
	CHECK(launcherCheckPrefix("") == LAUNCHER_PREFIX_EMPTY);
	CHECK(launcherCheckPrefix("prefix") == LAUNCHER_PREFIX_RELATIVE);
	CHECK(launcherCheckPrefix("./prefix") == LAUNCHER_PREFIX_RELATIVE);
	CHECK(launcherCheckPrefix("~/prefix") == LAUNCHER_PREFIX_RELATIVE);
	CHECK(launcherCheckPrefix(pathOfLength(max)) == LAUNCHER_PREFIX_OK);
	CHECK(launcherCheckPrefix(pathOfLength(max + 1)) == LAUNCHER_PREFIX_TOO_LONG);
	// The old limit, which let the socket path overflow sun_path.
	CHECK(launcherCheckPrefix(pathOfLength(255)) == LAUNCHER_PREFIX_TOO_LONG);

	CHECK(launcherPrefixCheckMessage(LAUNCHER_PREFIX_OK) == NULL);
	CHECK(launcherPrefixCheckMessage(LAUNCHER_PREFIX_RELATIVE) != NULL);
	CHECK(launcherPrefixCheckMessage(LAUNCHER_PREFIX_TOO_LONG) != NULL);
}

static void test_mode(void)
{
	CHECK(launcherPrefixModeAcceptable(S_IFDIR | 0700));
	CHECK(launcherPrefixModeAcceptable(S_IFDIR | 0755));
	CHECK(launcherPrefixModeAcceptable(S_IFDIR | 0775));
	CHECK(launcherPrefixModeAcceptable(S_IFDIR | 0750));
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | 0777));
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | 0757));
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | 0702));
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | S_ISVTX | 0777));
}

static void test_socket_path(void)
{
	struct
	{
		struct sockaddr_un addr;
		char canary[64];
	} s;
	size_t max = launcherMaxPrefixLength();

	memset(&s, 0x5a, sizeof(s));
	CHECK(launcherSocketPath(s.addr.sun_path, sizeof(s.addr.sun_path), "/home/user/.darling", SHELLSPAWN_SOCKPATH));
	CHECK(strcmp(s.addr.sun_path, "/home/user/.darling" SHELLSPAWN_SOCKPATH) == 0);

	memset(&s, 0x5a, sizeof(s));
	CHECK(launcherSocketPath(s.addr.sun_path, sizeof(s.addr.sun_path), pathOfLength(max), SHELLSPAWN_SOCKPATH));
	CHECK(strlen(s.addr.sun_path) == sizeof(s.addr.sun_path) - 1);

	// One byte too many, and far too many: refused, nothing written past sun_path.
	size_t lengths[] = { max + 1, 255, 900 };
	for (size_t i = 0; i < sizeof(lengths) / sizeof(lengths[0]); i++)
	{
		memset(&s, 0x5a, sizeof(s));
		CHECK(!launcherSocketPath(s.addr.sun_path, sizeof(s.addr.sun_path), pathOfLength(lengths[i]), SHELLSPAWN_SOCKPATH));
		CHECK(s.addr.sun_path[0] == '\0');
		for (size_t j = 0; j < sizeof(s.canary); j++)
			CHECK(s.canary[j] == 0x5a);
	}

	CHECK(!launcherSocketPath(s.addr.sun_path, 0, "/x", "/y"));
}

int main(void)
{
	test_max_length();
	test_check();
	test_mode();
	test_socket_path();

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("launcher prefix: all checks passed\n");
	return 0;
}
