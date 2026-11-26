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

#ifndef HITOOLBOX_PRIVATE_H
#define HITOOLBOX_PRIVATE_H

// Not part of the public headers, and not installed in the SDK: the interface of HIToolbox for the
// frameworks above it that need more than the documented event queue and event loop functions
// (a filter that is not a list of event types, a run loop mode, posting in front of the queue).

#include <HIToolbox/CarbonEventsCore.h>
#include <CoreGraphics/CGEvent.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef Boolean (*HIEventMatchProc)(EventRef event, void* info);

// Posts an event in front of every other event of the queue, whatever its priority. Like
// PostEventToQueue(), it retains the event.
OSStatus _HIPostEventToQueueAtHead(EventQueueRef queue, EventRef event);

// PostEventToQueue(), except that with mouse coalescing enabled (SetMouseCoalescingEnabled())
// a queued mouse moved or dragged event of the same kind and for the same window (for events made
// from CGEvents: of the same CGEvent type and kCGMouseEventWindowUnderMousePointer) is replaced
// by this one, which takes over its deltas. For the events that come from the window system.
OSStatus _HIPostEventToQueueCoalescing(EventQueueRef queue, EventRef event, EventPriority priority);

// ReceiveNextEvent() with an arbitrary filter (NULL matches every event) and the run loop mode
// to wait in (NULL for the default mode).
OSStatus _HIReceiveNextEventMatching(HIEventMatchProc match, void* info, CFStringRef runLoopMode, EventTimeout timeout, Boolean pull, EventRef* outEvent);

// Options of _HIReceiveNextEventMatchingWithOptions()
enum {
	// The event leaves the queue and the caller owns it (ReceiveNextEvent's inPullEvent)
	kHIReceiveNextEventPull = 1 << 0,
	// The filter looks only at the event it is given and calls no event queue function, so it
	// can run with the queue locked (no copy of the queue is made for it)
	kHIReceiveNextEventFilterIsPure = 1 << 1,
	// The run loop runs once, as -[NSRunLoop runMode:beforeDate:] does: without waiting when a
	// matching event is queued, otherwise until a source of the mode was handled or the timeout
	// is over; then the queue is looked at once more. eventLoopTimedOutErr when nothing matched.
	kHIReceiveNextEventSinglePass = 1 << 2,
};

// _HIReceiveNextEventMatching() with options. With kHIReceiveNextEventSinglePass the caller
// gets control back after every source the run loop handled, for the work it does between
// waits.
OSStatus _HIReceiveNextEventMatchingWithOptions(HIEventMatchProc match, void* info, CFStringRef runLoopMode, EventTimeout timeout, OptionBits options, EventRef* outEvent);

// The main event loop starts to read the window system events of CoreGraphics (its event stream)
// now, instead of at its first wait: what arrives before that wait is kept. For a framework that
// connects to the window system before it waits for events. Any thread.
void _HIBeginEventStreamReading(void);

// A framework above HIToolbox can take part in the reading of the stream: each event that
// arrives is handed to its filter on the reading (main) thread, in the order of the stream, and
// the filter passes on the events of the queue that come of it with emit: the event itself, none
// (it is dropped) or others (a window system event that the framework turns into events of
// another kind, such as a finger that moves the mouse). The filter may call emit only while it
// runs. NULL removes the filter. Any thread.
typedef void (*HIEventStreamEmitProc)(CGEventRef event, void* info);
typedef void (*HIEventStreamFilterProc)(CGEventRef event, HIEventStreamEmitProc emit, void* info);
void _HISetEventStreamFilter(HIEventStreamFilterProc filter);

// Removes the events of the queue the filter matches (NULL: every event). Only the events in
// front of beforeEvent are considered when it is not NULL and in the queue.
OSStatus _HIFlushEventsMatching(EventQueueRef queue, HIEventMatchProc match, void* info, EventRef beforeEvent);

// Event sources that are not run loop sources of their own, such as a stream of window system
// events that has to be read by the thread that waits: the function is called on the thread
// that waits for events in the event loop before it looks at its queue, each time it does so
// (before a wait, and after each wake up). It can post events to the queue. A source that must
// end the wait when new data arrives is a run loop source of the thread, as usual.
typedef void (*HIEventPollProc)(EventQueueRef queue, void* info);
OSStatus _HIAddEventPollProc(HIEventPollProc proc, void* info);
OSStatus _HIRemoveEventPollProc(HIEventPollProc proc, void* info);

// An event target of a framework's object (a menu, a window): it has handlers of its own, and the events
// that none of them handles go on to the target behind it (the application target, usually).
// _HIDestroyEventTarget() removes its handlers and the target itself.
EventTargetRef _HICreateEventTarget(EventTargetRef next);
void _HIDestroyEventTarget(EventTargetRef target);
// Changes the target that an event target passes events on to (NULL: the application target)
void _HISetEventTargetNext(EventTargetRef target, EventTargetRef next);

// The standard commands of the Window menu and of the application menu (see Menus.c, MacWindows.c)
#include <HIToolbox/MacWindows.h>
OSStatus _HIWindowCommand(UInt32 command, WindowRef window);
Boolean _HIWindowCommandEnabled(UInt32 command);
// The visible windows that can be active, front to back, minimized ones included; the caller frees the array
ItemCount _HICopyWindowList(WindowRef** outWindows);
void _HIHideApplicationWindows(Boolean hide);
void _HIObserveProcessCommands(void);
// Sends the application the quit Apple event; without a handler for it the application event loop ends
void _HIQuitApplication(void);

#ifdef __cplusplus
}
#endif

#endif
