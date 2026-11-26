// Copyright (C) 2026 Lubos Dolezel
//
// A thread that waits in mach_msg() for a message on a port must get the message
// (or the documented error) whatever else happens to the port and to other ports
// of the task meanwhile. Darling may keep the queue of such a port in memory the
// task shares with darlingserver (see mach_sq_semantics.c); a port can leave that
// arrangement (a port set, a notification request, a destroyed or moved receive
// right) while a thread waits, and the number the arrangement uses for it can
// be given to a new port at once. Each case here has a receiver blocked in
// mach_msg() and a watchdog that reports a receive that never returns, instead
// of hanging:
//   - the port joins a port set while its receiver is suspended, and a new port
//     is created before the receiver runs again; the message sent afterwards
//     reaches the receiver, and not one sent to the new port;
//   - the same with a no-senders request as the way the port changes;
//   - the receive right is destroyed while the receiver is suspended and a new
//     port is created: the receiver gets an error, not a wait on the new port;
//   - the rights of a waiting port change (a send right is inserted) and the
//     message that follows still arrives;
//   - the port joins a port set, or gets a kqueue watcher, while one thread waits
//     on it and several threads send to it, also with messages that come through
//     the server (a message with a reply port): every message is received;
//   - receives with timeouts on ports that change that way never lose a message.
// Every case also passes with DARLING_MACH_SERVER=1, which sends every message
// through the server.

#include "mach_sq_common.h"

#include <errno.h>
#include <pthread.h>
#include <sys/event.h>

#define WAIT_MS 5000

// ---- a receiver thread with a watchdog on its result ------------------------------------------

typedef struct
{
	mach_port_t port;
	mach_msg_option_t options;
	mach_msg_timeout_t timeout;
	mach_port_t thread_port;
	volatile int started;
	volatile int done;
	mach_msg_return_t result;
	mach_msg_id_t id;
} receiver_t;

static void* receiver_main(void* arg)
{
	receiver_t* r = arg;
	sq_buffer_t buffer;

	r->thread_port = mach_thread_self();
	__atomic_store_n(&r->started, 1, __ATOMIC_SEQ_CST);
	r->result = mach_msg(&buffer.header, MACH_RCV_MSG | r->options, 0, sizeof(buffer), r->port, r->timeout,
		MACH_PORT_NULL);
	r->id = r->result == MACH_MSG_SUCCESS ? buffer.header.msgh_id : -1;
	__atomic_store_n(&r->done, 1, __ATOMIC_SEQ_CST);
	return NULL;
}

static pthread_t receiver_start(receiver_t* r, mach_port_t port, mach_msg_option_t options, mach_msg_timeout_t timeout)
{
	pthread_t thread;
	memset(r, 0, sizeof(*r));
	r->port = port;
	r->options = options;
	r->timeout = timeout;
	SQ_CHECK(pthread_create(&thread, NULL, receiver_main, r) == 0, "pthread_create");
	while (!__atomic_load_n(&r->started, __ATOMIC_SEQ_CST))
		usleep(100);
	usleep(100000); // it is waiting in the receive by now
	return thread;
}

// waits for the receiver's mach_msg() to return; a stall is a failure
static void receiver_finish(receiver_t* r, pthread_t thread, const char* what)
{
	double end = sq_now() + WAIT_MS / 1000.0;
	while (!__atomic_load_n(&r->done, __ATOMIC_SEQ_CST))
	{
		SQ_CHECK(sq_now() < end, "%s: the receive did not return in %d ms (a lost receive)", what, WAIT_MS);
		usleep(1000);
	}
	pthread_join(thread, NULL);
	mach_port_deallocate(mach_task_self(), r->thread_port);
}

// ---- a port that changes while its receiver is suspended ----------------------------------------

enum change
{
	CHANGE_PSET,
	CHANGE_NSREQUEST,
	CHANGE_DESTROY,
};

static void test_change_suspended(enum change change, const char* name)
{
	for (int round = 0; round < 3; round++)
	{
		mach_port_t port = sq_port(true);
		mach_port_t other[4], pset = MACH_PORT_NULL, notify = MACH_PORT_NULL;
		receiver_t r;
		pthread_t thread = receiver_start(&r, port, 0, 0);
		mach_port_t previous;

		SQ_KR(thread_suspend(r.thread_port), "thread_suspend");

		if (change == CHANGE_PSET)
		{
			SQ_KR(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &pset), "pset");
			SQ_KR(mach_port_move_member(mach_task_self(), port, pset), "move_member");
		}
		else if (change == CHANGE_NSREQUEST)
		{
			notify = sq_port(false);
			SQ_KR(mach_port_request_notification(mach_task_self(), port, MACH_NOTIFY_NO_SENDERS, 1000, notify,
				MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous), "no-senders request");
		}
		else
			sq_destroy(port);

		// new ports now: one of them may get what the old port had
		for (int i = 0; i < 4; i++)
			other[i] = sq_port(true);

		SQ_KR(thread_resume(r.thread_port), "thread_resume");
		usleep(50000);

		if (change == CHANGE_DESTROY)
		{
			receiver_finish(&r, thread, name);
			SQ_CHECK(r.result == MACH_RCV_PORT_DIED || r.result == MACH_RCV_PORT_CHANGED ||
				r.result == MACH_RCV_INVALID_NAME, "%s: receive result 0x%x", name, (unsigned) r.result);
		}
		else
		{
			// the first round sends nothing to the new ports: the message to the old port alone must end
			// the wait. later rounds: a message to a new port must not be taken by the receiver of the old one
			if (round > 0)
			{
				for (int i = 0; i < 4; i++)
					SQ_KR(sq_send(other[i], 900 + i), "send to new port");
				usleep(50000);
				SQ_CHECK(!__atomic_load_n(&r.done, __ATOMIC_SEQ_CST),
					"%s: a message of another port was received (id %d)", name, (int) r.id);
			}

			SQ_KR(sq_send(port, 77), "send");
			receiver_finish(&r, thread, name);
			SQ_CHECK(r.result == MACH_MSG_SUCCESS && r.id == 77, "%s: result 0x%x id %d", name,
				(unsigned) r.result, (int) r.id);

			sq_buffer_t buffer;
			for (int i = 0; round > 0 && i < 4; i++)
			{
				SQ_KR(sq_receive(other[i], &buffer, 2000), "receive on new port");
				SQ_CHECK(buffer.header.msgh_id == 900 + i, "new port id %d", (int) buffer.header.msgh_id);
			}
			sq_destroy(port);
		}
		for (int i = 0; i < 4; i++)
			sq_destroy(other[i]);
		if (pset != MACH_PORT_NULL)
			mach_port_destroy(mach_task_self(), pset);
		if (notify != MACH_PORT_NULL)
			sq_destroy(notify);
	}
	printf("ok %s\n", name);
}

// ---- the rights of a waiting port change ----------------------------------------------------------

static void test_rights_change(void)
{
	for (int round = 0; round < 20; round++)
	{
		mach_port_t port = sq_port(false);
		receiver_t r;
		pthread_t thread = receiver_start(&r, port, 0, 0);

		// the name now has a send right too: what the senders see of the port changes
		SQ_KR(mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND), "insert_right");
		SQ_KR(sq_send(port, 31), "send");
		receiver_finish(&r, thread, "rights change");
		SQ_CHECK(r.result == MACH_MSG_SUCCESS && r.id == 31, "rights change: result 0x%x id %d", (unsigned) r.result,
			(int) r.id);
		sq_destroy(port);
	}
	printf("ok rights change\n");
}

// ---- traffic while the port changes -------------------------------------------------------------------

#define TRAFFIC_SENDERS 6
#define TRAFFIC_EACH 400

static mach_port_t traffic_port;
static volatile int traffic_go;

static void* traffic_sender(void* arg)
{
	int t = (int) (intptr_t) arg;
	mach_port_t reply = sq_port(false);

	while (!__atomic_load_n(&traffic_go, __ATOMIC_SEQ_CST))
		usleep(100);
	for (int i = 0; i < TRAFFIC_EACH; i++)
	{
		mach_msg_return_t kr;
		if (i % 5 == 4)
		{
			// a message with a reply port: it goes through the server whichever way the port is kept
			mach_msg_header_t header = {
				.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE),
				.msgh_size = sizeof(header),
				.msgh_remote_port = traffic_port,
				.msgh_local_port = reply,
				.msgh_id = t * 1000 + i,
			};
			kr = mach_msg(&header, MACH_SEND_MSG, sizeof(header), 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
		}
		else
			kr = sq_send_ex(traffic_port, MACH_MSG_TYPE_COPY_SEND, sizeof(mach_msg_header_t) + 8, t * 1000 + i, 0, 0);
		if (kr == MACH_SEND_NO_BUFFER || kr == MACH_SEND_TIMED_OUT)
		{
			// a full queue: try again
			i--;
			usleep(50);
			continue;
		}
		SQ_KR(kr, "traffic send");
		if (i % 50 == 0)
			usleep(200);
	}
	return (void*) (uintptr_t) reply;
}

static void test_traffic(int how, const char* name, mach_msg_timeout_t timeout)
{
	for (int round = 0; round < 10; round++)
	{
		pthread_t senders[TRAFFIC_SENDERS];
		int next[TRAFFIC_SENDERS] = { 0 };
		mach_port_t pset = MACH_PORT_NULL;
		int kq = -1;
		sq_buffer_t buffer;

		traffic_port = sq_port(true);
		sq_set_qlimit(traffic_port, 5);
		traffic_go = 0;
		for (int t = 0; t < TRAFFIC_SENDERS; t++)
			pthread_create(&senders[t], NULL, traffic_sender, (void*) (intptr_t) t);

		int total = TRAFFIC_SENDERS * TRAFFIC_EACH;
		for (int n = 0; n < total; n++)
		{
			if (n == 0)
				__atomic_store_n(&traffic_go, 1, __ATOMIC_SEQ_CST);
			if (n == 30 + round * 20)
			{
				// the receiver is (or soon will be) waiting: the port changes
				usleep(500);
				if (how == 0)
				{
					SQ_KR(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &pset), "pset");
					SQ_KR(mach_port_move_member(mach_task_self(), traffic_port, pset), "move_member");
				}
				else
				{
					kq = kqueue();
					SQ_CHECK(kq >= 0, "kqueue");
					struct kevent ev;
					EV_SET(&ev, traffic_port, EVFILT_MACHPORT, EV_ADD | EV_ENABLE, 0, 0, NULL);
					SQ_CHECK(kevent(kq, &ev, 1, NULL, 0, NULL) == 0, "kevent add");
				}
				// a port created now may take the slot of this one
				mach_port_t extra = sq_port(true);
				sq_destroy(extra);
			}
			mach_msg_return_t kr;
			double end = sq_now() + WAIT_MS / 1000.0;
			do
			{
				kr = sq_receive_ex(traffic_port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT,
					timeout ? timeout : 200);
				if (kr == MACH_RCV_TIMED_OUT && timeout == 0)
				{
					// the timed-out wait is the one with no timeout: repeat it as long as the watchdog allows
					SQ_CHECK(sq_now() < end, "%s: no message in %d ms with %d of %d received (a lost receive)", name,
						WAIT_MS, n, total);
				}
			}
			while (kr == MACH_RCV_TIMED_OUT && timeout == 0);
			SQ_CHECK(kr == MACH_MSG_SUCCESS, "%s: receive 0x%x with %d of %d received (a lost receive)", name,
				(unsigned) kr, n, total);
			int sender = buffer.header.msgh_id / 1000;
			int index = buffer.header.msgh_id % 1000;
			SQ_CHECK(sender >= 0 && sender < TRAFFIC_SENDERS && index == next[sender],
				"%s: sender %d: got %d, expected %d", name, sender, index, sender >= 0 && sender < TRAFFIC_SENDERS ? next[sender] : -1);
			next[sender]++;
			if (MACH_MSGH_BITS_REMOTE(buffer.header.msgh_bits) == MACH_MSG_TYPE_PORT_SEND_ONCE)
				mach_port_deallocate(mach_task_self(), buffer.header.msgh_remote_port);
		}

		for (int t = 0; t < TRAFFIC_SENDERS; t++)
		{
			void* reply;
			pthread_join(senders[t], &reply);
			sq_destroy((mach_port_t) (uintptr_t) reply);
		}
		SQ_CHECK(sq_receive(traffic_port, &buffer, 1) == MACH_RCV_TIMED_OUT, "%s: extra message", name);
		if (kq >= 0)
			close(kq);
		sq_destroy(traffic_port);
		if (pset != MACH_PORT_NULL)
			mach_port_destroy(mach_task_self(), pset);
	}
	printf("ok %s\n", name);
}

// ---- a port that changes while receivers come and go, with no thread suspended -----------------

#define CHURN_ROUNDS 1500

static void test_churn(void)
{
	for (int round = 0; round < CHURN_ROUNDS; round++)
	{
		mach_port_t port = sq_port(true), pset;
		receiver_t r;
		pthread_t thread;

		memset(&r, 0, sizeof(r));
		r.port = port;
		r.timeout = 0;
		SQ_CHECK(pthread_create(&thread, NULL, receiver_main, &r) == 0, "pthread_create");
		while (!__atomic_load_n(&r.started, __ATOMIC_SEQ_CST))
			usleep(20);
		usleep(round % 7 * 40);

		SQ_KR(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &pset), "pset");
		SQ_KR(mach_port_move_member(mach_task_self(), port, pset), "move_member");
		mach_port_t again = sq_port(true);
		SQ_KR(sq_send(port, round), "send");
		receiver_finish(&r, thread, "churn");
		SQ_CHECK(r.result == MACH_MSG_SUCCESS && r.id == round, "churn %d: result 0x%x id %d", round,
			(unsigned) r.result, (int) r.id);
		sq_destroy(again);
		sq_destroy(port);
		mach_port_destroy(mach_task_self(), pset);
	}
	printf("ok churn\n");
}

int main(void)
{
	sq_watchdog(240);
	setvbuf(stdout, NULL, _IOLBF, 0);

	test_change_suspended(CHANGE_PSET, "port set while suspended");
	test_change_suspended(CHANGE_NSREQUEST, "no-senders request while suspended");
	test_change_suspended(CHANGE_DESTROY, "destroy while suspended");
	test_rights_change();
	test_traffic(0, "traffic, port set", 0);
	test_traffic(1, "traffic, kqueue", 0);
	test_traffic(0, "traffic, port set, timed receives", 1000);
	test_churn();
	printf("PASS\n");
	return 0;
}
