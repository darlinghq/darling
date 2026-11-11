/*
 This file is part of Darling.

 Copyright (C) 2019-2020 Lubos Dolezel

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

#include <FSEvents/FSEvents.h>
#include <stdio.h>
#include <stdlib.h>
#import "FSEventsImpl.h"

FSEventStreamRef FSEventStreamCreate(
		CFAllocatorRef allocator,
		FSEventStreamCallback callback,
		FSEventStreamContext *context,
		CFArrayRef pathsToWatch,
		FSEventStreamEventId sinceWhen,
		CFTimeInterval latency,
		FSEventStreamCreateFlags flags)
{
	return (FSEventStreamRef) [[FSEventsImpl alloc] initWithPaths: (NSArray*)pathsToWatch
									latency: latency
									flags: flags
									context: context
									callback: callback];
}

extern FSEventStreamRef FSEventStreamCreateRelativeToDevice(
		CFAllocatorRef allocator,
		FSEventStreamCallback callback,
		FSEventStreamContext *context,
		dev_t deviceToWatch,
		CFArrayRef pathsToWatchRelativeToDevice,
		FSEventStreamEventId sinceWhen,
		CFTimeInterval latency,
		FSEventStreamCreateFlags flags)
{
	printf("STUB %s\n", __PRETTY_FUNCTION__);
    return nil;
}

CFArrayRef FSEventStreamCopyPathsBeingWatched(ConstFSEventStreamRef streamRef)
{
	FSEventsImpl* impl = (FSEventsImpl*) streamRef;
	return (CFArrayRef) [impl copyPathsToWatch];
}

FSEventStreamEventId FSEventStreamGetLatestEventId(ConstFSEventStreamRef streamRef)
{
	return [((FSEventsImpl*) streamRef) lastEventID];
}

// Delivers the events that are waiting for the latency to pass and returns the highest ID among
// them, or 0 if there were none. The callback then runs as for any other delivery: on the
// stream's dispatch queue, or from its run loop.
FSEventStreamEventId FSEventStreamFlushAsync(FSEventStreamRef streamRef)
{
	if (!streamRef)
		return 0;
	return [((FSEventsImpl*) streamRef) flushPendingEvents];
}

void FSEventStreamFlushSync(FSEventStreamRef streamRef)
{
	FSEventStreamFlushAsync(streamRef);
}

CFStringRef FSEventStreamCopyDescription(ConstFSEventStreamRef streamRef)
{
	if (!streamRef)
		return NULL;
	return (CFStringRef) [((FSEventsImpl*) streamRef) copyStreamDescription];
}

void FSEventStreamInvalidate(FSEventStreamRef streamRef)
{
	[((FSEventsImpl*) streamRef) invalidate];
}

void FSEventStreamRelease(FSEventStreamRef streamRef)
{
	[((FSEventsImpl*) streamRef) release];
}

void FSEventStreamRetain(FSEventStreamRef streamRef)
{
	[((FSEventsImpl*) streamRef) retain];
}

void FSEventStreamScheduleWithRunLoop(FSEventStreamRef streamRef, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	[((FSEventsImpl*) streamRef) scheduleWithRunLoop: runLoop
												mode: runLoopMode];
}

void FSEventStreamSetDispatchQueue(FSEventStreamRef streamRef, dispatch_queue_t q)
{
	[((FSEventsImpl*) streamRef) setDispatchQueue: q];
}

Boolean FSEventStreamStart(FSEventStreamRef streamRef)
{
	[((FSEventsImpl*) streamRef) start];
	return TRUE;
}

void FSEventStreamStop(FSEventStreamRef streamRef)
{
	[((FSEventsImpl*) streamRef) stop];
}

void FSEventStreamUnscheduleFromRunLoop(FSEventStreamRef streamRef, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	[((FSEventsImpl*) streamRef) unscheduleWithRunLoop: runLoop
												mode: runLoopMode];
}

// Event IDs are drawn from one counter shared by every stream of the process, and increase
// monotonically. The counter starts above zero. It holds the last ID handed out.
static uint64_t g_lastEventID = 1;

__attribute__((visibility("hidden"))) uint64_t _FSEventsAllocateEventID(void)
{
	return __atomic_add_fetch(&g_lastEventID, 1, __ATOMIC_RELAXED);
}

FSEventStreamEventId FSEventsGetCurrentEventId(void)
{
	return __atomic_load_n(&g_lastEventID, __ATOMIC_RELAXED);
}

// The remaining device-level entry points need the persistent, system-wide event log of
// macOS. There is none here, so no volume has a UUID, nothing is remembered about the past,
// and nothing can be purged.

CFUUIDRef FSEventsCopyUUIDForDevice(dev_t dev)
{
	return NULL;
}

FSEventStreamEventId FSEventsGetLastEventIdForDeviceBeforeTime(dev_t dev, CFAbsoluteTime time)
{
	return 0;
}

Boolean FSEventsPurgeEventsForDeviceUpToEventId(dev_t dev, FSEventStreamEventId eventId)
{
	return FALSE;
}
