/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * POSIX asynchronous I/O (aio_read, aio_write, aio_fsync, aio_error, aio_return, aio_suspend, aio_cancel,
 * lio_listio). Expected results are those of the aio(4)/<aio.h> contract plus black-box observation of macOS 26.7.3
 * (error numbers of invalid requests, the per-process limit of 16 unreaped requests; kern.aioprocmax is 16 on Mac
 * OS X 10.14.4 too).
 * Before the implementation every call failed with ENOSYS.
 */
#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

static volatile int usr1_count;
static void on_usr1(int sig) { (void) sig; usr1_count++; }

static struct aiocb mk(int fd, void* buf, size_t n, off_t off)
{
	struct aiocb cb;
	memset(&cb, 0, sizeof cb);
	cb.aio_fildes = fd;
	cb.aio_buf = buf;
	cb.aio_nbytes = n;
	cb.aio_offset = off;
	return cb;
}

// aio_error: the request's error, or -1 with errno EINVAL for a request that is not known
static int aerr(struct aiocb* cb)
{
	int r = aio_error(cb);
	return r == -1 ? errno : r;
}

static void wait_done(struct aiocb* cb)
{
	const struct aiocb* list[1] = { cb };
	int r;
	// a signal (SIGEV_SIGNAL below) may interrupt the wait
	while ((r = aio_suspend(list, 1, NULL)) == -1 && errno == EINTR)
		;
	CHECK(r == 0);
}

int main(void)
{
	char path[] = "/tmp/libsystem_kernel_aio.XXXXXX";
	int fd = mkstemp(path);
	CHECK(fd >= 0);
	unlink(path);

	// aio_write, aio_suspend, aio_error, aio_return
	char out[8] = "abcdefgh";
	struct aiocb w = mk(fd, out, 8, 0);
	CHECK(aio_write(&w) == 0);
	wait_done(&w);
	CHECK(aio_error(&w) == 0);
	CHECK(aio_return(&w) == 8);
	// the request is forgotten once reaped
	CHECK(aio_return(&w) == -1 && errno == EINVAL);
	CHECK(aerr(&w) == EINVAL);

	// aio_read of what was written, at an offset
	char in[8] = { 0 };
	struct aiocb r = mk(fd, in, 4, 2);
	CHECK(aio_read(&r) == 0);
	wait_done(&r);
	CHECK(aio_error(&r) == 0);
	CHECK(aio_return(&r) == 4);
	CHECK(memcmp(in, "cdef", 4) == 0);

	// reading past the end completes with 0 bytes
	struct aiocb eof = mk(fd, in, 4, 100);
	CHECK(aio_read(&eof) == 0);
	wait_done(&eof);
	CHECK(aio_error(&eof) == 0);
	CHECK(aio_return(&eof) == 0);

	// invalid requests fail the submission with their own error
	struct aiocb bad = mk(999, in, 4, 0);
	CHECK(aio_read(&bad) == -1 && errno == EBADF);
	bad = mk(fd, NULL, 4, 0);
	CHECK(aio_read(&bad) == -1 && errno == EINVAL);
	bad = mk(fd, in, 4, -1);
	CHECK(aio_read(&bad) == -1 && errno == EINVAL);
	int pp[2];
	CHECK(pipe(pp) == 0);
	bad = mk(pp[0], in, 4, 0);
	CHECK(aio_read(&bad) == -1 && errno == ESPIPE);
	int rfd = open("/dev/null", O_RDONLY);
	bad = mk(rfd, out, 4, 0);
	CHECK(aio_write(&bad) == -1 && errno == EBADF);
	close(rfd);
	close(pp[0]);
	close(pp[1]);
	// a rejected request is not known afterwards
	CHECK(aerr(&bad) == EINVAL);

	// nothing in flight: aio_cancel reports AIO_ALLDONE; aio_suspend with unknown requests is EINVAL
	CHECK(aio_cancel(fd, NULL) == AIO_ALLDONE);
	const struct aiocb* unknown[1] = { &bad };
	struct timespec ts = { 0, 50 * 1000 * 1000 };
	CHECK(aio_suspend(unknown, 1, &ts) == -1 && errno == EINVAL);

	// aio_fsync
	struct aiocb fs = mk(fd, NULL, 0, 0);
	CHECK(aio_fsync(O_SYNC, &fs) == 0);
	wait_done(&fs);
	CHECK(aio_return(&fs) == 0);
	CHECK(aio_fsync(7, &fs) == -1 && errno == EINVAL);

	// at most 16 requests not yet reaped, then EAGAIN; reaping makes room
	struct aiocb many[17];
	int accepted = 0;
	for (int i = 0; i < 17; i++)
	{
		many[i] = mk(fd, in, 1, 0);
		if (aio_read(&many[i]) == 0)
			accepted++;
		else
		{
			CHECK(errno == EAGAIN);
			break;
		}
	}
	CHECK(accepted == 16);
	for (int i = 0; i < accepted; i++)
	{
		wait_done(&many[i]);
		CHECK(aio_return(&many[i]) == 1);
	}
	struct aiocb after = mk(fd, in, 1, 0);
	CHECK(aio_read(&after) == 0);
	wait_done(&after);
	CHECK(aio_return(&after) == 1);

	// lio_listio with LIO_WAIT returns when every request is done; LIO_NOP entries are skipped
	char b1[4] = "WXYZ", b2[4] = { 0 };
	struct aiocb l1 = mk(fd, b1, 4, 0), l2 = mk(fd, b2, 4, 0), l3 = mk(fd, b2, 4, 0);
	l1.aio_lio_opcode = LIO_WRITE;
	l3.aio_lio_opcode = LIO_NOP;
	struct aiocb* wl[2] = { &l1, &l3 };
	CHECK(lio_listio(LIO_WAIT, wl, 2, NULL) == 0);
	CHECK(aio_error(&l1) == 0 && aio_return(&l1) == 4);
	l2.aio_lio_opcode = LIO_READ;
	struct aiocb* rl[1] = { &l2 };
	CHECK(lio_listio(LIO_WAIT, rl, 1, NULL) == 0);
	CHECK(aio_error(&l2) == 0 && aio_return(&l2) == 4);
	CHECK(memcmp(b2, "WXYZ", 4) == 0);

	// an invalid entry fails the call with its error and nothing is submitted
	l1 = mk(fd, b1, 4, 0);
	l2 = mk(999, b2, 4, 0);
	l1.aio_lio_opcode = LIO_WRITE;
	l2.aio_lio_opcode = LIO_READ;
	struct aiocb* list2[2] = { &l1, &l2 };
	CHECK(lio_listio(LIO_WAIT, list2, 2, NULL) == -1 && errno == EBADF);
	CHECK(aerr(&l1) == EINVAL);
	CHECK(lio_listio(LIO_WAIT, list2, 0, NULL) == -1 && errno == EINVAL);
	CHECK(lio_listio(LIO_WAIT, list2, 17, NULL) == -1 && errno == EINVAL);
	CHECK(lio_listio(7, list2, 2, NULL) == -1 && errno == EINVAL);

	// SIGEV_SIGNAL: delivered to the process on completion; lio_listio signals once for the whole list
	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = on_usr1;
	CHECK(sigaction(SIGUSR1, &sa, NULL) == 0);
	struct aiocb sg = mk(fd, in, 1, 0);
	sg.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	sg.aio_sigevent.sigev_signo = SIGUSR1;
	CHECK(aio_read(&sg) == 0);
	wait_done(&sg);
	for (int i = 0; i < 100 && usr1_count < 1; i++)
		usleep(10000);
	CHECK(usr1_count == 1);
	CHECK(aio_return(&sg) == 1);
	struct aiocb m1 = mk(fd, b1, 4, 0), m2 = mk(fd, b2, 4, 0);
	m1.aio_lio_opcode = LIO_READ;
	m2.aio_lio_opcode = LIO_READ;
	struct aiocb* list3[2] = { &m1, &m2 };
	struct sigevent lsev;
	memset(&lsev, 0, sizeof lsev);
	lsev.sigev_notify = SIGEV_SIGNAL;
	lsev.sigev_signo = SIGUSR1;
	CHECK(lio_listio(LIO_NOWAIT, list3, 2, &lsev) == 0);
	wait_done(&m1);
	wait_done(&m2);
	for (int i = 0; i < 100 && usr1_count < 2; i++)
		usleep(10000);
	usleep(100000);
	CHECK(usr1_count == 2);
	CHECK(aio_return(&m1) == 4 && aio_return(&m2) == 4);
	// an invalid signal number is rejected
	struct aiocb sgbad = mk(fd, in, 1, 0);
	sgbad.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	sgbad.aio_sigevent.sigev_signo = 0;
	CHECK(aio_read(&sgbad) == -1 && errno == EINVAL);

	// the child of a fork has no requests of its own
	struct aiocb pend = mk(fd, in, 1, 0);
	CHECK(aio_read(&pend) == 0);
	pid_t child = fork();
	if (child == 0)
		_exit(aerr(&pend) == EINVAL ? 0 : 1);
	int status = 0;
	CHECK(waitpid(child, &status, 0) == child);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	CHECK(aio_return(&pend) == 1);

	close(fd);
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
