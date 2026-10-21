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

FSEventStreamEventId FSEventStreamFlushAsync(FSEventStreamRef streamRef)
{
	if (!streamRef)
		return 0;
	return FSEventStreamGetLatestEventId(streamRef);
}

void FSEventStreamFlushSync(FSEventStreamRef streamRef)
{
}

CFStringRef FSEventStreamCopyDescription(ConstFSEventStreamRef streamRef)
{
	if (!streamRef)
		return NULL;
	FSEventsImpl* impl = (FSEventsImpl*) streamRef;
	NSArray* paths = [impl copyPathsToWatch];
	NSString* desc = [[NSString stringWithFormat: @"<FSEventStream %p: paths=%@>", streamRef, paths] retain];
	[paths release];
	return (CFStringRef) desc;
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

uint64_t g_globalFSEventID = 1000;

FSEventStreamEventId FSEventsGetCurrentEventId(void)
{
	return __atomic_load_n(&g_globalFSEventID, __ATOMIC_RELAXED);
}

CFUUIDRef FSEventsCopyUUIDForDevice(dev_t dev)
{
	// TODO: Retrieve persistent filesystem UUID via getattrlist(ATTR_VOL_UUID) or statvfs.
	// For now, generate a deterministic UUID derived from device number `dev` so that
	// distinct devices produce distinct, stable UUIDs.
	CFUUIDBytes bytes = { 0 };
	bytes.byte0 = 0x44; // 'D'
	bytes.byte1 = 0x41; // 'A'
	bytes.byte2 = 0x52; // 'R'
	bytes.byte3 = 0x4C; // 'L'
	bytes.byte4 = (uint8_t)(dev & 0xFF);
	bytes.byte5 = (uint8_t)((dev >> 8) & 0xFF);
	bytes.byte6 = 0x40 | ((uint8_t)((dev >> 16) & 0x0F)); // UUID version 4
	bytes.byte7 = (uint8_t)((dev >> 20) & 0xFF);
	bytes.byte8 = 0x80 | ((uint8_t)((dev >> 28) & 0x3F)); // variant
	bytes.byte9 = 0x64; // 'd'
	bytes.byte10 = 0x65; // 'e'
	bytes.byte11 = 0x76; // 'v'
	return CFUUIDCreateWithBytes(kCFAllocatorDefault,
		bytes.byte0, bytes.byte1, bytes.byte2, bytes.byte3,
		bytes.byte4, bytes.byte5, bytes.byte6, bytes.byte7,
		bytes.byte8, bytes.byte9, bytes.byte10, bytes.byte11,
		bytes.byte12, bytes.byte13, bytes.byte14, bytes.byte15);
}

FSEventStreamEventId FSEventsGetLastEventIdForDeviceBeforeTime(dev_t dev, CFAbsoluteTime time)
{
	// TODO: If persistent fseventsd journal storage is implemented, look up historical
	// event ID by device and timestamp.
	// Returning the current event ID safely resumes stream listening from "now" without crashing.
	return FSEventsGetCurrentEventId();
}

Boolean FSEventsPurgeEventsForDeviceUpToEventId(dev_t dev, FSEventStreamEventId eventId)
{
	return TRUE;
}
