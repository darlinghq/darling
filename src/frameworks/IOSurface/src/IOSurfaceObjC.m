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
#include "dmabuf.h"
#include <errno.h>
#include <fcntl.h>
#include <os/log.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/fileport.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static io_service_t g_surfaceService;

// Plane layouts are limited to this many planes.
#define MAX_PLANES 16

// A surface allocated in user space lives in a shared memory object, so that
// other processes can map it: IOSurfaceCreateMachPort wraps the object's
// descriptor in a fileport. The object starts with this header, which
// describes the surface to the process that receives it; the planes follow
// at dataOffset. The layout is the same for i386 and x86_64 processes.
#define SHARED_MAGIC 0x49535246 // 'ISRF'
#define SHARED_VERSION 1

enum
{
	kSharedPlanar = 1 << 0,
	kSharedPixelSizeCastingAllowed = 1 << 1,
	kSharedGlobal = 1 << 2,
};

struct SharedHeader
{
	uint32_t magic;
	uint32_t version;
	uint64_t dataOffset;
	uint64_t allocSize;
	IOSurfaceID surfaceID;
	uint32_t pixelFormat;
	uint32_t planeCount;
	uint32_t flags;
	struct _IOSurfacePlane planes[MAX_PLANES];

	// The state below is updated in place by every process that maps the
	// surface.

	// Incremented by every unlock that ends a lock for writing.
	_Atomic uint32_t seed;
	// The number of IOSurface objects, in all processes, whose use count
	// is not zero.
	_Atomic int32_t useCount;
};

_Static_assert(offsetof(struct SharedHeader, planes) == 40, "shared header layout");
_Static_assert(sizeof(struct SharedHeader) == 432, "shared header layout");

typedef struct
{
	struct _IOSurfaceObjectRetval* surface;
	_Atomic int32_t localUseCount;
	size_t allocSize;
	NSMutableDictionary* attachments;
	// Allocated in this process because IOSurfaceRoot could not create the
	// surface, or received from another process. The planes live in the
	// shared memory object (fd >= 0) or, if that could not be created, in an
	// anonymous mapping of allocSize bytes.
	bool userSpace;
	// The shared memory object, its mapping (header and planes), and its
	// identity, or -1 and NULL.
	int fd;
	void* mapping;
	size_t mappingSize;
	dev_t device;
	ino_t inode;
	// Where the planes start in the shared memory object.
	uint64_t dataOffset;
	// The planes exported as a DMA-BUF, or -1. Made on first request.
	_Atomic int dmabuf;
	bool dmabufTried;
	// The seed and the use count. Points into the mapping, or to memory
	// owned by this object when there is no shared memory object.
	struct SharedHeader* shared;
	// Counted in the registry that IOSurfaceLookup searches.
	bool registered;
	_Atomic uint32_t purgeability;
	bool allowsPixelSizeCasting;
	// Created with kIOSurfacePlaneInfo. A surface without it still has one
	// plane internally, but reports a plane count of 0.
	bool planar;
} ImplData;

static void initState(ImplData* idata)
{
	idata->localUseCount = 0;
	idata->attachments = [[NSMutableDictionary alloc] init];
	idata->fd = -1;
	idata->mapping = NULL;
	idata->mappingSize = 0;
	idata->device = 0;
	idata->inode = 0;
	idata->dataOffset = 0;
	idata->dmabuf = -1;
	idata->dmabufTried = false;
	idata->shared = NULL;
	idata->registered = false;
	idata->purgeability = kIOSurfacePurgeableNonVolatile;
	idata->allowsPixelSizeCasting = true;
	idata->planar = true;
}

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

// Creates an anonymous shared memory object: the name only has to be unique
// until it is unlinked, right after creation. Where the host can export
// surface memory to GPU drivers, the object is a memory file that can be.
static int createSharedObject(void)
{
	static _Atomic uint32_t counter;
	char name[32];

	int memory = _IOSurfaceDMABufCreateMemory();
	if (memory >= 0)
		return memory;

	for (int attempt = 0; attempt < 16; attempt++)
	{
		snprintf(name, sizeof(name), "/IOSurface.%d.%u", (int) getpid(), atomic_fetch_add(&counter, 1));
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0)
		{
			shm_unlink(name);
			return fd;
		}
		if (errno != EEXIST)
			break;
	}
	return -1;
}

// Whether a header read from a shared memory object of objectSize bytes
// describes a surface that fits in it. The header is checked on a private
// copy, since any process holding the object can write to it.
static bool validSharedHeader(const struct SharedHeader* header, uint64_t objectSize)
{
	if (header->magic != SHARED_MAGIC || header->version != SHARED_VERSION)
		return false;
	if (header->planeCount == 0 || header->planeCount > MAX_PLANES)
		return false;
	if (header->dataOffset < sizeof(struct SharedHeader) || header->dataOffset % getpagesize() != 0)
		return false;
	if (header->allocSize == 0 || header->allocSize > UINT64_MAX - header->dataOffset)
		return false;
	if (header->dataOffset + header->allocSize > objectSize || header->dataOffset + header->allocSize > SIZE_MAX)
		return false;

	for (uint32_t i = 0; i < header->planeCount; i++)
	{
		const struct _IOSurfacePlane* plane = &header->planes[i];
		if (plane->width == 0 || plane->height == 0 || plane->bytesPerElement == 0)
			return false;
		if ((uint64_t) plane->width * plane->bytesPerElement > plane->bytesPerRow)
			return false;
		if (plane->memoryOffset > header->allocSize
			|| (uint64_t) plane->bytesPerRow * plane->height > header->allocSize - plane->memoryOffset)
			return false;
	}
	return true;
}

// Surfaces created with kIOSurfaceIsGlobal, which IOSurfaceLookup finds by
// ID. Without IOSurfaceRoot, IDs are unique only within a process, so only
// surfaces created in this process are found. Each entry holds its own
// descriptor for the surface and counts the IOSurface objects that refer to
// it from this process's ID space; it goes away with the last of them.
struct GlobalSurface
{
	struct GlobalSurface* next;
	IOSurfaceID surfaceID;
	int fd;
	unsigned int references;
};

static pthread_mutex_t g_globalSurfacesLock = PTHREAD_MUTEX_INITIALIZER;
static struct GlobalSurface* g_globalSurfaces;

static struct GlobalSurface** findGlobalSurface(IOSurfaceID surfaceID)
{
	struct GlobalSurface** entry = &g_globalSurfaces;
	while (*entry && (*entry)->surfaceID != surfaceID)
		entry = &(*entry)->next;
	return entry;
}

static bool registerGlobalSurface(IOSurfaceID surfaceID, int fd)
{
	bool registered = false;

	pthread_mutex_lock(&g_globalSurfacesLock);
	struct GlobalSurface** entry = findGlobalSurface(surfaceID);
	if (*entry)
	{
		(*entry)->references++;
		registered = true;
	}
	else
	{
		struct GlobalSurface* surface = malloc(sizeof(*surface));
		int copy = fcntl(fd, F_DUPFD_CLOEXEC, 0);
		if (surface && copy >= 0)
		{
			*surface = (struct GlobalSurface) {
				.next = NULL,
				.surfaceID = surfaceID,
				.fd = copy,
				.references = 1,
			};
			*entry = surface;
			registered = true;
		}
		else
		{
			free(surface);
			if (copy >= 0)
				close(copy);
		}
	}
	pthread_mutex_unlock(&g_globalSurfacesLock);

	return registered;
}

static void unregisterGlobalSurface(IOSurfaceID surfaceID)
{
	pthread_mutex_lock(&g_globalSurfacesLock);
	struct GlobalSurface** entry = findGlobalSurface(surfaceID);
	struct GlobalSurface* surface = *entry;
	if (surface && --surface->references == 0)
		*entry = surface->next;
	else
		surface = NULL;
	pthread_mutex_unlock(&g_globalSurfacesLock);

	if (surface)
	{
		close(surface->fd);
		free(surface);
	}
}

// Returns a new descriptor for a registered surface, or -1.
static int copyGlobalSurface(IOSurfaceID surfaceID)
{
	int fd = -1;

	pthread_mutex_lock(&g_globalSurfacesLock);
	struct GlobalSurface* surface = *findGlobalSurface(surfaceID);
	if (surface)
		fd = fcntl(surface->fd, F_DUPFD_CLOEXEC, 0);
	pthread_mutex_unlock(&g_globalSurfacesLock);

	return fd;
}

// The GPU callbacks registered with _IOSurfaceSetGPUCallbacks. The state
// becomes 1 when a registration claims them and 2 once they are filled in.
static _IOSurfaceGPUCallbacks g_gpuCallbacks;
static _Atomic int g_gpuCallbacksState;

// Set while this thread runs the writeback callback, so that the locks the
// callback takes itself do not call it again.
static __thread bool t_inGPUWriteback;

void _IOSurfaceSetGPUCallbacks(const _IOSurfaceGPUCallbacks* callbacks)
{
	if (callbacks == NULL || callbacks->version != _IOSURFACE_GPU_CALLBACKS_VERSION)
	{
		os_log_error(OS_LOG_DEFAULT, "IOSurface: ignoring GPU callbacks with an unknown version");
		return;
	}

	int expected = 0;
	if (!atomic_compare_exchange_strong(&g_gpuCallbacksState, &expected, 1))
	{
		os_log_error(OS_LOG_DEFAULT, "IOSurface: GPU callbacks are already registered, ignoring another registration");
		return;
	}

	g_gpuCallbacks = *callbacks;
	atomic_store_explicit(&g_gpuCallbacksState, 2, memory_order_release);
}

static const _IOSurfaceGPUCallbacks* gpuCallbacks(void)
{
	if (atomic_load_explicit(&g_gpuCallbacksState, memory_order_acquire) != 2)
		return NULL;
	return &g_gpuCallbacks;
}

// Brings GPU work on the surface into its memory before the CPU or another
// process sees it.
static void writeBackGPUContents(IOSurface* surface)
{
	const _IOSurfaceGPUCallbacks* callbacks = gpuCallbacks();
	if (callbacks == NULL || callbacks->writeback == NULL || t_inGPUWriteback)
		return;

	t_inGPUWriteback = true;
	callbacks->writeback((IOSurfaceRef) surface);
	t_inGPUWriteback = false;
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
	struct _IOSurfaceObjectRetval* surface = malloc(length);
	struct SharedHeader* shared = calloc(1, sizeof(struct SharedHeader));
	if (!idata || !surface || !shared)
	{
		free(idata);
		free(surface);
		free(shared);
		[self release];
		return nil;
	}
	memcpy(surface, bytes, length);

	idata->surface = surface;
	initState(idata);
	idata->allocSize = planesEnd(idata->surface);
	idata->userSpace = false;
	idata->shared = shared;
	self->_impl = idata;

	return self;
}

// Creates the shared memory object for a surface of allocSize bytes and maps
// it. Returns the mapping, or NULL.
static struct SharedHeader* mapNewSharedObject(size_t allocSize, int* fd, size_t* mappingSize)
{
	size_t dataOffset = getpagesize();
	if (allocSize > SIZE_MAX - dataOffset)
		return NULL;

	int object = createSharedObject();
	if (object < 0)
		return NULL;

	size_t size = dataOffset + allocSize;
	void* mapping = MAP_FAILED;
	if ((off_t) size >= 0 && ftruncate(object, (off_t) size) == 0)
		mapping = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, object, 0);
	if (mapping == MAP_FAILED)
	{
		close(object);
		return NULL;
	}

	struct SharedHeader* header = mapping;
	header->dataOffset = dataOffset;
	header->allocSize = allocSize;
	*fd = object;
	*mappingSize = size;
	return header;
}

static void recordIdentity(ImplData* idata)
{
	struct stat st;
	if (fstat(idata->fd, &st) == 0)
	{
		idata->device = st.st_dev;
		idata->inode = st.st_ino;
	}
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
	if (!surface || !idata)
	{
		free(surface);
		free(idata);
		[self release];
		return nil;
	}

	// Without a shared memory object the surface still works in this
	// process, but cannot be passed to another one.
	int fd = -1;
	size_t mappingSize = 0;
	struct SharedHeader* shared = mapNewSharedObject(allocSize, &fd, &mappingSize);
	void* memory;
	if (shared)
		memory = (char*) shared + shared->dataOffset;
	else
	{
		shared = calloc(1, sizeof(struct SharedHeader));
		memory = mmap(NULL, allocSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
		if (!shared || memory == MAP_FAILED)
		{
			if (memory != MAP_FAILED)
				munmap(memory, allocSize);
			free(shared);
			free(surface);
			free(idata);
			[self release];
			return nil;
		}
	}

	surface->surfaceID = nextUserSpaceID();
	surface->pixelFormat = (uint32_t) pixelFormat;
	surface->address = (uint64_t)(uintptr_t) memory;
	surface->planeCount = planeCount;
	memcpy(surface->planes, planes, planeCount * sizeof(struct _IOSurfacePlane));

	idata->surface = surface;
	initState(idata);
	idata->allocSize = allocSize;
	idata->userSpace = true;
	idata->planar = planeInfo.count != 0;
	id casting = properties[(NSString*) kIOSurfacePixelSizeCastingAllowed];
	if ([casting respondsToSelector: @selector(boolValue)])
		idata->allowsPixelSizeCasting = [casting boolValue];
	id global = properties[(NSString*) kIOSurfaceIsGlobal];
	bool isGlobal = [global respondsToSelector: @selector(boolValue)] && [global boolValue];
	idata->shared = shared;
	if (fd >= 0)
	{
		idata->fd = fd;
		idata->mapping = shared;
		idata->mappingSize = mappingSize;
		idata->dataOffset = shared->dataOffset;
		recordIdentity(idata);
	}
	self->_impl = idata;

	shared->surfaceID = surface->surfaceID;
	shared->pixelFormat = surface->pixelFormat;
	shared->planeCount = planeCount;
	memcpy(shared->planes, planes, planeCount * sizeof(struct _IOSurfacePlane));
	shared->flags = (idata->planar ? kSharedPlanar : 0)
		| (idata->allowsPixelSizeCasting ? kSharedPixelSizeCastingAllowed : 0)
		| (isGlobal ? kSharedGlobal : 0);
	shared->magic = SHARED_MAGIC;
	shared->version = SHARED_VERSION;

	if (isGlobal && fd >= 0)
		idata->registered = registerGlobalSurface(surface->surfaceID, fd);

	return self;
}

// Maps a surface from a shared memory object made by another IOSurface
// object, possibly in another process. Takes ownership of fd. The geometry
// is copied from the header; only the seed and use count are used in place.
- (nullable instancetype)_initWithSharedObject:(int)fd
{
	struct stat st;
	struct SharedHeader header;
	if (fstat(fd, &st) != 0
		|| pread(fd, &header, sizeof(header), 0) != (ssize_t) sizeof(header)
		|| !validSharedHeader(&header, (uint64_t) st.st_size))
	{
		close(fd);
		[self release];
		return nil;
	}

	size_t mappingSize = (size_t)(header.dataOffset + header.allocSize);
	size_t retvalSize = offsetof(struct _IOSurfaceObjectRetval, planes) + header.planeCount * sizeof(struct _IOSurfacePlane);
	struct _IOSurfaceObjectRetval* surface = calloc(1, retvalSize);
	ImplData* idata = malloc(sizeof(ImplData));
	void* mapping = mmap(NULL, mappingSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!surface || !idata || mapping == MAP_FAILED)
	{
		if (mapping != MAP_FAILED)
			munmap(mapping, mappingSize);
		free(surface);
		free(idata);
		close(fd);
		[self release];
		return nil;
	}

	surface->surfaceID = header.surfaceID;
	surface->pixelFormat = header.pixelFormat;
	surface->address = (uint64_t)(uintptr_t)((char*) mapping + header.dataOffset);
	surface->planeCount = header.planeCount;
	memcpy(surface->planes, header.planes, header.planeCount * sizeof(struct _IOSurfacePlane));

	idata->surface = surface;
	initState(idata);
	idata->allocSize = (size_t) header.allocSize;
	idata->userSpace = true;
	idata->planar = (header.flags & kSharedPlanar) != 0;
	idata->allowsPixelSizeCasting = (header.flags & kSharedPixelSizeCastingAllowed) != 0;
	idata->fd = fd;
	idata->mapping = mapping;
	idata->mappingSize = mappingSize;
	idata->device = st.st_dev;
	idata->inode = st.st_ino;
	idata->dataOffset = header.dataOffset;
	idata->shared = mapping;
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

// The port is a fileport for the surface's shared memory object, made by
// -createMachPort. The caller keeps its right.
- (nullable instancetype)initWithMachPort:(mach_port_t)port
{
	int fd = MACH_PORT_VALID(port) ? fileport_makefd(port) : -1;
	if (fd < 0)
	{
		[self release];
		return nil;
	}
	return [self _initWithSharedObject: fd];
}

- (mach_port_t)createMachPort
{
	ImplData* data = (ImplData*) _impl;
	fileport_t port = MACH_PORT_NULL;
	if (data->fd >= 0)
		writeBackGPUContents(self);
	if (data->fd < 0 || fileport_makeport(data->fd, &port) != 0)
		return MACH_PORT_NULL;
	return port;
}

- (nullable instancetype)initWithSurfaceID:(IOSurfaceID)surfaceID
{
	uint8_t responseBuffer[3500];
	size_t responseLength = sizeof(responseBuffer);

	int fd = copyGlobalSurface(surfaceID);
	if (fd >= 0)
	{
		self = [self _initWithSharedObject: fd];
		if (self)
		{
			ImplData* data = (ImplData*) _impl;
			data->registered = registerGlobalSurface(surfaceID, data->fd);
		}
		return self;
	}

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
		const _IOSurfaceGPUCallbacks* callbacks = gpuCallbacks();
		if (callbacks != NULL && callbacks->forget != NULL)
			callbacks->forget((IOSurfaceRef) self);

		if (data->localUseCount > 0)
			atomic_fetch_sub(&data->shared->useCount, 1);
		if (data->registered)
			unregisterGlobalSurface(data->surface->surfaceID);

		if (data->dmabuf >= 0)
			close(data->dmabuf);
		if (data->fd >= 0)
		{
			munmap(data->mapping, data->mappingSize);
			close(data->fd);
		}
		else
		{
			if (data->userSpace)
				munmap((void*)(uintptr_t) data->surface->address, data->allocSize);
			else
				IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodRelease, NULL, 0, &data->surface->surfaceID, 1, NULL, 0, NULL, 0);
			free(data->shared);
		}

		free(data->surface);

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

	ImplData* data = (ImplData*) self->_impl;
	ImplData* other = (ImplData*) ((IOSurface*) object)->_impl;
	if (data->surface->surfaceID != other->surface->surfaceID)
		return NO;
	// IDs are unique only within the process that created the surface; a
	// surface received from another process can share an ID with one made
	// here. Shared memory objects tell the two apart.
	if (data->fd >= 0 && other->fd >= 0)
		return data->device == other->device && data->inode == other->inode;
	return data->fd < 0 && other->fd < 0;
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
	ImplData* data = (ImplData*) _impl;
	return data->planar ? data->surface->planeCount : 0;
}

// The planes that exist, including the single plane of a non-planar surface.
- (NSUInteger) _planeCount
{
	return ((ImplData*) _impl)->surface->planeCount;
}

- (NSInteger)widthOfPlaneAtIndex:(NSUInteger)planeIndex
{
	if (planeIndex >= [self _planeCount])
		return 0;

	ImplData* data = (ImplData*) _impl;
	return data->surface->planes[planeIndex].width;
}

- (NSInteger)heightOfPlaneAtIndex:(NSUInteger)planeIndex
{
	if (planeIndex >= [self _planeCount])
		return 0;

	ImplData* data = (ImplData*) _impl;
	return data->surface->planes[planeIndex].height;
}

- (NSInteger)bytesPerRowOfPlaneAtIndex:(NSUInteger)planeIndex
{
	if (planeIndex >= [self _planeCount])
		return 0;

	ImplData* data = (ImplData*) _impl;
	return data->surface->planes[planeIndex].bytesPerRow;
}

- (NSInteger)bytesPerElementOfPlaneAtIndex:(NSUInteger)planeIndex
{
	if (planeIndex >= [self _planeCount])
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
	if (planeIndex >= [self _planeCount])
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
	if ((data->localUseCount++) == 0)
	{
		atomic_fetch_add(&data->shared->useCount, 1);
		if (!data->userSpace)
		{
			uint64_t scalar = data->surface->surfaceID;
			IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodIncrementUseCount, &scalar, 1, NULL, 0, NULL, 0, NULL, 0);
		}
	}
}

- (void)decrementUseCount
{
	ImplData* data = (ImplData*) _impl;
	if (--data->localUseCount == 0)
	{
		atomic_fetch_sub(&data->shared->useCount, 1);
		if (!data->userSpace)
		{
			uint64_t scalar = data->surface->surfaceID;
			IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodDecrementUseCount, &scalar, 1, NULL, 0, NULL, 0, NULL, 0);
		}
	}
}

- (int32_t) localUseCount
{
	ImplData* data = (ImplData*) _impl;
	return data->localUseCount;
}

// Uses counted by any process that shares the surface.
- (BOOL) isInUse
{
	return ((ImplData*) _impl)->shared->useCount > 0;
}

- (uint32_t) seed
{
	return ((ImplData*) _impl)->shared->seed;
}

- (BOOL) allowsPixelSizeCasting
{
	return ((ImplData*) _impl)->allowsPixelSizeCasting;
}

// The state is recorded, but the contents of a volatile surface are never
// discarded, which the purgeability contract allows.
- (kern_return_t)setPurgeable:(IOSurfacePurgeabilityState)newState oldState:(IOSurfacePurgeabilityState * _Nullable)oldState
{
	ImplData* data = (ImplData*) _impl;
	if (newState > kIOSurfacePurgeableKeepCurrent)
		return kIOReturnBadArgument;
	uint32_t previous = newState == kIOSurfacePurgeableKeepCurrent
		? data->purgeability
		: atomic_exchange(&data->purgeability, newState);
	if (oldState)
		*oldState = previous;
	return kIOSurfaceSuccess;
}

- (kern_return_t)lockWithOptions:(IOSurfaceLockOptions)options seed:(nullable uint32_t *)seed
{
	ImplData* data = (ImplData*) _impl;
	// CPU access sees the GPU work submitted before the lock.
	writeBackGPUContents(self);

	struct _IOSurfaceLockUnlock args = {
		.surfaceID = data->surface->surfaceID,
		.options = options,
	};
	// A surface in this process's own memory needs no lock in IOSurfaceRoot.
	kern_return_t ret = kIOSurfaceSuccess;
	if (!data->userSpace)
		ret = IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodLock, NULL, 0, &args, sizeof(args), NULL, 0, NULL, 0);
	if (ret != kIOSurfaceSuccess)
		return ret;
	// Devices that share the memory finish with it before the CPU reads it.
	_IOSurfaceDMABufBeginCPUAccess(data->dmabuf, !(options & kIOSurfaceLockReadOnly));
	if (seed)
		*seed = data->shared->seed;
	return ret;
}

- (kern_return_t)unlockWithOptions:(IOSurfaceLockOptions)options seed:(nullable uint32_t *)seed
{
	ImplData* data = (ImplData*) _impl;
	struct _IOSurfaceLockUnlock args = {
		.surfaceID = data->surface->surfaceID,
		.options = options,
	};
	kern_return_t ret = kIOSurfaceSuccess;
	if (!data->userSpace)
		ret = IOConnectCallMethod(g_surfaceService, kIOSurfaceMethodUnlock, NULL, 0, &args, sizeof(args), NULL, 0, NULL, 0);
	if (ret != kIOSurfaceSuccess)
		return ret;
	_IOSurfaceDMABufEndCPUAccess(data->dmabuf, !(options & kIOSurfaceLockReadOnly));
	// The end of a lock for writing marks the contents as modified.
	uint32_t current = (options & kIOSurfaceLockReadOnly)
		? data->shared->seed
		: atomic_fetch_add(&data->shared->seed, 1) + 1;
	if (seed)
		*seed = current;
	return ret;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

static pthread_mutex_t g_dmabufLock = PTHREAD_MUTEX_INITIALIZER;

// Exports the planes of a surface in a shared memory object, once. Surfaces
// that IOSurfaceRoot allocated, and those in a private mapping, have none.
static int surfaceDMABuf(ImplData* data)
{
	int dmabuf = data->dmabuf;
	if (dmabuf >= 0 || data->fd < 0)
		return dmabuf;

	pthread_mutex_lock(&g_dmabufLock);
	if (!data->dmabufTried)
	{
		data->dmabufTried = true;
		size_t pageSize = (size_t) getpagesize();
		if (data->dataOffset % pageSize == 0 && data->allocSize % pageSize == 0)
			data->dmabuf = _IOSurfaceDMABufExport(data->fd, data->dataOffset, data->allocSize);
	}
	dmabuf = data->dmabuf;
	pthread_mutex_unlock(&g_dmabufLock);
	return dmabuf;
}

bool _IOSurfaceGetDMABuf(IOSurfaceRef buffer, size_t planeIndex, _IOSurfaceDMABuf* out)
{
	if (!buffer || !out || out->version != _IOSURFACE_DMABUF_VERSION)
		return false;

	ImplData* data = (ImplData*) ((IOSurface*) buffer)->_impl;
	if (planeIndex >= data->surface->planeCount)
		return false;

	int dmabuf = surfaceDMABuf(data);
	if (dmabuf < 0)
		return false;

	const struct _IOSurfacePlane* plane = &data->surface->planes[planeIndex];
	out->fd = dmabuf;
	out->size = data->allocSize;
	out->offset = plane->memoryOffset;
	out->bytesPerRow = plane->bytesPerRow;
	return true;
}

IOSurfaceID IOSurfaceGetID(IOSurfaceRef buffer)
{
	if (!buffer)
		return 0;
	return ((ImplData*) ((IOSurface*) buffer)->_impl)->surface->surfaceID;
}
