// Logging through _simple_asl_send() (and _simple_asl_log(), which uses it) is
// best effort: it must return when nobody drains the log socket, dropping the
// message once the socket's queue is full, instead of blocking the process.
// It still delivers messages while the queue has room.
//
// Part 1: a datagram socket that is never read fills up; a send flagged
// MSG_DONTWAIT then fails with EAGAIN (or ENOBUFS) and does not block.
// Part 2: the log socket is /var/run/syslog. The test binds an unread socket
// there itself, so the queue fills for certain, and _simple_asl_log must keep
// returning. When the test cannot own that path (a live reader such as
// launchd or syslogd holds it, or the path is not writable) part 2 would
// prove nothing, because a reader that drains the socket hides the hang; it
// is then skipped, reported as SKIP, and the exit status is 77 instead of 0.
// A socket file left behind by a killed run is removed only after a connect()
// to it fails with ECONNREFUSED, which means nothing listens on it.
// Part 2 is also skipped when ASL is disabled with ASL_DISABLE=1.
// Builds for x86_64 and i386 and runs on macOS.
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// Private libplatform interface (<_simple.h>), declared here to keep the
// test independent of the private headers.
extern void _simple_asl_log(int level, const char *facility, const char *message);

#define LOG_PATH "/var/run/syslog"
#define MARKER "simple_asl_send_full_queue"
// Far more than any datagram queue limit (Linux defaults to 10 messages).
#define MANY 400

// Exit status of a run in which part 1 passed and part 2 did not run
// (the automake convention for a skipped test).
#define EXIT_SKIPPED 77

static int failed;
static int skipped;
// Set while this process owns a socket bound at LOG_PATH, so that the alarm
// handler can remove it.
static volatile sig_atomic_t owns_log_path;

#define FAIL(...) do { printf("FAIL: " __VA_ARGS__); failed = 1; } while (0)

static void on_alarm(int sig)
{
	(void)sig;
	static const char msg[] = "FAIL: a log send blocked (no progress for 20 s)\n";
	write(1, msg, sizeof(msg) - 1);
	if (owns_log_path)
		unlink(LOG_PATH);
	_exit(1);
}

static void fill_addr(struct sockaddr_un *sun, const char *path)
{
	memset(sun, 0, sizeof(*sun));
	sun->sun_family = AF_UNIX;
	strcpy(sun->sun_path, path);
}

// Binds an unread datagram socket at path. Returns -1 with errno set.
static int bind_sink(const char *path)
{
	struct sockaddr_un sun;
	fill_addr(&sun, path);
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0)
		return -1;
	if (bind(fd, (struct sockaddr *)&sun, sizeof(sun)) != 0) {
		int e = errno;
		close(fd);
		errno = e;
		return -1;
	}
	return fd;
}

// True when a connect() to path is refused: the file is a socket nobody
// listens on. Any other outcome (connected, or another error) is false.
static int is_stale_socket(const char *path)
{
	struct sockaddr_un sun;
	fill_addr(&sun, path);
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0)
		return 0;
	int stale = connect(fd, (struct sockaddr *)&sun, sizeof(sun)) != 0 && errno == ECONNREFUSED;
	close(fd);
	return stale;
}

static void part1_nonblocking_send_contract(void)
{
	char dir[] = "/tmp/asl-fullq.XXXXXX";
	char path[200];
	if (!mkdtemp(dir)) {
		perror("mkdtemp");
		failed = 1;
		return;
	}
	snprintf(path, sizeof(path), "%s/sink", dir);

	int sink = bind_sink(path);
	int cli = socket(AF_UNIX, SOCK_DGRAM, 0);
	struct sockaddr_un sun;
	fill_addr(&sun, path);
	if (sink < 0 || cli < 0 || connect(cli, (struct sockaddr *)&sun, sizeof(sun)) != 0) {
		FAIL("part 1: cannot set up the sockets: %s\n", strerror(errno));
	} else {
		int sent = 0, err = 0;
		for (int i = 0; i < MANY; i++) {
			if (send(cli, "x\n", 2, MSG_DONTWAIT) < 0) {
				err = errno;
				break;
			}
			sent++;
		}
		if (err == 0)
			FAIL("part 1: %d datagrams into an unread socket never failed\n", MANY);
		else if (err != EAGAIN && err != ENOBUFS)
			FAIL("part 1: full queue gave errno %d (%s), expected EAGAIN or ENOBUFS\n", err, strerror(err));
		else if (sent == 0)
			FAIL("part 1: not even one datagram was accepted\n");
	}
	if (cli >= 0)
		close(cli);
	if (sink >= 0)
		close(sink);
	unlink(path);
	rmdir(dir);
}

static void part2_log_with_unread_socket(void)
{
	const char *disable = getenv("ASL_DISABLE");
	if (disable && !strcmp(disable, "1")) {
		printf("SKIP: part 2 (ASL_DISABLE=1)\n");
		skipped = 1;
		return;
	}

	// _simple_asl connects to LOG_PATH once per process, on the first log
	// call. Nothing may log through it before the sink below is bound, or
	// that connection is made to whatever else listens there (or to nothing).
	int sink = bind_sink(LOG_PATH);
	if (sink < 0 && errno == EADDRINUSE && is_stale_socket(LOG_PATH)) {
		unlink(LOG_PATH);
		sink = bind_sink(LOG_PATH);
	}
	if (sink < 0) {
		printf("SKIP: part 2 (cannot bind the syslog path: %s)\n", strerror(errno));
		skipped = 1;
		return;
	}
	owns_log_path = 1;

	for (int i = 0; i < MANY; i++)
		_simple_asl_log(5, MARKER, MARKER " message");
	// Reaching this line is the check: the sends did not block.

	int received = 0;
	char buf[1024];
	for (;;) {
		ssize_t n = recv(sink, buf, sizeof(buf) - 1, MSG_DONTWAIT);
		if (n < 0)
			break;
		buf[n] = 0;
		if (strstr(buf, MARKER " message"))
			received++;
	}
	if (received == 0)
		FAIL("part 2: no message was delivered to the log socket\n");
	else if (received >= MANY)
		FAIL("part 2: all %d messages were queued; the queue never filled\n", MANY);
	close(sink);
	unlink(LOG_PATH);
	owns_log_path = 0;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(20);

	part1_nonblocking_send_contract();
	part2_log_with_unread_socket();

	if (failed)
		return 1;
	if (skipped) {
		printf("SKIPPED: part 1 passed, part 2 did not run\n");
		return EXIT_SKIPPED;
	}
	printf("PASS\n");
	return 0;
}
