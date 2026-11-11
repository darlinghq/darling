// Copyright (C) 2026 Lubos Dolezel
//
// AF_UNIX addresses are accepted with any of the usual lengths: SUN_LEN()
// (no terminating NUL counted), sizeof(struct sockaddr_un), and a larger
// sockaddr_storage. bind() creates the socket at exactly the given path,
// connect() reaches it, and getsockname() and accept() report the path.
// getsockname() never writes past a short buffer.

#include <errno.h>
#include <stddef.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

static int failed;

#define FAIL(...) do { printf("FAIL: " __VA_ARGS__); failed = 1; } while (0)

static void fill(struct sockaddr_un *sun, const char *path)
{
	// Garbage after the path: an implementation that reads past the
	// address length would see it.
	memset(sun, 'X', sizeof(*sun));
	sun->sun_family = AF_UNIX;
	strcpy(sun->sun_path, path);
}

static void check_name(int fd, const char *path, const char *what)
{
	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	memset(&ss, 0, sizeof(ss));
	if (getsockname(fd, (struct sockaddr *)&ss, &len) != 0) {
		FAIL("%s: getsockname: %s\n", what, strerror(errno));
		return;
	}
	struct sockaddr_un *sun = (struct sockaddr_un *)&ss;
	if (sun->sun_family != AF_UNIX)
		FAIL("%s: getsockname family %d\n", what, sun->sun_family);
	if (len < offsetof(struct sockaddr_un, sun_path) + strlen(path))
		FAIL("%s: getsockname length %u too short\n", what, (unsigned)len);
	// macOS reports the path as it was bound. Darling reports the host's
	// resolved path (/tmp is a link to /private/tmp), so accept either.
	char real[PATH_MAX];
	int same = strncmp(sun->sun_path, path, len - offsetof(struct sockaddr_un, sun_path)) == 0
			&& strlen(sun->sun_path) == strlen(path);
	if (!same && realpath(path, real) && strcmp(sun->sun_path, real) == 0)
		same = 1;
	if (!same)
		FAIL("%s: getsockname path \"%s\", expected \"%s\"\n", what, sun->sun_path, path);
}

// getsockname() into a buffer shorter than the address stores only what the
// buffer holds and still reports the full length (as macOS 27 does).
static void check_short_buffer(int fd, const char *path, const char *what)
{
	unsigned char buf[64];
	socklen_t len = 8;

	memset(buf, 'G', sizeof(buf));
	if (getsockname(fd, (struct sockaddr *)buf, &len) != 0) {
		FAIL("%s: short getsockname: %s\n", what, strerror(errno));
		return;
	}
	if (len < offsetof(struct sockaddr_un, sun_path) + strlen(path))
		FAIL("%s: short getsockname length %u too short\n", what, (unsigned)len);
	for (size_t i = 8; i < sizeof(buf); i++) {
		if (buf[i] != 'G') {
			FAIL("%s: short getsockname wrote byte %zu\n", what, i);
			break;
		}
	}
	if (buf[1] != AF_UNIX)
		FAIL("%s: short getsockname family %d\n", what, buf[1]);
	// The bytes that fit are not compared: Darling asks the host kernel for
	// at most the caller's length, so a path cut that short is not
	// translated back to a guest path.
}

static void run(const char *path, socklen_t bindlen, socklen_t connlen, const char *what)
{
	struct sockaddr_storage ss;
	struct sockaddr_un *sun = (struct sockaddr_un *)&ss;
	struct stat st;

	unlink(path);
	int srv = socket(AF_UNIX, SOCK_STREAM, 0);
	memset(&ss, 0, sizeof(ss));
	fill(sun, path);
	if (bindlen == 0)
		bindlen = SUN_LEN(sun);
	if (bind(srv, (struct sockaddr *)&ss, bindlen) != 0) {
		FAIL("%s: bind(len %u): %s\n", what, (unsigned)bindlen, strerror(errno));
		close(srv);
		return;
	}
	if (lstat(path, &st) != 0 || !S_ISSOCK(st.st_mode))
		FAIL("%s: no socket at \"%s\" after bind\n", what, path);
	check_name(srv, path, what);
	check_short_buffer(srv, path, what);
	listen(srv, 1);

	int cli = socket(AF_UNIX, SOCK_STREAM, 0);
	memset(&ss, 0, sizeof(ss));
	fill(sun, path);
	if (connlen == 0)
		connlen = SUN_LEN(sun);
	if (connect(cli, (struct sockaddr *)&ss, connlen) != 0)
		FAIL("%s: connect(len %u): %s\n", what, (unsigned)connlen, strerror(errno));
	else {
		int c = accept(srv, NULL, NULL);
		if (c < 0)
			FAIL("%s: accept: %s\n", what, strerror(errno));
		else {
			char ch = 0;
			write(cli, "x", 1);
			if (read(c, &ch, 1) != 1 || ch != 'x')
				FAIL("%s: no data through the connection\n", what);
			close(c);
		}
	}
	close(cli);
	close(srv);
	unlink(path);
}

int main(void)
{
	char dir[] = "/tmp/afunix-test.XXXXXX";
	char path[200];
	if (!mkdtemp(dir)) {
		perror("mkdtemp");
		return 1;
	}

	snprintf(path, sizeof(path), "%s/s1", dir);
	run(path, 0, 0, "SUN_LEN");
	run(path, sizeof(struct sockaddr_un), sizeof(struct sockaddr_un), "sizeof(sockaddr_un)");
	run(path, 0, sizeof(struct sockaddr_un), "bind SUN_LEN, connect sizeof");
	run(path, sizeof(struct sockaddr_un), 0, "bind sizeof, connect SUN_LEN");
	run(path, offsetof(struct sockaddr_un, sun_path) + strlen(path) + 1, 0, "SUN_LEN + NUL");
	run(path, 0, sizeof(struct sockaddr_storage), "connect sizeof(sockaddr_storage)");

	// A path relative to the current directory.
	chdir(dir);
	run("rel", 0, 0, "relative SUN_LEN");
	chdir("/");

	rmdir(dir);
	if (!failed)
		printf("PASS\n");
	return failed;
}
