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

// The events of the window system: CoreGraphics keeps them, in the order they arrived and after the
// event taps have seen them, in the event stream of the process. The thread that waits for events
// in the main event loop reads the stream, and turns each CGEvent into an event of the main queue
// (CreateEventWithCGEvent). There is no thread of its own and no message per event: CoreGraphics
// signals a run loop source when the stream goes from empty to not empty, which ends the wait.
//
// Only HIToolbox depends on CoreGraphics here, never the other way round.

#include <HIToolbox/CarbonEventsCore.h>
#include <HIToolbox/CarbonEvents.h>
#include "HIToolboxInternal.h"
#include <CoreGraphics/CoreGraphicsPrivate.h>

static atomic_bool g_streamRegistered;
static pthread_mutex_t g_streamRegisterLock = PTHREAD_MUTEX_INITIALIZER;
static CFRunLoopSourceRef g_streamSource;

static void streamSourceFired(void* info)
{
	// The signal only ends the wait; the events are read before the next look at the queue
}

void _HIEventStreamWillWait(struct OpaqueEventLoopRef* loop, CFStringRef mode)
{
	// Window system events are for the main thread
	if (loop != _HIMainEventLoop() || !pthread_main_np())
		return;

	if (!atomic_load_explicit(&g_streamRegistered, memory_order_acquire))
	{
		pthread_mutex_lock(&g_streamRegisterLock);
		if (!atomic_load(&g_streamRegistered))
		{
			CFRunLoopSourceContext context = { .version = 0, .perform = streamSourceFired };

			g_streamSource = CFRunLoopSourceCreate(NULL, 0, &context);
			CFRunLoopAddSource(loop->runLoop, g_streamSource, kCFRunLoopCommonModes);
			// From now on the stream keeps the events, also between two waits
			_CGSEventStreamSetReadySource(g_streamSource, loop->runLoop);
			atomic_store_explicit(&g_streamRegistered, true, memory_order_release);
		}
		pthread_mutex_unlock(&g_streamRegisterLock);
	}

	if (!CFEqual(mode, kCFRunLoopDefaultMode))
		CFRunLoopAddSource(loop->runLoop, g_streamSource, mode);
}

void _HIDrainEventStream(EventQueueRef queue)
{
	if (!atomic_load_explicit(&g_streamRegistered, memory_order_acquire) || queue != &_HIMainEventLoop()->queue)
		return;

	// Until it is empty: the source is signalled again only after that
	CGEventRef cgEvent;
	while ((cgEvent = _CGSEventStreamCopyNextEvent()) != NULL)
	{
		EventRef event = NULL;

		if (CreateEventWithCGEvent(NULL, cgEvent, kEventAttributeNone, &event) == noErr)
		{
			_HIPostEventFromWaitingThread(queue, event);
			ReleaseEvent(event);
		}
		CFRelease(cgEvent);
	}
}
