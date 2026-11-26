// Copyright (C) 2026 Lubos Dolezel
//
// Throughput of simple Mach messages between threads of one task, for
// comparing Darling's message paths (run it with and without
// DARLING_MACH_SERVER=1; not a pass/fail test beyond completing):
// - ping-pong: two threads, each receiving on its own port and sending to the
//   other's; round trips per second;
// - event ping-pong: the same with WaitableEvent-style events (a port with a
//   queue limit of 1; signal = send with MACH_SEND_TIMEOUT 0, which times out
//   if the event is already signaled; wait = receive); round trips per second;
// - signal+wait: one thread signals and then waits on one event; pairs per
//   second (no thread switch);
// - poll: a receive with timeout 0 on an empty port (an IsSignaled() check);
//   calls per second.
// Usage: mach_sq_bench [seconds]

#include "mach_sq_common.h"

#include <pthread.h>

static double seconds = 2.0;
static volatile int stop;
static mach_port_t ports[2];
static long rounds[2];

static void signal_event(mach_port_t port)
{
	mach_msg_return_t mr = sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, sizeof(mach_msg_header_t), 0,
		MACH_SEND_TIMEOUT, 0);
	SQ_CHECK(mr == MACH_MSG_SUCCESS || mr == MACH_SEND_TIMED_OUT, "signal: 0x%x", mr);
}

static void wait_event(mach_port_t port)
{
	sq_buffer_t buffer;
	SQ_KR(sq_receive_ex(port, &buffer, sizeof(buffer), 0, 0), "wait");
}

static void* pinger(void* arg)
{
	int me = (int) (intptr_t) arg;
	sq_buffer_t buffer;
	long n = 0;

	while (!stop)
	{
		if (me == 0)
			SQ_KR(sq_send(ports[1], 0), "send");
		SQ_KR(sq_receive(ports[me], &buffer, 0), "receive");
		if (buffer.header.msgh_id == 1)
			break;
		if (me == 1)
			SQ_KR(sq_send(ports[0], 0), "send");
		n++;
	}
	rounds[me] = n;
	return NULL;
}

static void* event_pinger(void* arg)
{
	int me = (int) (intptr_t) arg;
	long n = 0;

	while (!stop)
	{
		if (me == 0)
			signal_event(ports[1]);
		wait_event(ports[me]);
		if (me == 1)
			signal_event(ports[0]);
		n++;
	}
	rounds[me] = n;
	return NULL;
}

static double run_pair(void* (*body)(void*), const char* what)
{
	pthread_t threads[2];
	ports[0] = sq_port(true);
	ports[1] = sq_port(true);
	stop = 0;

	double start = sq_now();
	for (int t = 0; t < 2; t++)
		pthread_create(&threads[t], NULL, body, (void*) (intptr_t) t);
	usleep((useconds_t) (seconds * 1e6));
	stop = 1;
	// whichever thread waits gets one more message and sees `stop`
	sq_send_ex(ports[0], MACH_MSG_TYPE_COPY_SEND, sizeof(mach_msg_header_t), 1, MACH_SEND_TIMEOUT, 0);
	sq_send_ex(ports[1], MACH_MSG_TYPE_COPY_SEND, sizeof(mach_msg_header_t), 1, MACH_SEND_TIMEOUT, 0);
	for (int t = 0; t < 2; t++)
		pthread_join(threads[t], NULL);
	double elapsed = sq_now() - start;

	sq_destroy(ports[0]);
	sq_destroy(ports[1]);
	double rate = rounds[0] / elapsed;
	printf("%-14s %10.0f round trips/s\n", what, rate);
	return rate;
}

int main(int argc, char** argv)
{
	if (argc > 1)
		seconds = atof(argv[1]);
	sq_watchdog((unsigned) (seconds * 4 + 60));
	printf("mode: %s\n", sq_server_only() ? "DARLING_MACH_SERVER=1" : "default");

	run_pair(pinger, "ping-pong");

	// events: a queue limit of 1
	{
		pthread_t threads[2];
		ports[0] = sq_port(false);
		ports[1] = sq_port(false);
		sq_set_qlimit(ports[0], 1);
		sq_set_qlimit(ports[1], 1);
		stop = 0;
		double start = sq_now();
		for (int t = 0; t < 2; t++)
			pthread_create(&threads[t], NULL, event_pinger, (void*) (intptr_t) t);
		usleep((useconds_t) (seconds * 1e6));
		stop = 1;
		signal_event(ports[0]);
		signal_event(ports[1]);
		for (int t = 0; t < 2; t++)
			pthread_join(threads[t], NULL);
		double elapsed = sq_now() - start;
		printf("%-14s %10.0f round trips/s\n", "event pingpong", rounds[0] / elapsed);
		sq_destroy(ports[0]);
		sq_destroy(ports[1]);
	}

	{
		mach_port_t event = sq_port(false);
		sq_set_qlimit(event, 1);
		long n = 0;
		double start = sq_now(), elapsed;
		do
		{
			for (int i = 0; i < 1000; i++)
			{
				signal_event(event);
				wait_event(event);
			}
			n += 1000;
			elapsed = sq_now() - start;
		} while (elapsed < seconds);
		printf("%-14s %10.0f pairs/s\n", "signal+wait", n / elapsed);

		sq_buffer_t buffer;
		n = 0;
		start = sq_now();
		do
		{
			for (int i = 0; i < 1000; i++)
				SQ_CHECK(sq_receive_ex(event, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT, 0) == MACH_RCV_TIMED_OUT, "poll");
			n += 1000;
			elapsed = sq_now() - start;
		} while (elapsed < seconds);
		printf("%-14s %10.0f calls/s\n", "poll", n / elapsed);
		sq_destroy(event);
	}

	printf("PASS\n");
	return 0;
}
