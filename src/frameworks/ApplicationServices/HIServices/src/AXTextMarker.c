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

#include <HIServices/AXTextMarker.h>
#include <CoreFoundation/CFRuntime.h>
#include <pthread.h>
#include <stdatomic.h>
#include <string.h>

struct __AXTextMarker {
	CFRuntimeBase base;
	CFIndex length;
	UInt8 bytes[];
};

struct __AXTextMarkerRange {
	CFRuntimeBase base;
	AXTextMarkerRef start;
	AXTextMarkerRef end;
};

static Boolean markerEqual(CFTypeRef a, CFTypeRef b)
{
	const struct __AXTextMarker *x = a, *y = b;
	return x->length == y->length && memcmp(x->bytes, y->bytes, x->length) == 0;
}

static CFHashCode markerHash(CFTypeRef cf)
{
	const struct __AXTextMarker *marker = cf;
	CFHashCode hash = 5381;
	for (CFIndex i = 0; i < marker->length; i++)
		hash = hash * 33 + marker->bytes[i];
	return hash;
}

static void rangeFinalize(CFTypeRef cf)
{
	const struct __AXTextMarkerRange *range = cf;
	CFRelease(range->start);
	CFRelease(range->end);
}

static Boolean rangeEqual(CFTypeRef a, CFTypeRef b)
{
	const struct __AXTextMarkerRange *x = a, *y = b;
	return CFEqual(x->start, y->start) && CFEqual(x->end, y->end);
}

static CFHashCode rangeHash(CFTypeRef cf)
{
	const struct __AXTextMarkerRange *range = cf;
	return CFHash(range->start) * 31 + CFHash(range->end);
}

static const CFRuntimeClass markerClass = {
	0, "AXTextMarker", NULL, NULL, NULL, markerEqual, markerHash, NULL, NULL,
};

static const CFRuntimeClass rangeClass = {
	0, "AXTextMarkerRange", NULL, NULL, rangeFinalize, rangeEqual, rangeHash, NULL, NULL,
};

static CFTypeID markerTypeID, rangeTypeID;
static pthread_once_t registerOnce = PTHREAD_ONCE_INIT;

static void registerClasses(void)
{
	markerTypeID = _CFRuntimeRegisterClass(&markerClass);
	rangeTypeID = _CFRuntimeRegisterClass(&rangeClass);
}

CFTypeID AXTextMarkerGetTypeID(void)
{
	pthread_once(&registerOnce, registerClasses);
	return markerTypeID;
}

CFTypeID AXTextMarkerRangeGetTypeID(void)
{
	pthread_once(&registerOnce, registerClasses);
	return rangeTypeID;
}

AXTextMarkerRef AXTextMarkerCreate(CFAllocatorRef allocator, const UInt8 *bytes, CFIndex length)
{
	if (length < 0 || (bytes == NULL && length > 0))
		return NULL;
	struct __AXTextMarker *marker = (struct __AXTextMarker *) _CFRuntimeCreateInstance(allocator,
		AXTextMarkerGetTypeID(), sizeof(struct __AXTextMarker) - sizeof(CFRuntimeBase) + length, NULL);
	if (marker == NULL)
		return NULL;
	marker->length = length;
	if (length > 0)
		memcpy(marker->bytes, bytes, length);
	return marker;
}

CFIndex AXTextMarkerGetLength(AXTextMarkerRef marker)
{
	return marker != NULL ? marker->length : 0;
}

const UInt8 *AXTextMarkerGetBytePtr(AXTextMarkerRef marker)
{
	return marker != NULL ? marker->bytes : NULL;
}

AXTextMarkerRangeRef AXTextMarkerRangeCreate(CFAllocatorRef allocator, AXTextMarkerRef startMarker, AXTextMarkerRef endMarker)
{
	if (startMarker == NULL || endMarker == NULL)
		return NULL;
	struct __AXTextMarkerRange *range = (struct __AXTextMarkerRange *) _CFRuntimeCreateInstance(allocator,
		AXTextMarkerRangeGetTypeID(), sizeof(struct __AXTextMarkerRange) - sizeof(CFRuntimeBase), NULL);
	if (range == NULL)
		return NULL;
	range->start = CFRetain(startMarker);
	range->end = CFRetain(endMarker);
	return range;
}

AXTextMarkerRef AXTextMarkerRangeCopyStartMarker(AXTextMarkerRangeRef range)
{
	return range != NULL ? CFRetain(range->start) : NULL;
}

AXTextMarkerRef AXTextMarkerRangeCopyEndMarker(AXTextMarkerRangeRef range)
{
	return range != NULL ? CFRetain(range->end) : NULL;
}

// Darling has no accessibility server, so no assistive application ever sends
// a request: there is no current client unless one has been set as an
// override, and requests are never served on a secondary thread.
static _Atomic AXClientType clientOverride = kAXClientTypeNoActiveRequestFound;

AXClientType _AXGetClientForCurrentRequestUntrusted(void)
{
	return clientOverride;
}

void _AXSetClientIdentificationOverride(AXClientType client)
{
	clientOverride = client;
}

bool _AXUIElementRequestServicedBySecondaryAXThread(void)
{
	return false;
}

AXError _AXUIElementUseSecondaryAXThread(bool enabled)
{
	(void) enabled;
	return kAXErrorSuccess;
}
