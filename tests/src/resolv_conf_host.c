// Copyright (C) 2026 Darling Developers
//
// The guest's /etc/resolv.conf (resolv.conf(5)) is a link to the host's
// file under /Volumes/SystemRoot, so it shows what the host resolver uses
// right now. The host file is often itself a link, either absolute
// (/run/systemd/resolve/stub-resolv.conf) or relative
// (../run/systemd/resolve/stub-resolv.conf); both kinds must be followed
// on the host side once the path has crossed into /Volumes/SystemRoot,
// while absolute links found inside the guest keep resolving in the guest.
//
// The host-side chains are modelled in a scratch directory under the
// host's /tmp, so the real host configuration is never touched.
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SYSTEM_ROOT "/Volumes/SystemRoot"
#define GUEST_RESOLV "/private/etc/resolv.conf"
#define HOST_RESOLV SYSTEM_ROOT "/etc/resolv.conf"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(2);
}

// Reads a whole (small) file; returns its length or -1
static ssize_t read_file(const char* path, char* buf, size_t size)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		printf("open(%s): %s\n", path, strerror(errno));
		return -1;
	}

	size_t total = 0;
	for (;;)
	{
		ssize_t n = read(fd, buf + total, size - 1 - total);
		if (n < 0)
		{
			if (errno == EINTR)
				continue;
			total = (size_t) -1;
			break;
		}
		if (n == 0)
			break;
		total += n;
		if (total == size - 1)
			break;
	}
	close(fd);

	if (total == (size_t) -1)
		return -1;
	buf[total] = '\0';
	return total;
}

static int write_file(const char* path, const char* text)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		return -1;
	size_t len = strlen(text);
	ssize_t n = write(fd, text, len);
	close(fd);
	return n == (ssize_t) len ? 0 : -1;
}

static int same_contents(const char* path, const char* expected)
{
	char buf[8192];
	if (read_file(path, buf, sizeof(buf)) < 0)
		return 0;
	if (strcmp(buf, expected) != 0)
	{
		printf("%s has unexpected contents:\n%s", path, buf);
		return 0;
	}
	return 1;
}

// The installed file is a link to the host's file, and reads through it
// give what the host has
static void check_installed_link(void)
{
	struct stat st;
	CHECK(lstat(GUEST_RESOLV, &st) == 0);
	CHECK(S_ISLNK(st.st_mode));

	char target[PATH_MAX];
	ssize_t len = readlink(GUEST_RESOLV, target, sizeof(target) - 1);
	CHECK(len > 0);
	if (len > 0)
	{
		target[len] = '\0';
		if (strcmp(target, HOST_RESOLV) != 0)
		{
			printf("%s -> %s\n", GUEST_RESOLV, target);
			g_failures++;
		}
	}

	char host[8192];
	if (read_file(HOST_RESOLV, host, sizeof(host)) < 0)
	{
		// A host without /etc/resolv.conf has nothing to compare with
		printf("note: the host has no readable resolv.conf\n");
		return;
	}
	CHECK(same_contents("/etc/resolv.conf", host));
}

static char g_host_dir[PATH_MAX];  // as seen by the guest
static char g_guest_dir[PATH_MAX];

static void path_in(char* out, const char* dir, const char* name)
{
	snprintf(out, PATH_MAX, "%s/%s", dir, name);
}

// Absolute and relative host links behind a guest link that points into
// /Volumes/SystemRoot
static void check_host_chains(void)
{
	char tmpl[] = SYSTEM_ROOT "/tmp/resolv_conf_host.XXXXXX";
	if (mkdtemp(tmpl) == NULL)
	{
		printf("FAIL: mkdtemp(%s): %s\n", tmpl, strerror(errno));
		g_failures++;
		return;
	}
	strcpy(g_host_dir, tmpl);

	// The same directory as the host sees it
	const char* host_view = g_host_dir + strlen(SYSTEM_ROOT);
	const char* base = strrchr(g_host_dir, '/') + 1;

	char data[PATH_MAX], relative[PATH_MAX], absolute[PATH_MAX], dotdot[PATH_MAX];
	char next[PATH_MAX], target[PATH_MAX];
	path_in(data, g_host_dir, "stub-resolv.conf");
	path_in(relative, g_host_dir, "relative");
	path_in(absolute, g_host_dir, "absolute");
	path_in(dotdot, g_host_dir, "dotdot");
	path_in(next, g_host_dir, "next");

	const char* first = "nameserver 192.0.2.53\nsearch example.test\n";
	CHECK(write_file(data, first) == 0);
	CHECK(symlink("stub-resolv.conf", relative) == 0);

	// An absolute link in host terms, to the relative one
	snprintf(target, sizeof(target), "%s/relative", host_view);
	CHECK(symlink(target, absolute) == 0);

	// A relative link that climbs out of the directory and back in
	snprintf(target, sizeof(target), "../%s/stub-resolv.conf", base);
	CHECK(symlink(target, dotdot) == 0);

	char tmpl2[] = "/private/tmp/resolv_conf_host.XXXXXX";
	if (mkdtemp(tmpl2) == NULL)
	{
		printf("FAIL: mkdtemp(%s): %s\n", tmpl2, strerror(errno));
		g_failures++;
		return;
	}
	strcpy(g_guest_dir, tmpl2);

	char via_absolute[PATH_MAX], via_dotdot[PATH_MAX], local[PATH_MAX], via_local[PATH_MAX];
	path_in(via_absolute, g_guest_dir, "via-absolute");
	path_in(via_dotdot, g_guest_dir, "via-dotdot");
	path_in(local, g_guest_dir, "local");
	path_in(via_local, g_guest_dir, "via-local");

	// Guest links into SystemRoot, like the installed resolv.conf
	CHECK(symlink(absolute, via_absolute) == 0);
	CHECK(symlink(dotdot, via_dotdot) == 0);

	CHECK(same_contents(via_absolute, first));
	CHECK(same_contents(via_dotdot, first));

	// Network managers replace the file by renaming a new one over it; a
	// link sees the new contents
	const char* second = "nameserver 192.0.2.54\n";
	CHECK(write_file(next, second) == 0);
	CHECK(rename(next, data) == 0);
	CHECK(same_contents(via_absolute, second));
	CHECK(same_contents(via_dotdot, second));

	// An absolute link that never leaves the guest resolves in the guest
	const char* guest = "nameserver 127.0.0.1\n";
	CHECK(write_file(local, guest) == 0);
	CHECK(symlink(local, via_local) == 0);
	CHECK(same_contents(via_local, guest));
}

static void cleanup(void)
{
	static const char* const host_names[] = { "stub-resolv.conf", "relative", "absolute", "dotdot", "next", NULL };
	static const char* const guest_names[] = { "via-absolute", "via-dotdot", "local", "via-local", NULL };
	char path[PATH_MAX];

	if (g_host_dir[0])
	{
		for (int i = 0; host_names[i]; i++)
		{
			path_in(path, g_host_dir, host_names[i]);
			unlink(path);
		}
		rmdir(g_host_dir);
	}
	if (g_guest_dir[0])
	{
		for (int i = 0; guest_names[i]; i++)
		{
			path_in(path, g_guest_dir, guest_names[i]);
			unlink(path);
		}
		rmdir(g_guest_dir);
	}
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	check_installed_link();
	check_host_chains();
	cleanup();

	if (g_failures == 0)
		puts("PASS");
	finish(g_failures != 0);
	return 0;
}
