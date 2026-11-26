// Copyright (C) 2026 Lubos Dolezel
//
// More cases for ports whose queue Darling may keep in memory shared between
// the task and darlingserver (see mach_sq_semantics.c). Each is the documented
// mach_msg() behavior and must hold the same way with DARLING_MACH_SERVER=1:
//   - signals: with a handler (installed without SA_RESTART) running in the
//     sending and in the receiving thread all the time, every message still
//     arrives once and in order; a thread waiting in an empty queue or in a
//     full one is interrupted (MACH_RCV_INTERRUPTED, MACH_SEND_INTERRUPTED)
//     with nothing lost or duplicated, and the destination right comes back
//     the way it does for a timed out send;
//   - a signal handler that runs for a long time delays a message only for as
//     long as the handler runs. This case prints the worst delay seen and
//     fails only if a message takes seconds longer than the handler;
//   - the queue limit raised while a sender waits for room lets that sender
//     finish without anything being received; lowered below the queue length,
//     a send waits (here: times out) until enough has been received;
//   - exec: a process with receive rights, messages queued and a thread
//     waiting in a receive execs; the new image's ports work, and the parent
//     keeps working;
//   - the security and audit tokens a message carries are the ones the task
//     had when it sent it, also after the task changed its user ID.

#include "mach_sq_common.h"

#include <errno.h>
#include <mach-o/dyld.h>
#include <pthread.h>
#include <sys/wait.h>

static const mach_msg_option_t audit_trailer =
	MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT);

// ---- signals -----------------------------------------------------------------------------------

static volatile sig_atomic_t signals_seen;
static volatile unsigned handler_sleep_us;

static void on_signal(int sig)
{
	(void) sig;
	signals_seen++;
	if (handler_sleep_us)
		usleep(handler_sleep_us);
	else
		for (volatile int i = 0; i < 200; i++) {}
}

static void install_handler(void)
{
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = on_signal;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0; // no SA_RESTART
	SQ_CHECK(sigaction(SIGUSR1, &action, NULL) == 0, "sigaction");
}

static mach_port_t storm_port;
static volatile int storm_sent, storm_received, storm_interrupted;
static double storm_latency_max;

static int storm_count = 20000;
static useconds_t storm_pace_us;

static void* storm_sender(void* arg)
{
	(void) arg;
	for (int i = 0; i < storm_count; i++)
	{
		mach_msg_return_t kr;
		do
		{
			sq_buffer_t msg;
			memset(&msg, 0, sizeof(msg));
			msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
			msg.header.msgh_size = sizeof(mach_msg_header_t) + sizeof(double);
			msg.header.msgh_remote_port = storm_port;
			msg.header.msgh_id = i;
			double now = sq_now();
			memcpy(msg.body, &now, sizeof(now));
			kr = mach_msg(&msg.header, MACH_SEND_MSG, msg.header.msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
			if (kr == MACH_SEND_INTERRUPTED)
				storm_interrupted++;
		} while (kr == MACH_SEND_INTERRUPTED);
		SQ_KR(kr, "storm send");
		storm_sent = i + 1;
		if (storm_pace_us)
			usleep(storm_pace_us);
	}
	return NULL;
}

static void* storm_receiver(void* arg)
{
	(void) arg;
	for (int i = 0; i < storm_count; i++)
	{
		sq_buffer_t buffer;
		mach_msg_return_t kr;
		do
		{
			kr = sq_receive_ex(storm_port, &buffer, sizeof(buffer), 0, 0);
			if (kr == MACH_RCV_INTERRUPTED)
				storm_interrupted++;
		} while (kr == MACH_RCV_INTERRUPTED);
		SQ_KR(kr, "storm receive");
		SQ_CHECK(buffer.header.msgh_id == i, "storm: id %d, expected %d", buffer.header.msgh_id, i);
		double sent;
		memcpy(&sent, buffer.body, sizeof(sent));
		double latency = sq_now() - sent;
		if (latency > storm_latency_max)
			storm_latency_max = latency;
		storm_received = i + 1;
	}
	return NULL;
}

// signals aimed at both threads until both are done
static void storm_signals(pthread_t a, pthread_t b, useconds_t period)
{
	while (storm_received < storm_count || storm_sent < storm_count)
	{
		pthread_kill(a, SIGUSR1);
		pthread_kill(b, SIGUSR1);
		usleep(period);
	}
}

static void run_storm(useconds_t period)
{
	pthread_t sender, receiver;
	storm_port = sq_port(true);
	storm_sent = storm_received = storm_interrupted = 0;
	storm_latency_max = 0;
	signals_seen = 0;
	pthread_create(&receiver, NULL, storm_receiver, NULL);
	pthread_create(&sender, NULL, storm_sender, NULL);
	storm_signals(sender, receiver, period);
	pthread_join(sender, NULL);
	pthread_join(receiver, NULL);
	sq_buffer_t buffer;
	mach_msg_return_t kr = sq_receive_ex(storm_port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT, 0);
	SQ_CHECK(kr == MACH_RCV_TIMED_OUT, "a message too many: 0x%x", (unsigned) kr);
	SQ_CHECK(signals_seen > 0, "no signal was handled");
	sq_destroy(storm_port);
}

static void test_signal_storm(void)
{
	handler_sleep_us = 0;
	storm_count = 20000;
	storm_pace_us = 0;
	run_storm(100);
	printf("ok signals while sending and receiving (%d messages, %d signals handled, %d interrupted calls)\n",
		storm_count, (int) signals_seen, storm_interrupted);
}

// a handler that sleeps: a message is delayed by about as long as the handler runs, not much more
static void test_long_handler(void)
{
	handler_sleep_us = 30000;
	storm_count = 600;
	storm_pace_us = 1000;
	run_storm(100000);
	handler_sleep_us = 0;
	SQ_CHECK(storm_latency_max < 0.030 + 2.0, "a message took %.3f s with a 30 ms handler", storm_latency_max);
	printf("ok long signal handler (30 ms handler, %d signals, worst message delay %.1f ms)\n",
		(int) signals_seen, storm_latency_max * 1000);
}

static mach_port_t interrupt_port;
static volatile mach_msg_return_t interrupt_result;
static volatile int interrupt_waiting;

static void* interrupted_receiver(void* arg)
{
	(void) arg;
	sq_buffer_t buffer;
	interrupt_waiting = 1;
	interrupt_result = sq_receive_ex(interrupt_port, &buffer, sizeof(buffer), 0, 0);
	interrupt_waiting = 2;
	return NULL;
}

static void* interrupted_sender(void* arg)
{
	(void) arg;
	interrupt_waiting = 1;
	interrupt_result = sq_send_ex(interrupt_port, MACH_MSG_TYPE_COPY_SEND, 32, 99, 0, 0);
	interrupt_waiting = 2;
	return NULL;
}

static void wait_until_blocked(void)
{
	while (interrupt_waiting != 1)
		usleep(1000);
	usleep(100000); // blocked in the call by now
}

// the call is still waiting after the signal's handler ran (it is restarted), or it returned
// the documented interruption code; either way nothing is lost or duplicated
static int signalled_wait_done(pthread_t thread)
{
	int seen = signals_seen;
	pthread_kill(thread, SIGUSR1);
	for (int i = 0; i < 2000 && signals_seen == seen; i++)
		usleep(1000);
	SQ_CHECK(signals_seen > seen, "the handler did not run");
	usleep(200000);
	return interrupt_waiting == 2;
}

static void test_interrupted_waits(void)
{
	pthread_t thread;
	sq_buffer_t buffer;
	mach_port_urefs_t before, after;
	handler_sleep_us = 0;
	int interrupted = 0;

	// a receiver waiting in an empty queue
	interrupt_port = sq_port(true);
	interrupt_waiting = 0;
	pthread_create(&thread, NULL, interrupted_receiver, NULL);
	wait_until_blocked();
	if (signalled_wait_done(thread))
	{
		SQ_CHECK(interrupt_result == MACH_RCV_INTERRUPTED, "receive: 0x%x", (unsigned) interrupt_result);
		interrupted++;
		pthread_join(thread, NULL);
		SQ_KR(sq_send(interrupt_port, 1), "send after interrupted receive");
		SQ_KR(sq_receive(interrupt_port, &buffer, 1000), "receive after interrupted receive");
		SQ_CHECK(buffer.header.msgh_id == 1, "id %d", buffer.header.msgh_id);
	}
	else
	{
		SQ_KR(sq_send(interrupt_port, 1), "send to the waiting receiver");
		pthread_join(thread, NULL);
		SQ_KR(interrupt_result, "the waiting receiver");
	}

	// a sender waiting in a full queue
	sq_set_qlimit(interrupt_port, 2);
	SQ_KR(sq_send(interrupt_port, 10), "fill 1");
	SQ_KR(sq_send(interrupt_port, 11), "fill 2");
	mach_port_get_refs(mach_task_self(), interrupt_port, MACH_PORT_RIGHT_SEND, &before);
	interrupt_waiting = 0;
	pthread_create(&thread, NULL, interrupted_sender, NULL);
	wait_until_blocked();
	if (signalled_wait_done(thread))
	{
		SQ_CHECK(interrupt_result == MACH_SEND_INTERRUPTED, "send: 0x%x", (unsigned) interrupt_result);
		interrupted++;
		pthread_join(thread, NULL);
		mach_port_get_refs(mach_task_self(), interrupt_port, MACH_PORT_RIGHT_SEND, &after);
		// the destination right of a send that did not complete comes back, as for a timed out send
		SQ_CHECK(after == before + 1, "urefs %u -> %u", before, after);
		SQ_KR(sq_receive(interrupt_port, &buffer, 1000), "receive 10");
		SQ_CHECK(buffer.header.msgh_id == 10, "id %d, expected 10", buffer.header.msgh_id);
		SQ_KR(sq_receive(interrupt_port, &buffer, 1000), "receive 11");
		SQ_CHECK(buffer.header.msgh_id == 11, "id %d, expected 11", buffer.header.msgh_id);
		SQ_CHECK(sq_receive(interrupt_port, &buffer, 50) == MACH_RCV_TIMED_OUT, "the interrupted message was queued");
	}
	else
	{
		// still waiting: room appears, and the message is sent once
		SQ_KR(sq_receive(interrupt_port, &buffer, 1000), "receive 10");
		SQ_CHECK(buffer.header.msgh_id == 10, "id %d, expected 10", buffer.header.msgh_id);
		pthread_join(thread, NULL);
		SQ_KR(interrupt_result, "the waiting sender");
		SQ_KR(sq_receive(interrupt_port, &buffer, 1000), "receive 11");
		SQ_CHECK(buffer.header.msgh_id == 11, "id %d, expected 11", buffer.header.msgh_id);
		SQ_KR(sq_receive(interrupt_port, &buffer, 1000), "receive 99");
		SQ_CHECK(buffer.header.msgh_id == 99, "id %d, expected 99", buffer.header.msgh_id);
		SQ_CHECK(sq_receive(interrupt_port, &buffer, 50) == MACH_RCV_TIMED_OUT, "a message too many");
	}
	sq_destroy(interrupt_port);
	printf("ok signals while waiting in a receive and in a send (%d of 2 interrupted)\n", interrupted);
}

// ---- the queue limit changes while a sender waits ----------------------------------------------------

static mach_port_t limit_port;
static volatile int limit_sent;

static void* limit_sender(void* arg)
{
	(void) arg;
	for (int i = 0; i < 6; i++)
	{
		SQ_KR(sq_send(limit_port, 100 + i), "limit send");
		limit_sent = i + 1;
	}
	return NULL;
}

static mach_port_msgcount_t queued_messages(mach_port_t port)
{
	mach_port_status_t status;
	mach_msg_type_number_t count = MACH_PORT_RECEIVE_STATUS_COUNT;
	SQ_KR(mach_port_get_attributes(mach_task_self(), port, MACH_PORT_RECEIVE_STATUS, (mach_port_info_t) &status,
		&count), "get_attributes");
	return status.mps_msgcount;
}

static void test_qlimit_raised(void)
{
	pthread_t thread;
	sq_buffer_t buffer;

	limit_port = sq_port(true);
	sq_set_qlimit(limit_port, 2);
	limit_sent = 0;
	pthread_create(&thread, NULL, limit_sender, NULL);
	for (int i = 0; i < 300 && limit_sent < 2; i++)
		usleep(1000);
	usleep(200000);
	SQ_CHECK(limit_sent == 2, "the sender is not waiting at the limit (%d sent)", limit_sent);

	// room for the rest, nothing received
	sq_set_qlimit(limit_port, 10);
	for (int i = 0; i < 500 && limit_sent < 6; i++)
		usleep(1000);
	SQ_CHECK(limit_sent == 6, "after the limit was raised, %d of 6 sent", limit_sent);
	pthread_join(thread, NULL);
	mach_port_msgcount_t queued = queued_messages(limit_port);
	SQ_CHECK(queued == 6, "queued: %u", queued);
	for (int i = 0; i < 6; i++)
	{
		SQ_KR(sq_receive(limit_port, &buffer, 1000), "limit receive");
		SQ_CHECK(buffer.header.msgh_id == 100 + i, "id %d, expected %d", buffer.header.msgh_id, 100 + i);
	}

	// lowered below the queue length: a send with a timeout times out until enough has been received
	for (int i = 0; i < 4; i++)
		SQ_KR(sq_send(limit_port, 200 + i), "fill");
	sq_set_qlimit(limit_port, 2);
	SQ_CHECK(sq_send_ex(limit_port, MACH_MSG_TYPE_COPY_SEND, 32, 204, MACH_SEND_TIMEOUT, 0) == MACH_SEND_TIMED_OUT,
		"send above a lowered limit");
	for (int i = 0; i < 3; i++)
	{
		SQ_KR(sq_receive(limit_port, &buffer, 1000), "receive");
		SQ_CHECK(buffer.header.msgh_id == 200 + i, "id %d, expected %d", buffer.header.msgh_id, 200 + i);
	}
	SQ_KR(sq_send_ex(limit_port, MACH_MSG_TYPE_COPY_SEND, 32, 205, MACH_SEND_TIMEOUT, 0), "send below the limit");
	SQ_KR(sq_receive(limit_port, &buffer, 1000), "receive 203");
	SQ_CHECK(buffer.header.msgh_id == 203, "id %d, expected 203", buffer.header.msgh_id);
	SQ_KR(sq_receive(limit_port, &buffer, 1000), "receive 205");
	SQ_CHECK(buffer.header.msgh_id == 205, "id %d, expected 205", buffer.header.msgh_id);
	sq_destroy(limit_port);
	printf("ok queue limit raised while a sender waits, and lowered\n");
}

// ---- exec -----------------------------------------------------------------------------------------

static void* exec_waiter(void* arg)
{
	sq_buffer_t buffer;
	sq_receive_ex((mach_port_t) (uintptr_t) arg, &buffer, sizeof(buffer), 0, 0);
	return NULL;
}

// the image an exec leads to: a new task whose ports work, locally and through the server
static int exec_second_image(void)
{
	sq_buffer_t buffer;
	mach_port_t port = sq_port(true);

	int mapped = sq_region_mapped();
	if (sq_server_only())
		SQ_CHECK(mapped == 0, "region mapped with DARLING_MACH_SERVER=1");
	else
		SQ_CHECK(mapped == 1, "the new image has no region");

	for (int i = 0; i < 1000; i++)
	{
		SQ_KR(sq_send(port, i), "exec: send");
		SQ_KR(sq_receive(port, &buffer, 1000), "exec: receive");
		SQ_CHECK(buffer.header.msgh_id == i, "exec: id %d, expected %d", buffer.header.msgh_id, i);
	}
	mach_port_t set;
	SQ_KR(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &set), "port set");
	SQ_KR(sq_send(port, 7), "exec: send 7");
	SQ_KR(mach_port_move_member(mach_task_self(), port, set), "move_member");
	SQ_KR(sq_receive(set, &buffer, 1000), "exec: receive from the set");
	SQ_CHECK(buffer.header.msgh_id == 7, "exec: set id %d", buffer.header.msgh_id);
	printf("ok exec image (%s)\n", mapped ? "region mapped" : "no region");
	return 0;
}

static void test_exec(const char* self)
{
	pid_t pid = fork();
	SQ_CHECK(pid >= 0, "fork");
	if (pid == 0)
	{
		// a region with queued messages, a full queue and a thread waiting in a receive
		mach_port_t queued = sq_port(true);
		mach_port_t waited = sq_port(true);
		mach_port_t full = sq_port(true);
		sq_set_qlimit(full, 1);
		for (int i = 0; i < 3; i++)
			SQ_KR(sq_send(queued, i), "child send");
		SQ_KR(sq_send(full, 0), "child fill");
		pthread_t thread;
		pthread_create(&thread, NULL, exec_waiter, (void*) (uintptr_t) waited);
		usleep(100000);
		char* args[] = { (char*) self, "--exec-image", NULL };
		execv(self, args);
		printf("FAIL: execv: %d\n", errno);
		_exit(1);
	}
	int status = 0;
	SQ_CHECK(waitpid(pid, &status, 0) == pid, "waitpid");
	SQ_CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "the exec'd child: status 0x%x", status);

	// the parent is unaffected
	sq_buffer_t buffer;
	mach_port_t port = sq_port(true);
	SQ_KR(sq_send(port, 5), "parent send");
	SQ_KR(sq_receive(port, &buffer, 1000), "parent receive");
	SQ_CHECK(buffer.header.msgh_id == 5, "parent id");
	sq_destroy(port);
	printf("ok exec with a region attached\n");
}

// ---- tokens --------------------------------------------------------------------------------------

static uid_t token_euid(mach_port_t port)
{
	sq_buffer_t buffer;
	SQ_KR(sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT | audit_trailer, 1000), "token receive");
	const mach_msg_audit_trailer_t* trailer =
		(const mach_msg_audit_trailer_t*) ((const uint8_t*) &buffer + buffer.header.msgh_size);
	return trailer->msgh_audit.val[1];
}

static void test_token_change(void)
{
	mach_port_t port = sq_port(true);
	uid_t original = geteuid();
	uid_t other = original + 1;

	// two messages queued under the old user ID, one sent under the new
	SQ_KR(sq_send(port, 1), "token send 1");
	SQ_KR(sq_send(port, 2), "token send 2");
	if (setuid(other) != 0)
	{
		printf("skip token change: setuid is not permitted (errno %d)\n", errno);
		sq_destroy(port);
		return;
	}
	SQ_KR(sq_send(port, 3), "token send 3");
	uid_t first = token_euid(port), second = token_euid(port), third = token_euid(port);
	SQ_CHECK(setuid(original) == 0, "setuid back");
	SQ_KR(sq_send(port, 4), "token send 4");
	uid_t fourth = token_euid(port);
	SQ_CHECK(first == original && second == original, "queued before the change: %u, %u (user %u)", first, second, original);
	SQ_CHECK(third == other, "sent after the change: %u (expected %u)", third, other);
	SQ_CHECK(fourth == original, "sent after the change back: %u (expected %u)", fourth, original);
	sq_destroy(port);
	printf("ok tokens follow the user ID (%u -> %u -> %u)\n", original, other, original);
}

int main(int argc, char** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	sq_watchdog(300);

	if (argc > 1 && strcmp(argv[1], "--exec-image") == 0)
		return exec_second_image();

	char self[4096];
	uint32_t size = sizeof(self);
	SQ_CHECK(_NSGetExecutablePath(self, &size) == 0, "_NSGetExecutablePath");

	printf("mode: %s\n", sq_server_only() ? "DARLING_MACH_SERVER=1" : "default");
	install_handler();

	test_signal_storm();
	test_long_handler();
	test_interrupted_waits();
	test_qlimit_raised();
	test_exec(self);
	test_token_change();

	printf("PASS\n");
	return 0;
}
