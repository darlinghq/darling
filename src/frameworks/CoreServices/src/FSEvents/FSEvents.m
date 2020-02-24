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
									flags: flags
									context: context
									callback: callback];
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
