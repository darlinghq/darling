// Host-side test of shellspawn's descriptor helpers. The descriptors a client
// sends can land on 0, 1 or 2 when shellspawn runs with closed standard I/O, and
// they can alias each other; the shell must still get them as its standard
// input, output and error, and cleanup must close each descriptor exactly once.
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "shellspawn-fds.h"

// Exit codes of the forked checks; the children cannot print because they
// replace their own standard I/O.
enum {
	CHILD_OK = 0,
	CHILD_SETUP,
	CHILD_REMAP,
	CHILD_WRONG_FILE,
	CHILD_SOURCES_OPEN,
	CHILD_STDIO_CLOSED,
};

static ino_t inode_of(int fd)
{
	struct stat st;

	if (fstat(fd, &st) != 0)
		return 0;
	return st.st_ino;
}

static int is_open(int fd)
{
	return fcntl(fd, F_GETFD) != -1;
}

// Runs check() in a child with 0, 1 and 2 closed and returns its exit code.
static int run_without_stdio(int (*check)(void))
{
	fflush(stdout);

	pid_t pid = fork();
	if (pid == 0)
	{
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		_exit(check());
	}

	int status;
	if (pid == -1 || waitpid(pid, &status, 0) != pid || !WIFEXITED(status))
		return -1;
	return WEXITSTATUS(status);
}

// The received descriptors are exactly 0, 1 and 2, in a rotated order.
static int check_rotated(void)
{
	int null_fd, sock_fd, p[2];

	// With 0..2 closed, these land on 0, 1 and 2 (and 3).
	null_fd = open("/dev/null", O_RDWR);
	sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (null_fd != 0 || sock_fd != 1 || pipe(p) != 0 || p[0] != 2)
		return CHILD_SETUP;

	ino_t ino_null = inode_of(null_fd), ino_sock = inode_of(sock_fd), ino_pipe = inode_of(p[0]);
	int streams[3] = { p[0], null_fd, sock_fd }; // { 2, 0, 1 }

	if (shellspawn_remap_stdio(streams) != 0)
		return CHILD_REMAP;
	if (inode_of(STDIN_FILENO) != ino_pipe || inode_of(STDOUT_FILENO) != ino_null
			|| inode_of(STDERR_FILENO) != ino_sock)
		return CHILD_WRONG_FILE;

	shellspawn_close_streams(streams, STDERR_FILENO + 1);
	if (!is_open(STDIN_FILENO) || !is_open(STDOUT_FILENO) || !is_open(STDERR_FILENO))
		return CHILD_STDIO_CLOSED;
	if (inode_of(STDIN_FILENO) != ino_pipe)
		return CHILD_STDIO_CLOSED;
	return CHILD_OK;
}

// One received descriptor is used for all three streams and sits above 2,
// another sits on 0 and is not used at all.
static int check_shared(void)
{
	int unused[2], tty[2];

	if (pipe(unused) != 0 || unused[0] != 0)
		return CHILD_SETUP;
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, tty) != 0 || dup2(tty[0], 10) != 10)
		return CHILD_SETUP;

	ino_t ino = inode_of(10);
	int streams[3] = { 10, 10, 10 };

	if (shellspawn_remap_stdio(streams) != 0)
		return CHILD_REMAP;
	for (int i = 0; i < 3; i++)
	{
		if (inode_of(i) != ino)
			return CHILD_WRONG_FILE;
	}

	// The standard descriptors must not be close-on-exec.
	for (int i = 0; i < 3; i++)
	{
		if (fcntl(i, F_GETFD) & FD_CLOEXEC)
			return CHILD_WRONG_FILE;
	}

	shellspawn_close_streams(streams, STDERR_FILENO + 1);
	if (is_open(10))
		return CHILD_SOURCES_OPEN;
	for (int i = 0; i < 3; i++)
	{
		if (!is_open(i) || inode_of(i) != ino)
			return CHILD_STDIO_CLOSED;
	}
	return CHILD_OK;
}

// A control socket that arrived on 0 must be moved above 2.
static int check_move(void)
{
	int sv[2];

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0 || sv[0] != 0)
		return CHILD_SETUP;

	ino_t ino = inode_of(sv[0]);
	int moved = shellspawn_move_above_stdio(sv[0]);

	if (moved <= STDERR_FILENO || inode_of(moved) != ino)
		return CHILD_REMAP;
	if (is_open(0))
		return CHILD_SOURCES_OPEN;
	if (!(fcntl(moved, F_GETFD) & FD_CLOEXEC))
		return CHILD_WRONG_FILE;
	if (shellspawn_move_above_stdio(moved) != moved)
		return CHILD_REMAP;
	return CHILD_OK;
}

// Closing the streams resets them, so a second cleanup does not close a
// descriptor that has been reused in between.
static int test_close_once(void)
{
	int p[2];

	if (pipe(p) != 0)
	{
		printf("FAIL: cannot create a pipe\n");
		return 1;
	}

	int streams[3] = { p[0], p[0], p[1] };
	shellspawn_close_streams(streams, 0);

	if (streams[0] != -1 || streams[1] != -1 || streams[2] != -1)
	{
		printf("FAIL: closed streams were not reset\n");
		return 1;
	}
	if (is_open(p[0]) || is_open(p[1]))
	{
		printf("FAIL: streams were left open\n");
		return 1;
	}

	int reused = dup(STDOUT_FILENO);
	if (reused != p[0])
	{
		printf("FAIL: descriptor %d was not reused (got %d)\n", p[0], reused);
		return 1;
	}

	shellspawn_close_streams(streams, 0);
	if (!is_open(reused))
	{
		printf("FAIL: a second cleanup closed a reused descriptor\n");
		return 1;
	}
	close(reused);
	return 0;
}

int main(void)
{
	static const struct {
		const char* name;
		int (*check)(void);
	} checks[] = {
		{ "rotated standard descriptors", check_rotated },
		{ "one descriptor for all streams", check_shared },
		{ "control socket on 0", check_move },
	};

	for (size_t i = 0; i < sizeof(checks) / sizeof(checks[0]); i++)
	{
		int code = run_without_stdio(checks[i].check);
		if (code != CHILD_OK)
		{
			printf("FAIL: %s (code %d)\n", checks[i].name, code);
			return 1;
		}
	}

	if (test_close_once() != 0)
		return 1;

	printf("shellspawn fds: OK\n");
	return 0;
}
