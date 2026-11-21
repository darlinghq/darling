// Copyright (C) 2026 Lubos Dolezel
//
// A task that is stopped (SIGSTOP) must not hold up anyone who sends to its
// ports or looks at them: sending to a port of the stopped task, reading the
// port's status (mach_port_get_attributes, as lsmp does), the task's port
// space (mach_port_names, mach_port_space_info) and peeking at the queue all complete at once,
// whatever the stopped task's threads were doing. The stopped task's own
// threads keep messaging each other until they are stopped, so the stops land
// anywhere in their sends and receives, also in the middle of messages that
// never leave the task.
//
// The stopped task's make-send count is exact at every stop: one of its
// threads sends MAKE_SEND messages to a port that has no other send rights
// and another receives them, and the task is stopped many times while they
// do. A stop can land anywhere, also right after a receiver took a message
// from the queue and before it copied it out; at each stop the port's status
// must still be one state of the port, in which every message sent was
// received (seqno) or is queued: make-send count = seqno + queued messages.
//
// After it is continued, the task receives every message: the ones it queued
// itself before, then the other task's, in order; a thread of it that was
// waiting on an empty port while it was stopped gets the message sent there
// meanwhile.
//
// The parent passes the child a send right through the bootstrap port the
// child inherits; the child sends back send rights to its ports.

#include "mach_sq_common.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <sys/wait.h>

#define ROUNDS 8
#define PER_ROUND 40
#define FINISH_ID 99999
#define MADE_STOPS 1500
#define MADE_WINDOW 16

typedef struct
{
	mach_msg_header_t header;
	mach_msg_body_t body;
	mach_msg_port_descriptor_t queue;
	mach_msg_port_descriptor_t wait;
	mach_port_name_t queue_name;
	mach_port_name_t wait_name;
	mach_port_name_t made_name;
} hello_t;

typedef struct
{
	hello_t msg;
	mach_msg_trailer_t trailer;
} hello_in_t;

static volatile int child_stop;
static mach_port_t busy_ports[2];
static mach_port_t made_port;
static volatile int made_sent, made_received, made_done;

static void* child_made_sender(void* arg)
{
	(void) arg;
	while (!child_stop)
	{
		if (made_sent - __atomic_load_n(&made_received, __ATOMIC_ACQUIRE) >= MADE_WINDOW)
		{
			sched_yield();
			continue;
		}
		if (sq_send_ex(made_port, MACH_MSG_TYPE_MAKE_SEND, 32, made_sent, 0, 0) != MACH_MSG_SUCCESS)
			_exit(22);
		__atomic_store_n(&made_sent, made_sent + 1, __ATOMIC_RELEASE);
	}
	__atomic_store_n(&made_done, 1, __ATOMIC_RELEASE);
	return NULL;
}

static void* child_made_receiver(void* arg)
{
	(void) arg;
	sq_buffer_t buffer;
	for (;;)
	{
		int received = made_received;
		if (__atomic_load_n(&made_done, __ATOMIC_ACQUIRE) && received == __atomic_load_n(&made_sent, __ATOMIC_ACQUIRE))
			return NULL;
		mach_msg_return_t mr = sq_receive(made_port, &buffer, 100);
		if (mr == MACH_RCV_TIMED_OUT)
			continue;
		if (mr != MACH_MSG_SUCCESS || buffer.header.msgh_id != received)
			_exit(23);
		__atomic_store_n(&made_received, received + 1, __ATOMIC_RELEASE);
	}
}

static void* child_busy(void* arg)
{
	int me = (int) (intptr_t) arg;
	sq_buffer_t buffer;

	while (!child_stop)
	{
		if (sq_send(busy_ports[1 - me], me) != MACH_MSG_SUCCESS)
			_exit(20);
		if (sq_receive(busy_ports[me], &buffer, 0) != MACH_MSG_SUCCESS)
			_exit(21);
	}
	// one more, so that the other thread is not left waiting
	sq_send(busy_ports[1 - me], me);
	return NULL;
}

static int child(void)
{
	mach_port_t parent = MACH_PORT_NULL;
	if (task_get_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, &parent) != KERN_SUCCESS)
		return 10;

	mach_port_t queue = sq_port(true);
	mach_port_t wait = sq_port(true);
	sq_set_qlimit(queue, MACH_PORT_QLIMIT_LARGE);
	for (int i = 0; i < 3; i++)
		if (sq_send(queue, i) != MACH_MSG_SUCCESS)
			return 11;

	busy_ports[0] = sq_port(true);
	busy_ports[1] = sq_port(true);
	sq_send(busy_ports[0], 1); // a message in flight from the start
	pthread_t threads[4];
	for (int t = 0; t < 2; t++)
		pthread_create(&threads[t], NULL, child_busy, (void*) (intptr_t) t);

	// no send right of its own: only the MAKE_SEND messages count
	made_port = sq_port(false);
	sq_set_qlimit(made_port, MADE_WINDOW * 2);
	pthread_create(&threads[2], NULL, child_made_sender, NULL);
	pthread_create(&threads[3], NULL, child_made_receiver, NULL);

	hello_t hello = {
		.header = {
			.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0) | MACH_MSGH_BITS_COMPLEX,
			.msgh_size = sizeof(hello),
			.msgh_remote_port = parent,
			.msgh_id = 1,
		},
		.body = { .msgh_descriptor_count = 2 },
		.queue = { .name = queue, .disposition = MACH_MSG_TYPE_COPY_SEND, .type = MACH_MSG_PORT_DESCRIPTOR },
		.wait = { .name = wait, .disposition = MACH_MSG_TYPE_COPY_SEND, .type = MACH_MSG_PORT_DESCRIPTOR },
		.queue_name = queue,
		.wait_name = wait,
		.made_name = made_port,
	};
	if (mach_msg(&hello.header, MACH_SEND_MSG, sizeof(hello), 0, MACH_PORT_NULL, 0, MACH_PORT_NULL) != MACH_MSG_SUCCESS)
		return 12;

	// waits (on an empty queue) through all the stops
	sq_buffer_t buffer;
	if (sq_receive(wait, &buffer, 0) != MACH_MSG_SUCCESS || buffer.header.msgh_id != FINISH_ID)
		return 13;

	child_stop = 1;
	for (int t = 0; t < 4; t++)
		pthread_join(threads[t], NULL);

	mach_port_status_t ps;
	mach_msg_type_number_t count = MACH_PORT_RECEIVE_STATUS_COUNT;
	if (mach_port_get_attributes(mach_task_self(), made_port, MACH_PORT_RECEIVE_STATUS, (mach_port_info_t) &ps,
			&count) != KERN_SUCCESS)
		return 17;
	if (ps.mps_mscount != (mach_port_mscount_t) made_sent || ps.mps_seqno != (mach_port_seqno_t) made_sent ||
			ps.mps_msgcount != 0 || made_sent == 0)
		return 18;

	for (int i = 0; i < 3; i++)
		if (sq_receive(queue, &buffer, 1000) != MACH_MSG_SUCCESS || buffer.header.msgh_id != i)
			return 14;
	for (int i = 0; i < ROUNDS * PER_ROUND; i++)
		if (sq_receive(queue, &buffer, 1000) != MACH_MSG_SUCCESS || buffer.header.msgh_id != 1000 + i)
			return 15;
	if (sq_receive(queue, &buffer, 1) != MACH_RCV_TIMED_OUT)
		return 16;
	return 0;
}

static double timed(double start, const char* what)
{
	double elapsed = sq_now() - start;
	SQ_CHECK(elapsed < 2.0, "%s took %.3f s with the receiver stopped", what, elapsed);
	return elapsed;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	sq_watchdog(120);
	printf("mode: %s\n", sq_server_only() ? "DARLING_MACH_SERVER=1" : "default");

	mach_port_t hello_port = sq_port(true);
	mach_port_t old_bootstrap;
	SQ_KR(task_get_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, &old_bootstrap), "get bootstrap");
	SQ_KR(task_set_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, hello_port), "set bootstrap");

	pid_t pid = fork();
	SQ_CHECK(pid >= 0, "fork");
	if (pid == 0)
		_exit(child());
	SQ_KR(task_set_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, old_bootstrap), "restore bootstrap");

	hello_in_t hello;
	SQ_KR(mach_msg(&hello.msg.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(hello), hello_port, 10000,
		MACH_PORT_NULL), "receive hello");
	mach_port_t queue = hello.msg.queue.name;
	mach_port_t wait = hello.msg.wait.name;

	mach_port_t child_task = MACH_PORT_NULL;
	bool introspect = task_for_pid(mach_task_self(), pid, &child_task) == KERN_SUCCESS;
	if (!introspect)
		printf("note: task_for_pid not permitted, no introspection\n");

	// stops while the child's threads send and receive MAKE_SEND messages
	if (introspect)
	{
		unsigned queued_stops = 0;
		mach_port_mscount_t last = 0;
		srandom(1);
		for (int n = 0; n < MADE_STOPS; n++)
		{
			usleep(200 + random() % 1500);
			SQ_CHECK(kill(pid, SIGSTOP) == 0, "SIGSTOP");
			int status;
			SQ_CHECK(waitpid(pid, &status, WUNTRACED) == pid && WIFSTOPPED(status), "child not stopped (0x%x)", status);

			mach_port_status_t ps;
			mach_msg_type_number_t count = MACH_PORT_RECEIVE_STATUS_COUNT;
			double start = sq_now();
			SQ_KR(mach_port_get_attributes(child_task, hello.msg.made_name, MACH_PORT_RECEIVE_STATUS,
				(mach_port_info_t) &ps, &count), "get_attributes made");
			timed(start, "mach_port_get_attributes");
			SQ_CHECK(ps.mps_mscount == ps.mps_seqno + ps.mps_msgcount,
				"stop %d: mscount %u, seqno %u, msgcount %u", n, ps.mps_mscount, ps.mps_seqno, ps.mps_msgcount);
			SQ_CHECK(ps.mps_mscount >= last, "stop %d: mscount went back from %u to %u", n, last, ps.mps_mscount);
			SQ_CHECK(ps.mps_msgcount == 0 || ps.mps_srights, "stop %d: queued, no send rights", n);
			last = ps.mps_mscount;
			if (ps.mps_msgcount != 0)
				queued_stops++;

			SQ_CHECK(kill(pid, SIGCONT) == 0, "SIGCONT");
		}
		SQ_CHECK(last > 0, "no MAKE_SEND traffic");
		printf("%d stops during MAKE_SEND traffic, %u with messages queued, %u messages\n", MADE_STOPS, queued_stops,
			last);
	}

	double worst = 0;
	for (int round = 0; round < ROUNDS; round++)
	{
		usleep(20000 + round * 7000);
		SQ_CHECK(kill(pid, SIGSTOP) == 0, "SIGSTOP");
		int status;
		SQ_CHECK(waitpid(pid, &status, WUNTRACED) == pid && WIFSTOPPED(status), "child not stopped (0x%x)", status);

		for (int i = 0; i < PER_ROUND; i++)
		{
			double start = sq_now();
			mach_msg_return_t mr = sq_send_ex(queue, MACH_MSG_TYPE_COPY_SEND, 32, 1000 + round * PER_ROUND + i,
				MACH_SEND_TIMEOUT, 5000);
			SQ_CHECK(mr == MACH_MSG_SUCCESS, "send to the stopped task: 0x%x", mr);
			double t = timed(start, "a send");
			if (t > worst)
				worst = t;
		}

		if (introspect)
		{
			mach_port_status_t ps;
			mach_msg_type_number_t count = MACH_PORT_RECEIVE_STATUS_COUNT;
			double start = sq_now();
			SQ_KR(mach_port_get_attributes(child_task, hello.msg.queue_name, MACH_PORT_RECEIVE_STATUS,
				(mach_port_info_t) &ps, &count), "get_attributes");
			timed(start, "mach_port_get_attributes");
			SQ_CHECK(ps.mps_msgcount == 3 + (round + 1) * PER_ROUND, "msgcount %u, expected %d", ps.mps_msgcount,
				3 + (round + 1) * PER_ROUND);

			count = MACH_PORT_RECEIVE_STATUS_COUNT;
			start = sq_now();
			SQ_KR(mach_port_get_attributes(child_task, hello.msg.wait_name, MACH_PORT_RECEIVE_STATUS,
				(mach_port_info_t) &ps, &count), "get_attributes wait");
			timed(start, "mach_port_get_attributes");
			SQ_CHECK(ps.mps_msgcount == 0, "wait port msgcount %u", ps.mps_msgcount);

			ipc_info_space_t info;
			ipc_info_name_array_t table;
			mach_msg_type_number_t table_count;
			ipc_info_tree_name_array_t tree;
			mach_msg_type_number_t tree_count;
			start = sq_now();
			kern_return_t kr = mach_port_space_info(child_task, &info, &table, &table_count, &tree, &tree_count);
			timed(start, "mach_port_space_info");
			// a kernel without MACH_DEBUG fails it at once
			SQ_CHECK(kr == KERN_SUCCESS || kr == KERN_FAILURE, "space_info: 0x%x", kr);
			if (kr == KERN_SUCCESS)
			{
				vm_deallocate(mach_task_self(), (vm_address_t) table, table_count * sizeof(*table));
				if (tree_count)
					vm_deallocate(mach_task_self(), (vm_address_t) tree, tree_count * sizeof(*tree));
			}

			mach_port_name_array_t port_names;
			mach_msg_type_number_t port_names_count;
			mach_port_type_array_t port_types;
			mach_msg_type_number_t port_types_count;
			start = sq_now();
			SQ_KR(mach_port_names(child_task, &port_names, &port_names_count, &port_types, &port_types_count),
				"mach_port_names");
			timed(start, "mach_port_names");
			bool listed = false;
			for (mach_msg_type_number_t n = 0; n < port_names_count; n++)
				if (port_names[n] == hello.msg.queue_name && (port_types[n] & MACH_PORT_TYPE_RECEIVE))
					listed = true;
			SQ_CHECK(listed, "the queue's receive right is not listed");
			vm_deallocate(mach_task_self(), (vm_address_t) port_names, port_names_count * sizeof(*port_names));
			vm_deallocate(mach_task_self(), (vm_address_t) port_types, port_types_count * sizeof(*port_types));

			if (round == ROUNDS - 1)
			{
				// takes the queue out of the task's hands
				mach_port_seqno_t seqno = 0;
				mach_msg_size_t size;
				mach_msg_id_t id;
				start = sq_now();
				mach_msg_max_trailer_t trailer;
				mach_msg_type_number_t trailer_size = sizeof(trailer);
				kr = mach_port_peek(child_task, hello.msg.queue_name, MACH_RCV_TRAILER_NULL, &seqno,
					&size, &id, (mach_msg_trailer_info_t) &trailer, &trailer_size);
				timed(start, "mach_port_peek");
				SQ_CHECK(kr == KERN_SUCCESS && id == 0, "peek: 0x%x id %d", kr, id);
			}
		}

		SQ_CHECK(kill(pid, SIGCONT) == 0, "SIGCONT");
	}

	SQ_KR(sq_send_ex(wait, MACH_MSG_TYPE_COPY_SEND, 32, FINISH_ID, MACH_SEND_TIMEOUT, 5000), "finish");

	int status;
	SQ_CHECK(waitpid(pid, &status, 0) == pid, "waitpid");
	SQ_CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "child status 0x%x", status);
	printf("slowest send to the stopped task: %.1f ms\n", worst * 1000);
	printf("PASS\n");
	return 0;
}
