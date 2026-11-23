/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * kevent(2): EVFILT_READ reports a descriptor that has data to read, and a
 * filter added with EV_ENABLE reports a descriptor that is already readable
 * at the time it is added. EV_DISPATCH disables the filter after one event
 * is delivered, and EV_ENABLE re-arms it: if data is still waiting, the
 * event must be reported again. dispatch_source_create() with
 * DISPATCH_SOURCE_TYPE_READ has the same contract: once the source is
 * resumed, its handler runs for a descriptor that was readable before the
 * source existed, and keeps running until the descriptor is drained.
 *
 * Datagrams are queued on a unix datagram socket before anything watches
 * it. Variants: a socketpair, a bound socket written by another socket, a
 * receive queue filled until the sender would block, and a descriptor that
 * is also watched by a kqueue in another process. Each variant is checked
 * with plain kevent() (EV_ADD alone, EV_DISPATCH with a manual re-arm
 * after every datagram, and EV_DISABLE then EV_ENABLE) and with a
 * dispatch read source whose handler reads one datagram per call. On the
 * first two variants the registration is also made while another thread is
 * already blocked in kevent() on the same kqueue.
 *
 * Runs on macOS and in Darling; prints PASS and exits 0. Every wait is
 * bounded, so a lost event fails the test with FAIL lines instead of
 * hanging.
 */
#include <dispatch/dispatch.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#define WAIT_SECONDS 5
#define PAIR_DATAGRAMS 5

static int failures;

#define CHECK(cond, ...) do { \
	if (!(cond)) { \
		failures++; \
		printf("FAIL %s:%d: ", __FILE__, __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
	} \
} while (0)

enum variant {
	VARIANT_SOCKETPAIR,
	VARIANT_BOUND,
	VARIANT_FULL_QUEUE,
	VARIANT_SHARED_WATCHER,
	VARIANT_COUNT
};

static const char *variant_name[VARIANT_COUNT] = {
	"socketpair", "bound socket", "full queue", "shared with a watcher process"
};

struct dgram {
	int rx;
	int tx;
	int queued;
	char path[104];
	pid_t watcher;
	int stop_fd;
};

static void make_nonblocking(int fd)
{
	int fl = fcntl(fd, F_GETFL);
	if (fl < 0 || fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0)
		CHECK(0, "fcntl O_NONBLOCK: %s", strerror(errno));
}

static int send_datagram(const struct dgram *d, int n)
{
	char msg[16];
	int len = snprintf(msg, sizeof(msg), "dgram-%d", n);

	if (d->path[0]) {
		struct sockaddr_un sun;
		memset(&sun, 0, sizeof(sun));
		sun.sun_family = AF_UNIX;
		strcpy(sun.sun_path, d->path);
		return (int)sendto(d->tx, msg, len, 0, (struct sockaddr *)&sun, sizeof(sun));
	}
	return (int)send(d->tx, msg, len, 0);
}

/* A watcher process that holds the descriptor in its own kqueue, as a
 * service manager that checked a socket in with the service would. */
static void run_watcher(int rx, int stop_fd)
{
	int kq = kqueue();
	struct kevent ch[2], out;
	struct timespec ts = {WAIT_SECONDS * 2, 0};

	if (kq < 0)
		_exit(1);
	EV_SET(&ch[0], rx, EVFILT_READ, EV_ADD | EV_DISPATCH, 0, 0, NULL);
	EV_SET(&ch[1], stop_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	if (kevent(kq, ch, 2, NULL, 0, NULL) < 0)
		_exit(2);
	/* Wait for the parent to close its end of the stop pipe. */
	for (;;) {
		int n = kevent(kq, NULL, 0, &out, 1, &ts);
		if (n <= 0)
			_exit(3);
		if ((int)out.ident == stop_fd)
			_exit(0);
		/* The datagrams belong to the parent: leave them queued and
		 * keep the event disabled. */
	}
}

static void setup(struct dgram *d, enum variant v)
{
	memset(d, 0, sizeof(*d));
	d->rx = d->tx = d->stop_fd = -1;

	if (v == VARIANT_SOCKETPAIR) {
		int sv[2];
		CHECK(socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) == 0, "socketpair: %s", strerror(errno));
		d->rx = sv[0];
		d->tx = sv[1];
	} else {
		struct sockaddr_un sun;
		snprintf(d->path, sizeof(d->path), "/tmp/kq-ready-fd-%d.sock", (int)getpid());
		unlink(d->path);
		memset(&sun, 0, sizeof(sun));
		sun.sun_family = AF_UNIX;
		strcpy(sun.sun_path, d->path);
		d->rx = socket(AF_UNIX, SOCK_DGRAM, 0);
		d->tx = socket(AF_UNIX, SOCK_DGRAM, 0);
		CHECK(d->rx >= 0 && d->tx >= 0, "socket: %s", strerror(errno));
		CHECK(bind(d->rx, (struct sockaddr *)&sun, sizeof(sun)) == 0, "bind: %s", strerror(errno));
	}
	make_nonblocking(d->rx);
	make_nonblocking(d->tx);

	if (v == VARIANT_SHARED_WATCHER) {
		int stop[2];
		CHECK(pipe(stop) == 0, "pipe: %s", strerror(errno));
		d->watcher = fork();
		if (d->watcher == 0) {
			close(stop[1]);
			run_watcher(d->rx, stop[0]);
		}
		close(stop[0]);
		d->stop_fd = stop[1];
		/* Let the watcher register before data arrives. */
		usleep(300 * 1000);
	}

	if (v == VARIANT_FULL_QUEUE) {
		/* Queue until the sender would block: the receive queue is full. */
		while (send_datagram(d, d->queued) >= 0)
			d->queued++;
		CHECK(errno == EAGAIN || errno == ENOBUFS, "fill: %s", strerror(errno));
		CHECK(d->queued > 0, "nothing could be queued");
	} else {
		for (int i = 0; i < PAIR_DATAGRAMS; i++) {
			CHECK(send_datagram(d, i) > 0, "send %d: %s", i, strerror(errno));
			d->queued++;
		}
	}
}

static void teardown(struct dgram *d)
{
	if (d->watcher > 0) {
		int st;
		close(d->stop_fd);
		waitpid(d->watcher, &st, 0);
		CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 0, "watcher process status 0x%x", st);
	}
	close(d->rx);
	close(d->tx);
	if (d->path[0])
		unlink(d->path);
}

/* Reads one queued datagram; returns 1 if there was one. */
static int read_one(int fd)
{
	char buf[64];
	ssize_t n = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
	return n > 0;
}

enum arm_style {
	ARM_ADD_ONLY,        /* EV_ADD, level triggered */
	ARM_DISPATCH_REARM,  /* EV_ADD|EV_DISPATCH, EV_ENABLE after each event */
	ARM_DISABLE_ENABLE,  /* EV_ADD|EV_DISABLE, then EV_ENABLE */
	ARM_STYLE_COUNT
};

static const char *arm_name[ARM_STYLE_COUNT] = {
	"EV_ADD", "EV_ADD|EV_DISPATCH with re-arm", "EV_DISABLE then EV_ENABLE"
};

static void check_kevent(const struct dgram *d, enum arm_style style, const char *vname)
{
	int kq = kqueue();
	struct kevent ch, out;
	struct timespec ts = {WAIT_SECONDS, 0};
	int got = 0;

	CHECK(kq >= 0, "%s/%s: kqueue: %s", vname, arm_name[style], strerror(errno));

	switch (style) {
	case ARM_ADD_ONLY:
		EV_SET(&ch, d->rx, EVFILT_READ, EV_ADD, 0, 0, NULL);
		break;
	case ARM_DISPATCH_REARM:
		EV_SET(&ch, d->rx, EVFILT_READ, EV_ADD | EV_ENABLE | EV_DISPATCH, 0, 0, NULL);
		break;
	default:
		EV_SET(&ch, d->rx, EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, NULL);
		break;
	}
	CHECK(kevent(kq, &ch, 1, NULL, 0, NULL) == 0, "%s/%s: add: %s", vname, arm_name[style], strerror(errno));
	if (style == ARM_DISABLE_ENABLE) {
		EV_SET(&ch, d->rx, EVFILT_READ, EV_ENABLE, 0, 0, NULL);
		CHECK(kevent(kq, &ch, 1, NULL, 0, NULL) == 0, "%s/%s: enable: %s", vname, arm_name[style], strerror(errno));
	}

	while (got < d->queued) {
		memset(&out, 0, sizeof(out));
		int n = kevent(kq, NULL, 0, &out, 1, &ts);
		if (n != 1) {
			CHECK(0, "%s/%s: no event with %d of %d datagrams still queued (kevent returned %d)",
					vname, arm_name[style], d->queued - got, d->queued, n);
			break;
		}
		CHECK(out.filter == EVFILT_READ && (int)out.ident == d->rx && out.data > 0,
				"%s/%s: bad event filter %d ident %d data %ld",
				vname, arm_name[style], out.filter, (int)out.ident, (long)out.data);
		if (!read_one(d->rx)) {
			CHECK(0, "%s/%s: event reported but nothing to read", vname, arm_name[style]);
			break;
		}
		got++;
		if (style == ARM_DISPATCH_REARM) {
			EV_SET(&ch, d->rx, EVFILT_READ, EV_ENABLE | EV_DISPATCH, 0, 0, NULL);
			CHECK(kevent(kq, &ch, 1, NULL, 0, NULL) == 0, "%s/%s: re-arm: %s", vname, arm_name[style], strerror(errno));
		}
	}
	CHECK(got == d->queued, "%s/%s: read %d of %d datagrams", vname, arm_name[style], got, d->queued);
	close(kq);
}

/*
 * Another thread is already waiting on the kqueue when the readable
 * descriptor is registered, as when an event loop thread sits in kevent()
 * and a different thread resumes a source. The registration must wake the
 * waiter with the event. Repeated, since a lost event depends on timing.
 */
#define RACE_ROUNDS 200

struct waiter {
	int kq;
	int rx;
	int n;
	struct kevent out;
};

static void *waiter_main(void *arg)
{
	struct waiter *w = arg;
	struct timespec ts = {WAIT_SECONDS, 0};

	memset(&w->out, 0, sizeof(w->out));
	w->n = kevent(w->kq, NULL, 0, &w->out, 1, &ts);
	return NULL;
}

static void check_registration_while_waiting(const struct dgram *d, enum arm_style style, const char *vname)
{
	for (int round = 0; round < RACE_ROUNDS; round++) {
		struct waiter w = { .kq = kqueue(), .rx = d->rx };
		struct kevent ch;
		pthread_t th;

		CHECK(w.kq >= 0, "%s: kqueue: %s", vname, strerror(errno));
		CHECK(send_datagram(d, round) > 0, "%s: send: %s", vname, strerror(errno));
		pthread_create(&th, NULL, waiter_main, &w);
		/* Vary how long the waiter has been blocked: 0 to about 1 ms. */
		usleep((round % 8) * 150);

		EV_SET(&ch, d->rx, EVFILT_READ,
				style == ARM_DISPATCH_REARM ? EV_ADD | EV_ENABLE | EV_DISPATCH
				: style == ARM_ADD_ONLY ? EV_ADD : EV_ADD | EV_DISABLE, 0, 0, NULL);
		CHECK(kevent(w.kq, &ch, 1, NULL, 0, NULL) == 0, "%s: add: %s", vname, strerror(errno));
		if (style == ARM_DISABLE_ENABLE) {
			EV_SET(&ch, d->rx, EVFILT_READ, EV_ENABLE, 0, 0, NULL);
			CHECK(kevent(w.kq, &ch, 1, NULL, 0, NULL) == 0, "%s: enable: %s", vname, strerror(errno));
		}
		pthread_join(th, NULL);

		if (w.n != 1 || w.out.filter != EVFILT_READ || (int)w.out.ident != d->rx) {
			CHECK(0, "%s/%s: round %d: waiter got %d events instead of the readable descriptor",
					vname, arm_name[style], round, w.n);
			close(w.kq);
			return;
		}
		CHECK(read_one(d->rx), "%s/%s: round %d: nothing to read", vname, arm_name[style], round);
		close(w.kq);
	}
}

static void refill(struct dgram *d)
{
	d->queued = 0;
	for (int i = 0; i < PAIR_DATAGRAMS; i++) {
		CHECK(send_datagram(d, i) > 0, "refill %d: %s", i, strerror(errno));
		d->queued++;
	}
}

struct source_state {
	int handled;
	int calls;
	int fd;
	dispatch_semaphore_t done;
	int want;
};

static void check_source(const struct dgram *d, const char *vname)
{
	struct source_state st = {0};
	dispatch_queue_t q = dispatch_queue_create("kqueue-ready-fd", DISPATCH_QUEUE_SERIAL);
	dispatch_source_t src = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)d->rx, 0, q);
	struct source_state *s = &st;

	st.fd = d->rx;
	st.want = d->queued;
	st.done = dispatch_semaphore_create(0);
	CHECK(src != NULL, "%s: dispatch_source_create failed", vname);
	if (!src)
		return;

	dispatch_source_set_event_handler(src, ^{
		s->calls++;
		if (read_one(s->fd) && ++s->handled == s->want)
			dispatch_semaphore_signal(s->done);
	});
	dispatch_resume(src);

	long timed_out = dispatch_semaphore_wait(st.done,
			dispatch_time(DISPATCH_TIME_NOW, (int64_t)WAIT_SECONDS * NSEC_PER_SEC));

	/* The handler state is owned by the serial queue: stop it before reading. */
	dispatch_source_cancel(src);
	dispatch_sync(q, ^{});
	CHECK(!timed_out, "%s/dispatch source: handler read %d of %d datagrams after %d s (%d handler calls)",
			vname, st.handled, st.want, WAIT_SECONDS, st.calls);
	dispatch_release(src);
	dispatch_release(q);
}

int main(void)
{
	signal(SIGPIPE, SIG_IGN);

	for (int v = 0; v < VARIANT_COUNT; v++) {
		struct dgram d;
		const char *vname = variant_name[v];

		setup(&d, (enum variant)v);
		for (int s = 0; s < ARM_STYLE_COUNT; s++) {
			check_kevent(&d, (enum arm_style)s, vname);
			/* A full queue is drained by now; the others need refilling. */
			refill(&d);
		}
		check_source(&d, vname);
		if (v == VARIANT_SOCKETPAIR || v == VARIANT_BOUND) {
			for (int s = 0; s < ARM_STYLE_COUNT; s++)
				check_registration_while_waiting(&d, (enum arm_style)s, vname);
		}
		teardown(&d);
	}

	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
