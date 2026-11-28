// Copyright (C) 2026 Lubos Dolezel
//
// kqueue(2): an EVFILT_SIGNAL filter reports "the number of times the signal
// occurred since the last call to kevent()", also for a signal that is
// ignored. A filter that is disabled still records the signals; they are
// reported once it is enabled again. libdispatch disables a signal source's
// filter (EV_DISPATCH) while the event handler runs, so signals that arrive
// during a long handler must not be lost.
//
// Part 1 (kevent): registers SIGUSR1 with EV_DISPATCH, takes the event,
// sends two more signals while the filter is disabled and expects them to be
// reported by the next kevent after EV_ENABLE (as one or more occurrences:
// signals that are pending at the same time may be merged).
// Part 2 (dispatch): a signal source whose handler takes 300 ms. A child
// process sends a signal, and another 100 ms later while the handler runs;
// the handler must run a second time for it. Repeated for several pairs.
// Build for x86_64 and i386; prints PASS or FAIL.
#include <dispatch/dispatch.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void part_kevent(void)
{
	signal(SIGUSR1, SIG_IGN);

	int kq = kqueue();
	CHECK(kq >= 0);

	struct kevent ev;
	EV_SET(&ev, SIGUSR1, EVFILT_SIGNAL, EV_ADD | EV_DISPATCH, 0, 0, NULL);
	CHECK(kevent(kq, &ev, 1, NULL, 0, NULL) == 0);

	struct timespec wait = { 2, 0 };
	struct timespec none = { 0, 0 };
	struct kevent out;

	kill(getpid(), SIGUSR1);
	int n = kevent(kq, NULL, 0, &out, 1, &wait);
	CHECK(n == 1);
	CHECK(n == 1 && out.ident == SIGUSR1 && out.filter == EVFILT_SIGNAL && out.data == 1);

	// EV_DISPATCH disabled the filter when it delivered the event.
	kill(getpid(), SIGUSR1);
	kill(getpid(), SIGUSR1);
	usleep(100000);
	CHECK(kevent(kq, NULL, 0, &out, 1, &none) == 0);

	EV_SET(&ev, SIGUSR1, EVFILT_SIGNAL, EV_ENABLE, 0, 0, NULL);
	CHECK(kevent(kq, &ev, 1, NULL, 0, NULL) == 0);
	n = kevent(kq, NULL, 0, &out, 1, &wait);
	printf("kevent: %d event(s), data=%ld after two signals while disabled\n", n, n == 1 ? (long) out.data : -1L);
	CHECK(n == 1);
	CHECK(n == 1 && out.data >= 1);

	close(kq);
}

#define PAIRS 4

static void part_dispatch(void)
{
	__block int runs = 0;
	dispatch_queue_t q = dispatch_queue_create("signal-busy", DISPATCH_QUEUE_SERIAL);

	signal(SIGUSR1, SIG_IGN);
	dispatch_source_t src = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGUSR1, 0, q);
	dispatch_source_set_event_handler(src, ^{
		runs++;
		usleep(300000);
	});
	dispatch_resume(src);
	usleep(200000);

	pid_t parent = getpid();
	pid_t child = fork();
	if (child == 0)
	{
		for (int i = 0; i < PAIRS; i++)
		{
			kill(parent, SIGUSR1);
			usleep(100000);
			kill(parent, SIGUSR1);
			usleep(1200000);
		}
		_exit(0);
	}
	waitpid(child, NULL, 0);

	usleep(500000);
	dispatch_sync(q, ^{ });
	printf("dispatch: %d pairs of signals, handler ran %d time(s)\n", PAIRS, runs);
	CHECK(runs == 2 * PAIRS);
}

int main(void)
{
	part_kevent();
	part_dispatch();
	printf("%s\n", g_failures ? "FAIL" : "PASS");
	return g_failures ? 1 : 0;
}
