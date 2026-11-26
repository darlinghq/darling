// Copyright (C) 2026 Lubos Dolezel
//
// Wakeups and races around port sets whose members Darling keeps in memory the
// task shares with darlingserver (see mach_sq_sets.c). Each case is documented
// (or XNU) behavior of mach_msg() on a port set and also holds with
// DARLING_MACH_SERVER=1:
//   - a member's queue is taken out of the shared memory on its own (the server
//     could not apply a rights change because the task kept the queue busy; the
//     test makes that happen by marking the queue closed itself): a message
//     already queued on the member is still received through the set. Forced
//     only in the default mode;
//   - the shared memory is disabled for the task (the server found a damaged
//     queue; the test damages one) while a thread waits on a set: the waiter
//     still receives the next message sent to a member. Forced only in the
//     default mode, in a child process;
//   - two threads wait on a set, the first with MACH_RCV_LARGE and a buffer too
//     small for the message that arrives on a member: the second receives it;
//   - a thread waits on a set; while it is suspended the set gets a kqueue
//     watcher, and a new set is created and destroyed: the thread still waits
//     for its set, which is alive, and receives the next message;
//   - stress: two threads receive on a set through darlingserver while the set
//     leaves the shared memory and messages arrive on a member: every message is
//     received once. The race this guards against needs two server threads, which
//     darlingserver's default build does not have; the case checks the path.
// Each case prints a line; the program prints PASS and exits 0 when all pass.

#include "mach_sq_region.h"

#include <errno.h>
#include <pthread.h>
#include <sys/event.h>
#include <sys/wait.h>

#define WAIT_MS 3000

// libsystem_kernel's trap (mach_msg() calls it); its declaration in the SDK is private
extern mach_msg_return_t mach_msg_trap(mach_msg_header_t* msg, mach_msg_option_t option, mach_msg_size_t send_size,
	mach_msg_size_t rcv_size, mach_port_name_t rcv_name, mach_msg_timeout_t timeout, mach_port_name_t notify);

static mach_port_t new_set(void)
{
	mach_port_t set;
	SQ_KR(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &set), "port set");
	return set;
}

static void join_set(mach_port_t port, mach_port_t set)
{
	SQ_KR(mach_port_insert_member(mach_task_self(), port, set), "insert_member");
}

// ---- a receiver thread --------------------------------------------------------------------------

typedef struct
{
	mach_port_t port;
	mach_msg_option_t options;
	mach_msg_size_t size;
	mach_msg_timeout_t timeout;
	mach_port_t thread_port;
	volatile int started;
	volatile int done;
	mach_msg_return_t result;
	mach_msg_id_t id;
	mach_port_t local;
} receiver_t;

static void* receiver_main(void* arg)
{
	receiver_t* r = arg;
	sq_buffer_t buffer;

	memset(&buffer, 0, sizeof(buffer));
	r->thread_port = mach_thread_self();
	__atomic_store_n(&r->started, 1, __ATOMIC_SEQ_CST);
	r->result = mach_msg(&buffer.header, MACH_RCV_MSG | r->options | (r->timeout ? MACH_RCV_TIMEOUT : 0), 0, r->size,
		r->port, r->timeout, MACH_PORT_NULL);
	r->id = r->result == MACH_MSG_SUCCESS ? buffer.header.msgh_id : -1;
	r->local = buffer.header.msgh_local_port;
	__atomic_store_n(&r->done, 1, __ATOMIC_SEQ_CST);
	return NULL;
}

static pthread_t receiver_start(receiver_t* r, mach_port_t port, mach_msg_option_t options, mach_msg_size_t size,
		mach_msg_timeout_t timeout)
{
	pthread_t thread;
	memset(r, 0, sizeof(*r));
	r->port = port;
	r->options = options;
	r->size = size;
	r->timeout = timeout;
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

// ---- a member leaves the shared memory on its own -----------------------------------------------

static void test_member_closed_alone(void)
{
	mach_port_t set = new_set();
	mach_port_t member = sq_port(false); // a receive right only: the send right comes later
	mach_port_t other = sq_port(true);
	sq_region_t region;
	sq_buffer_t buffer;
	bool forced = false;

	join_set(member, set);
	join_set(other, set);
	SQ_KR(sq_send_ex(member, MACH_MSG_TYPE_MAKE_SEND, 32, 31, 0, 0), "send to the member");

	if (sq_region_find(&region))
	{
		int slot = sq_region_slot(&region, member);
		SQ_CHECK(slot >= 0, "the member is not in the shared queue");
		// what the server finds when the task keeps the queue busy for longer than it tries
		sq_region_close_head(&region, slot);
		forced = true;
	}
	// a rights change the server can't apply to the busy queue: it closes the member's queue instead
	SQ_KR(mach_port_insert_right(mach_task_self(), member, member, MACH_MSG_TYPE_MAKE_SEND), "insert_right");

	mach_msg_return_t mr = sq_receive(set, &buffer, WAIT_MS);
	SQ_CHECK(mr == MACH_MSG_SUCCESS, "receive on the set: 0x%x (the member's message was not seen)", (unsigned) mr);
	SQ_CHECK(buffer.header.msgh_id == 31 && buffer.header.msgh_local_port == member, "id %d from 0x%x",
		buffer.header.msgh_id, buffer.header.msgh_local_port);
	SQ_CHECK(sq_receive(set, &buffer, 50) == MACH_RCV_TIMED_OUT, "a message too many");

	sq_destroy(member);
	sq_destroy(other);
	sq_destroy(set);
	printf("ok a member whose queue left the shared memory alone is still received from through its set%s\n",
		forced ? "" : " (not forced: no shared memory)");
}

// ---- the shared memory is disabled while a thread waits on a set --------------------------------

static int poisoned_child(void)
{
	mach_port_t set = new_set();
	mach_port_t member = sq_port(true);
	mach_port_t unrelated = sq_port(true);
	sq_region_t region;
	receiver_t r;

	join_set(member, set);
	// a timeout far beyond what the case waits for: the receive has to be woken, not time out
	pthread_t thread = receiver_start(&r, set, 0, sizeof(sq_buffer_t), 20000);

	SQ_CHECK(sq_region_find(&region), "no shared region");
	int slot = sq_region_slot(&region, unrelated);
	SQ_CHECK(slot >= 0, "the port is not in the shared queue");
	// a queue only the server may close is closed: the server disables the shared memory of the task when it
	// sends the next message there
	sq_region_close_head(&region, slot);
	SQ_KR(sq_send(unrelated, 1), "send to the damaged queue");
	usleep(200000);

	SQ_KR(sq_send(member, 41), "send to the member");
	SQ_CHECK(receiver_wait(&r, WAIT_MS), "the receive on the set did not return in %d ms after the message was sent "
		"(it slept through the disabling)", WAIT_MS);
	SQ_CHECK(r.result == MACH_MSG_SUCCESS, "the waiting receive on the set: 0x%x (it slept through the disabling)",
		(unsigned) r.result);
	SQ_CHECK(r.id == 41, "id %d", r.id);
	receiver_join(&r, thread);
	return 0;
}

static void test_poisoned_with_set_waiter(void)
{
	if (sq_server_only())
	{
		printf("ok shared memory disabled while a thread waits on a set (not forced: no shared memory)\n");
		return;
	}
	pid_t pid = fork();
	SQ_CHECK(pid >= 0, "fork");
	if (pid == 0)
	{
		// the child has its own region; its first messages attach it
		sq_buffer_t buffer;
		mach_port_t port = sq_port(true);
		SQ_KR(sq_send(port, 1), "child warm-up send");
		SQ_KR(sq_receive(port, &buffer, 1000), "child warm-up receive");
		sq_destroy(port);
		_exit(poisoned_child());
	}
	int status = 0;
	waitpid(pid, &status, 0);
	SQ_CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "the child failed (status 0x%x)", status);
	printf("ok shared memory disabled while a thread waits on a set: the waiter receives the next message\n");
}

// ---- a set receiver that leaves a too large message queued ----------------------------------------

static void test_set_too_large_handoff(void)
{
	mach_port_t set = new_set();
	mach_port_t member = sq_port(true);
	receiver_t small, big;
	sq_buffer_t buffer;

	join_set(member, set);
	pthread_t small_thread = receiver_start(&small, set, MACH_RCV_LARGE, 64, 0);
	pthread_t big_thread = receiver_start(&big, set, 0, sizeof(sq_buffer_t), 0);

	SQ_KR(sq_send_ex(member, MACH_MSG_TYPE_COPY_SEND, 200, 7, 0, 0), "send");
	SQ_CHECK(receiver_wait(&big, WAIT_MS), "a message too large for one receiver waiting on the set was not received "
		"by the other one in %d ms (the wake was not passed on)", WAIT_MS);
	SQ_KR(big.result, "the receiver with room");
	SQ_CHECK(big.id == 7 && big.local == member, "id %d from 0x%x", big.id, big.local);
	receiver_join(&big, big_thread);

	if (receiver_wait(&small, 500))
		SQ_CHECK(small.result == MACH_RCV_TOO_LARGE, "the small receiver: 0x%x", (unsigned) small.result);
	else
	{
		SQ_KR(sq_send(member, 8), "release the small receiver");
		SQ_CHECK(receiver_wait(&small, WAIT_MS), "the small receiver did not return");
		SQ_KR(small.result, "the small receiver");
	}
	receiver_join(&small, small_thread);
	SQ_CHECK(sq_receive(set, &buffer, 50) == MACH_RCV_TIMED_OUT, "a message too many");

	sq_destroy(member);
	sq_destroy(set);
	printf("ok too large for the first receiver on a set, received by the second\n");
}

// ---- the set's slot is reused and closed again while its receiver waits -------------------------

static void test_set_reused_slot_reason(void)
{
	mach_port_t set = new_set();
	mach_port_t member = sq_port(true);
	receiver_t r;
	struct kevent change, event;
	struct timespec zero = { 0, 0 };

	join_set(member, set);
	pthread_t thread = receiver_start(&r, set, 0, sizeof(sq_buffer_t), 0);

	SQ_KR(thread_suspend(r.thread_port), "thread_suspend");
	// a kqueue watcher: the set leaves the shared memory (neither moved nor destroyed)
	int kq = kqueue();
	SQ_CHECK(kq >= 0, "kqueue");
	EV_SET(&change, set, EVFILT_MACHPORT, EV_ADD, 0, 0, 0);
	SQ_CHECK(kevent(kq, &change, 1, &event, 0, &zero) == 0, "kevent add: errno %d", errno);
	// a new set gets the number the set used, and is destroyed
	mach_port_t other = new_set();
	sq_destroy(other);
	SQ_KR(thread_resume(r.thread_port), "thread_resume");

	SQ_CHECK(!receiver_wait(&r, 300), "the receive on a live set returned 0x%x after another set was destroyed",
		(unsigned) r.result);
	SQ_KR(sq_send(member, 51), "send");
	SQ_CHECK(receiver_wait(&r, WAIT_MS), "the receive did not return in %d ms", WAIT_MS);
	SQ_KR(r.result, "receive");
	SQ_CHECK(r.id == 51 && r.local == member, "id %d from 0x%x", r.id, r.local);
	receiver_join(&r, thread);

	close(kq);
	sq_destroy(member);
	sq_destroy(set);
	printf("ok a waiting set receiver keeps its set when the set's queue number is reused and destroyed\n");
}

// ---- stress: receives through the server while the set leaves the shared memory -------------------

static mach_port_t stress_set;
static volatile int stress_go, stress_received;
static volatile mach_msg_return_t stress_error;

static void* stress_receiver(void* arg)
{
	(void) arg;
	for (int i = 1;; i++)
	{
		while (__atomic_load_n(&stress_go, __ATOMIC_ACQUIRE) != i)
		{
			if (__atomic_load_n(&stress_go, __ATOMIC_ACQUIRE) < 0)
				return NULL;
		}
		sq_buffer_t buffer;
		// MACH_RCV_INTERRUPT (which mach_msg() takes away) makes the receive go to darlingserver as it is
		mach_msg_return_t mr = mach_msg_trap(&buffer.header, MACH_RCV_MSG | MACH_RCV_INTERRUPT | MACH_RCV_TIMEOUT, 0,
			sizeof(buffer), stress_set, WAIT_MS, MACH_PORT_NULL);
		if (mr == MACH_MSG_SUCCESS)
			__atomic_add_fetch(&stress_received, 1, __ATOMIC_ACQ_REL);
		else
			stress_error = mr;
	}
}

static void test_server_set_receives(int rounds)
{
	pthread_t threads[2];

	stress_go = 0;
	stress_received = 0;
	stress_error = MACH_MSG_SUCCESS;
	for (int i = 0; i < 2; i++)
		SQ_CHECK(pthread_create(&threads[i], NULL, stress_receiver, NULL) == 0, "pthread_create");

	for (int i = 1; i <= rounds; i++)
	{
		mach_port_t quiet = sq_port(true), busy = sq_port(true);
		stress_set = new_set();
		join_set(quiet, stress_set);
		join_set(busy, stress_set);
		__atomic_store_n(&stress_received, 0, __ATOMIC_RELEASE);
		__atomic_store_n(&stress_go, i, __ATOMIC_RELEASE);
		for (volatile int k = 0; k < (i % 50) * 20; k++)
			;
		SQ_KR(sq_send(busy, 2 * i), "send 1");
		SQ_KR(sq_send(busy, 2 * i + 1), "send 2");

		double end = sq_now() + (WAIT_MS + 1000) / 1000.0;
		while (__atomic_load_n(&stress_received, __ATOMIC_ACQUIRE) < 2)
		{
			SQ_CHECK(stress_error == MACH_MSG_SUCCESS, "round %d: a receive returned 0x%x", i, (unsigned) stress_error);
			SQ_CHECK(sq_now() < end, "round %d: %d of 2 messages received", i, stress_received);
			usleep(100);
		}
		sq_destroy(quiet);
		sq_destroy(busy);
		sq_destroy(stress_set);
	}
	__atomic_store_n(&stress_go, -1, __ATOMIC_RELEASE);
	for (int i = 0; i < 2; i++)
		pthread_join(threads[i], NULL);
	printf("ok stress: two server receives on a set leaving the shared memory, %d rounds\n", rounds);
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

	if (wanted(argc, argv, "member_closed"))
		test_member_closed_alone();
	if (wanted(argc, argv, "too_large"))
		test_set_too_large_handoff();
	if (wanted(argc, argv, "reused_slot"))
		test_set_reused_slot_reason();
	if (wanted(argc, argv, "server_receives"))
		test_server_set_receives(1000);
	// last: it disables the shared memory, of a child process
	if (wanted(argc, argv, "poisoned"))
		test_poisoned_with_set_waiter();

	printf("PASS\n");
	return 0;
}
