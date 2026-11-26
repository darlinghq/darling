// Host-side test of the conditional host-file links made by the launcher
// (src/startup/launcher-hostlink.c).
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "launcher-hostlink.h"

static int g_failures = 0;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s (errno %d)\n", __LINE__, #cond, errno); g_failures++; } \
	} while (0)

static void makeDir(const char* path)
{
	if (mkdir(path, 0755) != 0 && errno != EEXIST)
	{
		perror(path);
		exit(1);
	}
}

static void join(char* out, size_t size, const char* a, const char* b)
{
	if (snprintf(out, size, "%s/%s", a, b) >= (int) size)
	{
		fprintf(stderr, "path too long\n");
		exit(1);
	}
}

static bool pointsTo(const char* path, const char* hostPath)
{
	char target[PATH_MAX], current[PATH_MAX];
	ssize_t length;

	snprintf(target, sizeof(target), "/Volumes/SystemRoot%s", hostPath);
	length = readlink(path, current, sizeof(current) - 1);
	if (length < 0 || (size_t) length >= sizeof(current))
		return false;
	current[length] = '\0';
	return strcmp(current, target) == 0;
}

static int makeSocket(const char* path)
{
	struct sockaddr_un addr;
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (fd < 0 || strlen(path) >= sizeof(addr.sun_path))
		return -1;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path);
	if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) != 0)
	{
		close(fd);
		return -1;
	}
	return fd;
}

int main(void)
{
	char temp[] = "/tmp/darling-hostlink-XXXXXX";
	char prefix[PATH_MAX], hostDir[PATH_MAX], socketPath[PATH_MAX];
	char configPath[PATH_MAX], guestSocket[PATH_MAX], guestConfig[PATH_MAX];
	char secondarySocket[PATH_MAX], command[PATH_MAX];
	struct stat st;
	int socketFd, configFd;
	FILE* file;

	if (mkdtemp(temp) == NULL)
	{
		perror("mkdtemp");
		return 1;
	}
	join(prefix, sizeof(prefix), temp, "prefix");
	join(hostDir, sizeof(hostDir), temp, "host");
	makeDir(prefix);
	makeDir(hostDir);
	{
		char dir[PATH_MAX];
		const char* dirs[] = { "private", "private/etc", "private/var", "private/var/run", "var", "var/run" };

		for (size_t i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++)
		{
			join(dir, sizeof(dir), prefix, dirs[i]);
			makeDir(dir);
		}
	}
	join(socketPath, sizeof(socketPath), hostDir, "kcm.sock");
	join(configPath, sizeof(configPath), hostDir, "krb5.conf");
	join(guestSocket, sizeof(guestSocket), prefix, "private/var/run/cupsd");
	join(guestConfig, sizeof(guestConfig), prefix, "private/etc/krb5.conf");
	join(secondarySocket, sizeof(secondarySocket), prefix, "var/run/.heim_org.h5l.kcm-socket");

	// Neither host file exists: the guest paths stay exactly as they were.
	CHECK(!launcherLinkHostSocket(prefix, socketPath, "/private/var/run/cupsd"));
	CHECK(lstat(guestSocket, &st) != 0 && errno == ENOENT);
	CHECK(!launcherLinkHostFile(prefix, configPath, "/private/etc/krb5.conf"));
	CHECK(lstat(guestConfig, &st) != 0 && errno == ENOENT);

	// Existing files are linked through /Volumes/SystemRoot, not copied.
	socketFd = makeSocket(socketPath);
	CHECK(socketFd >= 0);
	configFd = open(configPath, O_WRONLY | O_CREAT | O_EXCL, 0644);
	CHECK(configFd >= 0);
	if (configFd >= 0)
	{
		write(configFd, "[libdefaults]\n", 14);
		close(configFd);
	}
	CHECK(launcherLinkHostSocket(prefix, socketPath, "/private/var/run/cupsd"));
	CHECK(pointsTo(guestSocket, socketPath));
	CHECK(launcherLinkHostSocket(prefix, socketPath, "/var/run/.heim_org.h5l.kcm-socket"));
	CHECK(pointsTo(secondarySocket, socketPath));
	CHECK(launcherLinkHostFile(prefix, configPath, "/private/etc/krb5.conf"));
	CHECK(pointsTo(guestConfig, configPath));

	// $KRB5_CONFIG is preferred over the default host path.
	CHECK(launcherLinkHostKrb5Config(prefix, configPath));
	CHECK(pointsTo(guestConfig, configPath));

	// When the host files disappear, only our links are removed.
	close(socketFd);
	unlink(socketPath);
	CHECK(!launcherLinkHostSocket(prefix, socketPath, "/private/var/run/cupsd"));
	CHECK(lstat(guestSocket, &st) != 0 && errno == ENOENT);
	CHECK(!launcherLinkHostSocket(prefix, socketPath, "/var/run/.heim_org.h5l.kcm-socket"));
	CHECK(lstat(secondarySocket, &st) != 0 && errno == ENOENT);
	unlink(configPath);
	CHECK(!launcherLinkHostFile(prefix, configPath, "/private/etc/krb5.conf"));
	CHECK(lstat(guestConfig, &st) != 0 && errno == ENOENT);

	// A file the user placed at the guest path is never replaced or removed.
	file = fopen(guestConfig, "w");
	CHECK(file != NULL);
	if (file)
	{
		fputs("[libdefaults]\n", file);
		fclose(file);
	}
	CHECK(!launcherLinkHostFile(prefix, configPath, "/private/etc/krb5.conf"));
	CHECK(lstat(guestConfig, &st) == 0 && S_ISREG(st.st_mode));

	snprintf(command, sizeof(command), "rm -rf \"%s\"", temp);
	if (system(command) != 0)
		CHECK(!"cleanup failed");

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("PASS: conditional host links are made, mapped and removed\n");
	return 0;
}
