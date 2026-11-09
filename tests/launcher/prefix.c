// Host-side test of the setuid launcher's prefix path checks
// (src/startup/launcher-prefix.c).
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
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
	// Not tied to sun_path: sockets are reached through a descriptor.
	CHECK(launcherMaxPrefixLength() == 255);
	CHECK(launcherMaxPrefixLength() > SUN_PATH_SIZE);
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
	// Longer than sun_path can hold together with the socket names.
	CHECK(launcherCheckPrefix(pathOfLength(SUN_PATH_SIZE)) == LAUNCHER_PREFIX_OK);

	CHECK(launcherPrefixCheckMessage(LAUNCHER_PREFIX_OK) == NULL);
	CHECK(launcherPrefixCheckMessage(LAUNCHER_PREFIX_RELATIVE) != NULL);
	CHECK(launcherPrefixCheckMessage(LAUNCHER_PREFIX_TOO_LONG) != NULL);
}

static void test_mode(void)
{
	CHECK(launcherPrefixModeAcceptable(S_IFDIR | 0700, false));
	CHECK(launcherPrefixModeAcceptable(S_IFDIR | 0755, false));
	CHECK(launcherPrefixModeAcceptable(S_IFDIR | 0750, false));
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | 0777, false));
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | 0757, false));
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | 0702, false));
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | S_ISVTX | 0777, false));

	// Group write: only with the user's private group.
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | 0775, false));
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | 0770, false));
	CHECK(launcherPrefixModeAcceptable(S_IFDIR | 0775, true));
	CHECK(launcherPrefixModeAcceptable(S_IFDIR | 0770, true));
	CHECK(!launcherPrefixModeAcceptable(S_IFDIR | 0777, true));
}

static void test_private_group(void)
{
	char* none[] = { NULL };
	char* self[] = { "alice", NULL };
	char* others[] = { "alice", "bob", NULL };

	CHECK(launcherGroupIsUserPrivate(1000, "alice", none, 1000, "alice"));
	CHECK(launcherGroupIsUserPrivate(1000, "alice", self, 1000, "alice"));
	CHECK(launcherGroupIsUserPrivate(1000, "alice", NULL, 1000, "alice"));
	CHECK(!launcherGroupIsUserPrivate(1000, "alice", others, 1000, "alice"));
	// A shared primary group, as with a "users" group.
	CHECK(!launcherGroupIsUserPrivate(100, "users", none, 100, "alice"));
	// Not the user's primary group.
	CHECK(!launcherGroupIsUserPrivate(1001, "alice", none, 1000, "alice"));
}

static void test_socket_path(void)
{
	struct
	{
		struct sockaddr_un addr;
		char canary[64];
	} s;
	size_t max = SUN_PATH_SIZE - 1 - strlen(SHELLSPAWN_SOCKPATH);

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

// A socket inside a directory whose path is far too long for sun_path can be
// bound and connected to through a descriptor of the directory.
static void test_dir_socket_path(void)
{
	char base[] = "/tmp/launcher-prefix-XXXXXX";
	char dir[1024];
	char real[1100];
	struct sockaddr_un addr;
	int dirfd, server, client;
	struct stat st;

	CHECK(mkdtemp(base) != NULL);
	snprintf(dir, sizeof(dir), "%s", base);
	// Grow the path to over 250 bytes, one level at a time.
	while (strlen(dir) < 250)
	{
		strcat(dir, "/abcdefghijklmnopqrstuvwxyz0123456789");
		CHECK(mkdir(dir, 0700) == 0);
	}
	snprintf(real, sizeof(real), "%s/test.sock", dir);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	CHECK(!launcherSocketPath(addr.sun_path, sizeof(addr.sun_path), dir, "/test.sock"));

	dirfd = open(dir, O_PATH | O_DIRECTORY | O_CLOEXEC);
	CHECK(dirfd >= 0);
	CHECK(launcherDirSocketPath(addr.sun_path, sizeof(addr.sun_path), dirfd, "/test.sock"));
	CHECK(strlen(addr.sun_path) < 40);

	server = socket(AF_UNIX, SOCK_STREAM, 0);
	client = socket(AF_UNIX, SOCK_STREAM, 0);
	CHECK(server >= 0 && client >= 0);
	CHECK(bind(server, (struct sockaddr*) &addr, sizeof(addr)) == 0);
	CHECK(listen(server, 1) == 0);
	// The socket is where the long path says.
	CHECK(lstat(real, &st) == 0 && S_ISSOCK(st.st_mode));
	CHECK(connect(client, (struct sockaddr*) &addr, sizeof(addr)) == 0);

	close(client);
	close(server);
	close(dirfd);
	unlink(real);
	// Remove the directories, innermost first.
	while (strlen(dir) > strlen(base))
	{
		rmdir(dir);
		*strrchr(dir, '/') = '\0';
	}
	rmdir(base);

	CHECK(!launcherDirSocketPath(addr.sun_path, sizeof(addr.sun_path), -1, "/test.sock"));
	CHECK(addr.sun_path[0] == '\0');
}

int main(void)
{
	test_max_length();
	test_check();
	test_mode();
	test_private_group();
	test_socket_path();
	test_dir_socket_path();

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("launcher prefix: all checks passed\n");
	return 0;
}
