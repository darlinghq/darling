// Copyright (C) 2026 Lubos Dolezel
//
// Mach message semantics around ports whose queue Darling may keep in memory
// shared between the task and darlingserver. Each case is the documented (or
// XNU-observed) behavior of mach_msg() and mach_port_*() and must hold the
// same way with DARLING_MACH_SERVER=1:
//   - order: messages from other threads, and messages only the server
//     delivers (with a reply port), arrive in send order with dense seqnos;
//   - a receiver waiting on an empty queue is woken by a send from another
//     thread; MACH_RCV_TIMEOUT ends the wait with MACH_RCV_TIMED_OUT;
//   - full queue: a sender with no timeout waits until the receiver makes
//     room; MACH_SEND_TIMEOUT gives MACH_SEND_TIMED_OUT and returns the
//     destination right to the sender (one more uref for COPY_SEND);
//   - the receive right leaves the task with messages queued (sent in a
//     message to another port and received back under a new name): every
//     message is still there, in order; a thread waiting on the old name gets
//     MACH_RCV_PORT_CHANGED;
//   - the port is destroyed with messages queued: a send-once right a queued
//     message carried yields a send-once notification, and a waiting thread
//     gets MACH_RCV_PORT_CHANGED;
//   - a no-senders request made while queued messages hold the only send
//     rights fires only after the last of them has been received;
//   - inserting the port into a port set keeps its queue: the set receives
//     every message in order with the seqnos the port would have given;
//   - fork: the child has ports of its own that work (also many forks while
//     the parent keeps messaging), and the parent's ports are unaffected;
//   - kill switch: with DARLING_MACH_SERVER=1 the shared region is never
//     mapped; without it, it is mapped once the task has a receive right.

#include "mach_sq_common.h"

#include <pthread.h>
#include <sys/wait.h>

static mach_msg_trailer_size_t seqno_of(const sq_buffer_t* buffer)
{
	const mach_msg_seqno_trailer_t* trailer =
		(const mach_msg_seqno_trailer_t*) ((const uint8_t*) buffer + buffer->header.msgh_size);
	return trailer->msgh_seqno;
}

static mach_msg_return_t receive_seqno(mach_port_t port, sq_buffer_t* buffer, mach_msg_timeout_t timeout)
{
	return sq_receive_ex(port, buffer, sizeof(*buffer), MACH_RCV_TIMEOUT |
		MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SEQNO), timeout);
}

// ---- order ----------------------------------------------------------------------------

#define ORDER_THREADS 4
#define ORDER_COUNT 2000

static mach_port_t order_port;

static void* order_sender(void* arg)
{
	int me = (int) (intptr_t) arg;
	mach_port_t reply = sq_port(false);

	for (int i = 0; i < ORDER_COUNT; i++)
	{
		mach_msg_id_t id = me * 100000 + i;
		if (i % 7 == 3)
		{
			// one that only the server delivers
			mach_msg_header_t header = {
				.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE),
				.msgh_size = sizeof(header),
				.msgh_remote_port = order_port,
				.msgh_local_port = reply,
				.msgh_id = id,
			};
			SQ_KR(mach_msg(&header, MACH_SEND_MSG, sizeof(header), 0, MACH_PORT_NULL, 0, MACH_PORT_NULL), "send reply");
		}
		else
			SQ_KR(sq_send(order_port, id), "send");
	}
	return (void*) (uintptr_t) reply;
}

static void test_order(void)
{
	pthread_t threads[ORDER_THREADS];
	int next[ORDER_THREADS] = { 0 };
	sq_buffer_t buffer;

	order_port = sq_port(true);
	sq_set_qlimit(order_port, MACH_PORT_QLIMIT_LARGE);

	for (int t = 0; t < ORDER_THREADS; t++)
		pthread_create(&threads[t], NULL, order_sender, (void*) (intptr_t) t);

	for (int n = 0; n < ORDER_THREADS * ORDER_COUNT; n++)
	{
		SQ_KR(receive_seqno(order_port, &buffer, 10000), "order receive");
		int sender = buffer.header.msgh_id / 100000;
		int index = buffer.header.msgh_id % 100000;
		SQ_CHECK(sender >= 0 && sender < ORDER_THREADS, "bad id %d", buffer.header.msgh_id);
		SQ_CHECK(index == next[sender], "sender %d: got %d, expected %d", sender, index, next[sender]);
		SQ_CHECK(seqno_of(&buffer) == (mach_msg_trailer_size_t) n, "seqno %u, expected %d", seqno_of(&buffer), n);
		next[sender]++;
		if (MACH_MSGH_BITS_REMOTE(buffer.header.msgh_bits) == MACH_MSG_TYPE_PORT_SEND_ONCE)
			mach_port_deallocate(mach_task_self(), buffer.header.msgh_remote_port);
	}

	for (int t = 0; t < ORDER_THREADS; t++)
	{
		void* reply;
		pthread_join(threads[t], &reply);
		sq_destroy((mach_port_t) (uintptr_t) reply);
	}
	SQ_CHECK(sq_receive(order_port, &buffer, 1) == MACH_RCV_TIMED_OUT, "extra message");
	sq_destroy(order_port);
	printf("ok order\n");
}

// ---- wake and timeouts -------------------------------------------------------------------

static mach_port_t wake_port;
static volatile int wake_state;

static void* wake_receiver(void* arg)
{
	(void) arg;
	sq_buffer_t buffer;
	for (int i = 0; i < 200; i++)
	{
		__atomic_store_n(&wake_state, 2 * i + 1, __ATOMIC_SEQ_CST);
		SQ_KR(sq_receive(wake_port, &buffer, 0), "wake receive");
		SQ_CHECK(buffer.header.msgh_id == i, "wake: id %d, expected %d", buffer.header.msgh_id, i);
	}
	return NULL;
}

static void test_wake_and_timeouts(void)
{
	pthread_t thread;
	sq_buffer_t buffer;

	wake_port = sq_port(true);
	pthread_create(&thread, NULL, wake_receiver, NULL);
	for (int i = 0; i < 200; i++)
	{
		while (__atomic_load_n(&wake_state, __ATOMIC_SEQ_CST) != 2 * i + 1)
			usleep(10);
		if (i % 4 == 0)
			usleep(2000); // let it fall asleep
		SQ_KR(sq_send(wake_port, i), "wake send");
	}
	pthread_join(thread, NULL);

	double start = sq_now();
	SQ_CHECK(sq_receive(wake_port, &buffer, 200) == MACH_RCV_TIMED_OUT, "timeout");
	double elapsed = sq_now() - start;
	SQ_CHECK(elapsed > 0.15 && elapsed < 5, "timeout after %.3f s", elapsed);
	SQ_CHECK(sq_receive_ex(wake_port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT, 0) == MACH_RCV_TIMED_OUT, "poll");
	sq_destroy(wake_port);
	printf("ok wake and timeouts\n");
}

// ---- full queue ------------------------------------------------------------------------------

static mach_port_t full_port;

static void* full_receiver(void* arg)
{
	(void) arg;
	sq_buffer_t buffer;
	usleep(200000);
	for (int i = 0; i < 50; i++)
	{
		SQ_KR(sq_receive(full_port, &buffer, 10000), "full receive");
		SQ_CHECK(buffer.header.msgh_id == i, "full: id %d, expected %d", buffer.header.msgh_id, i);
		if (i % 10 == 0)
			usleep(5000);
	}
	return NULL;
}

static void test_full(void)
{
	pthread_t thread;
	mach_port_urefs_t before, after;

	full_port = sq_port(true);
	sq_set_qlimit(full_port, 2);

	// a sender without a timeout waits for room
	pthread_create(&thread, NULL, full_receiver, NULL);
	double start = sq_now();
	for (int i = 0; i < 50; i++)
		SQ_KR(sq_send(full_port, i), "full send");
	SQ_CHECK(sq_now() - start > 0.15, "the sender did not wait");
	pthread_join(thread, NULL);

	// MACH_SEND_TIMEOUT: the send times out and the destination right comes back to the sender
	SQ_KR(sq_send(full_port, 0), "fill 1");
	SQ_KR(sq_send(full_port, 1), "fill 2");
	mach_port_get_refs(mach_task_self(), full_port, MACH_PORT_RIGHT_SEND, &before);
	SQ_CHECK(sq_send_ex(full_port, MACH_MSG_TYPE_COPY_SEND, 32, 2, MACH_SEND_TIMEOUT, 0) == MACH_SEND_TIMED_OUT,
		"timeout 0");
	start = sq_now();
	SQ_CHECK(sq_send_ex(full_port, MACH_MSG_TYPE_COPY_SEND, 32, 3, MACH_SEND_TIMEOUT, 100) == MACH_SEND_TIMED_OUT,
		"timeout 100");
	SQ_CHECK(sq_now() - start > 0.08, "timed out early");
	mach_port_get_refs(mach_task_self(), full_port, MACH_PORT_RIGHT_SEND, &after);
	SQ_CHECK(after == before + 2, "urefs %u -> %u", before, after);
	sq_destroy(full_port);
	printf("ok full queue\n");
}

// ---- the receive right leaves ------------------------------------------------------------------

static mach_port_t waited_port;
static volatile mach_msg_return_t waited_result;

static void* waiter(void* arg)
{
	(void) arg;
	sq_buffer_t buffer;
	waited_result = sq_receive(waited_port, &buffer, 10000);
	return NULL;
}

// sends the receive right of `port` to `carrier` and receives it back; returns its new name
static mach_port_t move_receive_right(mach_port_t port, mach_port_t carrier)
{
	struct
	{
		mach_msg_header_t header;
		mach_msg_body_t body;
		mach_msg_port_descriptor_t descriptor;
	} msg = {
		.header = {
			.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0) | MACH_MSGH_BITS_COMPLEX,
			.msgh_size = sizeof(msg),
			.msgh_remote_port = carrier,
			.msgh_id = 77,
		},
		.body = { .msgh_descriptor_count = 1 },
		.descriptor = { .name = port, .disposition = MACH_MSG_TYPE_MOVE_RECEIVE, .type = MACH_MSG_PORT_DESCRIPTOR },
	};
	SQ_KR(mach_msg(&msg.header, MACH_SEND_MSG, sizeof(msg), 0, MACH_PORT_NULL, 0, MACH_PORT_NULL), "send receive right");

	struct
	{
		mach_msg_header_t header;
		mach_msg_body_t body;
		mach_msg_port_descriptor_t descriptor;
		mach_msg_trailer_t trailer;
	} in;
	SQ_KR(mach_msg(&in.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(in), carrier, 5000, MACH_PORT_NULL),
		"receive receive right");
	return in.descriptor.name;
}

static void test_receive_right_moves(void)
{
	mach_port_t carrier = sq_port(true);
	mach_port_t port = sq_port(false);
	mach_port_t send_name;
	mach_msg_type_name_t type;
	sq_buffer_t buffer;

	// a separate send right (under its own name only for a port this task does not receive)
	SQ_KR(mach_port_extract_right(mach_task_self(), port, MACH_MSG_TYPE_MAKE_SEND, &send_name, &type), "extract");
	// one below the default queue limit, so that the send after the move does not wait
	for (int i = 0; i < 4; i++)
		SQ_KR(sq_send_ex(send_name, MACH_MSG_TYPE_COPY_SEND, 40, i, 0, 0), "queue");

	// a thread waiting on another port that then leaves
	waited_port = sq_port(false);
	pthread_t thread;
	pthread_create(&thread, NULL, waiter, NULL);
	usleep(200000);
	mach_port_t moved_waited = move_receive_right(waited_port, carrier);
	pthread_join(thread, NULL);
	SQ_CHECK(waited_result == MACH_RCV_PORT_CHANGED || waited_result == MACH_RCV_INVALID_NAME,
		"waiter on a moved port: 0x%x", waited_result);
	sq_destroy(moved_waited);

	mach_port_t moved = move_receive_right(port, carrier);
	SQ_KR(sq_send_ex(send_name, MACH_MSG_TYPE_COPY_SEND, 40, 4, MACH_SEND_TIMEOUT, 5000), "send after the move");
	for (int i = 0; i < 5; i++)
	{
		SQ_KR(receive_seqno(moved, &buffer, 5000), "receive after the move");
		SQ_CHECK(buffer.header.msgh_id == i, "moved: id %d, expected %d", buffer.header.msgh_id, i);
		SQ_CHECK(seqno_of(&buffer) == (mach_msg_trailer_size_t) i, "moved: seqno %u, expected %d", seqno_of(&buffer), i);
	}
	SQ_CHECK(sq_receive(moved, &buffer, 1) == MACH_RCV_TIMED_OUT, "extra message");
	sq_destroy(moved);
	mach_port_deallocate(mach_task_self(), send_name);
	sq_destroy(carrier);
	printf("ok receive right moves\n");
}

// ---- destroyed with messages queued ----------------------------------------------------------------

static void test_destroyed(void)
{
	mach_port_t port = sq_port(true);
	mach_port_t reply = sq_port(false);
	sq_buffer_t buffer;

	for (int i = 0; i < 3; i++)
		SQ_KR(sq_send(port, i), "queue");
	mach_msg_header_t header = {
		.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE),
		.msgh_size = sizeof(header),
		.msgh_remote_port = port,
		.msgh_local_port = reply,
		.msgh_id = 3,
	};
	SQ_KR(mach_msg(&header, MACH_SEND_MSG, sizeof(header), 0, MACH_PORT_NULL, 0, MACH_PORT_NULL), "send reply");
	SQ_KR(sq_send(port, 4), "queue");

	waited_port = sq_port(false);
	pthread_t thread;
	pthread_create(&thread, NULL, waiter, NULL);
	usleep(200000);
	SQ_KR(mach_port_mod_refs(mach_task_self(), waited_port, MACH_PORT_RIGHT_RECEIVE, -1), "destroy waited");
	pthread_join(thread, NULL);
	SQ_CHECK(waited_result == MACH_RCV_PORT_CHANGED || waited_result == MACH_RCV_INVALID_NAME ||
		waited_result == MACH_RCV_PORT_DIED, "waiter on a destroyed port: 0x%x", waited_result);

	SQ_KR(mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1), "destroy");
	mach_port_deallocate(mach_task_self(), port);

	SQ_KR(sq_receive(reply, &buffer, 5000), "send-once notification");
	SQ_CHECK(buffer.header.msgh_id == MACH_NOTIFY_SEND_ONCE, "notification id %d", buffer.header.msgh_id);
	sq_destroy(reply);
	printf("ok destroyed with messages queued\n");
}

// ---- no-senders ------------------------------------------------------------------------------

static void test_no_senders(void)
{
	mach_port_t port = sq_port(true);
	mach_port_t notify = sq_port(false);
	mach_port_t previous = MACH_PORT_NULL;
	sq_buffer_t buffer;

	for (int i = 0; i < 3; i++)
		SQ_KR(sq_send_ex(port, i == 1 ? MACH_MSG_TYPE_MAKE_SEND : MACH_MSG_TYPE_COPY_SEND, 32, i, 0, 0), "queue");
	SQ_KR(mach_port_deallocate(mach_task_self(), port), "drop own send right");

	SQ_KR(mach_port_request_notification(mach_task_self(), port, MACH_NOTIFY_NO_SENDERS, 0, notify,
		MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous), "request no-senders");
	SQ_CHECK(sq_receive(notify, &buffer, 100) == MACH_RCV_TIMED_OUT, "no-senders while messages hold send rights");

	for (int i = 0; i < 3; i++)
	{
		SQ_KR(sq_receive(port, &buffer, 5000), "receive");
		SQ_CHECK(buffer.header.msgh_id == i, "id %d, expected %d", buffer.header.msgh_id, i);
		if (i < 2)
			SQ_CHECK(sq_receive(notify, &buffer, 1) == MACH_RCV_TIMED_OUT, "no-senders too early");
	}
	SQ_KR(sq_receive(notify, &buffer, 5000), "no-senders notification");
	SQ_CHECK(buffer.header.msgh_id == MACH_NOTIFY_NO_SENDERS, "notification id %d", buffer.header.msgh_id);
	mach_no_senders_notification_t* ns = (mach_no_senders_notification_t*) &buffer;
	SQ_CHECK(ns->not_count == 2, "mscount %u", ns->not_count);
	sq_destroy(port);
	sq_destroy(notify);
	printf("ok no-senders\n");
}

// ---- port set ------------------------------------------------------------------------------------

static void test_port_set(void)
{
	mach_port_t port = sq_port(true);
	mach_port_t set;
	sq_buffer_t buffer;

	SQ_KR(sq_send(port, 0), "queue");
	SQ_KR(receive_seqno(port, &buffer, 1000), "receive");
	for (int i = 1; i < 5; i++)
		SQ_KR(sq_send(port, i), "queue");

	SQ_KR(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &set), "allocate set");
	SQ_KR(mach_port_insert_member(mach_task_self(), port, set), "insert member");
	SQ_KR(sq_send(port, 5), "send after insert");

	for (int i = 1; i < 6; i++)
	{
		SQ_KR(receive_seqno(set, &buffer, 5000), "receive from set");
		SQ_CHECK(buffer.header.msgh_id == i, "set: id %d, expected %d", buffer.header.msgh_id, i);
		SQ_CHECK(seqno_of(&buffer) == (mach_msg_trailer_size_t) i, "set: seqno %u", seqno_of(&buffer));
		SQ_CHECK(buffer.header.msgh_local_port == port, "set: local port");
	}
	SQ_CHECK(sq_receive(set, &buffer, 1) == MACH_RCV_TIMED_OUT, "extra message");
	sq_destroy(port);
	sq_destroy(set);
	printf("ok port set\n");
}

// ---- fork ----------------------------------------------------------------------------------------

static volatile int fork_stop;
static mach_port_t fork_port;

static void* fork_busy(void* arg)
{
	(void) arg;
	sq_buffer_t buffer;
	while (!fork_stop)
	{
		SQ_KR(sq_send(fork_port, 1), "busy send");
		SQ_KR(sq_receive(fork_port, &buffer, 5000), "busy receive");
	}
	return NULL;
}

static void test_fork(void)
{
	pthread_t thread;
	fork_port = sq_port(true);
	pthread_create(&thread, NULL, fork_busy, NULL);

	mach_port_t parent_port = sq_port(true);
	SQ_KR(sq_send(parent_port, 42), "queue before fork");

	for (int f = 0; f < 20; f++)
	{
		pid_t pid = fork();
		SQ_CHECK(pid >= 0, "fork");
		if (pid == 0)
		{
			sq_buffer_t buffer;
			mach_port_t port = sq_port(true);
			for (int i = 0; i < 100; i++)
			{
				if (sq_send(port, i) != MACH_MSG_SUCCESS)
					_exit(2);
				if (sq_receive(port, &buffer, 5000) != MACH_MSG_SUCCESS || buffer.header.msgh_id != i)
					_exit(3);
			}
			// the parent's port is not the child's
			if (sq_receive(parent_port, &buffer, 1) == MACH_MSG_SUCCESS)
				_exit(4);
			if (!sq_server_only() && sq_region_mapped() == 0)
				_exit(5);
			_exit(0);
		}
		int status;
		SQ_CHECK(waitpid(pid, &status, 0) == pid, "waitpid");
		SQ_CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "child %d: status 0x%x", f, status);
	}

	fork_stop = 1;
	pthread_join(thread, NULL);
	sq_buffer_t buffer;
	SQ_KR(sq_receive(parent_port, &buffer, 1000), "parent's message after forks");
	SQ_CHECK(buffer.header.msgh_id == 42, "parent's message");
	sq_destroy(parent_port);
	sq_destroy(fork_port);
	printf("ok fork\n");
}

// ---- kill switch ---------------------------------------------------------------------------------

static void test_kill_switch(void)
{
	int mapped = sq_region_mapped();
	if (mapped < 0)
	{
		printf("skip kill switch (no /proc/self/maps)\n");
		return;
	}
	if (sq_server_only())
		SQ_CHECK(mapped == 0, "region mapped with DARLING_MACH_SERVER=1");
	else
		SQ_CHECK(mapped == 1, "no region mapped");
	printf("ok kill switch (%s)\n", mapped ? "region mapped" : "no region");
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	sq_watchdog(300);
	printf("mode: %s\n", sq_server_only() ? "DARLING_MACH_SERVER=1" : "default");

	test_order();
	test_wake_and_timeouts();
	test_full();
	test_receive_right_moves();
	test_destroyed();
	test_no_senders();
	test_port_set();
	test_fork();
	test_kill_switch();

	printf("PASS\n");
	return 0;
}
