/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
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
 * kevent64() with EVFILT_MACHPORT, as documented in kevent(2) and the
 * public xnu sources (filt_machportprocess):
 *
 * - without MACH_RCV_MSG, an event reports the name of the port that has a
 *   message in `data` and leaves the message queued; the knote stays active
 *   while the port has messages (level-triggered);
 * - with MACH_RCV_MSG and a buffer in ext[0]/ext[1], the event receives the
 *   message into the buffer, with the receive result in `fflags` and the size
 *   received (with the trailer) in ext[1]; a message that does not fit gives
 *   MACH_RCV_TOO_LARGE with the needed size in ext[1], and stays queued with
 *   MACH_RCV_LARGE;
 * - EV_DISPATCH disables the knote after an event until EV_ENABLE;
 * - a knote on a port set reports (or receives from) the member port that has
 *   a message;
 * - a message is received once: by a kevent call or by another receiver
 *   (mach_msg or another kqueue), never by both and never lost;
 * - a message that arrives while no kevent call processes the knote stays
 *   queued: deleting the knote or closing the kqueue leaves it on the port;
 * - the thread that registered a knote may meanwhile receive other messages
 *   with mach_msg.
 *
 * Each case prints a line; the program prints PASS and exits 0 when all pass.
 * An optional argument names a single case. Runs on macOS and in Darling.
 */
#include <errno.h>
#include <mach/mach.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ROUNDS 2000
#define RACE_MESSAGES 4000

typedef struct {
	mach_msg_header_t header;
	uint32_t seq;
	char pad[200];
} big_msg_t;

typedef struct {
	mach_msg_header_t header;
	uint32_t seq;
} small_msg_t;

typedef struct {
	small_msg_t msg;
	mach_msg_max_trailer_t trailer;
} small_rcv_t;

static int failures;

#define CHECK(cond, ...) do { \
	if (!(cond)) { \
		printf("  FAIL %s:%d: ", __func__, __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		failures++; \
		return -1; \
	} \
} while (0)

static mach_port_t new_port(void)
{
	mach_port_t port = MACH_PORT_NULL;
	kern_return_t kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
	if (kr != KERN_SUCCESS) {
		printf("mach_port_allocate: %d\n", kr);
		exit(1);
	}
	kr = mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
	if (kr != KERN_SUCCESS) {
		printf("mach_port_insert_right: %d\n", kr);
		exit(1);
	}
	return port;
}

static void free_port(mach_port_t port)
{
	mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
	mach_port_deallocate(mach_task_self(), port);
}

static kern_return_t send_seq(mach_port_t port, uint32_t seq, mach_msg_size_t size)
{
	big_msg_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
	msg.header.msgh_size = size;
	msg.header.msgh_remote_port = port;
	msg.header.msgh_id = 0x4b51;
	msg.seq = seq;
	return mach_msg(&msg.header, MACH_SEND_MSG, size, 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
}

/* receives one message with a timeout (ms); returns its seq or -1 */
static int64_t recv_seq(mach_port_t port, mach_msg_timeout_t timeout)
{
	struct {
		big_msg_t msg;
		mach_msg_max_trailer_t trailer;
	} buf;
	kern_return_t kr = mach_msg(&buf.msg.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(buf), port, timeout, MACH_PORT_NULL);
	if (kr != KERN_SUCCESS)
		return -1;
	return buf.msg.seq;
}

static int kq_register(int kq, mach_port_t port, uint16_t flags, uint32_t fflags, void *buffer, uint64_t size)
{
	struct kevent64_s kev;
	EV_SET64(&kev, port, EVFILT_MACHPORT, flags, fflags, 0, 0x1234, (uint64_t)(uintptr_t)buffer, size);
	return kevent64(kq, &kev, 1, NULL, 0, 0, NULL);
}

static int kq_wait(int kq, struct kevent64_s *out, int nout, int timeout_ms)
{
	struct timespec ts = { timeout_ms / 1000, (timeout_ms % 1000) * 1000000L };
	return kevent64(kq, NULL, 0, out, nout, 0, timeout_ms < 0 ? NULL : &ts);
}

static double now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

/* ping-pong: a thread answers each message on a second port */

struct echo_ctx {
	mach_port_t in;
	mach_port_t out;
	int rounds;
};

static void *echo_thread(void *arg)
{
	struct echo_ctx *ctx = arg;
	for (int i = 0; i < ctx->rounds; i++) {
		int64_t seq = recv_seq(ctx->in, 10000);
		if (seq < 0)
			break;
		send_seq(ctx->out, (uint32_t)seq, sizeof(small_msg_t));
	}
	return NULL;
}

static int case_pingpong_receive(void)
{
	mach_port_t a = new_port(), b = new_port();
	int kq = kqueue();
	small_rcv_t buf;
	struct echo_ctx ctx = { a, b, ROUNDS };
	pthread_t t;

	CHECK(kq_register(kq, b, EV_ADD, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "register: %d", errno);
	pthread_create(&t, NULL, echo_thread, &ctx);

	double start = now();
	for (uint32_t i = 0; i < ROUNDS; i++) {
		struct kevent64_s ev;
		memset(&buf, 0xa5, sizeof(buf));
		CHECK(send_seq(a, i, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
		int n = kq_wait(kq, &ev, 1, 10000);
		CHECK(n == 1, "round %u: kevent64 returned %d (errno %d)", i, n, errno);
		CHECK(ev.ident == b && ev.filter == EVFILT_MACHPORT, "round %u: ident %llu filter %d", i, ev.ident, ev.filter);
		CHECK(ev.fflags == MACH_MSG_SUCCESS, "round %u: fflags 0x%x", i, ev.fflags);
		CHECK(ev.data == 0, "round %u: data %lld", i, ev.data);
		CHECK(ev.ext[0] == (uint64_t)(uintptr_t)&buf, "round %u: ext[0] 0x%llx", i, ev.ext[0]);
		/* the size received, with the default trailer */
		CHECK(ev.ext[1] == sizeof(small_msg_t) + sizeof(mach_msg_trailer_t), "round %u: ext[1] %llu", i, ev.ext[1]);
		CHECK(ev.udata == 0x1234, "round %u: udata", i);
		CHECK(buf.msg.seq == i && buf.msg.header.msgh_id == 0x4b51, "round %u: received seq %u id %d", i, buf.msg.seq, buf.msg.header.msgh_id);
		CHECK(buf.msg.header.msgh_local_port == b, "round %u: local port", i);
	}
	double elapsed = now() - start;
	pthread_join(t, NULL);

	/* nothing is left: the next poll returns no event */
	struct kevent64_s ev;
	CHECK(kq_wait(kq, &ev, 1, 0) == 0, "event after the last message");

	close(kq);
	free_port(a);
	free_port(b);
	printf("  %d round trips, %.1f us each\n", ROUNDS, elapsed * 1e6 / ROUNDS);
	return 0;
}

static int case_pingpong_peek(void)
{
	mach_port_t a = new_port(), b = new_port();
	int kq = kqueue();
	struct echo_ctx ctx = { a, b, ROUNDS };
	pthread_t t;

	CHECK(kq_register(kq, b, EV_ADD, 0, NULL, 0) == 0, "register: %d", errno);
	pthread_create(&t, NULL, echo_thread, &ctx);

	double start = now();
	for (uint32_t i = 0; i < ROUNDS; i++) {
		struct kevent64_s ev;
		CHECK(send_seq(a, i, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
		int n = kq_wait(kq, &ev, 1, 10000);
		CHECK(n == 1, "round %u: kevent64 returned %d (errno %d)", i, n, errno);
		CHECK(ev.ident == b && (mach_port_t)ev.data == b, "round %u: ident %llu data %lld", i, ev.ident, ev.data);
		if (i == 0) {
			/* level-triggered: the message is still queued, so the knote is still active */
			n = kq_wait(kq, &ev, 1, 0);
			CHECK(n == 1 && (mach_port_t)ev.data == b, "second look at a queued message returned %d", n);
		}
		int64_t seq = recv_seq(b, 0);
		CHECK(seq == i, "round %u: drained seq %lld", i, seq);
		/* the message is gone: no event for it any more */
		n = kq_wait(kq, &ev, 1, 0);
		CHECK(n == 0, "round %u: stale event after the drain", i);
	}
	double elapsed = now() - start;
	pthread_join(t, NULL);

	close(kq);
	free_port(a);
	free_port(b);
	printf("  %d round trips, %.1f us each\n", ROUNDS, elapsed * 1e6 / ROUNDS);
	return 0;
}

static int case_too_large(void)
{
	mach_port_t p = new_port();
	int kq = kqueue();
	small_rcv_t buf;
	struct kevent64_s ev;

	/* with MACH_RCV_LARGE, the message stays queued and ext[1] gives the size needed */
	CHECK(kq_register(kq, p, EV_ADD | EV_DISPATCH, MACH_RCV_MSG | MACH_RCV_LARGE, &buf, sizeof(buf)) == 0, "register");
	CHECK(send_seq(p, 7, sizeof(big_msg_t)) == KERN_SUCCESS, "send");
	CHECK(kq_wait(kq, &ev, 1, 5000) == 1, "no event");
	CHECK(ev.fflags == MACH_RCV_TOO_LARGE, "fflags 0x%x", ev.fflags);
	CHECK(ev.ext[1] >= sizeof(big_msg_t), "ext[1] %llu", ev.ext[1]);
	CHECK(recv_seq(p, 0) == 7, "the message did not stay queued");

	/* a buffer that fits receives it */
	big_msg_t bigbuf[2];
	struct kevent64_s mod;
	EV_SET64(&mod, p, EVFILT_MACHPORT, EV_ADD | EV_ENABLE | EV_DISPATCH, MACH_RCV_MSG | MACH_RCV_LARGE, 0, 0x1234, (uint64_t)(uintptr_t)bigbuf, sizeof(bigbuf));
	CHECK(kevent64(kq, &mod, 1, NULL, 0, 0, NULL) == 0, "modify");
	CHECK(send_seq(p, 8, sizeof(big_msg_t)) == KERN_SUCCESS, "send");
	CHECK(kq_wait(kq, &ev, 1, 5000) == 1, "no event");
	CHECK(ev.fflags == MACH_MSG_SUCCESS && bigbuf[0].seq == 8, "fflags 0x%x seq %u", ev.fflags, bigbuf[0].seq);

	/* without MACH_RCV_LARGE, the event reports the error and the message is gone */
	close(kq);
	kq = kqueue();
	CHECK(kq_register(kq, p, EV_ADD | EV_DISPATCH, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "register");
	CHECK(send_seq(p, 9, sizeof(big_msg_t)) == KERN_SUCCESS, "send");
	CHECK(kq_wait(kq, &ev, 1, 5000) == 1, "no event");
	CHECK(ev.fflags == MACH_RCV_TOO_LARGE, "fflags 0x%x", ev.fflags);
	CHECK(recv_seq(p, 0) == -1, "a message too large for the event stayed queued");

	close(kq);
	free_port(p);
	return 0;
}

static int case_port_set(void)
{
	enum { MEMBERS = 4 };
	mach_port_t set, members[MEMBERS];
	int kq = kqueue();
	struct kevent64_s ev;

	CHECK(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &set) == KERN_SUCCESS, "set");
	for (int i = 0; i < MEMBERS; i++) {
		members[i] = new_port();
		CHECK(mach_port_insert_member(mach_task_self(), members[i], set) == KERN_SUCCESS, "insert");
	}

	/* peek: data names the member that has a message, which stays queued */
	CHECK(kq_register(kq, set, EV_ADD, 0, NULL, 0) == 0, "register");
	for (int round = 0; round < 50; round++) {
		int which = round % MEMBERS;
		CHECK(send_seq(members[which], round, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
		CHECK(kq_wait(kq, &ev, 1, 5000) == 1, "round %d: no event", round);
		CHECK(ev.ident == set && (mach_port_t)ev.data == members[which], "round %d: data %lld, want %u", round, ev.data, members[which]);
		CHECK(recv_seq(members[which], 0) == round, "round %d: drain", round);
		CHECK(kq_wait(kq, &ev, 1, 0) == 0, "round %d: stale event", round);
	}
	close(kq);

	/* receive: the event takes the messages of all members, each once */
	kq = kqueue();
	small_rcv_t buf;
	CHECK(kq_register(kq, set, EV_ADD, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "register");
	for (int round = 0; round < 20; round++) {
		bool seen[MEMBERS] = { false };
		for (int i = 0; i < MEMBERS; i++)
			CHECK(send_seq(members[i], round * MEMBERS + i, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
		for (int i = 0; i < MEMBERS; i++) {
			CHECK(kq_wait(kq, &ev, 1, 5000) == 1, "round %d: missing event %d", round, i);
			CHECK(ev.fflags == MACH_MSG_SUCCESS, "fflags 0x%x", ev.fflags);
			int idx = buf.msg.seq - round * MEMBERS;
			CHECK(idx >= 0 && idx < MEMBERS && !seen[idx], "round %d: seq %u", round, buf.msg.seq);
			CHECK(buf.msg.header.msgh_local_port == members[idx], "round %d: local port", round);
			seen[idx] = true;
		}
		CHECK(kq_wait(kq, &ev, 1, 0) == 0, "round %d: extra event", round);
	}

	close(kq);
	for (int i = 0; i < MEMBERS; i++)
		free_port(members[i]);
	mach_port_mod_refs(mach_task_self(), set, MACH_PORT_RIGHT_PORT_SET, -1);
	return 0;
}

/* two receivers on one port: a kevent loop (one or two kqueues) and a mach_msg loop */

struct race_ctx {
	mach_port_t port;
	int kq;
	volatile int stop;
	uint8_t *seen;
	int count;
	int dup;
};

static pthread_mutex_t race_lock = PTHREAD_MUTEX_INITIALIZER;

static void race_note(struct race_ctx *ctx, uint32_t seq)
{
	pthread_mutex_lock(&race_lock);
	if (seq >= RACE_MESSAGES || ctx->seen[seq]++)
		ctx->dup++;
	ctx->count++;
	pthread_mutex_unlock(&race_lock);
}

static void *race_kevent(void *arg)
{
	struct race_ctx *ctx = arg;
	small_rcv_t buf;
	int kq = kqueue();
	kq_register(kq, ctx->port, EV_ADD, MACH_RCV_MSG, &buf, sizeof(buf));
	while (!ctx->stop) {
		struct kevent64_s ev;
		if (kq_wait(kq, &ev, 1, 50) == 1 && ev.fflags == MACH_MSG_SUCCESS)
			race_note(ctx, buf.msg.seq);
	}
	close(kq);
	return NULL;
}

static void *race_mach_msg(void *arg)
{
	struct race_ctx *ctx = arg;
	while (!ctx->stop) {
		int64_t seq = recv_seq(ctx->port, 50);
		if (seq >= 0)
			race_note(ctx, (uint32_t)seq);
	}
	return NULL;
}

static int run_race(void *(*second)(void *))
{
	struct race_ctx ctx = { new_port(), -1, 0, calloc(RACE_MESSAGES, 1), 0, 0 };
	pthread_t t1, t2;

	pthread_create(&t1, NULL, race_kevent, &ctx);
	pthread_create(&t2, NULL, second, &ctx);
	usleep(100000);

	for (uint32_t i = 0; i < RACE_MESSAGES; i++) {
		CHECK(send_seq(ctx.port, i, sizeof(small_msg_t)) == KERN_SUCCESS, "send %u", i);
		if ((i & 63) == 0)
			usleep(200);
	}
	for (int i = 0; i < 500 && ctx.count < RACE_MESSAGES; i++)
		usleep(10000);
	ctx.stop = 1;
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	int missing = 0;
	for (int i = 0; i < RACE_MESSAGES; i++)
		missing += ctx.seen[i] == 0;
	free(ctx.seen);
	free_port(ctx.port);
	CHECK(ctx.dup == 0 && missing == 0 && ctx.count == RACE_MESSAGES, "%d received, %d duplicated, %d missing", ctx.count, ctx.dup, missing);
	return 0;
}

static int case_race_mach_msg(void)
{
	return run_race(race_mach_msg);
}

static int case_race_two_kqueues(void)
{
	return run_race(race_kevent);
}

static int case_dispatch(void)
{
	mach_port_t p = new_port();
	int kq = kqueue();
	small_rcv_t buf;
	struct kevent64_s ev;

	CHECK(kq_register(kq, p, EV_ADD | EV_DISPATCH, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "register");
	for (uint32_t round = 0; round < 200; round++) {
		CHECK(send_seq(p, 2 * round, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
		CHECK(send_seq(p, 2 * round + 1, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
		CHECK(kq_wait(kq, &ev, 1, 5000) == 1 && buf.msg.seq == 2 * round, "round %u: first event (seq %u)", round, buf.msg.seq);
		CHECK((ev.flags & EV_DISPATCH) != 0, "round %u: flags 0x%x", round, ev.flags);
		/* disabled: the second message waits */
		CHECK(kq_wait(kq, &ev, 1, round == 0 ? 100 : 0) == 0, "round %u: event while disabled", round);
		CHECK(kq_register(kq, p, EV_ENABLE | EV_DISPATCH, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "enable");
		CHECK(kq_wait(kq, &ev, 1, 5000) == 1 && buf.msg.seq == 2 * round + 1, "round %u: second event (seq %u)", round, buf.msg.seq);
		CHECK(kq_register(kq, p, EV_ENABLE | EV_DISPATCH, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "enable");
	}
	close(kq);
	free_port(p);
	return 0;
}

/* a message that arrives while no kevent call processes the knote stays on the port */

struct waiter_ctx {
	int kq;
	int timeout_ms;
	int result;
};

static void *waiter_thread(void *arg)
{
	struct waiter_ctx *ctx = arg;
	struct kevent64_s ev;
	ctx->result = kq_wait(ctx->kq, &ev, 1, ctx->timeout_ms);
	return NULL;
}

static int case_close_and_delete(void)
{
	mach_port_t p = new_port();
	small_rcv_t buf;

	for (int round = 0; round < 50; round++) {
		/* a kevent call timed out before the message came; closing the kqueue leaves it queued */
		int kq = kqueue();
		struct waiter_ctx ctx = { kq, 20, -2 };
		pthread_t t;
		CHECK(kq_register(kq, p, EV_ADD, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "register");
		pthread_create(&t, NULL, waiter_thread, &ctx);
		pthread_join(t, NULL);
		CHECK(ctx.result == 0, "round %d: the wait returned %d", round, ctx.result);
		CHECK(send_seq(p, round, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
		usleep(1000);
		close(kq);
		CHECK(recv_seq(p, 1000) == round, "round %d: message lost when the kqueue was closed", round);

		/* the same after EV_DELETE */
		kq = kqueue();
		CHECK(kq_register(kq, p, EV_ADD, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "register");
		ctx.kq = kq;
		pthread_create(&t, NULL, waiter_thread, &ctx);
		pthread_join(t, NULL);
		CHECK(send_seq(p, 1000 + round, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
		usleep(1000);
		CHECK(kq_register(kq, p, EV_DELETE, 0, NULL, 0) == 0, "delete");
		CHECK(recv_seq(p, 1000) == 1000 + round, "round %d: message lost when the knote was deleted", round);
		close(kq);
	}

	/* a kqueue closed while another thread waits on it; the message that comes later stays on the port */
	for (int round = 0; round < 20; round++) {
		int kq = kqueue();
		struct waiter_ctx ctx = { kq, 100, -2 };
		pthread_t t;
		CHECK(kq_register(kq, p, EV_ADD, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "register");
		pthread_create(&t, NULL, waiter_thread, &ctx);
		usleep(20000);
		CHECK(kq_register(kq, p, EV_DELETE, 0, NULL, 0) == 0, "delete while waiting");
		CHECK(send_seq(p, 2000 + round, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
		pthread_join(t, NULL);
		close(kq);
		CHECK(recv_seq(p, 1000) == 2000 + round, "round %d: message taken by a deleted knote", round);
	}

	free_port(p);
	return 0;
}

static int case_fork(void)
{
	mach_port_t p = new_port();
	int kq = kqueue();
	small_rcv_t buf;
	struct kevent64_s ev;

	CHECK(kq_register(kq, p, EV_ADD, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "register");
	CHECK(send_seq(p, 1, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
	CHECK(kq_wait(kq, &ev, 1, 5000) == 1 && buf.msg.seq == 1, "before fork");

	for (int i = 0; i < 5; i++) {
		pid_t child = fork();
		if (child == 0)
			_exit(0);
		CHECK(child > 0, "fork");
		int status;
		waitpid(child, &status, 0);
		CHECK(send_seq(p, 10 + i, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
		CHECK(kq_wait(kq, &ev, 1, 5000) == 1 && buf.msg.seq == 10 + i, "after fork %d: seq %u", i, buf.msg.seq);
	}
	CHECK(kq_wait(kq, &ev, 1, 0) == 0, "extra event");
	close(kq);
	free_port(p);
	return 0;
}

static int case_port_destroyed(void)
{
	mach_port_t p = new_port();
	int kq = kqueue();
	small_rcv_t buf;
	struct kevent64_s ev;

	CHECK(kq_register(kq, p, EV_ADD, MACH_RCV_MSG, &buf, sizeof(buf)) == 0, "register");
	CHECK(send_seq(p, 1, sizeof(small_msg_t)) == KERN_SUCCESS, "send");
	CHECK(kq_wait(kq, &ev, 1, 5000) == 1 && buf.msg.seq == 1, "event");
	free_port(p);
	/* no message can come any more; whatever the knote reports, it must not deliver a message */
	int n = kq_wait(kq, &ev, 1, 100);
	CHECK(n == 0 || ev.fflags != MACH_MSG_SUCCESS || (ev.flags & (EV_EOF | EV_VANISHED)), "event for a destroyed port: n %d fflags 0x%x flags 0x%x", n, ev.fflags, ev.flags);
	close(kq);
	return 0;
}

/*
 * the thread that registered a knote receives on another port with mach_msg while a second thread
 * waits for the knote; each gets its own messages, in order
 */

#define BUSY_ROUNDS 500

struct busy_ctx {
	int kq;
	mach_port_t knote_port;
	mach_port_t other_port;
	small_rcv_t buffer;
	int received;
	int bad;
};

static void *busy_waiter(void *arg)
{
	struct busy_ctx *ctx = arg;
	while (ctx->received < BUSY_ROUNDS) {
		struct kevent64_s out;
		int n = kq_wait(ctx->kq, &out, 1, 5000);
		if (n <= 0)
			break;
		if (out.fflags != MACH_MSG_SUCCESS || ctx->buffer.msg.seq != (uint32_t)ctx->received)
			ctx->bad++;
		ctx->received++;
	}
	return NULL;
}

static void *busy_sender(void *arg)
{
	struct busy_ctx *ctx = arg;
	for (int i = 0; i < BUSY_ROUNDS; i++) {
		send_seq(ctx->knote_port, (uint32_t)i, sizeof(small_msg_t));
		send_seq(ctx->other_port, (uint32_t)i, sizeof(small_msg_t));
		usleep(200);
	}
	return NULL;
}

static int case_registering_thread_busy(void)
{
	struct busy_ctx ctx;
	pthread_t waiter, sender;
	int got = 0, in_order = 1;

	memset(&ctx, 0, sizeof(ctx));
	ctx.kq = kqueue();
	ctx.knote_port = new_port();
	ctx.other_port = new_port();
	CHECK(kq_register(ctx.kq, ctx.knote_port, EV_ADD | EV_ENABLE, MACH_RCV_MSG, &ctx.buffer, sizeof(ctx.buffer)) == 0, "register");

	pthread_create(&waiter, NULL, busy_waiter, &ctx);
	usleep(100000);
	pthread_create(&sender, NULL, busy_sender, &ctx);

	for (int i = 0; i < BUSY_ROUNDS; i++) {
		int64_t seq = recv_seq(ctx.other_port, 5000);
		if (seq < 0)
			break;
		if (seq != i)
			in_order = 0;
		got++;
	}

	pthread_join(sender, NULL);
	pthread_join(waiter, NULL);
	close(ctx.kq);
	free_port(ctx.knote_port);
	free_port(ctx.other_port);

	CHECK(got == BUSY_ROUNDS && in_order, "mach_msg received %d of %d messages%s", got, BUSY_ROUNDS, in_order ? "" : ", out of order");
	CHECK(ctx.received == BUSY_ROUNDS && ctx.bad == 0, "kevent received %d of %d messages, %d wrong", ctx.received, BUSY_ROUNDS, ctx.bad);
	return 0;
}

static const struct {
	const char *name;
	int (*fn)(void);
} cases[] = {
	{ "pingpong-receive", case_pingpong_receive },
	{ "pingpong-peek", case_pingpong_peek },
	{ "too-large", case_too_large },
	{ "port-set", case_port_set },
	{ "race-mach-msg", case_race_mach_msg },
	{ "race-two-kqueues", case_race_two_kqueues },
	{ "dispatch", case_dispatch },
	{ "close-and-delete", case_close_and_delete },
	{ "fork", case_fork },
	{ "port-destroyed", case_port_destroyed },
	{ "registering-thread-busy", case_registering_thread_busy },
};

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IOLBF, 0);
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		if (argc > 1 && strcmp(argv[1], cases[i].name) != 0)
			continue;
		int before = failures;
		cases[i].fn();
		printf("%s: %s\n", cases[i].name, failures == before ? "ok" : "FAILED");
	}
	if (failures) {
		printf("FAIL (%d)\n", failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
