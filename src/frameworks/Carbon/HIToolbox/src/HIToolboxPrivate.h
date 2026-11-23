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

#ifdef __cplusplus
extern "C" {
#endif

typedef Boolean (*HIEventMatchProc)(EventRef event, void* info);

// Posts an event in front of every other event of the queue, whatever its priority. Like
// PostEventToQueue(), it retains the event.
OSStatus _HIPostEventToQueueAtHead(EventQueueRef queue, EventRef event);

// PostEventToQueue(), except that with mouse coalescing enabled (SetMouseCoalescingEnabled())
// a queued mouse moved or dragged event of the same kind and for the same window is replaced
// by this one. For the events that come from the window system.
OSStatus _HIPostEventToQueueCoalescing(EventQueueRef queue, EventRef event, EventPriority priority);

// ReceiveNextEvent() with an arbitrary filter (NULL matches every event) and the run loop mode
// to wait in (NULL for the default mode).
OSStatus _HIReceiveNextEventMatching(HIEventMatchProc match, void* info, CFStringRef runLoopMode, EventTimeout timeout, Boolean pull, EventRef* outEvent);

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

#ifdef __cplusplus
}
#endif

#endif
