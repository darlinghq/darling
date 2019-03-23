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

#include <FSEvents/FSEvents.h>
#include <stdio.h>
#include <stdlib.h>

struct __FSEventStream {
	CFAllocatorRef allocator;
	CFArrayRef pathsToWatch;
};

FSEventStreamRef FSEventStreamCreate(
		CFAllocatorRef allocator,
		FSEventStreamCallback callback,
		FSEventStreamContext *context,
		CFArrayRef pathsToWatch,
		FSEventStreamEventId sinceWhen,
		CFTimeInterval latency,
		FSEventStreamCreateFlags flags)
{
	FSEventStreamRef stream = NULL;
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	stream = CFAllocatorAllocate(allocator, sizeof(struct __FSEventStream), 0);
	stream->allocator = allocator;
	stream->pathsToWatch = pathsToWatch;
	return stream;
}

CFArrayRef FSEventStreamCopyPathsBeingWatched(ConstFSEventStreamRef streamRef)
{
	CFArrayRef copy;
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	copy = CFArrayCreateCopy(streamRef->allocator, streamRef->pathsToWatch);
	copy = CFRetain(copy);
	return copy;
}
