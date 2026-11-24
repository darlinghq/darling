// Copyright (C) 2026 Lubos Dolezel
//
// Carbon event queues and event loops: priorities, the queue functions, ReceiveNextEvent
// with its timeouts and quit, RunCurrentEventLoop, event loop timers and posting from another
// thread. Observed on macOS 26.7.3; the program passes unchanged there (the checks of the
// framework's private functions are skipped, as they do not exist there).
#include <Carbon/Carbon.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

OSStatus SetMouseCoalescingEnabled(Boolean inNewState, Boolean* outOldState);

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

enum { kClass = 'test' };

static EventRef make(UInt32 kind)
{
	EventRef event = NULL;
	CreateEvent(NULL, kClass, kind, 0, kEventAttributeNone, &event);
	return event;
}

// Takes every event out of the queue, including the ones the system may have posted
static void drain(void)
{
	EventRef event;
	while (ReceiveNextEvent(0, NULL, 0.05, true, &event) == noErr)
		ReleaseEvent(event);
	FlushEventQueue(GetMainEventQueue());
}

static UInt32 pullKind(void)
{
	EventTypeSpec only = { kClass, 0 };
	EventRef event = NULL;
	UInt32 kind = 0;

	if (ReceiveNextEvent(0, NULL, kEventDurationNoWait, true, &event) == noErr)
	{
		kind = GetEventKind(event);
		ReleaseEvent(event);
	}
	(void) only;
	return kind;
}

static Boolean kindIs(EventRef event, void* data)
{
	return GetEventKind(event) == *(UInt32*) data;
}

static void quitFromTimer(EventLoopTimerRef timer, void* data)
{
	CHECK(QuitEventLoop(GetCurrentEventLoop()) == noErr);
}

static int g_fires;

static void countFire(EventLoopTimerRef timer, void* data)
{
	g_fires++;
}

static void removeSelf(EventLoopTimerRef timer, void* data)
{
	g_fires++;
	CHECK(RemoveEventLoopTimer(timer) == noErr);
}

static void* postFromThread(void* arg)
{
	usleep(200000);
	EventRef event = make(77);
	CHECK(PostEventToQueue(GetMainEventQueue(), event, kEventPriorityStandard) == noErr);
	ReleaseEvent(event);
	return NULL;
}

static int g_handled;

static OSStatus countHandler(EventHandlerCallRef call, EventRef event, void* data)
{
	g_handled++;
	return noErr;
}

static void testPriorities(void)
{
	EventQueueRef queue = GetMainEventQueue();
	EventRef a = make(1), b = make(2), c = make(3), d = make(4);

	CHECK(GetEventRetainCount(a) == 1);
	CHECK(PostEventToQueue(queue, a, kEventPriorityStandard) == noErr);
	CHECK(GetEventRetainCount(a) == 2);   // the queue holds a reference
	CHECK(PostEventToQueue(queue, b, kEventPriorityLow) == noErr);
	CHECK(PostEventToQueue(queue, c, kEventPriorityHigh) == noErr);
	CHECK(PostEventToQueue(queue, d, kEventPriorityStandard) == noErr);
	CHECK(GetNumEventsInQueue(queue) == 4);
	CHECK(IsEventInQueue(queue, a));

	// High first, low last, posting order within a priority
	CHECK(pullKind() == 3);
	CHECK(pullKind() == 1);
	CHECK(pullKind() == 4);
	CHECK(pullKind() == 2);
	CHECK(pullKind() == 0);
	CHECK(GetNumEventsInQueue(queue) == 0);
	CHECK(!IsEventInQueue(queue, a));
	// A pulled event is the caller's alone: the queue's reference went with it
	CHECK(GetEventRetainCount(a) == 1);

	ReleaseEvent(a); ReleaseEvent(b); ReleaseEvent(c); ReleaseEvent(d);
}

static void testQueueFunctions(void)
{
	EventQueueRef queue = GetMainEventQueue();
	EventRef a = make(1), b = make(2), c = make(3);

	CHECK(PostEventToQueue(queue, a, kEventPriorityStandard) == noErr);
	CHECK(PostEventToQueue(queue, a, kEventPriorityStandard) == eventAlreadyPostedErr);
	CHECK(PostEventToQueue(queue, a, kEventPriorityHigh) == eventAlreadyPostedErr);
	CHECK(GetEventRetainCount(a) == 2);

	// Bad arguments
	CHECK(PostEventToQueue(queue, NULL, kEventPriorityStandard) == paramErr);
	CHECK(PostEventToQueue(NULL, b, kEventPriorityStandard) == paramErr);
	CHECK(PostEventToQueue(queue, b, 77) == paramErr);
	CHECK(!IsEventInQueue(queue, b));
	CHECK(RemoveEventFromQueue(queue, b) == eventNotInQueueErr);

	CHECK(PostEventToQueue(queue, b, kEventPriorityLow) == noErr);
	CHECK(PostEventToQueue(queue, c, kEventPriorityHigh) == noErr);

	// Find leaves the event where it is and does not retain it
	UInt32 kind = 2;
	CHECK(FindSpecificEventInQueue(queue, kindIs, &kind) == b);
	CHECK(GetEventRetainCount(b) == 2);
	kind = 9;
	CHECK(FindSpecificEventInQueue(queue, kindIs, &kind) == NULL);

	// Acquire gives the caller a reference to the first event of the types; it stays queued
	EventTypeSpec kindOne = { kClass, 1 };
	EventRef acquired = AcquireFirstMatchingEventInQueue(queue, 1, &kindOne, kEventQueueOptionsNone);
	CHECK(acquired == a);
	CHECK(GetEventRetainCount(a) == 3);
	CHECK(IsEventInQueue(queue, a));
	ReleaseEvent(acquired);
	EventTypeSpec kindNine = { kClass, 9 };
	CHECK(AcquireFirstMatchingEventInQueue(queue, 1, &kindNine, kEventQueueOptionsNone) == NULL);

	// Remove takes the event out and drops the queue's reference
	CHECK(RemoveEventFromQueue(queue, c) == noErr);
	CHECK(GetEventRetainCount(c) == 1);
	CHECK(!IsEventInQueue(queue, c));
	CHECK(RemoveEventFromQueue(queue, c) == eventNotInQueueErr);
	CHECK(GetNumEventsInQueue(queue) == 2);

	// Flush of a list of types, and of events a comparator selects
	EventTypeSpec kindOneAndThree[2] = { { kClass, 1 }, { kClass, 3 } };
	CHECK(FlushEventsMatchingListFromQueue(queue, 2, kindOneAndThree) == noErr);
	CHECK(GetNumEventsInQueue(queue) == 1);
	CHECK(GetEventRetainCount(a) == 1);
	kind = 2;
	CHECK(FlushSpecificEventsFromQueue(queue, kindIs, &kind) == noErr);
	CHECK(GetNumEventsInQueue(queue) == 0);
	CHECK(GetEventRetainCount(b) == 1);

	// FlushEventQueue drops everything
	CHECK(PostEventToQueue(queue, a, kEventPriorityStandard) == noErr);
	CHECK(PostEventToQueue(queue, b, kEventPriorityStandard) == noErr);
	CHECK(FlushEventQueue(queue) == noErr);
	CHECK(GetNumEventsInQueue(queue) == 0);
	CHECK(GetEventRetainCount(a) == 1 && GetEventRetainCount(b) == 1);

	// An event that left a queue can be posted again
	CHECK(PostEventToQueue(queue, a, kEventPriorityStandard) == noErr);
	CHECK(FlushEventQueue(queue) == noErr);

	ReleaseEvent(a); ReleaseEvent(b); ReleaseEvent(c);
}

static void testReceiveNextEvent(void)
{
	EventQueueRef queue = GetMainEventQueue();
	EventRef a = make(1), b = make(2);
	EventRef got = NULL;

	CHECK(ReceiveNextEvent(0, NULL, kEventDurationNoWait, true, &got) == eventLoopTimedOutErr);
	CHECK(ReceiveNextEvent(0, NULL, 0.1, true, &got) == eventLoopTimedOutErr);

	CHECK(PostEventToQueue(queue, a, kEventPriorityStandard) == noErr);
	CHECK(PostEventToQueue(queue, b, kEventPriorityStandard) == noErr);

	// A type filter picks events in the middle of the queue; events that do not match stay
	EventTypeSpec kindTwo = { kClass, 2 };
	CHECK(ReceiveNextEvent(1, &kindTwo, kEventDurationNoWait, true, &got) == noErr);
	CHECK(got == b);
	ReleaseEvent(got);
	CHECK(GetNumEventsInQueue(queue) == 1);
	CHECK(ReceiveNextEvent(1, &kindTwo, kEventDurationNoWait, true, &got) == eventLoopTimedOutErr);

	// Without pull the event stays queued, and the queue keeps the only reference to it
	CHECK(ReceiveNextEvent(0, NULL, kEventDurationNoWait, false, &got) == noErr);
	CHECK(got == a);
	CHECK(IsEventInQueue(queue, a));
	CHECK(GetEventRetainCount(a) == 2);   // the creator's and the queue's
	CHECK(GetNumEventsInQueue(queue) == 1);

	FlushEventQueue(queue);
	ReleaseEvent(a); ReleaseEvent(b);
}

static void testQuit(void)
{
	EventLoopTimerRef timer;
	EventRef got = NULL;

	CHECK(GetMainEventLoop() == GetCurrentEventLoop());
	CHECK(GetCFRunLoopFromEventLoop(GetCurrentEventLoop()) == CFRunLoopGetCurrent());
	CHECK(QuitEventLoop(NULL) == paramErr);

	CHECK(RunCurrentEventLoop(0.1) == eventLoopTimedOutErr);
	CHECK(RunCurrentEventLoop(kEventDurationNoWait) == eventLoopTimedOutErr);

	// A quit ends the wait, once
	CHECK(InstallEventLoopTimer(GetCurrentEventLoop(), 0.1, 0, quitFromTimer, NULL, &timer) == noErr);
	CHECK(RunCurrentEventLoop(kEventDurationForever) == eventLoopQuitErr);
	CHECK(RunCurrentEventLoop(0.05) == eventLoopTimedOutErr);

	CHECK(InstallEventLoopTimer(GetCurrentEventLoop(), 0.1, 0, quitFromTimer, NULL, &timer) == noErr);
	CHECK(ReceiveNextEvent(0, NULL, kEventDurationForever, true, &got) == eventLoopQuitErr);
	CHECK(got == NULL);
	CHECK(ReceiveNextEvent(0, NULL, 0.05, true, &got) == eventLoopTimedOutErr);

	// A quit before the wait is not forgotten
	CHECK(QuitEventLoop(GetCurrentEventLoop()) == noErr);
	CHECK(RunCurrentEventLoop(0.05) == eventLoopQuitErr);
	CHECK(RunCurrentEventLoop(0.05) == eventLoopTimedOutErr);
}

static void testTimers(void)
{
	EventLoopTimerRef timer;

	// A repeating timer
	g_fires = 0;
	CHECK(InstallEventLoopTimer(GetCurrentEventLoop(), 0.05, 0.05, countFire, NULL, &timer) == noErr);
	RunCurrentEventLoop(0.36);
	CHECK(g_fires >= 3 && g_fires <= 8);

	// The next fire time can stop it, and start it again
	CHECK(SetEventLoopTimerNextFireTime(timer, kEventDurationForever) == noErr);
	g_fires = 0;
	RunCurrentEventLoop(0.2);
	CHECK(g_fires == 0);
	CHECK(SetEventLoopTimerNextFireTime(timer, 0.05) == noErr);
	RunCurrentEventLoop(0.3);
	CHECK(g_fires >= 3);

	CHECK(RemoveEventLoopTimer(timer) == noErr);
	g_fires = 0;
	RunCurrentEventLoop(0.2);
	CHECK(g_fires == 0);
	CHECK(RemoveEventLoopTimer(timer) == paramErr);

	// An interval of zero fires once, but the timer can be set to fire again
	CHECK(InstallEventLoopTimer(GetCurrentEventLoop(), 0.05, 0, countFire, NULL, &timer) == noErr);
	g_fires = 0;
	RunCurrentEventLoop(0.3);
	CHECK(g_fires == 1);
	CHECK(SetEventLoopTimerNextFireTime(timer, 0.05) == noErr);
	RunCurrentEventLoop(0.3);
	CHECK(g_fires == 2);
	CHECK(RemoveEventLoopTimer(timer) == noErr);

	// A timer that is due runs while a wait with no time to wait looks at the queue
	CHECK(InstallEventLoopTimer(GetCurrentEventLoop(), 0, 0, countFire, NULL, &timer) == noErr);
	g_fires = 0;
	EventRef got = NULL;
	CHECK(ReceiveNextEvent(0, NULL, kEventDurationNoWait, true, &got) == eventLoopTimedOutErr);
	CHECK(g_fires == 1);
	CHECK(RemoveEventLoopTimer(timer) == noErr);

	// A timer function can remove its own timer
	CHECK(InstallEventLoopTimer(GetCurrentEventLoop(), 0.05, 0.05, removeSelf, NULL, &timer) == noErr);
	g_fires = 0;
	RunCurrentEventLoop(0.3);
	CHECK(g_fires == 1);

}

static void testThreadPost(void)
{
	pthread_t thread;
	EventRef got = NULL;

	CHECK(pthread_create(&thread, NULL, postFromThread, NULL) == 0);
	// A post from another thread ends the wait of this one
	CHECK(ReceiveNextEvent(0, NULL, 5.0, true, &got) == noErr);
	CHECK(got && GetEventKind(got) == 77);
	if (got)
		ReleaseEvent(got);
	pthread_join(thread, NULL);

	// The queue of a thread is its own
	CHECK(GetCurrentEventQueue() == GetMainEventQueue());
}

static void testRunLoopDoesNotDispatch(void)
{
	EventRef event = make(5);
	EventTypeSpec kindFive = { kClass, 5 };
	EventHandlerRef handler;

	g_handled = 0;
	CHECK(InstallEventHandler(GetApplicationEventTarget(), countHandler, 1, &kindFive, NULL, &handler) == noErr);
	CHECK(PostEventToQueue(GetMainEventQueue(), event, kEventPriorityStandard) == noErr);
	// RunCurrentEventLoop runs the run loop; the events stay queued
	CHECK(RunCurrentEventLoop(0.1) == eventLoopTimedOutErr);
	CHECK(g_handled == 0);
	CHECK(IsEventInQueue(GetMainEventQueue(), event));
	FlushEventQueue(GetMainEventQueue());
	CHECK(RemoveEventHandler(handler) == noErr);
	ReleaseEvent(event);
}

static void testCoalescing(void)
{
	Boolean old = 2;

	CHECK(SetMouseCoalescingEnabled(true, &old) == noErr);
	CHECK(old == true);   // the default

	// PostEventToQueue never coalesces: only the events of the window system do
	for (int i = 0; i < 3; i++)
	{
		EventRef event = NULL;
		CHECK(CreateEvent(NULL, kEventClassMouse, kEventMouseMoved, 0, 0, &event) == noErr);
		CHECK(PostEventToQueue(GetMainEventQueue(), event, kEventPriorityStandard) == noErr);
		ReleaseEvent(event);
	}
	CHECK(GetNumEventsInQueue(GetMainEventQueue()) == 3);
	FlushEventQueue(GetMainEventQueue());

	CHECK(SetMouseCoalescingEnabled(false, &old) == noErr);
	CHECK(old == true);
	CHECK(SetMouseCoalescingEnabled(true, &old) == noErr);
	CHECK(old == false);
}

// Functions of the framework that macOS does not export. Found at run time, so that the program
// links and runs there.
static int g_polls;

static void pollProc(EventQueueRef queue, void* info)
{
	// A source of events that is read by the thread that waits: the first poll finds an event
	if (g_polls++ == 0)
	{
		EventRef event = make(55);
		PostEventToQueue(queue, event, kEventPriorityStandard);
		ReleaseEvent(event);
	}
}

static Boolean matchKindOver(EventRef event, void* info)
{
	return GetEventKind(event) > *(UInt32*) info;
}

static void testPrivateFunctions(void)
{
	OSStatus (*atHead)(EventQueueRef, EventRef) = dlsym(RTLD_DEFAULT, "_HIPostEventToQueueAtHead");
	OSStatus (*coalescing)(EventQueueRef, EventRef, EventPriority) = dlsym(RTLD_DEFAULT, "_HIPostEventToQueueCoalescing");
	EventQueueRef queue = GetMainEventQueue();
	OSStatus (*addPoll)(void (*)(EventQueueRef, void*), void*) = dlsym(RTLD_DEFAULT, "_HIAddEventPollProc");
	OSStatus (*removePoll)(void (*)(EventQueueRef, void*), void*) = dlsym(RTLD_DEFAULT, "_HIRemoveEventPollProc");
	OSStatus (*receiveMatching)(Boolean (*)(EventRef, void*), void*, CFStringRef, EventTimeout, Boolean, EventRef*) = dlsym(RTLD_DEFAULT, "_HIReceiveNextEventMatching");
	OSStatus (*flushMatching)(EventQueueRef, Boolean (*)(EventRef, void*), void*, EventRef) = dlsym(RTLD_DEFAULT, "_HIFlushEventsMatching");

	if (addPoll && removePoll)
	{
		EventRef got = NULL;
		g_polls = 0;
		CHECK(addPoll(pollProc, NULL) == noErr);
		// The waiting thread calls the poll function, and sees what it posted
		CHECK(ReceiveNextEvent(0, NULL, 1.0, true, &got) == noErr);
		CHECK(got && GetEventKind(got) == 55);
		if (got)
			ReleaseEvent(got);
		CHECK(g_polls >= 1);
		CHECK(removePoll(pollProc, NULL) == noErr);
		CHECK(removePoll(pollProc, NULL) == paramErr);
		int polls = g_polls;
		RunCurrentEventLoop(0.05);
		CHECK(g_polls == polls);
	}
	if (receiveMatching && flushMatching)
	{
		// An arbitrary filter, in the default run loop mode (NULL) or a named one
		EventRef a = make(1), b = make(5), c = make(9), got = NULL;
		UInt32 limit = 4;
		PostEventToQueue(queue, a, kEventPriorityStandard);
		PostEventToQueue(queue, b, kEventPriorityStandard);
		PostEventToQueue(queue, c, kEventPriorityStandard);
		CHECK(receiveMatching(matchKindOver, &limit, NULL, kEventDurationNoWait, true, &got) == noErr);
		CHECK(got == b);
		ReleaseEvent(got);
		CHECK(receiveMatching(matchKindOver, &limit, kCFRunLoopDefaultMode, kEventDurationNoWait, false, &got) == noErr);
		CHECK(got == c && IsEventInQueue(queue, c));
		limit = 20;
		CHECK(receiveMatching(matchKindOver, &limit, NULL, 0.05, true, &got) == eventLoopTimedOutErr);
		// Flush the events a filter matches, only those in front of an event when one is given
		PostEventToQueue(queue, b, kEventPriorityStandard);
		limit = 0;
		CHECK(flushMatching(queue, matchKindOver, &limit, c) == noErr);
		CHECK(!IsEventInQueue(queue, a) && IsEventInQueue(queue, c) && IsEventInQueue(queue, b));
		CHECK(flushMatching(queue, NULL, NULL, NULL) == noErr);
		CHECK(GetNumEventsInQueue(queue) == 0);
		ReleaseEvent(a); ReleaseEvent(b); ReleaseEvent(c);
	}

	if (atHead)
	{
		// In front of everything, high priority included; the last one posted in front is the first
		EventRef a = make(1), b = make(2), c = make(3);
		CHECK(PostEventToQueue(queue, a, kEventPriorityHigh) == noErr);
		CHECK(PostEventToQueue(queue, b, kEventPriorityStandard) == noErr);
		CHECK(atHead(queue, c) == noErr);
		CHECK(atHead(queue, c) == eventAlreadyPostedErr);
		CHECK(GetEventRetainCount(c) == 2);
		CHECK(pullKind() == 3);
		CHECK(pullKind() == 1);
		CHECK(pullKind() == 2);
		ReleaseEvent(a); ReleaseEvent(b); ReleaseEvent(c);
	}
	if (coalescing)
	{
		for (int enabled = 1; enabled >= 0; enabled--)
		{
			SetMouseCoalescingEnabled(enabled, NULL);
			for (int i = 0; i < 3; i++)
			{
				EventRef event = NULL;
				CreateEvent(NULL, kEventClassMouse, kEventMouseMoved, 0, 0, &event);
				CHECK(coalescing(queue, event, kEventPriorityStandard) == noErr);
				ReleaseEvent(event);
			}
			// A key event between is not touched, and a drag does not replace a move
			EventRef drag = NULL;
			CreateEvent(NULL, kEventClassMouse, kEventMouseDragged, 0, 0, &drag);
			CHECK(coalescing(queue, drag, kEventPriorityStandard) == noErr);
			ReleaseEvent(drag);
			CHECK(GetNumEventsInQueue(queue) == (enabled ? 2 : 4));
			FlushEventQueue(queue);
		}
		SetMouseCoalescingEnabled(true, NULL);
	}
}

int main(void)
{
	drain();
	testPriorities();
	testQueueFunctions();
	testReceiveNextEvent();
	testQuit();
	testTimers();
	testThreadPost();
	testRunLoopDoesNotDispatch();
	testCoalescing();
	testPrivateFunctions();
	CHECK(GetCurrentEventTime() > 0);
	CHECK(GetCurrentEvent() == NULL);
	// The state of the keyboard and the mouse can be read (with no input there is none)
	(void) GetCurrentEventKeyModifiers();
	(void) GetCurrentButtonState();
	printf(failures ? "FAILED\n" : "PASS\n");
	return failures != 0;
}
