/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#ifndef _FSEVENTS_H
#define _FSEVENTS_H
#include <CoreFoundation/CoreFoundation.h>
#include <sys/types.h>
#include <Availability.h>

typedef struct __FSEventStream* FSEventStreamRef;
typedef const struct __FSEventStream* ConstFSEventStreamRef;

typedef UInt32 FSEventStreamCreateFlags;
typedef UInt64 FSEventStreamEventId;
typedef UInt32 FSEventStreamEventFlags;

enum {
	kFSEventStreamEventFlagNone = 0,
	kFSEventStreamEventFlagMustScanSubDirs = 1,
	kFSEventStreamEventFlagUserDropped = 2,
	kFSEventStreamEventFlagKernelDropped = 4,
	kFSEventStreamEventFlagEventIdsWrapped = 8,
	kFSEventStreamEventFlagHistoryDone = 0x10,
	kFSEventStreamEventFlagRootChanged = 0x20,
	kFSEventStreamEventFlagMount = 0x40,
	kFSEventStreamEventFlagUnmount = 0x80,
	kFSEventStreamEventFlagItemCreated = 0x100,
	kFSEventStreamEventFlagItemRemoved = 0x200,
	kFSEventStreamEventFlagItemInodeMetaMod = 0x400,
	kFSEventStreamEventFlagItemRenamed = 0x800,
	kFSEventStreamEventFlagItemModified = 0x1000,
	kFSEventStreamEventFlagItemFinderInfoMod = 0x2000,
	kFSEventStreamEventFlagItemChangeOwner = 0x4000,
	kFSEventStreamEventFlagItemXattrMod = 0x8000,
	kFSEventStreamEventFlagItemIsFile = 0x10000,
	kFSEventStreamEventFlagItemIsDir = 0x20000,
	kFSEventStreamEventFlagItemIsSymlink = 0x40000,
	kFSEventStreamEventFlagOwnEvent = 0x80000,
	kFSEventStreamEventFlagItemIsHardlink = 0x100000,
	kFSEventStreamEventFlagItemIsLastHardlink = 0x200000,
	kFSEventStreamEventFlagItemCloned = 0x400000,
};

enum {
	kFSEventStreamCreateFlagUseCFTypes = 1,
	kFSEventStreamCreateFlagNoDefer = 2,
	kFSEventStreamCreateFlagWatchRoot = 4,
	kFSEventStreamCreateFlagIgnoreSelf = 8,
	kFSEventStreamCreateFlagFileEvents = 0x10,
	kFSEventStreamCreateFlagMarkSelf = 0x20,
	kFSEventStreamCreateFlagUseExtendedData = 0x40,
};

#define kFSEventStreamEventExtendedDataPathKey      CFSTR("path")
#define kFSEventStreamEventExtendedFileIDKey        CFSTR("fileID")

typedef CALLBACK_API_C(void, FSEventStreamCallback)(
		ConstFSEventStreamRef streamRef,
		void *clientCallBackInfo,
		size_t numEvents,
		void *eventPaths,
		const FSEventStreamEventFlags *eventFlags,
		const FSEventStreamEventId *eventIds);

struct FSEventStreamContext {
	CFIndex version;
	void *info;
	CFAllocatorRetainCallBack retain;
	CFAllocatorReleaseCallBack release;
	CFAllocatorCopyDescriptionCallBack copyDesc;
};
typedef struct FSEventStreamContext FSEventStreamContext;

extern FSEventStreamRef FSEventStreamCreate(
		CFAllocatorRef allocator,
		FSEventStreamCallback callback,
		FSEventStreamContext *context,
		CFArrayRef pathsToWatch,
		FSEventStreamEventId sinceWhen,
		CFTimeInterval latency,
		FSEventStreamCreateFlags flags);

extern CF_RETURNS_RETAINED CFArrayRef FSEventStreamCopyPathsBeingWatched(ConstFSEventStreamRef streamRef);

extern FSEventStreamEventId FSEventStreamGetLatestEventId(ConstFSEventStreamRef streamRef);

extern void FSEventStreamInvalidate(FSEventStreamRef streamRef);

extern void FSEventStreamRelease(FSEventStreamRef streamRef);

extern void FSEventStreamRetain(FSEventStreamRef streamRef);

extern void FSEventStreamScheduleWithRunLoop(FSEventStreamRef streamRef, CFRunLoopRef runLoop, CFStringRef runLoopMode);

extern void FSEventStreamSetDispatchQueue(FSEventStreamRef streamRef, dispatch_queue_t q);

extern Boolean FSEventStreamStart(FSEventStreamRef streamRef);

extern void FSEventStreamStop(FSEventStreamRef streamRef);

#endif
