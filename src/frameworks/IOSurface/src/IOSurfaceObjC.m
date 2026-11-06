/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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

#import <IOSurface/IOSurface.h>
#import <IOSurface/IOSurfaceObjC.h>
#import <IOSurface/IOSurfacePriv.h>
#import <Foundation/Foundation.h>
#import <IOKit/IOCFSerialize.h>
#include <stdatomic.h>
#include <stddef.h>
#include <sys/mman.h>

static io_service_t g_surfaceService;

typedef struct
{
	struct _IOSurfaceObjectRetval* surface;
	_Atomic int32_t localUseCount;
	size_t allocSize;
	NSMutableDictionary* attachments;
	// Allocated in this process because IOSurfaceRoot could not create the
	// surface. The planes live in an anonymous mapping of allocSize bytes.
	bool userSpace;
} ImplData;

// Plane layouts are limited to this many planes.
#define MAX_PLANES 16

// When the caller gives no element size, assume 4 bytes, the size of the
// common 32-bit pixel formats, so that the allocation is never too small for
// them.
#define DEFAULT_BYTES_PER_ELEMENT 4

// IDs of surfaces allocated in user space. They are unique only within this
// process: IOSurfaceRoot hands out global IDs, and it is not involved here.
static _Atomic uint32_t g_lastUserSpaceID;

static IOSurfaceID nextUserSpaceID(void)
{
	IOSurfaceID surfaceID;
	do
		surfaceID = atomic_fetch_add(&g_lastUserSpaceID, 1) + 1;
	while (surfaceID == 0);
	return surfaceID;
}

// Whether an IOSurfaceRoot reply describes a complete surface.
static bool validResponse(const void* bytes, size_t length)
{
	const struct _IOSurfaceObjectRetval* surface = bytes;
	size_t header = offsetof(struct _IOSurfaceObjectRetval, planes);
	if (length < header + sizeof(struct _IOSurfacePlane))
		return false;
	if (surface->planeCount == 0 || surface->planeCount > MAX_PLANES || surface->address == 0)
		return false;
	return length >= header + surface->planeCount * sizeof(struct _IOSurfacePlane);
}

static bool numberValue(id object, uint64_t* value)
{
	if (![object isKindOfClass: [NSNumber class]])
		return false;
	*value = [object unsignedLongLongValue];
	return true;
}

// Reads one plane's layout. defaults holds the values to use for missing
// keys and the minimum offset; the result is checked for overflow.
static bool readPlane(NSDictionary* info, CFStringRef widthKey, CFStringRef heightKey,
	CFStringRef bytesPerElementKey, CFStringRef bytesPerRowKey, CFStringRef offsetKey, CFStringRef sizeKey,
	uint64_t defaultWidth, uint64_t defaultHeight, uint64_t defaultBytesPerElement, uint64_t minimumOffset,
	struct _IOSurfacePlane* plane, uint64_t* end)
{
	uint64_t width = defaultWidth, height = defaultHeight, bytesPerElement = defaultBytesPerElement;
	uint64_t bytesPerRow = 0, offset = 0, size = 0;
	numberValue(info[(NSString*) widthKey], &width);
	numberValue(info[(NSString*) heightKey], &height);
	numberValue(info[(NSString*) bytesPerElementKey], &bytesPerElement);

	if (width == 0 || height == 0 || bytesPerElement == 0)
		return false;
	if (width > UINT32_MAX || height > UINT32_MAX || bytesPerElement > UINT32_MAX)
		return false;
	if (width > UINT64_MAX / bytesPerElement)
		return false;

	if (!numberValue(info[(NSString*) bytesPerRowKey], &bytesPerRow) || bytesPerRow == 0)
	{
		if (width * bytesPerElement > SIZE_MAX)
			return false;
		bytesPerRow = IOSurfaceAlignProperty(bytesPerRowKey, (size_t)(width * bytesPerElement));
	}
	if (bytesPerRow < width * bytesPerElement || bytesPerRow > UINT32_MAX)
		return false;

	if (!numberValue(info[(NSString*) offsetKey], &offset))
	{
		if (minimumOffset > SIZE_MAX)
			return false;
		offset = IOSurfaceAlignProperty(offsetKey, (size_t) minimumOffset);
		if (offset == 0 && minimumOffset != 0)
			return false;
	}
	if (offset < minimumOffset)
		return false;

	if (height > UINT64_MAX / bytesPerRow)
		return false;
	uint64_t bytes = bytesPerRow * height;
	if (sizeKey && numberValue(info[(NSString*) sizeKey], &size) && size > bytes)
		bytes = size;
	if (offset > UINT64_MAX - bytes || offset + bytes > SIZE_MAX)
		return false;

	*plane = (struct _IOSurfacePlane) {
		.memoryOffset = offset,
		.bytesPerElement = (uint32_t) bytesPerElement,
		.width = (uint32_t) width,
		.height = (uint32_t) height,
		.bytesPerRow = (uint32_t) bytesPerRow,
	};
	*end = offset + bytes;
	return true;
}

// The size of the memory that holds all planes: the end of the last plane.
static size_t planesEnd(const struct _IOSurfaceObjectRetval* surface)
{
	size_t end = 0;
	for (uint32_t i = 0; i < surface->planeCount; i++)
	{
		const struct _IOSurfacePlane* plane = &surface->planes[i];
		size_t planeEnd = plane->memoryOffset + (size_t) plane->bytesPerRow * plane->height;
		if (planeEnd > end)
			end = planeEnd;
	}
	return end;
}

@implementation IOSurface

+ (void)initialize
{
	if (self == [IOSurface self])
	{
		CFMutableDictionaryRef matching = IOServiceNameMatching("IOSurfaceRoot");

		g_surfaceService = IOServiceGetMatchingService(kIOMasterPortDefault, matching);

		CFRelease(matching);

		// Without IOSurfaceRoot, surfaces are allocated in user space.
	}
}

- (nullable instancetype)initWithResponse:(const void*)bytes length:(size_t)length
{
	ImplData* idata = (ImplData*) malloc(sizeof(ImplData));
	idata->surface = malloc(length);
	memcpy(idata->surface, bytes, length);

	idata->localUseCount = 0;
	idata->allocSize = planesEnd(idata->surface);
	idata->attachments = [[NSMutableDictionary alloc] init];
	idata->userSpace = false;
	self->_impl = idata;

	return self;
}

// Creates the surface in this process, with the layout the properties
// describe: kIOSurfacePlaneInfo for a planar surface, or the top-level
// width, height, element size and row stride for a single plane.
- (nullable instancetype)_initWithUserSpaceProperties:(NSDictionary <IOSurfacePropertyKey, id> *)properties
{
	if (![properties isKindOfClass: [NSDictionary class]])
	{
		[self release];
		return nil;
	}

	uint64_t width = 0, height = 0, bytesPerElement = DEFAULT_BYTES_PER_ELEMENT, pixelFormat = 0;
	numberValue(properties[(NSString*) kIOSurfaceWidth], &width);
	numberValue(properties[(NSString*) kIOSurfaceHeight], &height);
	numberValue(properties[(NSString*) kIOSurfaceBytesPerElement], &bytesPerElement);
	numberValue(properties[(NSString*) kIOSurfacePixelFormat], &pixelFormat);

	NSArray* planeInfo = properties[(NSString*) kIOSurfacePlaneInfo];
	if (planeInfo && (![planeInfo isKindOfClass: [NSArray class]] || planeInfo.count > MAX_PLANES))
	{
		[self release];
		return nil;
	}

	struct _IOSurfacePlane planes[MAX_PLANES];
	uint32_t planeCount = planeInfo.count ? (uint32_t) planeInfo.count : 1;
	uint64_t end = 0;

	if (!planeInfo.count)
	{
		if (!readPlane(properties, kIOSurfaceWidth, kIOSurfaceHeight, kIOSurfaceBytesPerElement,
				kIOSurfaceBytesPerRow, kIOSurfaceOffset, NULL, width, height, bytesPerElement, 0,
				&planes[0], &end))
		{
			[self release];
			return nil;
		}
	}
	else
	{
		for (uint32_t i = 0; i < planeCount; i++)
		{
			NSDictionary* info = planeInfo[i];
			if (![info isKindOfClass: [NSDictionary class]]
				|| !readPlane(info, kIOSurfacePlaneWidth, kIOSurfacePlaneHeight, kIOSurfacePlaneBytesPerElement,
					kIOSurfacePlaneBytesPerRow, kIOSurfacePlaneOffset, kIOSurfacePlaneSize,
					width, height, bytesPerElement, end, &planes[i], &end))
			{
				[self release];
				return nil;
			}
		}
	}

	uint64_t requested = 0;
	if (numberValue(properties[(NSString*) kIOSurfaceAllocSize], &requested) && requested > end)
	{
		if (requested > SIZE_MAX)
		{
			[self release];
			return nil;
		}
		end = requested;
	}
	size_t allocSize = IOSurfaceAlignProperty(kIOSurfaceAllocSize, (size_t) end);
	if (allocSize == 0)
	{
		[self release];
		return nil;
	}

	size_t retvalSize = offsetof(struct _IOSurfaceObjectRetval, planes) + planeCount * sizeof(struct _IOSurfacePlane);
	struct _IOSurfaceObjectRetval* surface = calloc(1, retvalSize);
	ImplData* idata = malloc(sizeof(ImplData));
	void* memory = mmap(NULL, allocSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if (!surface || !idata || memory == MAP_FAILED)
	{
		if (memory != MAP_FAILED)
			munmap(memory, allocSize);
		free(surface);
		free(idata);
		[self release];
		return nil;
	}

	surface->surfaceID = nextUserSpaceID();
	surface->pixelFormat = (uint32_t) pixelFormat;
	surface->address = (uint64_t)(uintptr_t) memory;
	surface->planeCount = planeCount;
	memcpy(surface->planes, planes, planeCount * sizeof(struct _IOSurfacePlane));

	idata->surface = surface;
	idata->localUseCount = 0;
	idata->allocSize = allocSize;
	idata->attachments = [[NSMutableDictionary alloc] init];
	idata->userSpace = true;
	self->_impl = idata;

	return self;
}

- (nullable instancetype)initWithProperties:(NSDictionary <IOSurfacePropertyKey, id> *)properties
{
	if (!g_surfaceService)
		return [self _initWithUserSpaceProperties: properties];

	CFDataRef data = IOCFSerialize((CFTypeRef) properties, kIOCFSerializeToBinary);
	uint8_t responseBuffer[3500];

	const void* bytes = CFDataGetBytePtr(data);
	size_t length = CFDataGetLength(data);
	size_t responseLength = sizeof(responseBuffer);

	kern_return_t ret = IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodCreate, NULL, 0, bytes, length, NULL, 0, responseBuffer, &responseLength);
	CFRelease(data);

	// Use IOSurfaceRoot's surface only if it described one completely.
	if (ret != kIOSurfaceSuccess || !validResponse(responseBuffer, responseLength))
		return [self _initWithUserSpaceProperties: properties];

	return [self initWithResponse: responseBuffer length: responseLength];
}

- (nullable instancetype)initWithSurfaceID:(IOSurfaceID)surfaceID
{
	uint8_t responseBuffer[3500];
	size_t responseLength = sizeof(responseBuffer);

	if (!g_surfaceService)
	{
		[self release];
		return nil;
	}

	uint64_t scalar = surfaceID;
	kern_return_t ret = IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodLookupByID, &scalar, 1, NULL, 0, NULL, 0, responseBuffer, &responseLength);

	if (ret != kIOSurfaceSuccess || !validResponse(responseBuffer, responseLength))
	{
		[self release];
		return nil;
	}

	return [self initWithResponse: responseBuffer length: responseLength];
}

- (void) dealloc
{
	ImplData* data = (ImplData*) _impl;
	if (data != NULL)
	{
		if (data->surface != NULL)
		{
			if (data->userSpace)
				munmap((void*)(uintptr_t) data->surface->address, data->allocSize);
			else
				IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodRelease, NULL, 0, &data->surface->surfaceID, 1, NULL, 0, NULL, 0);

			free(data->surface);
		}

		[data->attachments release];
		free(data);
	}
	[super dealloc];
}

- (NSUInteger) hash
{
	ImplData* data = (ImplData*) _impl;
	return data->surface->surfaceID;
}

- (BOOL)isEqual:(id)object
{
	if (![object isKindOfClass: [self class]])
		return NO;

	IOSurface* that = (IOSurface*) object;
	return ((ImplData*) self->_impl)->surface->surfaceID == ((ImplData*) that->_impl)->surface->surfaceID;
}

- (CFTypeID) _cfTypeID
{
    return IOSurfaceGetTypeID();
}

- (NSInteger) width
{
	return [self widthOfPlaneAtIndex: 0];
}

- (NSInteger) height
{
	return [self heightOfPlaneAtIndex: 0];
}

- (void *) baseAddress
{
	return [self baseAddressOfPlaneAtIndex: 0];
}

- (NSInteger) bytesPerRow
{
	return [self bytesPerRowOfPlaneAtIndex: 0];
}

- (NSInteger) bytesPerElement
{
	return [self bytesPerElementOfPlaneAtIndex: 0];
}

- (NSInteger) elementWidth
{
	return [self elementWidthOfPlaneAtIndex: 0];
}

- (NSInteger) elementHeight
{
	return [self elementHeightOfPlaneAtIndex: 0];
}

- (OSType) pixelFormat
{
	return ((ImplData*) _impl)->surface->pixelFormat;
}

- (NSUInteger) planeCount
{
	return ((ImplData*) _impl)->surface->planeCount;
}

- (NSInteger)widthOfPlaneAtIndex:(NSUInteger)planeIndex
{
	if (planeIndex >= [self planeCount])
		return 0;

	ImplData* data = (ImplData*) _impl;
	return data->surface->planes[planeIndex].width;
}

- (NSInteger)heightOfPlaneAtIndex:(NSUInteger)planeIndex
{
	if (planeIndex >= [self planeCount])
		return 0;

	ImplData* data = (ImplData*) _impl;
	return data->surface->planes[planeIndex].height;
}

- (NSInteger)bytesPerRowOfPlaneAtIndex:(NSUInteger)planeIndex
{
	if (planeIndex >= [self planeCount])
		return 0;

	ImplData* data = (ImplData*) _impl;
	return data->surface->planes[planeIndex].bytesPerRow;
}

- (NSInteger)bytesPerElementOfPlaneAtIndex:(NSUInteger)planeIndex
{
	if (planeIndex >= [self planeCount])
		return 0;

	ImplData* data = (ImplData*) _impl;
	return data->surface->planes[planeIndex].bytesPerElement;
}

- (NSInteger)elementWidthOfPlaneAtIndex:(NSUInteger)planeIndex
{
	return 1;
}

- (NSInteger)elementHeightOfPlaneAtIndex:(NSUInteger)planeIndex
{
	return 1;
}

- (void *)baseAddressOfPlaneAtIndex:(NSUInteger)planeIndex
{
	if (planeIndex >= [self planeCount])
		return NULL;

	ImplData* data = (ImplData*) _impl;
	return (void*)(data->surface->address + data->surface->planes[planeIndex].memoryOffset);
}

- (NSInteger) allocationSize
{
	return ((ImplData*) _impl)->allocSize;
}

// Attachments are values that belong to the surface object; the surface
// itself does not interpret them.
- (void)setAttachment:(id)anObject forKey:(NSString *)key
{
	if (!key)
		return;
	ImplData* data = (ImplData*) _impl;
	@synchronized(self)
	{
		if (anObject)
			[data->attachments setObject: anObject forKey: key];
		else
			[data->attachments removeObjectForKey: key];
	}
}

// Returns a retained value, for IOSurfaceCopyValue, which may be called
// without an autorelease pool.
- (nullable id)_copyAttachmentForKey:(NSString *)key
{
	if (!key)
		return nil;
	ImplData* data = (ImplData*) _impl;
	id value;
	@synchronized(self)
	{
		value = [[data->attachments objectForKey: key] retain];
	}
	return value;
}

- (nullable id)attachmentForKey:(NSString *)key
{
	return [[self _copyAttachmentForKey: key] autorelease];
}

- (void)removeAttachmentForKey:(NSString *)key
{
	if (!key)
		return;
	ImplData* data = (ImplData*) _impl;
	@synchronized(self)
	{
		[data->attachments removeObjectForKey: key];
	}
}

- (void)setAllAttachments:(NSDictionary<NSString *, id> *)dict
{
	ImplData* data = (ImplData*) _impl;
	@synchronized(self)
	{
		[data->attachments addEntriesFromDictionary: dict];
	}
}

- (NSDictionary<NSString *, id> *)_copyAllAttachments
{
	ImplData* data = (ImplData*) _impl;
	NSDictionary* copy;
	@synchronized(self)
	{
		copy = [data->attachments copy];
	}
	return copy;
}

- (nullable NSDictionary<NSString *, id> *)allAttachments
{
	return [[self _copyAllAttachments] autorelease];
}

- (void)removeAllAttachments
{
	ImplData* data = (ImplData*) _impl;
	@synchronized(self)
	{
		[data->attachments removeAllObjects];
	}
}

- (void)incrementUseCount
{
	ImplData* data = (ImplData*) _impl;
	if ((data->localUseCount++) == 0 && !data->userSpace)
	{
		uint64_t scalar = data->surface->surfaceID;
		IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodIncrementUseCount, &scalar, 1, NULL, 0, NULL, 0, NULL, 0);
	}
}

- (void)decrementUseCount
{
	ImplData* data = (ImplData*) _impl;
	if (--data->localUseCount == 0 && !data->userSpace)
	{
		uint64_t scalar = data->surface->surfaceID;
		IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodDecrementUseCount, &scalar, 1, NULL, 0, NULL, 0, NULL, 0);
	}
}

- (int32_t) localUseCount
{
	ImplData* data = (ImplData*) _impl;
	return data->localUseCount;
}

- (kern_return_t)lockWithOptions:(IOSurfaceLockOptions)options seed:(nullable uint32_t *)seed
{
	ImplData* data = (ImplData*) _impl;
	struct _IOSurfaceLockUnlock args = {
		.surfaceID = data->surface->surfaceID,
		.options = options,
	};
	// A surface in this process's own memory needs no lock in IOSurfaceRoot.
	if (data->userSpace)
		return kIOSurfaceSuccess;
	return IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodLock, NULL, 0, &args, sizeof(args), NULL, 0, NULL, 0);
}

- (kern_return_t)unlockWithOptions:(IOSurfaceLockOptions)options seed:(nullable uint32_t *)seed
{
	ImplData* data = (ImplData*) _impl;
	struct _IOSurfaceLockUnlock args = {
		.surfaceID = data->surface->surfaceID,
		.options = options,
	};
	if (data->userSpace)
		return kIOSurfaceSuccess;
	return IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodUnlock, NULL, 0, &args, sizeof(args), NULL, 0, NULL, 0);
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
