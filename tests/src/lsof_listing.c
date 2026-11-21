/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * lsof(8) must list a process's open files and its sockets. Opens a loopback
 * TCP listener, then checks that "lsof -F up -i TCP@127.0.0.1:<port>"
 * reports this process (field output: p<pid> and u<uid>), and that
 * "lsof -p <pid>" lists the listener and the process's working directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static int failures;
#define CHECK(name, cond) do { int ok_ = !!(cond); \
	printf("%s: %s\n", ok_ ? "PASS" : "FAIL", name); if (!ok_) failures++; } while (0)

static int run_capture(const char *cmd, char *out, size_t outlen)
{
	FILE *f = popen(cmd, "r");
	size_t n = 0, r;
	if (!f) return -1;
	while (n + 1 < outlen && (r = fread(out + n, 1, outlen - 1 - n, f)) > 0)
		n += r;
	out[n] = 0;
	return pclose(f);
}

int main(void)
{
	struct sockaddr_in sa = { .sin_len = sizeof sa, .sin_family = AF_INET };
	socklen_t len = sizeof sa;
	char cmd[256], out[16384], want[64];
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	CHECK("listen on loopback", fd >= 0 &&
		bind(fd, (struct sockaddr *) &sa, sizeof sa) == 0 &&
		listen(fd, 4) == 0 &&
		getsockname(fd, (struct sockaddr *) &sa, &len) == 0);
	int port = ntohs(sa.sin_port);

	CHECK("lsof -v", run_capture("/usr/sbin/lsof -v 2>&1", out, sizeof out) == 0 &&
		strstr(out, "revision:"));

	snprintf(cmd, sizeof cmd, "/usr/sbin/lsof -F up -i TCP@127.0.0.1:%d 2>&1", port);
	int rc = run_capture(cmd, out, sizeof out);
	printf("--- %s (rc=%d)\n%s---\n", cmd, rc, out);
	snprintf(want, sizeof want, "p%d\n", (int) getpid());
	CHECK("lsof -i lists this process", rc == 0 && strstr(out, want));
	snprintf(want, sizeof want, "u%d\n", (int) getuid());
	CHECK("lsof -i reports the uid", strstr(out, want));

	snprintf(cmd, sizeof cmd, "/usr/sbin/lsof -nP -p %d 2>&1", (int) getpid());
	rc = run_capture(cmd, out, sizeof out);
	printf("--- %s (rc=%d)\n%s---\n", cmd, rc, out);
	CHECK("lsof -p lists the cwd", rc == 0 && strstr(out, " cwd "));
	snprintf(want, sizeof want, "127.0.0.1:%d", port);
	CHECK("lsof -p lists the listener", strstr(out, want) && strstr(out, "LISTEN"));

	close(fd);
	return failures;
}
