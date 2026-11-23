// Copyright (C) 2026 Lubos Dolezel
//
// Throughput of the path from CGEventPost() to ReceiveNextEvent() in a pure Carbon process:
// 100000 mouse moved events in bursts of 100, posted and then taken by the thread that waits,
// with mouse coalescing off, in microseconds per event; and the same with the events made by
// CreateEvent() and posted straight to the main queue, as the reference for what the
// CoreGraphics hop costs. With coalescing on, the queue holds at most one moved event after a
// burst. Prints the numbers; fails above 10 microseconds per event (the design limit).
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <time.h>

OSStatus SetMouseCoalescingEnabled(Boolean inNewState, Boolean* outOldState);

enum { kTotal = 100000, kBurst = 100 };

static double now(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

static int received(void)
{
	int count = 0;
	EventRef event;

	while (ReceiveNextEvent(0, NULL, count == 0 ? 2.0 : 0.0, true, &event) == noErr)
	{
		count++;
		ReleaseEvent(event);
	}
	return count;
}

int main(void)
{
	Boolean old;
	int failures = 0;

	received();   // registers the reader
	SetMouseCoalescingEnabled(false, &old);

	double start = now();
	int got = 0;
	for (int burst = 0; burst < kTotal / kBurst; burst++)
	{
		for (int i = 0; i < kBurst; i++)
		{
			CGEventRef event = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, CGPointMake(i, burst), kCGMouseButtonLeft);

			CGEventPost(kCGSessionEventTap, event);
			CFRelease(event);
		}
		got += received();
	}
	double viaStream = (now() - start) / kTotal * 1e6;
	if (got != kTotal)
	{
		printf("FAIL: %d events of %d arrived\n", got, kTotal);
		failures++;
	}

	start = now();
	got = 0;
	for (int burst = 0; burst < kTotal / kBurst; burst++)
	{
		for (int i = 0; i < kBurst; i++)
		{
			EventRef event;

			CreateEvent(NULL, kEventClassMouse, kEventMouseMoved, 0, kEventAttributeNone, &event);
			PostEventToQueue(GetMainEventQueue(), event, kEventPriorityStandard);
			ReleaseEvent(event);
		}
		got += received();
	}
	double direct = (now() - start) / kTotal * 1e6;

	printf("CGEventPost -> ReceiveNextEvent: %.2f us per event\n", viaStream);
	printf("PostEventToQueue -> ReceiveNextEvent (reference): %.2f us per event\n", direct);

	// Coalescing on: one moved event stays of a burst
	SetMouseCoalescingEnabled(true, NULL);
	for (int i = 0; i < kBurst; i++)
	{
		CGEventRef event = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, CGPointMake(i, 0), kCGMouseButtonLeft);

		CGEventPost(kCGSessionEventTap, event);
		CFRelease(event);
	}
	int coalesced = received();
	printf("burst of %d with coalescing: %d event(s) taken\n", kBurst, coalesced);
	if (coalesced != 1)
	{
		printf("FAIL: expected one event\n");
		failures++;
	}
	if (viaStream >= 10.0)
	{
		printf("FAIL: %.2f us per event is over the limit of 10\n", viaStream);
		failures++;
	}
	printf(failures ? "FAIL\n" : "PASS\n");
	return failures != 0;
}
