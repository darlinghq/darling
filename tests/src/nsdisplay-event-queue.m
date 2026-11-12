/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * NSDisplay event queue (through NSApplication): events posted from another
 * thread are all delivered to a main-thread drain with a mask, events of
 * other types stay queued, a wait for a type that is not queued blocks until
 * its date, and removing periodic events from an empty queue (and from one
 * holding a few) returns instead of looping. Needs a display (Xvfb).
 */
#import <AppKit/AppKit.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#define COUNT 10000

// NSDisplay is private
@interface NSObject (PeriodicEvents)
- (BOOL) containsAndRemovePeriodicEvents;
@end

static int failed;

static void expect(int cond, const char* what)
{
	if (!cond)
	{
		printf("FAIL: %s\n", what);
		failed = 1;
	}
}

static void watchdog(int sig)
{
	static const char msg[] = "FAIL: timed out\n";
	write(1, msg, sizeof(msg) - 1);
	_exit(1);
}

static NSEvent* makeEvent(NSEventType type, NSInteger subtype)
{
	return [NSEvent otherEventWithType: type
		location: NSZeroPoint modifierFlags: 0 timestamp: 0 windowNumber: 0
		context: nil subtype: (short) subtype data1: 0 data2: 0];
}

static void* poster(void* arg)
{
	NSAutoreleasePool* pool = [NSAutoreleasePool new];

	for (int i = 0; i < COUNT; i++)
	{
		[NSApp postEvent: makeEvent(NSApplicationDefined, 1) atStart: NO];
		if (i % 100 == 0)
			[NSApp postEvent: makeEvent(NSSystemDefined, 2) atStart: NO];
	}
	[pool release];
	return NULL;
}

int main(void)
{
	NSAutoreleasePool* pool = [NSAutoreleasePool new];
	pthread_t th;
	int got = 0, other = 0;

	signal(SIGALRM, watchdog);
	alarm(120);

	[NSApplication sharedApplication];

	// 1. cross-thread posting while draining with a mask
	pthread_create(&th, NULL, poster, NULL);
	while (got < COUNT)
	{
		NSEvent* e = [NSApp nextEventMatchingMask: NSApplicationDefinedMask
			untilDate: [NSDate dateWithTimeIntervalSinceNow: 10]
			inMode: NSDefaultRunLoopMode dequeue: YES];

		if (e == nil)
			break;
		got++;
	}
	pthread_join(th, NULL);
	expect(got == COUNT, "all application-defined events delivered");

	// the other type was not consumed by the masked drain
	for (;;)
	{
		NSEvent* e = [NSApp nextEventMatchingMask: (1ULL << NSSystemDefined)
			untilDate: [NSDate dateWithTimeIntervalSinceNow: 0]
			inMode: NSDefaultRunLoopMode dequeue: YES];

		if (e == nil)
			break;
		other++;
	}
	expect(other == COUNT / 100, "events of other types stayed queued");

	// 2. a queued event of another type does not shorten the wait
	[NSApp postEvent: makeEvent(NSApplicationDefined, 3) atStart: NO];
	NSDate* start = [NSDate date];
	NSEvent* e = [NSApp nextEventMatchingMask: NSKeyDownMask
		untilDate: [NSDate dateWithTimeIntervalSinceNow: 0.4]
		inMode: NSDefaultRunLoopMode dequeue: YES];
	expect(e == nil, "no key event");
	expect(-[start timeIntervalSinceNow] >= 0.3, "wait for an unqueued type blocks until its date");
	e = [NSApp nextEventMatchingMask: NSApplicationDefinedMask
		untilDate: [NSDate dateWithTimeIntervalSinceNow: 0]
		inMode: NSDefaultRunLoopMode dequeue: YES];
	expect(e != nil, "the queued event is still there");

	// 3. periodic events: empty queue, then a few of them
	id display = [NSClassFromString(@"NSDisplay") performSelector: @selector(currentDisplay)];
	expect(![display containsAndRemovePeriodicEvents], "empty queue has no periodic events");
	for (int i = 0; i < 3; i++)
		[NSApp postEvent: makeEvent(NSPeriodic, 0) atStart: NO];
	expect([display containsAndRemovePeriodicEvents], "periodic events found and removed");
	expect(![display containsAndRemovePeriodicEvents], "no periodic events left");

	printf("%s\n", failed ? "FAIL" : "PASS");
	[pool release];
	return failed;
}
