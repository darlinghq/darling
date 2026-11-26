// Copyright (C) 2026 Lubos Dolezel
//
// Receives on a Mach port set whose member ports Darling keeps in memory shared
// with darlingserver (see mach_sq_common.h). Each case is documented (or XNU
// observed) behavior of mach_msg() on a port set and must hold the same way with
// DARLING_MACH_SERVER=1, which sends everything through the server:
//   - which message a receive on a set returns: the member that became ready
//     first is served until its queue is empty, then the next one; a member that
//     is inserted with messages queued becomes ready at the insertion; within a
//     member the order is the send order;
//   - the received message is the member's: its name in the local port field,
//     its seqno, its context;
//   - timeouts: 0 polls, a finite one ends with MACH_RCV_TIMED_OUT, a message that
//     arrives in time is received;
//   - a member inserted or extracted, or destroyed, while a thread waits on the set;
//     a member can be received from directly;
//   - two receivers on one set receive every message exactly once;
//   - messages only the server delivers (with a reply port, mk_timer expiry) are
//     received in order with the ones a thread of the task sends;
//   - the set gets a kqueue watcher, a member gets a notification request, a port
//     joins two sets, or the set is destroyed: no message is lost or received twice;
//   - MACH_RCV_LARGE and LARGE_IDENTITY on a set;
//   - one thread inserting and extracting a member again and again while senders
//     send to it and a receiver receives from the set: every message exactly once;
//   - in the default mode a send and a receive through a set make no darlingserver
//     call: the process never blocks for the server (no voluntary context switch).
// Each case prints a line; the program prints PASS and exits 0 when all pass.

#include "mach_sq_common.h"

#include <pthread.h>
#include <sched.h>
#include <sys/event.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <mach/mach_time.h>

extern mach_port_name_t mk_timer_create(void);
extern kern_return_t mk_timer_arm(mach_port_name_t name, uint64_t expire_time);
extern kern_return_t mk_timer_destroy(mach_port_name_t name);

#define SEQNO_OPTIONS (MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SEQNO))
#define CTX_OPTIONS (MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_CTX))

static mach_port_t new_set(void)
{
	mach_port_t set;
	SQ_KR(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &set), "allocate set");
	return set;
}

static void add(mach_port_t set, mach_port_t port)
{
	SQ_KR(mach_port_insert_member(mach_task_self(), port, set), "insert_member");
}

static void drop(mach_port_t set, mach_port_t port)
{
	SQ_KR(mach_port_extract_member(mach_task_self(), port, set), "extract_member");
}

static mach_msg_return_t rx(mach_port_t port, sq_buffer_t* b, mach_msg_option_t options, mach_msg_timeout_t timeout)
{
	return sq_receive_ex(port, b, sizeof(*b), MACH_RCV_TIMEOUT | options, timeout);
}

static mach_msg_trailer_size_t seqno_of(const sq_buffer_t* b)
{
	return ((const mach_msg_seqno_trailer_t*) ((const uint8_t*) b + b->header.msgh_size))->msgh_seqno;
}

static uint64_t context_of(const sq_buffer_t* b)
{
	return ((const mach_msg_context_trailer_t*) ((const uint8_t*) b + b->header.msgh_size))->msgh_context;
}

// the ids of the next `n` messages of the set, which must be these (and from these ports, if given)
static void expect_ids(mach_port_t set, const int* ids, int n, const char* what)
{
	for (int i = 0; i < n; i++)
	{
		sq_buffer_t b;
		mach_msg_return_t kr = rx(set, &b, 0, 5000);
		SQ_CHECK(kr == MACH_MSG_SUCCESS, "%s: receive %d: 0x%x", what, i, (unsigned) kr);
		SQ_CHECK(b.header.msgh_id == ids[i], "%s: message %d has id %d, expected %d", what, i, b.header.msgh_id, ids[i]);
	}
}

static void expect_empty(mach_port_t set, const char* what)
{
	sq_buffer_t b;
	mach_msg_return_t kr = rx(set, &b, 0, 0);
	SQ_CHECK(kr == MACH_RCV_TIMED_OUT, "%s: expected nothing, got 0x%x (id %d)", what, (unsigned) kr, b.header.msgh_id);
}

static void send_ids(mach_port_t port, const int* ids, int n)
{
	for (int i = 0; i < n; i++)
		SQ_KR(sq_send(port, ids[i]), "send");
}

// ---- locality ----------------------------------------------------------------------------

static long voluntary_switches(void)
{
	struct rusage r;
	getrusage(RUSAGE_SELF, &r);
	return r.ru_nvcsw;
}

static void test_local(void)
{
	mach_port_t a = sq_port(true), b = sq_port(true), set = new_set();
	sq_buffer_t buffer;

	add(set, a);
	add(set, b);
	if (!sq_server_only())
		SQ_CHECK(sq_region_mapped() == 1, "no shared region");

	long before = voluntary_switches();
	for (int i = 0; i < 2000; i++)
	{
		SQ_KR(sq_send((i & 1) ? a : b, i), "send");
		SQ_KR(rx(set, &buffer, 0, 1000), "receive");
		SQ_CHECK(buffer.header.msgh_id == i, "id %d", buffer.header.msgh_id);
	}
	long switches = voluntary_switches() - before;
	// every call that goes to the server waits for its reply: 4000 calls, a switch for most of them
	if (sq_server_only())
		SQ_CHECK(switches > 1000, "the kill switch: only %ld voluntary switches", switches);
	else
		SQ_CHECK(switches < 200, "a send and a receive through a set blocked %ld times: they went to the server", switches);

	sq_destroy(set);
	sq_destroy(a);
	sq_destroy(b);
	printf("ok local (%ld voluntary switches for 2000 send+receive pairs)\n", switches);
}

// ---- which message ---------------------------------------------------------------------------

static void test_order(void)
{
	mach_port_t a = sq_port(true), b = sq_port(true), c = sq_port(true), set = new_set();
	sq_set_qlimit(a, 50);
	sq_set_qlimit(b, 50);
	sq_set_qlimit(c, 50);
	add(set, a);
	add(set, b);
	add(set, c);

	// b becomes ready first, then a, then c; a keeps being served until it is empty
	SQ_KR(sq_send(b, 11), "send");
	SQ_KR(sq_send(a, 21), "send");
	SQ_KR(sq_send(a, 22), "send");
	SQ_KR(sq_send(c, 31), "send");
	SQ_KR(sq_send(a, 23), "send");
	expect_ids(set, (int[]) { 11, 21, 22, 23, 31 }, 5, "ready order");
	expect_empty(set, "ready order");

	// not first come, first served: a, b, a again is a, a, b
	SQ_KR(sq_send(a, 41), "send");
	SQ_KR(sq_send(b, 51), "send");
	SQ_KR(sq_send(a, 42), "send");
	expect_ids(set, (int[]) { 41, 42, 51 }, 3, "run");

	// a member that emptied and gets a message again is ready again, behind the one that is ready
	SQ_KR(sq_send(a, 61), "send");
	SQ_KR(sq_send(b, 71), "send");
	expect_ids(set, (int[]) { 61 }, 1, "re-ready 1");
	SQ_KR(sq_send(a, 62), "send");
	expect_ids(set, (int[]) { 71, 62 }, 2, "re-ready 2");
	expect_empty(set, "re-ready");

	// members inserted with messages queued become ready at the insertion, in the order of the insertions
	drop(set, a);
	drop(set, b);
	drop(set, c);
	SQ_KR(sq_send(a, 81), "send");
	SQ_KR(sq_send(b, 91), "send");
	add(set, b);
	add(set, a);
	expect_ids(set, (int[]) { 91, 81 }, 2, "insertion order");

	// ... and behind a member that was ready already
	drop(set, a);
	drop(set, b);
	SQ_KR(sq_send(b, 101), "send");
	add(set, a);
	SQ_KR(sq_send(a, 111), "send");
	add(set, b);
	expect_ids(set, (int[]) { 111, 101 }, 2, "insertion behind a ready member");
	expect_empty(set, "insertion");

	sq_destroy(set);
	sq_destroy(a);
	sq_destroy(b);
	sq_destroy(c);
	printf("ok order\n");
}

// ---- the received message ----------------------------------------------------------------------

static void test_attributes(void)
{
	mach_port_t a = sq_port(true), b = sq_port(true), set = new_set();
	sq_buffer_t buffer;

	SQ_KR(mach_port_set_context(mach_task_self(), a, 0xaaaa1111), "context a");
	SQ_KR(mach_port_set_context(mach_task_self(), b, 0xbbbb2222), "context b");
	add(set, a);
	add(set, b);
	SQ_KR(sq_send(a, 1), "send");
	SQ_KR(sq_send(a, 2), "send");
	SQ_KR(sq_send(b, 3), "send");

	SQ_KR(rx(set, &buffer, SEQNO_OPTIONS, 1000), "receive");
	SQ_CHECK(buffer.header.msgh_id == 1 && buffer.header.msgh_local_port == a, "first: id %d port %u", buffer.header.msgh_id, buffer.header.msgh_local_port);
	SQ_CHECK(seqno_of(&buffer) == 0, "seqno %u", seqno_of(&buffer));
	SQ_CHECK(buffer.header.msgh_remote_port == MACH_PORT_NULL, "remote port");
	SQ_KR(rx(set, &buffer, SEQNO_OPTIONS, 1000), "receive");
	SQ_CHECK(buffer.header.msgh_id == 2 && seqno_of(&buffer) == 1, "second: id %d seqno %u", buffer.header.msgh_id, seqno_of(&buffer));
	SQ_KR(rx(set, &buffer, SEQNO_OPTIONS, 1000), "receive");
	SQ_CHECK(buffer.header.msgh_id == 3 && buffer.header.msgh_local_port == b, "third: id %d port %u", buffer.header.msgh_id, buffer.header.msgh_local_port);
	SQ_CHECK(seqno_of(&buffer) == 0, "the other member counts its own seqnos: %u", seqno_of(&buffer));

	SQ_KR(sq_send(a, 4), "send");
	SQ_KR(sq_send(b, 5), "send");
	SQ_KR(rx(set, &buffer, CTX_OPTIONS, 1000), "receive");
	SQ_CHECK(buffer.header.msgh_local_port == a && context_of(&buffer) == 0xaaaa1111, "context of a: 0x%llx", (unsigned long long) context_of(&buffer));
	SQ_KR(rx(set, &buffer, CTX_OPTIONS, 1000), "receive");
	SQ_CHECK(buffer.header.msgh_local_port == b && context_of(&buffer) == 0xbbbb2222, "context of b: 0x%llx", (unsigned long long) context_of(&buffer));

	// the member's status counts what is queued, and the set lists its members
	SQ_KR(sq_send(b, 6), "send");
	mach_port_status_t status;
	mach_msg_type_number_t count = MACH_PORT_RECEIVE_STATUS_COUNT;
	SQ_KR(mach_port_get_attributes(mach_task_self(), b, MACH_PORT_RECEIVE_STATUS, (mach_port_info_t) &status, &count), "status");
	SQ_CHECK(status.mps_msgcount == 1 && status.mps_seqno == 2 && status.mps_pset != 0, "status: %u messages, seqno %u, pset %u",
		status.mps_msgcount, status.mps_seqno, status.mps_pset);
	mach_port_name_array_t members;
	mach_msg_type_number_t member_count;
	SQ_KR(mach_port_get_set_status(mach_task_self(), set, &members, &member_count), "get_set_status");
	SQ_CHECK(member_count == 2, "members: %u", member_count);
	SQ_KR(rx(set, &buffer, 0, 1000), "receive");
	SQ_CHECK(buffer.header.msgh_id == 6, "last");

	sq_destroy(set);
	sq_destroy(a);
	sq_destroy(b);
	printf("ok attributes\n");
}

// ---- timeouts ------------------------------------------------------------------------------------

static mach_port_t late_port;
static void* late_sender(void* arg)
{
	usleep((useconds_t) (uintptr_t) arg);
	SQ_KR(sq_send(late_port, 77), "late send");
	return NULL;
}

static void test_timeouts(void)
{
	mach_port_t a = sq_port(true), set = new_set();
	sq_buffer_t buffer;

	// no member at all, and an empty member
	SQ_CHECK(rx(set, &buffer, 0, 0) == MACH_RCV_TIMED_OUT, "poll of an empty set");
	add(set, a);
	SQ_CHECK(rx(set, &buffer, 0, 0) == MACH_RCV_TIMED_OUT, "poll");

	double start = sq_now();
	SQ_CHECK(rx(set, &buffer, 0, 150) == MACH_RCV_TIMED_OUT, "timeout");
	double took = sq_now() - start;
	SQ_CHECK(took > 0.12 && took < 1.0, "a 150 ms timeout took %.3f s", took);

	// a message that arrives in time
	late_port = a;
	pthread_t thread;
	pthread_create(&thread, NULL, late_sender, (void*) (uintptr_t) 60000);
	start = sq_now();
	SQ_KR(rx(set, &buffer, 0, 3000), "receive in time");
	took = sq_now() - start;
	SQ_CHECK(buffer.header.msgh_id == 77 && took > 0.04 && took < 2.0, "late message: id %d after %.3f s", buffer.header.msgh_id, took);
	pthread_join(thread, NULL);

	sq_destroy(set);
	sq_destroy(a);
	printf("ok timeouts\n");
}

// ---- membership changes while a thread waits ---------------------------------------------------------

struct waiter
{
	mach_port_t set;
	mach_msg_timeout_t timeout;
	volatile mach_msg_return_t result;
	sq_buffer_t buffer;
};

static void* waiter_thread(void* arg)
{
	struct waiter* w = arg;
	w->result = rx(w->set, &w->buffer, 0, w->timeout);
	return NULL;
}

static pthread_t start_waiter(struct waiter* w, mach_port_t set, mach_msg_timeout_t timeout)
{
	pthread_t thread;
	w->set = set;
	w->timeout = timeout;
	w->result = (mach_msg_return_t) -1;
	pthread_create(&thread, NULL, waiter_thread, w);
	usleep(80000);
	return thread;
}

static void test_membership_while_waiting(void)
{
	mach_port_t p = sq_port(true), q = sq_port(true), set = new_set();
	struct waiter w;
	sq_buffer_t buffer;
	pthread_t thread;

	// a member that has a message joins while the set is empty and a thread waits on it
	thread = start_waiter(&w, set, 5000);
	SQ_KR(sq_send(p, 1), "send");
	SQ_CHECK(w.result == (mach_msg_return_t) -1, "received from a port that is not a member");
	add(set, p);
	pthread_join(thread, NULL);
	SQ_CHECK(w.result == MACH_MSG_SUCCESS && w.buffer.header.msgh_id == 1 && w.buffer.header.msgh_local_port == p, "joined member: 0x%x", (unsigned) w.result);

	// an empty member joins, and then gets a message
	thread = start_waiter(&w, set, 5000);
	add(set, q);
	usleep(30000);
	SQ_CHECK(w.result == (mach_msg_return_t) -1, "woke without a message");
	SQ_KR(sq_send(q, 2), "send");
	pthread_join(thread, NULL);
	SQ_CHECK(w.result == MACH_MSG_SUCCESS && w.buffer.header.msgh_id == 2 && w.buffer.header.msgh_local_port == q, "second: 0x%x", (unsigned) w.result);

	// a member leaves while a thread waits: its messages are not the set's any more
	thread = start_waiter(&w, set, 500);
	drop(set, q);
	SQ_KR(sq_send(q, 3), "send");
	pthread_join(thread, NULL);
	SQ_CHECK(w.result == MACH_RCV_TIMED_OUT, "received %d from an extracted member: 0x%x", w.buffer.header.msgh_id, (unsigned) w.result);
	// ... and a receive on the port itself gets it
	SQ_KR(rx(q, &buffer, 0, 1000), "receive on the extracted port");
	SQ_CHECK(buffer.header.msgh_id == 3, "extracted port: id %d", buffer.header.msgh_id);

	// a member can be received from directly: the set does not have the message afterwards
	SQ_KR(sq_send(p, 8), "send");
	SQ_KR(rx(p, &buffer, 0, 1000), "receive on a member");
	SQ_CHECK(buffer.header.msgh_id == 8, "receive on a member: id %d", buffer.header.msgh_id);
	expect_empty(set, "after a receive on a member");

	// and joins again
	SQ_KR(sq_send(q, 4), "send");
	add(set, q);
	SQ_KR(rx(set, &buffer, 0, 1000), "receive after rejoin");
	SQ_CHECK(buffer.header.msgh_id == 4, "rejoin: id %d", buffer.header.msgh_id);

	sq_destroy(set);
	sq_destroy(p);
	sq_destroy(q);
	printf("ok membership while waiting\n");
}

// ---- members and sets destroyed --------------------------------------------------------------------

static void test_destroyed(void)
{
	mach_port_t p = sq_port(true), q = sq_port(true), set = new_set();
	struct waiter w;
	sq_buffer_t buffer;
	pthread_t thread;

	add(set, p);
	add(set, q);
	SQ_KR(sq_send(p, 1), "send");
	SQ_KR(sq_send(p, 2), "send");
	SQ_KR(sq_send(q, 3), "send");
	// destroyed with messages queued: they are gone, and the set goes on with the other member
	sq_destroy(p);
	SQ_KR(rx(set, &buffer, 0, 1000), "receive");
	SQ_CHECK(buffer.header.msgh_id == 3, "after the destruction of a member: id %d", buffer.header.msgh_id);
	expect_empty(set, "destroyed member");

	// destroyed while a thread waits: it goes on waiting for the other members
	thread = start_waiter(&w, set, 600);
	sq_destroy(q);
	pthread_join(thread, NULL);
	SQ_CHECK(w.result == MACH_RCV_TIMED_OUT, "after both members were destroyed: 0x%x", (unsigned) w.result);

	// a member that is gone and a new port that gets the slot, in a set that goes on
	mach_port_t r = sq_port(true);
	add(set, r);
	SQ_KR(sq_send(r, 4), "send");
	SQ_KR(rx(set, &buffer, 0, 1000), "receive");
	SQ_CHECK(buffer.header.msgh_id == 4, "new member");

	// the set is destroyed while a thread waits: what the server path gives (MACH_RCV_PORT_CHANGED in XNU)
	thread = start_waiter(&w, set, 5000);
	mach_port_destroy(mach_task_self(), set);
	pthread_join(thread, NULL);
	SQ_CHECK(w.result == MACH_RCV_PORT_CHANGED, "receiver of a destroyed set: 0x%x", (unsigned) w.result);

	// its members are plain ports again
	SQ_KR(sq_send(r, 5), "send");
	SQ_KR(rx(r, &buffer, 0, 1000), "receive on the former member");
	SQ_CHECK(buffer.header.msgh_id == 5, "former member");
	sq_destroy(r);
	printf("ok destroyed\n");
}

// ---- two receivers ------------------------------------------------------------------------------------

#define TWO_PORTS 3
#define TWO_COUNT 3000

static mach_port_t two_ports[TWO_PORTS];
static mach_port_t two_set;
static volatile int two_done;
static int two_seen[TWO_PORTS][TWO_COUNT];
static volatile int two_total;

static void* two_receiver(void* arg)
{
	(void) arg;
	sq_buffer_t b;
	while (!two_done)
	{
		mach_msg_return_t kr = rx(two_set, &b, 0, 100);
		if (kr == MACH_RCV_TIMED_OUT)
			continue;
		SQ_CHECK(kr == MACH_MSG_SUCCESS, "two receivers: 0x%x", (unsigned) kr);
		int port = b.header.msgh_id / 100000, seq = b.header.msgh_id % 100000;
		SQ_CHECK(port < TWO_PORTS && seq < TWO_COUNT, "id %d", b.header.msgh_id);
		SQ_CHECK(b.header.msgh_local_port == two_ports[port], "wrong member name for id %d", b.header.msgh_id);
		__sync_fetch_and_add(&two_seen[port][seq], 1);
		__sync_fetch_and_add(&two_total, 1);
	}
	return NULL;
}

static void* two_sender(void* arg)
{
	int port = (int) (uintptr_t) arg;
	for (int i = 0; i < TWO_COUNT; i++)
		SQ_KR(sq_send_ex(two_ports[port], MACH_MSG_TYPE_COPY_SEND, sizeof(mach_msg_header_t) + 8, port * 100000 + i, 0, 0), "send");
	return NULL;
}

static void test_two_receivers(void)
{
	pthread_t receivers[2], senders[TWO_PORTS];

	two_set = new_set();
	for (int i = 0; i < TWO_PORTS; i++)
	{
		two_ports[i] = sq_port(true);
		add(two_set, two_ports[i]);
	}
	for (int i = 0; i < 2; i++)
		pthread_create(&receivers[i], NULL, two_receiver, NULL);
	for (int i = 0; i < TWO_PORTS; i++)
		pthread_create(&senders[i], NULL, two_sender, (void*) (uintptr_t) i);
	for (int i = 0; i < TWO_PORTS; i++)
		pthread_join(senders[i], NULL);

	double start = sq_now();
	while (two_total < TWO_PORTS * TWO_COUNT && sq_now() - start < 20)
		usleep(1000);
	two_done = 1;
	for (int i = 0; i < 2; i++)
		pthread_join(receivers[i], NULL);

	SQ_CHECK(two_total == TWO_PORTS * TWO_COUNT, "received %d of %d", two_total, TWO_PORTS * TWO_COUNT);
	for (int p = 0; p < TWO_PORTS; p++)
		for (int i = 0; i < TWO_COUNT; i++)
			SQ_CHECK(two_seen[p][i] == 1, "message %d of port %d received %d times", i, p, two_seen[p][i]);

	sq_destroy(two_set);
	for (int i = 0; i < TWO_PORTS; i++)
		sq_destroy(two_ports[i]);
	printf("ok two receivers\n");
}

// ---- messages only the server delivers -------------------------------------------------------------------

static mach_msg_return_t send_with_reply(mach_port_t dest, mach_port_t reply, mach_msg_id_t id)
{
	mach_msg_header_t m;
	memset(&m, 0, sizeof(m));
	m.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	m.msgh_size = sizeof(m);
	m.msgh_remote_port = dest;
	m.msgh_local_port = reply;
	m.msgh_id = id;
	return mach_msg(&m, MACH_SEND_MSG, sizeof(m), 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
}

static void test_server_messages(void)
{
	mach_port_t a = sq_port(true), b = sq_port(true), reply = sq_port(false), set = new_set();
	struct waiter w;
	sq_buffer_t buffer;
	pthread_t thread;

	sq_set_qlimit(a, 20);
	add(set, a);
	add(set, b);

	// a message the server queues arrives while a thread waits on the set
	thread = start_waiter(&w, set, 5000);
	SQ_KR(send_with_reply(a, reply, 5), "send with reply port");
	pthread_join(thread, NULL);
	SQ_CHECK(w.result == MACH_MSG_SUCCESS && w.buffer.header.msgh_id == 5 && w.buffer.header.msgh_local_port == a, "reply-port message: 0x%x", (unsigned) w.result);
	SQ_CHECK(w.buffer.header.msgh_remote_port != MACH_PORT_NULL, "no reply port in the message");
	mach_port_deallocate(mach_task_self(), w.buffer.header.msgh_remote_port);

	// and in the order of a queue it shares with messages of the task
	SQ_KR(sq_send(a, 1), "send");
	SQ_KR(send_with_reply(a, reply, 2), "send with reply port");
	SQ_KR(sq_send(a, 3), "send");
	SQ_KR(send_with_reply(a, reply, 4), "send with reply port");
	for (int i = 1; i <= 4; i++)
	{
		SQ_KR(rx(set, &buffer, SEQNO_OPTIONS, 1000), "receive");
		// the message of the first part took seqno 0
		SQ_CHECK(buffer.header.msgh_id == i && seqno_of(&buffer) == (mach_msg_trailer_size_t) i,
			"mixed: id %d seqno %u", buffer.header.msgh_id, seqno_of(&buffer));
		if (buffer.header.msgh_remote_port != MACH_PORT_NULL)
			mach_port_deallocate(mach_task_self(), buffer.header.msgh_remote_port);
	}
	expect_empty(set, "mixed");

	// a timer is a member: its expiry message is a message of the kernel
	mach_port_t timer = mk_timer_create();
	SQ_CHECK(timer != MACH_PORT_NULL, "mk_timer_create");
	add(set, timer);
	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);
	thread = start_waiter(&w, set, 5000);
	SQ_KR(mk_timer_arm(timer, mach_absolute_time() + 100000000ull * tb.denom / tb.numer), "mk_timer_arm");
	pthread_join(thread, NULL);
	SQ_CHECK(w.result == MACH_MSG_SUCCESS && w.buffer.header.msgh_local_port == timer, "timer message: 0x%x port %u", (unsigned) w.result, w.buffer.header.msgh_local_port);
	SQ_CHECK(w.buffer.header.msgh_id == 0, "timer message id %d", w.buffer.header.msgh_id);

	// the timer again, together with a port that gets messages at the same time
	for (int i = 0; i < 20; i++)
	{
		SQ_KR(mk_timer_arm(timer, mach_absolute_time() + 1000000ull * tb.denom / tb.numer), "mk_timer_arm");
		SQ_KR(sq_send(b, 100 + i), "send");
		int seen_timer = 0, seen_port = 0;
		while (seen_timer + seen_port < 2)
		{
			SQ_KR(rx(set, &buffer, 0, 3000), "receive");
			if (buffer.header.msgh_local_port == timer)
				seen_timer++;
			else
			{
				SQ_CHECK(buffer.header.msgh_local_port == b && buffer.header.msgh_id == 100 + i, "round %d: id %d", i, buffer.header.msgh_id);
				seen_port++;
			}
		}
		SQ_CHECK(seen_timer == 1 && seen_port == 1, "round %d", i);
	}

	mk_timer_destroy(timer);
	sq_destroy(set);
	sq_destroy(a);
	sq_destroy(b);
	sq_destroy(reply);
	printf("ok server messages\n");
}

// ---- the set (or a member) leaves ---------------------------------------------------------------------------

static void test_leaving(void)
{
	mach_port_t a = sq_port(true), b = sq_port(true), c = sq_port(true), set = new_set();
	sq_buffer_t buffer;

	sq_set_qlimit(a, 50);
	sq_set_qlimit(b, 50);
	sq_set_qlimit(c, 50);
	add(set, a);
	add(set, b);
	add(set, c);

	// a kqueue watcher on the set: the queues move to the ports, and every message is received once
	for (int i = 0; i < 6; i++)
		SQ_KR(sq_send(i % 2 ? a : b, 10 + i), "send");
	SQ_KR(sq_send(c, 20), "send");
	int kq = kqueue();
	SQ_CHECK(kq >= 0, "kqueue");
	struct kevent ev;
	EV_SET(&ev, set, EVFILT_MACHPORT, EV_ADD | EV_CLEAR, 0, 0, NULL);
	SQ_CHECK(kevent(kq, &ev, 1, NULL, 0, NULL) == 0, "kevent add");
	struct timespec zero = { 1, 0 };
	SQ_CHECK(kevent(kq, NULL, 0, &ev, 1, &zero) == 1, "the watcher saw no message");
	int seen[40] = { 0 };
	int total = 0, last_a = 0, last_b = 0;
	for (;;)
	{
		mach_msg_return_t kr = rx(set, &buffer, 0, 200);
		if (kr == MACH_RCV_TIMED_OUT)
			break;
		SQ_KR(kr, "receive after the watcher");
		int id = buffer.header.msgh_id;
		SQ_CHECK(id >= 10 && id < 40 && seen[id] == 0, "id %d received twice", id);
		seen[id] = 1;
		total++;
		if (buffer.header.msgh_local_port == a)
		{
			SQ_CHECK(id > last_a, "a out of order");
			last_a = id;
		}
		if (buffer.header.msgh_local_port == b)
		{
			SQ_CHECK(id > last_b, "b out of order");
			last_b = id;
		}
	}
	SQ_CHECK(total == 7, "%d of 7 messages after the watcher", total);
	close(kq);

	// a member gets a no-senders request while others have messages
	mach_port_t d = sq_port(true), set2 = new_set();
	mach_port_t previous;
	sq_set_qlimit(d, 50);
	add(set2, d);
	for (int i = 0; i < 5; i++)
		SQ_KR(sq_send(d, 100 + i), "send");
	mach_port_t notify = sq_port(false);
	SQ_KR(mach_port_request_notification(mach_task_self(), d, MACH_NOTIFY_NO_SENDERS, 0, notify, MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous), "request_notification");
	for (int i = 0; i < 5; i++)
	{
		SQ_KR(rx(set2, &buffer, 0, 1000), "receive after the notification request");
		SQ_CHECK(buffer.header.msgh_id == 100 + i, "after the request: id %d, expected %d", buffer.header.msgh_id, 100 + i);
	}
	expect_empty(set2, "after the request");

	// a port that is not in the mode (a notification request) joins a set: the messages of the others stay
	mach_port_t e = sq_port(true), f = sq_port(true), set3 = new_set();
	sq_set_qlimit(e, 50);
	sq_set_qlimit(f, 50);
	add(set3, e);
	for (int i = 0; i < 5; i++)
		SQ_KR(sq_send(e, 200 + i), "send");
	SQ_KR(mach_port_request_notification(mach_task_self(), f, MACH_NOTIFY_NO_SENDERS, 0, notify, MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous), "request_notification");
	SQ_KR(sq_send(f, 300), "send");
	add(set3, f);
	SQ_KR(sq_send(f, 301), "send");
	SQ_KR(sq_send(e, 205), "send");
	int expected_total = 8, got = 0, last_e = 0;
	while (got < expected_total)
	{
		SQ_KR(rx(set3, &buffer, 0, 2000), "receive from a mixed set");
		int id = buffer.header.msgh_id;
		if (id >= 200 && id < 300)
		{
			SQ_CHECK(buffer.header.msgh_local_port == e && id > last_e, "e: id %d", id);
			last_e = id;
		}
		else
			SQ_CHECK(buffer.header.msgh_local_port == f && id >= 300, "f: id %d", id);
		got++;
	}
	expect_empty(set3, "mixed set");

	// the set leaves the mode while a thread waits on it: the thread still gets the next message
	{
		mach_port_t x = sq_port(true), xset = new_set();
		struct waiter w;
		pthread_t thread;
		add(xset, x);
		thread = start_waiter(&w, xset, 5000);
		int kq2 = kqueue();
		struct kevent ev2;
		EV_SET(&ev2, xset, EVFILT_MACHPORT, EV_ADD | EV_CLEAR, 0, 0, NULL);
		SQ_CHECK(kevent(kq2, &ev2, 1, NULL, 0, NULL) == 0, "kevent add");
		usleep(30000);
		SQ_KR(sq_send(x, 500), "send");
		pthread_join(thread, NULL);
		SQ_CHECK(w.result == MACH_MSG_SUCCESS && w.buffer.header.msgh_id == 500, "waiter after the watcher: 0x%x", (unsigned) w.result);
		close(kq2);

		// ... and when a member gets a notification request
		mach_port_t y = sq_port(true), yset = new_set();
		add(yset, y);
		thread = start_waiter(&w, yset, 5000);
		SQ_KR(mach_port_request_notification(mach_task_self(), y, MACH_NOTIFY_NO_SENDERS, 0, notify, MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous), "request_notification");
		usleep(30000);
		SQ_KR(sq_send(y, 501), "send");
		pthread_join(thread, NULL);
		SQ_CHECK(w.result == MACH_MSG_SUCCESS && w.buffer.header.msgh_id == 501, "waiter after the request: 0x%x", (unsigned) w.result);
		sq_destroy(x);
		sq_destroy(xset);
		sq_destroy(y);
		sq_destroy(yset);
	}

	// a port in two sets
	mach_port_t g = sq_port(true), set4 = new_set(), set5 = new_set();
	add(set4, g);
	SQ_KR(sq_send(g, 400), "send");
	kern_return_t kr = mach_port_insert_member(mach_task_self(), g, set5);
	SQ_CHECK(kr == KERN_SUCCESS, "second set: 0x%x", (unsigned) kr);
	SQ_KR(sq_send(g, 401), "send");
	SQ_KR(rx(set4, &buffer, 0, 1000), "receive");
	SQ_CHECK(buffer.header.msgh_id == 400, "first set: id %d", buffer.header.msgh_id);
	SQ_KR(rx(set5, &buffer, 0, 1000), "receive");
	SQ_CHECK(buffer.header.msgh_id == 401, "second set: id %d", buffer.header.msgh_id);

	sq_destroy(set);
	sq_destroy(set2);
	sq_destroy(set3);
	sq_destroy(set4);
	sq_destroy(set5);
	sq_destroy(a);
	sq_destroy(b);
	sq_destroy(c);
	sq_destroy(d);
	sq_destroy(e);
	sq_destroy(f);
	sq_destroy(g);
	sq_destroy(notify);
	printf("ok leaving\n");
}

// ---- large messages ----------------------------------------------------------------------------------------

static void test_large(void)
{
	mach_port_t a = sq_port(true), b = sq_port(true), set = new_set();
	sq_buffer_t buffer;

	add(set, a);
	add(set, b);
	SQ_KR(sq_send_ex(b, MACH_MSG_TYPE_COPY_SEND, sizeof(mach_msg_header_t) + 100, 1, 0, 0), "send big");
	SQ_KR(sq_send_ex(a, MACH_MSG_TYPE_COPY_SEND, sizeof(mach_msg_header_t) + 8, 2, 0, 0), "send small");

	// too large with MACH_RCV_LARGE: the message stays, and the buffer says how large it is and (identity) whose it is
	memset(&buffer, 0, sizeof(buffer));
	mach_msg_return_t kr = sq_receive_ex(set, &buffer, sizeof(mach_msg_header_t) + 16, MACH_RCV_TIMEOUT | MACH_RCV_LARGE | MACH_RCV_LARGE_IDENTITY, 1000);
	SQ_CHECK(kr == MACH_RCV_TOO_LARGE, "too large: 0x%x", (unsigned) kr);
	SQ_CHECK(buffer.header.msgh_size == sizeof(mach_msg_header_t) + 100 && buffer.header.msgh_local_port == b,
		"too large: size %u, port %u", buffer.header.msgh_size, buffer.header.msgh_local_port);
	SQ_KR(rx(set, &buffer, 0, 1000), "receive it");
	SQ_CHECK(buffer.header.msgh_id == 1 && buffer.header.msgh_size == sizeof(mach_msg_header_t) + 100, "big: id %d", buffer.header.msgh_id);

	// without MACH_RCV_LARGE the message is lost and the next one is received
	SQ_KR(rx(set, &buffer, 0, 1000), "small one first");
	SQ_CHECK(buffer.header.msgh_id == 2, "the small message of a that was queued first: id %d", buffer.header.msgh_id);
	SQ_KR(sq_send_ex(a, MACH_MSG_TYPE_COPY_SEND, sizeof(mach_msg_header_t) + 100, 3, 0, 0), "send big");
	SQ_KR(sq_send_ex(a, MACH_MSG_TYPE_COPY_SEND, sizeof(mach_msg_header_t) + 8, 4, 0, 0), "send small");
	kr = sq_receive_ex(set, &buffer, sizeof(mach_msg_header_t) + 16, MACH_RCV_TIMEOUT, 1000);
	SQ_CHECK(kr == MACH_RCV_TOO_LARGE, "too large without LARGE: 0x%x", (unsigned) kr);
	SQ_KR(rx(set, &buffer, 0, 1000), "next");
	SQ_CHECK(buffer.header.msgh_id == 4, "next: id %d", buffer.header.msgh_id);
	expect_empty(set, "large");

	sq_destroy(set);
	sq_destroy(a);
	sq_destroy(b);
	printf("ok large\n");
}

// ---- membership churn under traffic -----------------------------------------------------------------------

#define CHURN_PORTS 3
#define CHURN_COUNT 4000

static mach_port_t churn_ports[CHURN_PORTS];
static mach_port_t churn_set;
static volatile int churn_stop;
static int churn_seen[CHURN_PORTS][CHURN_COUNT];
static int churn_last_by_set[CHURN_PORTS] = { -1, -1, -1 };
static volatile int churn_total;

static void churn_record(sq_buffer_t* b, bool by_set)
{
	int port = b->header.msgh_id / 100000, seq = b->header.msgh_id % 100000;
	SQ_CHECK(port < CHURN_PORTS && seq < CHURN_COUNT, "id %d", b->header.msgh_id);
	SQ_CHECK(b->header.msgh_local_port == churn_ports[port], "wrong member name for id %d", b->header.msgh_id);
	SQ_CHECK(__sync_fetch_and_add(&churn_seen[port][seq], 1) == 0, "message %d of port %d received twice", seq, port);
	if (by_set && port != CHURN_PORTS - 1)
	{
		SQ_CHECK(seq > churn_last_by_set[port], "port %d: %d after %d", port, seq, churn_last_by_set[port]);
		churn_last_by_set[port] = seq;
	}
	__sync_fetch_and_add(&churn_total, 1);
}

static void* churn_receiver(void* arg)
{
	(void) arg;
	sq_buffer_t b;
	while (!churn_stop)
	{
		mach_msg_return_t kr = rx(churn_set, &b, 0, 50);
		if (kr == MACH_RCV_TIMED_OUT)
			continue;
		SQ_CHECK(kr == MACH_MSG_SUCCESS, "churn receiver: 0x%x", (unsigned) kr);
		churn_record(&b, true);
	}
	return NULL;
}

static void* churn_sender(void* arg)
{
	int port = (int) (uintptr_t) arg;
	for (int i = 0; i < CHURN_COUNT; i++)
		SQ_KR(sq_send_ex(churn_ports[port], MACH_MSG_TYPE_COPY_SEND, sizeof(mach_msg_header_t) + 8, port * 100000 + i, 0, 0), "send");
	return NULL;
}

static void* churner(void* arg)
{
	(void) arg;
	sq_buffer_t b;
	mach_port_t port = churn_ports[CHURN_PORTS - 1];
	unsigned round = 0;
	while (!churn_stop)
	{
		// out of the set: this thread receives from the port itself, until it is empty
		SQ_KR(mach_port_extract_member(mach_task_self(), port, churn_set), "extract_member");
		for (;;)
		{
			mach_msg_return_t kr = rx(port, &b, 0, 0);
			if (kr == MACH_RCV_TIMED_OUT)
				break;
			SQ_CHECK(kr == MACH_MSG_SUCCESS, "churner: 0x%x", (unsigned) kr);
			churn_record(&b, false);
		}
		if (round++ % 4 == 0)
			sched_yield();
		SQ_KR(mach_port_insert_member(mach_task_self(), port, churn_set), "insert_member");
		usleep(50 + (round % 7) * 40);
	}
	return NULL;
}

static void test_churn(void)
{
	pthread_t receiver, senders[CHURN_PORTS], churn;

	churn_set = new_set();
	for (int i = 0; i < CHURN_PORTS; i++)
	{
		churn_ports[i] = sq_port(true);
		add(churn_set, churn_ports[i]);
	}
	pthread_create(&receiver, NULL, churn_receiver, NULL);
	pthread_create(&churn, NULL, churner, NULL);
	for (int i = 0; i < CHURN_PORTS; i++)
		pthread_create(&senders[i], NULL, churn_sender, (void*) (uintptr_t) i);
	for (int i = 0; i < CHURN_PORTS; i++)
		pthread_join(senders[i], NULL);

	double start = sq_now();
	while (churn_total < CHURN_PORTS * CHURN_COUNT && sq_now() - start < 30)
		usleep(1000);
	churn_stop = 1;
	pthread_join(receiver, NULL);
	pthread_join(churn, NULL);

	SQ_CHECK(churn_total == CHURN_PORTS * CHURN_COUNT, "received %d of %d", churn_total, CHURN_PORTS * CHURN_COUNT);
	for (int p = 0; p < CHURN_PORTS; p++)
		for (int i = 0; i < CHURN_COUNT; i++)
			SQ_CHECK(churn_seen[p][i] == 1, "message %d of port %d received %d times", i, p, churn_seen[p][i]);

	sq_destroy(churn_set);
	for (int i = 0; i < CHURN_PORTS; i++)
		sq_destroy(churn_ports[i]);
	printf("ok churn\n");
}

int main(void)
{
	sq_watchdog(240);
	setvbuf(stdout, NULL, _IOLBF, 0);

	test_local();
	test_order();
	test_attributes();
	test_timeouts();
	test_membership_while_waiting();
	test_destroyed();
	test_two_receivers();
	test_server_messages();
	test_leaving();
	test_large();
	test_churn();

	printf("PASS\n");
	return 0;
}
