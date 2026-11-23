/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// Event loops: the wait for events (ReceiveNextEvent, RunCurrentEventLoop), the loop that dispatches the
// events of the application (RunApplicationEventLoop) and the event loop timers.
//
// An event loop is built on the CFRunLoop of its thread. The wait runs the run loop, so its sources and
// timers (also the event loop timers, which are run loop timers) work while a thread waits for events. A
// post to the queue, and QuitEventLoop(), signal a run loop source that ends the wait.

#include <HIToolbox/CarbonEventsCore.h>
#include <HIToolbox/CarbonEvents.h>
#include "HIToolboxInternal.h"
#include "HIToolboxPrivate.h"
#include <CarbonCore/MacErrors.h>

#include <stdlib.h>
#include <string.h>

static pthread_once_t g_mainOnce = PTHREAD_ONCE_INIT;
static pthread_once_t g_keyOnce = PTHREAD_ONCE_INIT;
static pthread_key_t g_threadLoopKey;
static struct OpaqueEventLoopRef g_mainLoop;

static void wakeSourceNothing(void* info)
{
	// The signal only ends the wait of the run loop
}

static void initLoop(struct OpaqueEventLoopRef* loop, CFRunLoopRef runLoop)
{
	pthread_mutex_init(&loop->queue.lock, NULL);
	loop->queue.loop = loop;
	atomic_init(&loop->quitRequested, false);
	loop->runLoop = (CFRunLoopRef) CFRetain(runLoop);

	CFRunLoopSourceContext context = { .version = 0, .perform = wakeSourceNothing };
	loop->wakeSource = CFRunLoopSourceCreate(NULL, 0, &context);
	CFRunLoopAddSource(runLoop, loop->wakeSource, kCFRunLoopCommonModes);
}

static void initMainLoop(void)
{
	initLoop(&g_mainLoop, CFRunLoopGetMain());
}

static void threadLoopDestructor(void* value)
{
	// The loop itself stays: other threads may hold its queue. Its events do not.
	if (value)
		_HIDiscardQueuedEvents(&((struct OpaqueEventLoopRef*) value)->queue);
}

static void makeThreadKey(void)
{
	pthread_key_create(&g_threadLoopKey, threadLoopDestructor);
}

struct OpaqueEventLoopRef* _HIMainEventLoop(void)
{
	pthread_once(&g_mainOnce, initMainLoop);
	return &g_mainLoop;
}

struct OpaqueEventLoopRef* _HIEventLoopForCurrentThread(void)
{
	if (pthread_main_np())
		return _HIMainEventLoop();

	pthread_once(&g_keyOnce, makeThreadKey);
	struct OpaqueEventLoopRef* loop = pthread_getspecific(g_threadLoopKey);
	if (!loop)
	{
		loop = calloc(1, sizeof(*loop));
		if (!loop)
			abort();
		initLoop(loop, CFRunLoopGetCurrent());
		pthread_setspecific(g_threadLoopKey, loop);
	}
	return loop;
}

void _HIWakeEventLoop(struct OpaqueEventLoopRef* loop)
{
	CFRunLoopSourceSignal(loop->wakeSource);
	CFRunLoopWakeUp(loop->runLoop);
}

EventLoopRef GetCurrentEventLoop(void)
{
	return _HIEventLoopForCurrentThread();
}

EventLoopRef GetMainEventLoop(void)
{
	return _HIMainEventLoop();
}

CFRunLoopRef GetCFRunLoopFromEventLoop(EventLoopRef inEventLoop)
{
	return inEventLoop ? inEventLoop->runLoop : NULL;
}

OSStatus QuitEventLoop(EventLoopRef inEventLoop)
{
	if (!inEventLoop)
		return paramErr;

	atomic_store(&inEventLoop->quitRequested, true);
	_HIWakeEventLoop(inEventLoop);
	return noErr;
}

// Event poll functions

struct PollProc
{
	HIEventPollProc proc;
	void* info;
};

static pthread_mutex_t g_pollLock = PTHREAD_MUTEX_INITIALIZER;
static struct PollProc* g_pollProcs;
static size_t g_pollProcCount;

OSStatus _HIAddEventPollProc(HIEventPollProc proc, void* info)
{
	if (!proc)
		return paramErr;

	pthread_mutex_lock(&g_pollLock);
	struct PollProc* procs = realloc(g_pollProcs, (g_pollProcCount + 1) * sizeof(*procs));
	if (!procs)
	{
		pthread_mutex_unlock(&g_pollLock);
		return memFullErr;
	}
	g_pollProcs = procs;
	g_pollProcs[g_pollProcCount].proc = proc;
	g_pollProcs[g_pollProcCount].info = info;
	g_pollProcCount++;
	pthread_mutex_unlock(&g_pollLock);
	return noErr;
}

OSStatus _HIRemoveEventPollProc(HIEventPollProc proc, void* info)
{
	OSStatus result = paramErr;

	pthread_mutex_lock(&g_pollLock);
	for (size_t i = 0; i < g_pollProcCount; i++)
	{
		if (g_pollProcs[i].proc == proc && g_pollProcs[i].info == info)
		{
			memmove(&g_pollProcs[i], &g_pollProcs[i + 1], (g_pollProcCount - i - 1) * sizeof(*g_pollProcs));
			g_pollProcCount--;
			result = noErr;
			break;
		}
	}
	pthread_mutex_unlock(&g_pollLock);
	return result;
}

void _HIRunEventPollProcs(EventQueueRef queue)
{
	// The window system events of CoreGraphics come first: they arrived before whatever the poll
	// functions find
	_HIDrainEventStream(queue);

	// Without any poll function, which is the usual case, this takes the lock once
	pthread_mutex_lock(&g_pollLock);
	size_t count = g_pollProcCount;
	struct PollProc* copy = NULL;

	if (count && (copy = malloc(count * sizeof(*copy))) != NULL)
		memcpy(copy, g_pollProcs, count * sizeof(*copy));
	pthread_mutex_unlock(&g_pollLock);

	for (size_t i = 0; copy && i < count; i++)
		copy[i].proc(queue, copy[i].info);
	free(copy);
}

// Waiting for events

static Boolean matchNothing(EventRef event, void* info)
{
	return false;
}

// Waits until the filter matches an event of the queue, the loop is told to quit or the timeout
// is over, whichever is first. A timeout of zero checks once; a negative one never ends.
static OSStatus waitForEvent(Boolean (*match)(EventRef, void*), void* info, bool pure, CFStringRef mode, EventTimeout timeout, Boolean pull, EventRef* outEvent)
{
	struct OpaqueEventLoopRef* loop = _HIEventLoopForCurrentThread();
	EventTime deadline = timeout < 0 ? 0 : _HICurrentTime() + timeout;
	bool waited = false;

	if (!mode)
		mode = kCFRunLoopDefaultMode;
	else if (!CFEqual(mode, kCFRunLoopDefaultMode))
		CFRunLoopAddSource(loop->runLoop, loop->wakeSource, mode);
	_HIEventStreamWillWait(loop, mode);

	if (outEvent)
		*outEvent = NULL;

	for (;;)
	{
		_HIRunEventPollProcs(&loop->queue);

		EventRef event = _HIQueueTakeMatching(&loop->queue, match, info, pure, outEvent != NULL && pull);
		if (event)
		{
			if (outEvent)
				*outEvent = event;
			return noErr;
		}
		if (atomic_exchange(&loop->quitRequested, false))
			return eventLoopQuitErr;

		EventTime remaining = 0;
		if (timeout >= 0)
		{
			remaining = deadline - _HICurrentTime();
			if (remaining <= 0 && waited)
				return eventLoopTimedOutErr;
			if (remaining < 0)
				remaining = 0;
		}
		else
			remaining = 1e9;

		// Returns after a source was handled, which includes the signal of a post or a quit. A
		// timer that fires does not end the wait, but its function can post or quit.
		CFRunLoopRunInMode(mode, remaining, true);
		waited = true;
	}
}

OSStatus _HIReceiveNextEventMatching(HIEventMatchProc match, void* info, CFStringRef runLoopMode, EventTimeout timeout, Boolean pull, EventRef* outEvent)
{
	if (!outEvent)
		return paramErr;
	return waitForEvent(match, info, false, runLoopMode, timeout, pull, outEvent);
}

OSStatus ReceiveNextEvent(ItemCount inNumTypes, const EventTypeSpec* inList, EventTimeout inTimeout, Boolean inPullEvent, EventRef* outEvent)
{
	if (!outEvent || (inNumTypes != 0 && !inList))
		return paramErr;

	struct HIEventTypeList list = { inNumTypes, inList };
	return waitForEvent(_HIMatchesEventTypes, &list, true, NULL, inTimeout, inPullEvent, outEvent);
}

OSStatus RunCurrentEventLoop(EventTimeout inTimeout)
{
	// The events stay in the queue: only RunApplicationEventLoop() and callers of
	// ReceiveNextEvent() take them
	return waitForEvent(matchNothing, NULL, true, NULL, inTimeout, false, NULL);
}

// The application event loop

static atomic_int g_applicationLoopsRunning;
static atomic_bool g_applicationQuitRequested;

void QuitApplicationEventLoop(void)
{
	// Without a loop running, this is a quit of the main event loop that its next wait sees
	if (atomic_load(&g_applicationLoopsRunning) > 0)
		atomic_store(&g_applicationQuitRequested, true);
	QuitEventLoop(_HIMainEventLoop());
}

OSStatus _HIStandardApplicationHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	HICommand command;

	if (event->eventClass != kEventClassCommand || event->kind != kEventCommandProcess)
		return eventNotHandledErr;
	if (GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command) != noErr)
		return eventNotHandledErr;
	// Quit ends the loop of an application that runs one; any other application decides
	// what quitting means.
	if (command.commandID != kHICommandQuit || atomic_load(&g_applicationLoopsRunning) == 0)
		return eventNotHandledErr;

	QuitApplicationEventLoop();
	return noErr;
}

void RunApplicationEventLoop(void)
{
	atomic_fetch_add(&g_applicationLoopsRunning, 1);
	atomic_store(&g_applicationQuitRequested, false);

	while (!atomic_load(&g_applicationQuitRequested))
	{
		EventRef event = NULL;
		OSStatus status = ReceiveNextEvent(0, NULL, kEventDurationForever, true, &event);

		if (status == eventLoopQuitErr)
			break;
		if (status != noErr)
			continue;

		// An event made from a CGEvent is the current event while its handlers run
		EventRef previous = _HISetCurrentEvent(event->cgEvent ? event : NULL);
		SendEventToEventTarget(event, GetEventDispatcherTarget());
		_HISetCurrentEvent(previous);
		ReleaseEvent(event);
	}

	atomic_store(&g_applicationQuitRequested, false);
	atomic_fetch_sub(&g_applicationLoopsRunning, 1);
}

// Event loop timers

struct __EventLoopTimer
{
	struct __EventLoopTimer* next;
	struct OpaqueEventLoopRef* loop;
	CFRunLoopTimerRef timer;   // NULL while the timer is not scheduled
	EventLoopTimerProcPtr proc;
	void* userData;
	EventTimerInterval interval;
	int running;               // calls of the timer function in progress
	bool removed;
};

static pthread_mutex_t g_timerLock = PTHREAD_MUTEX_INITIALIZER;
static struct __EventLoopTimer* g_timers;

static bool timerIsLive(struct __EventLoopTimer* timer)
{
	for (struct __EventLoopTimer* other = g_timers; other; other = other->next)
	{
		if (other == timer)
			return true;
	}
	return false;
}

static void timerFired(CFRunLoopTimerRef cfTimer, void* info)
{
	struct __EventLoopTimer* timer = info;

	pthread_mutex_lock(&g_timerLock);
	if (timer->removed)
	{
		pthread_mutex_unlock(&g_timerLock);
		return;
	}
	timer->running++;
	pthread_mutex_unlock(&g_timerLock);

	timer->proc(timer, timer->userData);

	pthread_mutex_lock(&g_timerLock);
	bool freeIt = --timer->running == 0 && timer->removed;
	pthread_mutex_unlock(&g_timerLock);
	if (freeIt)
		free(timer);
}

// With g_timerLock held. The timer fires after the delay, then at its interval.
static void scheduleTimer(struct __EventLoopTimer* timer, EventTimerInterval delay)
{
	if (timer->timer)
	{
		CFRunLoopTimerInvalidate(timer->timer);
		CFRelease(timer->timer);
		timer->timer = NULL;
	}
	if (delay < 0)
		return;

	CFRunLoopTimerContext context = { .version = 0, .info = timer };
	timer->timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + delay, timer->interval, 0, 0, timerFired, &context);
	CFRunLoopAddTimer(timer->loop->runLoop, timer->timer, kCFRunLoopCommonModes);
	// A timer that is added to the loop of another thread has to be noticed by it
	CFRunLoopWakeUp(timer->loop->runLoop);
}

OSStatus InstallEventLoopTimer(EventLoopRef inEventLoop, EventTimerInterval inFireDelay, EventTimerInterval inInterval, EventLoopTimerUPP inTimerProc, void* inTimerData, EventLoopTimerRef* outTimer)
{
	if (!inEventLoop || !inTimerProc || !outTimer || inInterval < 0)
		return paramErr;

	struct __EventLoopTimer* timer = calloc(1, sizeof(*timer));
	if (!timer)
		return memFullErr;
	timer->loop = inEventLoop;
	timer->proc = inTimerProc;
	timer->userData = inTimerData;
	timer->interval = inInterval;

	pthread_mutex_lock(&g_timerLock);
	timer->next = g_timers;
	g_timers = timer;
	scheduleTimer(timer, inFireDelay);
	pthread_mutex_unlock(&g_timerLock);

	*outTimer = timer;
	return noErr;
}

OSStatus RemoveEventLoopTimer(EventLoopTimerRef inTimer)
{
	OSStatus result = paramErr;

	pthread_mutex_lock(&g_timerLock);
	for (struct __EventLoopTimer** link = &g_timers; *link; link = &(*link)->next)
	{
		if (*link != inTimer)
			continue;

		*link = inTimer->next;
		scheduleTimer(inTimer, -1);
		inTimer->removed = true;
		// A timer function that removes its own timer is still running: the last call frees it
		if (inTimer->running == 0)
			free(inTimer);
		result = noErr;
		break;
	}
	pthread_mutex_unlock(&g_timerLock);
	return result;
}

OSStatus SetEventLoopTimerNextFireTime(EventLoopTimerRef inTimer, EventTimerInterval inNextFire)
{
	OSStatus result = paramErr;

	pthread_mutex_lock(&g_timerLock);
	if (inTimer && timerIsLive(inTimer))
	{
		scheduleTimer(inTimer, inNextFire);
		result = noErr;
	}
	pthread_mutex_unlock(&g_timerLock);
	return result;
}

EventLoopTimerUPP NewEventLoopTimerUPP(EventLoopTimerProcPtr userRoutine)
{
	return userRoutine;
}

void DisposeEventLoopTimerUPP(EventLoopTimerUPP userUPP)
{
}
