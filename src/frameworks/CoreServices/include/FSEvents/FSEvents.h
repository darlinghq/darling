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

#include <CoreFoundation/CoreFoundation.h>
#include <sys/types.h>
#include <Availability.h>

typedef struct __FSEventStream* FSEventStreamRef;
typedef const struct __FSEventStream* ConstFSEventStreamRef;

typedef UInt32 FSEventStreamCreateFlags;
typedef UInt64 FSEventStreamEventId;
typedef UInt32 FSEventStreamEventFlags;

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
