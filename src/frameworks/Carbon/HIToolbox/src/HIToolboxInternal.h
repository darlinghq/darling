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

#ifndef HITOOLBOX_INTERNAL_H
#define HITOOLBOX_INTERNAL_H

// Not part of the public headers.
//
// The Carbon events made from CoreGraphics events (CreateEventWithCGEvent) have this class, and the
// CGEventType as their kind; kind 14 is a system-defined event. Observed on macOS 26.7.3 (tier 2,
// mac-probes/v443-systemdefined-carbon-event.m, MC-V443-APPKIT-IMPORTS in MAC-EVIDENCE.md).
#define kHIEventClassRaw 'cgs '
#define kHIEventRawSystemDefined 14

#include <HIToolbox/CarbonEventsCore.h>
#include <HIToolbox/MacWindows.h>
#include <CoreGraphics/CGSServerSPI.h>
#include <CoreGraphics/CGEvent.h>
#include <CoreFoundation/CFRunLoop.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

// An event. The queue fields belong to CarbonEventQueue.c.
struct EventParameter;
struct OpaqueEventRef
{
	atomic_uint retainCount;
	OSType eventClass;
	UInt32 kind;
	EventTime time;
	EventAttributes attributes;
	struct EventParameter* parameters;
	CGEventRef cgEvent;   // for the events made from a CGEvent

	_Atomic(struct OpaqueEventQueueRef*) queue;   // the queue holding the event, or NULL
	struct OpaqueEventRef* queueNext;
	EventPriority queuePriority;
};

// The event being dispatched by the calling thread (see GetCurrentEvent). Returns the
// previous value, to be restored.
EventRef _HISetCurrentEvent(EventRef event);

// Time now, in the unit of EventTime
EventTime _HICurrentTime(void);

// The Carbon modifier bits that correspond to CGEventFlags
UInt32 _HIModifiersFromCGEventFlags(CGEventFlags flags);

// The Carbon event class and kind of a CGEvent type. Types without a Carbon equivalent give
// kHIEventClassRaw and the CGEventType.
void _HIClassAndKindForCGEventType(CGEventType type, OSType* outClass, UInt32* outKind);

// A parameter derived from the CGEvent of an event, for the parameters not set on the event.
// Returns false if there is none.
bool _HICopyCGEventParameter(EventRef event, EventParamName name, EventParamType* outType, ByteCount* outSize, void* outData, ByteCount dataCapacity);

// The standard handler of the application target
OSStatus _HIStandardApplicationHandler(EventHandlerCallRef call, EventRef event, void* userData);


// The event queues and event loops. Every thread has one of each, which are made when the thread
// first uses them; the main thread's are the main queue and the main event loop.
struct OpaqueEventQueueRef
{
	pthread_mutex_t lock;
	// The queued events, in the order they are taken: events posted in front of the
	// others, then high, standard and low priority
	struct OpaqueEventRef* head[4];
	struct OpaqueEventRef* tail[4];
	ItemCount count;
	struct OpaqueEventLoopRef* loop;
};

struct OpaqueEventLoopRef
{
	CFRunLoopRef runLoop;
	CFRunLoopSourceRef wakeSource;   // signalled when something happens the waiting thread must see
	atomic_bool quitRequested;
	struct OpaqueEventQueueRef queue;
};

struct OpaqueEventLoopRef* _HIEventLoopForCurrentThread(void);
struct OpaqueEventLoopRef* _HIMainEventLoop(void);
void _HIWakeEventLoop(struct OpaqueEventLoopRef* loop);
void _HIRunEventPollProcs(EventQueueRef queue);

// The event stream of CoreGraphics (CarbonEventStream.c). The thread that waits for events in the
// main event loop is its reader: _HIEventStreamWillWait() registers it before the first wait, and
// _HIDrainEventStream() moves the events that have arrived into the queue, in the order they came.
void _HIEventStreamWillWait(struct OpaqueEventLoopRef* loop, CFStringRef mode);
void _HIDrainEventStream(EventQueueRef queue);
// Posts an event at standard priority, coalescing mouse moves, from the thread that owns the
// queue and is about to look into it: nothing needs to be woken.
OSStatus _HIPostEventFromWaitingThread(EventQueueRef queue, EventRef event);

// Takes the first event of the queue that match accepts. With pull, the event leaves the queue
// and the queue's reference to it becomes the caller's; without, it stays and nothing is
// retained. A filter that is only a function of the event (it uses no event queue function)
// can be applied with the queue locked; others see a snapshot of the queue.
EventRef _HIQueueTakeMatching(EventQueueRef queue, Boolean (*match)(EventRef, void*), void* info, bool pure, bool pull);
void _HIDiscardQueuedEvents(EventQueueRef queue);

// Reads a parameter that was set on the event (not the ones derived from its CGEvent). False if
// there is none; the value is copied up to dataCapacity bytes.
bool _HICopyEventParameter(EventRef event, EventParamName name, EventParamType* outType, ByteCount* outSize, void* outData, ByteCount dataCapacity);

struct HIEventTypeList { ItemCount count; const EventTypeSpec* types; };
Boolean _HIMatchesEventTypes(EventRef event, void* typeList);   // typeList: struct HIEventTypeList*, count 0 matches everything

// Starts the delivery of the application events about the front process (kEventAppFrontSwitched,
// kEventAppActivated, kEventAppDeactivated) to the application target. Once per process, any thread.
void _HIObserveFrontProcess(void);

// The application target without starting the front process events (for them, which run inside
// their own start).
EventTargetRef _HIApplicationEventTarget(void);

// The event types of the standard handler of the windows (MacWindows.c)
#define kHIStandardWindowEventTypeCount 21
// The CoreGraphics window of a Carbon window (MacWindows.c)
CGSWindowID _HIWindowGetCGSWindowID(WindowRef window);

#endif
