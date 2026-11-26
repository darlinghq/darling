// Copyright (C) 2026 Lubos Dolezel
//
// Wakeups and races around Mach ports whose queue Darling keeps in memory the
// task shares with darlingserver (see mach_sq_common.h). Each case is documented
// (or XNU) behavior of mach_msg() and also holds with DARLING_MACH_SERVER=1:
//   - two threads wait on a port, the first with MACH_RCV_LARGE and a buffer too
//     small for the message that arrives: it gets MACH_RCV_TOO_LARGE and the
//     second receives the message (XNU hands the message on to the next
//     receiver), for a message the task sends and for one only the server sends;
//   - a thread waits on a port; while it is suspended the port gets a no-senders
//     request, and ports are created and destroyed: the thread still waits for
//     the port, which is alive, and receives the message sent to it afterwards;
//   - the audit trailer of a message is the sender's at the time of the send,
//     also when the task changed its user ID in between and the message reaches
//     the receiver through the server; in the default mode the task also tries
//     to label a message sent after the change with the earlier tokens (by
//     changing the queued message), which must not show in the trailer;
//   - stress: the queue limit is raised while a sender is about to wait for room
//     in a full queue: the sender always completes (no lost wake). The race is
//     narrow, so this is a statistical test;
//   - stress: a child task exits while this task changes the reference counts
//     of its send rights to the child's ports. darlingserver's default build runs
//     one server thread, so the race this guards against (a server thread reading
//     the exiting child's queue state) cannot happen there; the case checks the
//     path stays correct.
// Each case prints a line; the program prints PASS and exits 0 when all pass.

#include "mach_sq_region.h"

#include <errno.h>
#include <pthread.h>
#include <sys/wait.h>
#include <mach/mach_time.h>

#define WAIT_MS 3000

static const mach_msg_option_t audit_trailer =
	MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT);

// ---- a receiver thread --------------------------------------------------------------------------

typedef struct
{
	mach_port_t port;
	mach_msg_option_t options;
	mach_msg_size_t size;
	mach_port_t thread_port;
	volatile int started;
	volatile int done;
	mach_msg_return_t result;
	mach_msg_id_t id;
	mach_msg_size_t msg_size;
} receiver_t;

static void* receiver_main(void* arg)
{
	receiver_t* r = arg;
	sq_buffer_t buffer;

	memset(&buffer, 0, sizeof(buffer));
	r->thread_port = mach_thread_self();
	__atomic_store_n(&r->started, 1, __ATOMIC_SEQ_CST);
	r->result = mach_msg(&buffer.header, MACH_RCV_MSG | r->options, 0, r->size, r->port, 0, MACH_PORT_NULL);
	r->id = r->result == MACH_MSG_SUCCESS ? buffer.header.msgh_id : -1;
	r->msg_size = buffer.header.msgh_size;
	__atomic_store_n(&r->done, 1, __ATOMIC_SEQ_CST);
	return NULL;
}

static pthread_t receiver_start(receiver_t* r, mach_port_t port, mach_msg_option_t options, mach_msg_size_t size)
{
	pthread_t thread;
	memset(r, 0, sizeof(*r));
	r->port = port;
	r->options = options;
	r->size = size;
	SQ_CHECK(pthread_create(&thread, NULL, receiver_main, r) == 0, "pthread_create");
	while (!__atomic_load_n(&r->started, __ATOMIC_SEQ_CST))
		usleep(100);
	usleep(100000); // it is waiting in the receive by now
	return thread;
}

static bool receiver_wait(receiver_t* r, int ms)
{
	double end = sq_now() + ms / 1000.0;
	while (!__atomic_load_n(&r->done, __ATOMIC_SEQ_CST))
	{
		if (sq_now() >= end)
			return false;
		usleep(1000);
	}
	return true;
}

static void receiver_join(receiver_t* r, pthread_t thread)
{
	pthread_join(thread, NULL);
	mach_port_deallocate(mach_task_self(), r->thread_port);
}

// a message with a reply port: only darlingserver sends it
static mach_msg_return_t send_with_reply(mach_port_t dest, mach_port_t reply, mach_msg_size_t size, mach_msg_id_t id)
{
	sq_buffer_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	msg.header.msgh_size = size;
	msg.header.msgh_remote_port = dest;
	msg.header.msgh_local_port = reply;
	msg.header.msgh_id = id;
	return mach_msg(&msg.header, MACH_SEND_MSG, size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
}

// ---- a receiver that leaves a too large message queued -------------------------------------------

static void test_too_large_handoff(bool from_server)
{
	mach_port_t port = sq_port(true);
	mach_port_t reply = sq_port(false);
	receiver_t small, big;
	sq_buffer_t buffer;

	// the first waiter's buffer has room for a header and the trailer only
	pthread_t small_thread = receiver_start(&small, port, MACH_RCV_LARGE, 64);
	pthread_t big_thread = receiver_start(&big, port, 0, sizeof(sq_buffer_t));

	if (from_server)
		SQ_KR(send_with_reply(port, reply, 200, 7), "send with a reply port");
	else
		SQ_KR(sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 200, 7, 0, 0), "send");

	SQ_CHECK(receiver_wait(&big, WAIT_MS), "%s: a message too large for one waiting receiver was not received by "
		"the other one in %d ms (the wake was not passed on)", from_server ? "server message" : "task message", WAIT_MS);
	SQ_KR(big.result, "the receiver with room");
	SQ_CHECK(big.id == 7 && big.msg_size == 200, "id %d size %u", big.id, big.msg_size);
	receiver_join(&big, big_thread);

	// the small receiver got the indication; if the kernel woke the other one first it still waits
	if (receiver_wait(&small, 500))
		SQ_CHECK(small.result == MACH_RCV_TOO_LARGE, "the small receiver: 0x%x", (unsigned) small.result);
	else
	{
		SQ_KR(sq_send(port, 8), "release the small receiver");
		SQ_CHECK(receiver_wait(&small, WAIT_MS), "the small receiver did not return");
		SQ_KR(small.result, "the small receiver");
	}
	receiver_join(&small, small_thread);
	SQ_CHECK(sq_receive(port, &buffer, 50) == MACH_RCV_TIMED_OUT, "a message too many");

	sq_destroy(port);
	sq_destroy(reply);
	printf("ok too large for the first waiter, received by the second (%s)\n",
		from_server ? "message sent by the server" : "message sent by the task");
}

// ---- the port's slot is reused and closed again while its receiver waits ------------------------

static void test_reused_slot_reason(void)
{
	mach_port_t notify = sq_port(false);
	mach_port_t port = sq_port(true);
	mach_port_t previous, others[4];
	receiver_t r;
	pthread_t thread = receiver_start(&r, port, 0, sizeof(sq_buffer_t));

	SQ_KR(thread_suspend(r.thread_port), "thread_suspend");
	// the port leaves the shared queue (for a reason that is neither a move nor a destroy)
	SQ_KR(mach_port_request_notification(mach_task_self(), port, MACH_NOTIFY_NO_SENDERS, 1000, notify,
		MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous), "no-senders request");
	// a new port gets the number the queue used, and that port is destroyed
	for (int i = 0; i < 4; i++)
		others[i] = sq_port(true);
	for (int i = 0; i < 4; i++)
		sq_destroy(others[i]);
	SQ_KR(thread_resume(r.thread_port), "thread_resume");

	SQ_CHECK(!receiver_wait(&r, 300), "the receive on a live port returned 0x%x after another port was destroyed",
		(unsigned) r.result);
	SQ_KR(sq_send(port, 21), "send");
	SQ_CHECK(receiver_wait(&r, WAIT_MS), "the receive did not return in %d ms", WAIT_MS);
	SQ_KR(r.result, "receive");
	SQ_CHECK(r.id == 21, "id %d", r.id);
	receiver_join(&r, thread);

	sq_destroy(port);
	sq_destroy(notify);
	printf("ok a waiting receiver keeps its port when the port's queue number is reused and destroyed\n");
}

// ---- audit tokens of messages sent around a user ID change -------------------------------------

static uid_t receive_euid(mach_port_t port, mach_msg_id_t expected)
{
	sq_buffer_t buffer;
	SQ_KR(sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT | audit_trailer, 1000), "token receive");
	SQ_CHECK(buffer.header.msgh_id == expected, "id %d, expected %d", buffer.header.msgh_id, expected);
	const mach_msg_audit_trailer_t* trailer =
		(const mach_msg_audit_trailer_t*) ((const uint8_t*) &buffer + buffer.header.msgh_size);
	return trailer->msgh_audit.val[1];
}

static void test_token_label(void)
{
	mach_port_t port = sq_port(true);
	mach_port_t notify = sq_port(false);
	mach_port_t previous;
	uid_t original = geteuid();
	uid_t other = original + 1;
	sq_region_t region;
	bool forged = false;

	SQ_KR(sq_send(port, 1), "send before the change");
	if (setuid(other) != 0)
	{
		printf("skip token labels: setuid is not permitted (errno %d)\n", errno);
		sq_destroy(port);
		sq_destroy(notify);
		return;
	}
	SQ_KR(sq_send(port, 2), "send after the change");

	// the task labels the second message with the generation of the first (what a task that was privileged
	// before the change would like its new messages to carry)
	if (sq_region_find(&region))
	{
		int slot = sq_region_slot(&region, port);
		SQ_CHECK(slot >= 0, "the port is not in the shared queue");
		uint64_t head = __atomic_load_n(&region.q->slots[slot].head, __ATOMIC_ACQUIRE);
		dsq_node_t* second = &region.q->nodes[DSQ_H_TOP(head)];
		dsq_node_t* first = &region.q->nodes[second->next];
		SQ_CHECK(DSQ_H_TOP(head) < DSQ_TNODES && second->next < DSQ_TNODES, "the messages are not the task's nodes");
		forged = second->aux != first->aux;
		second->aux = first->aux;
	}

	// the messages reach the receiver through the server: the port leaves the shared queue
	SQ_KR(mach_port_request_notification(mach_task_self(), port, MACH_NOTIFY_NO_SENDERS, 1000, notify,
		MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous), "no-senders request");
	SQ_CHECK(setuid(original) == 0, "setuid back");

	uid_t first = receive_euid(port, 1), second = receive_euid(port, 2);
	SQ_CHECK(first == original, "the message sent before the change carries user %u (expected %u)", first, original);
	SQ_CHECK(second == other, "the message sent after the change carries user %u (expected %u)%s", second, other,
		forged ? ": the task's own label was used" : "");
	sq_destroy(port);
	sq_destroy(notify);
	printf("ok the audit trailer is the sender's at the send (%u, %u)%s\n", first, second,
		forged ? ", a label the task changed is not used" : "");
}

// ---- stress: the queue limit is raised while a sender is about to wait -----------------------------

static mach_port_t raise_port;
static volatile int raise_go, raise_done;
static volatile uint64_t raise_delay_ns;
static mach_timebase_info_data_t timebase;

static uint64_t now_ns(void)
{
	return mach_absolute_time() * timebase.numer / timebase.denom;
}

static void* raise_sender(void* arg)
{
	(void) arg;
	for (int i = 1;; i++)
	{
		while (__atomic_load_n(&raise_go, __ATOMIC_ACQUIRE) != i)
		{
			if (__atomic_load_n(&raise_go, __ATOMIC_ACQUIRE) < 0)
				return NULL;
		}
		uint64_t until = now_ns() + raise_delay_ns;
		while (now_ns() < until)
			;
		SQ_KR(sq_send(raise_port, i), "the waiting send");
		__atomic_store_n(&raise_done, i, __ATOMIC_RELEASE);
	}
}

static void test_qlimit_raise_race(int rounds)
{
	pthread_t thread;
	sq_buffer_t buffer;
	uint64_t rpc_ns;

	mach_timebase_info(&timebase);

	// how long a limit change takes: the sender's wait starts at random points within it
	{
		mach_port_t port = sq_port(true);
		uint64_t start = now_ns();
		for (int i = 0; i < 100; i++)
			sq_set_qlimit(port, 1 + i % 2);
		rpc_ns = (now_ns() - start) / 100;
		sq_destroy(port);
	}

	raise_go = 0;
	raise_done = 0;
	SQ_CHECK(pthread_create(&thread, NULL, raise_sender, NULL) == 0, "pthread_create");
	srandom(1);

	for (int i = 1; i <= rounds; i++)
	{
		raise_port = sq_port(true);
		sq_set_qlimit(raise_port, 1);
		SQ_KR(sq_send(raise_port, 0), "fill");
		raise_delay_ns = (uint64_t) random() % (2 * rpc_ns + 1);
		__atomic_store_n(&raise_go, i, __ATOMIC_RELEASE);
		sq_set_qlimit(raise_port, 2);

		double end = sq_now() + WAIT_MS / 1000.0;
		while (__atomic_load_n(&raise_done, __ATOMIC_ACQUIRE) != i)
		{
			SQ_CHECK(sq_now() < end, "round %d: the limit was raised, but the sender still waits after %d ms (lost "
				"wake)", i, WAIT_MS);
			usleep(50);
		}
		SQ_KR(sq_receive(raise_port, &buffer, 1000), "receive the fill");
		SQ_CHECK(buffer.header.msgh_id == 0, "id %d, expected 0", buffer.header.msgh_id);
		SQ_KR(sq_receive(raise_port, &buffer, 1000), "receive the waiting send");
		SQ_CHECK(buffer.header.msgh_id == i, "id %d, expected %d", buffer.header.msgh_id, i);
		sq_destroy(raise_port);
	}
	__atomic_store_n(&raise_go, -1, __ATOMIC_RELEASE);
	pthread_join(thread, NULL);
	printf("ok stress: queue limit raised while a sender is about to wait, %d rounds (limit change %llu us)\n",
		rounds, (unsigned long long) (rpc_ns / 1000));
}

// ---- stress: send rights to the ports of a task that exits -------------------------------------

#define CHILD_PORTS 32

static mach_port_t foreign_names[CHILD_PORTS];
static volatile int foreign_count, foreign_stop;
static volatile long foreign_changes;

static void* foreign_hammer(void* arg)
{
	(void) arg;
	while (!__atomic_load_n(&foreign_stop, __ATOMIC_ACQUIRE))
	{
		int count = __atomic_load_n(&foreign_count, __ATOMIC_ACQUIRE);
		for (int i = 0; i < count; i++)
		{
			mach_port_t name = foreign_names[i];
			if (mach_port_mod_refs(mach_task_self(), name, MACH_PORT_RIGHT_SEND, 1) == KERN_SUCCESS)
			{
				mach_port_mod_refs(mach_task_self(), name, MACH_PORT_RIGHT_SEND, -1);
				__atomic_add_fetch(&foreign_changes, 1, __ATOMIC_RELAXED);
			}
		}
		if (count == 0)
			usleep(100);
	}
	return NULL;
}

typedef struct
{
	mach_msg_header_t header;
	mach_msg_body_t body;
	mach_msg_ool_ports_descriptor_t ports;
} ports_message_t;

static int foreign_child(void)
{
	mach_port_t* registered;
	mach_msg_type_number_t count;
	mach_port_t ports[CHILD_PORTS];
	sq_buffer_t buffer;

	if (mach_ports_lookup(mach_task_self(), &registered, &count) != KERN_SUCCESS || count < 1 ||
			!MACH_PORT_VALID(registered[0]))
		return 2;

	// the first messages attach the region, so the ports below are in the shared queue
	mach_port_t warm = sq_port(true);
	if (sq_send(warm, 1) != KERN_SUCCESS || sq_receive(warm, &buffer, 1000) != KERN_SUCCESS)
		return 3;
	for (int i = 0; i < CHILD_PORTS; i++)
		ports[i] = sq_port(true);

	ports_message_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0) | MACH_MSGH_BITS_COMPLEX;
	msg.header.msgh_size = sizeof(msg);
	msg.header.msgh_remote_port = registered[0];
	msg.header.msgh_id = 77;
	msg.body.msgh_descriptor_count = 1;
	msg.ports.address = ports;
	msg.ports.count = CHILD_PORTS;
	msg.ports.deallocate = FALSE;
	msg.ports.disposition = MACH_MSG_TYPE_MAKE_SEND;
	msg.ports.type = MACH_MSG_OOL_PORTS_DESCRIPTOR;
	if (mach_msg(&msg.header, MACH_SEND_MSG, sizeof(msg), 0, MACH_PORT_NULL, 0, MACH_PORT_NULL) != KERN_SUCCESS)
		return 4;
	// and the ports go away with the task right away
	return 0;
}

static void test_foreign_rights_task_exit(int children)
{
	mach_port_t collector = sq_port(true);
	pthread_t threads[3];

	SQ_KR(mach_ports_register(mach_task_self(), &collector, 1), "mach_ports_register");
	foreign_count = 0;
	foreign_stop = 0;
	foreign_changes = 0;
	for (int i = 0; i < 3; i++)
		SQ_CHECK(pthread_create(&threads[i], NULL, foreign_hammer, NULL) == 0, "pthread_create");

	for (int c = 0; c < children; c++)
	{
		pid_t pid = fork();
		SQ_CHECK(pid >= 0, "fork");
		if (pid == 0)
			_exit(foreign_child());

		struct
		{
			ports_message_t msg;
			uint8_t trailer[64];
		} received;
		memset(&received, 0, sizeof(received));
		mach_msg_return_t mr = mach_msg(&received.msg.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(received),
			collector, 5000, MACH_PORT_NULL);
		int status = 0;
		if (mr != MACH_MSG_SUCCESS)
		{
			waitpid(pid, &status, 0);
			SQ_CHECK(false, "child %d: no ports received (0x%x, child status 0x%x)", c, (unsigned) mr, status);
		}
		SQ_CHECK(received.msg.header.msgh_id == 77 && received.msg.ports.count == CHILD_PORTS, "the child's message");

		mach_port_t* names = received.msg.ports.address;
		for (int i = 0; i < CHILD_PORTS; i++)
			foreign_names[i] = names[i];
		__atomic_store_n(&foreign_count, CHILD_PORTS, __ATOMIC_RELEASE);

		waitpid(pid, &status, 0);
		SQ_CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "child %d: status 0x%x", c, status);
		usleep(20000);

		__atomic_store_n(&foreign_count, 0, __ATOMIC_RELEASE);
		usleep(2000);
		for (int i = 0; i < CHILD_PORTS; i++)
			mach_port_deallocate(mach_task_self(), names[i]);
		vm_deallocate(mach_task_self(), (vm_address_t) names, CHILD_PORTS * sizeof(mach_port_t));
	}

	__atomic_store_n(&foreign_stop, 1, __ATOMIC_RELEASE);
	for (int i = 0; i < 3; i++)
		pthread_join(threads[i], NULL);
	SQ_KR(mach_ports_register(mach_task_self(), NULL, 0), "mach_ports_register (none)");
	sq_destroy(collector);
	printf("ok stress: rights to the ports of %d exiting children changed %ld times\n", children,
		(long) foreign_changes);
}

// with arguments, only the cases named (see main())
static bool wanted(int argc, char** argv, const char* name)
{
	if (argc < 2)
		return true;
	for (int i = 1; i < argc; i++)
		if (strcmp(argv[i], name) == 0)
			return true;
	return false;
}

int main(int argc, char** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	sq_watchdog(600);

	printf("mode: %s\n", sq_server_only() ? "DARLING_MACH_SERVER=1" : "default");

	// the first Mach messages attach the shared region; ports made afterwards are in the mode
	{
		sq_buffer_t buffer;
		mach_port_t port = sq_port(true);
		SQ_KR(sq_send(port, 1), "warm-up send");
		SQ_KR(sq_receive(port, &buffer, 1000), "warm-up receive");
		sq_destroy(port);
	}
	if (!sq_server_only())
	{
		sq_region_t region;
		SQ_CHECK(sq_region_find(&region), "the shared region is not mapped");
	}

	if (wanted(argc, argv, "too_large_task"))
		test_too_large_handoff(false);
	if (wanted(argc, argv, "too_large_server"))
		test_too_large_handoff(true);
	if (wanted(argc, argv, "reused_slot"))
		test_reused_slot_reason();
	if (wanted(argc, argv, "qlimit_raise"))
		test_qlimit_raise_race(4000);
	if (wanted(argc, argv, "foreign_rights"))
		test_foreign_rights_task_exit(20);
	if (wanted(argc, argv, "token_label"))
		test_token_label();

	printf("PASS\n");
	return 0;
}
