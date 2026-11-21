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
#include "gpubuffer.h"
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
#include <string.h>
#include <unistd.h>
#include <darling/emulation/linux_premigration/ext/for-iosurface.h>
#import <darling-stub-forwarding.h>

static io_service_t g_surfaceService;

// Plane layouts are limited to this many planes.
#define MAX_PLANES 16

// A surface allocated in user space lives in a shared memory object, so that
// other processes can map it: IOSurfaceCreateMachPort wraps the object's
// descriptor in a fileport. The object starts with this header, which
// describes the surface to the process that receives it; the planes follow
// at dataOffset. The layout is the same for i386 and x86_64 processes.
#define SHARED_MAGIC 0x49535246 // 'ISRF'
#define SHARED_VERSION 3

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

	// Changed whenever the contents may have changed: by every lock for
	// writing, and when GPU rendering reaches the memory (see
	// "Versions of the contents").
	_Atomic uint32_t seed;
	// The number of IOSurface objects, in all processes, whose use count
	// is not zero.
	_Atomic int32_t useCount;
	// The version of the contents in the surface's memory, and the newest
	// version a GPU API holds outside of it. The memory is up to date while
	// cpuVersion >= gpuVersion.
	_Atomic uint64_t cpuVersion;
	_Atomic uint64_t gpuVersion;
	// The version of the contents the surface's GPU buffer holds (see "GPU
	// buffers"); 0 while it has none.
	_Atomic uint64_t bufferVersion;
	// Written by the IOSurface service only (see "The IOSurface service"):
	// the number of processes holding the surface that may read its memory,
	// or UINT32_MAX while that is not known.
	_Atomic uint32_t memoryProcesses;
	// Not 0 once the IOSurface service keeps a GPU buffer for the surface.
	_Atomic uint32_t hasGPUBuffer;
};

_Static_assert(offsetof(struct SharedHeader, planes) == 40, "shared header layout");
_Static_assert(offsetof(struct SharedHeader, cpuVersion) == 432, "shared header layout");
_Static_assert(offsetof(struct SharedHeader, memoryProcesses) == 456, "shared header layout");
_Static_assert(sizeof(struct SharedHeader) == 464, "shared header layout");

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
	// The surface's GPU buffer in this process, once this object has found
	// it (see "GPU buffers"); guarded by g_gpuSurfacesLock.
	struct GPUSurface* gpu;
	// The surface's ID in the IOSurface service, or 0 if the service does
	// not hold it for this process.
	uint32_t serviceID;
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
	idata->gpu = NULL;
	idata->serviceID = 0;
}

// When the caller gives no element size, assume 4 bytes, the size of the
// common 32-bit pixel formats, so that the allocation is never too small for
// them.
#define DEFAULT_BYTES_PER_ELEMENT 4

// IDs of surfaces allocated in user space when the IOSurface service is not
// available. They are unique only within this process.
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

// The GPU callbacks registered with _IOSurfaceSetGPUCallbacks, one slot per
// GPU API. A registration fills in the next slot, then publishes it by
// raising g_gpuCallbacksCount; slots are never changed afterwards.
#define MAX_GPU_CALLBACKS 4
static _IOSurfaceGPUCallbacks g_gpuCallbacks[MAX_GPU_CALLBACKS];
static _Atomic int g_gpuCallbacksCount;
static pthread_mutex_t g_gpuCallbacksMutex = PTHREAD_MUTEX_INITIALIZER;

// Set while this thread runs a writeback callback, so that the locks the
// callback takes itself do not call any writeback again.
static __thread bool t_inGPUWriteback;

// Versions of the contents.
//
// A GPU API that keeps its own copy of a surface (a texture that is not a
// view of the surface's memory) can hold contents newer than the memory:
// rendering into the copy reaches the memory only when the API's writeback
// callback copies it there. The shared header records this with two
// counters, so that every process sharing the surface agrees on it:
//  - a GPU API that renders into its copy raises gpuVersion above both
//    (_IOSurfaceGPUDidWrite); the seed does not change yet, since the memory
//    has not;
//  - a lock runs the writeback callbacks and then raises cpuVersion to the
//    gpuVersion it saw before them: the memory holds the newest contents,
//    or the APIs chose to keep it as it was;
//  - a lock with kIOSurfaceLockAvoidSync fails with kIOReturnCannotLock
//    instead while gpuVersion is above cpuVersion, as that lock would have
//    to copy GPU contents into the memory first;
//  - the end of a lock for writing, and GPU rendering into the memory
//    itself (_IOSurfaceGPUDidWriteMemory), raise cpuVersion above both, so
//    that GPU APIs know their copies are older.

// Raises *version to at least value.
static void raiseVersion(_Atomic uint64_t* version, uint64_t value)
{
	uint64_t current = atomic_load(version);
	while (current < value && !atomic_compare_exchange_weak(version, &current, value))
		;
}

// Makes *version the newest of the two versions, and returns it.
static uint64_t bumpVersion(struct SharedHeader* shared, _Atomic uint64_t* version)
{
	for (;;)
	{
		uint64_t cpu = atomic_load(&shared->cpuVersion);
		uint64_t gpu = atomic_load(&shared->gpuVersion);
		uint64_t current = version == &shared->cpuVersion ? cpu : gpu;
		uint64_t next = (cpu > gpu ? cpu : gpu) + 1;
		if (atomic_compare_exchange_weak(version, &current, next))
			return next;
	}
}

static bool gpuContentsPending(struct SharedHeader* shared)
{
	return atomic_load(&shared->gpuVersion) > atomic_load(&shared->cpuVersion);
}

// The IOSurface service.
//
// darlingserver keeps, for surfaces in shared memory objects, the state
// that IOSurfaceRoot keeps in the kernel on macOS (see "IOSurface service"
// in darlingserver's rpc-supplement.h): IDs that are unique across
// processes, the global surfaces any process finds by ID, and the surface's
// GPU buffer, which every process that shares the surface can get from it.
// It also counts the processes that hold the surface and may read its
// memory, and writes that count into the shared header (memoryProcesses).
// A process holds a surface once, however many IOSurface objects it has for
// it; the service drops the holds of a process that exits or execs. Without
// the service, IDs are unique only within a process and GPU buffers stay in
// the process that made them.

struct ServiceSurface
{
	struct ServiceSurface* next;
	dev_t device;
	ino_t inode;
	uint32_t serviceID;
	// IOSurface objects of this process for the surface.
	unsigned int references;
	// DSERVER_IOSURFACE_USAGE_* bits the service has recorded.
	uint32_t usage;
	bool global;
	// The memory object, for a child process to hold the surface with.
	int fd;
};

// Lock order: g_serviceLock before g_gpuSurfacesLock.
static pthread_mutex_t g_serviceLock = PTHREAD_MUTEX_INITIALIZER;
static struct ServiceSurface* g_serviceSurfaces;

static pthread_once_t g_forkHandlersOnce = PTHREAD_ONCE_INIT;
static void registerForkHandlers(void);

static struct ServiceSurface** findServiceSurface(dev_t device, ino_t inode)
{
	struct ServiceSurface** entry = &g_serviceSurfaces;
	while (*entry && ((*entry)->device != device || (*entry)->inode != inode))
		entry = &(*entry)->next;
	return entry;
}

// Holds the surface in the memory object fd for this process. Returns its
// ID in the service, or 0. idHint and global describe the surface to a
// service that does not know it.
static uint32_t serviceRetain(int fd, dev_t device, ino_t inode, uint32_t idHint, bool global)
{
	pthread_once(&g_forkHandlersOnce, registerForkHandlers);

	uint32_t serviceID = 0;
	pthread_mutex_lock(&g_serviceLock);
	struct ServiceSurface** entry = findServiceSurface(device, inode);
	if (*entry)
	{
		(*entry)->references++;
		serviceID = (*entry)->serviceID;
	}
	else
	{
		struct ServiceSurface* surface = malloc(sizeof(*surface));
		int copy = fcntl(fd, F_DUPFD_CLOEXEC, 0);
		if (surface && copy >= 0
			&& __darling_iosurface_retain(fd, idHint, global ? DSERVER_IOSURFACE_GLOBAL : 0,
				offsetof(struct SharedHeader, memoryProcesses), &serviceID) == 0)
		{
			*surface = (struct ServiceSurface) {
				.next = NULL,
				.device = device,
				.inode = inode,
				.serviceID = serviceID,
				.references = 1,
				.usage = 0,
				.global = global,
				.fd = copy,
			};
			*entry = surface;
		}
		else
		{
			serviceID = 0;
			free(surface);
			if (copy >= 0)
				close(copy);
		}
	}
	pthread_mutex_unlock(&g_serviceLock);
	return serviceID;
}

static void serviceRelease(ImplData* data)
{
	if (data->serviceID == 0)
		return;
	pthread_mutex_lock(&g_serviceLock);
	struct ServiceSurface** entry = findServiceSurface(data->device, data->inode);
	struct ServiceSurface* surface = *entry;
	if (surface && --surface->references == 0)
	{
		*entry = surface->next;
		__darling_iosurface_release(surface->serviceID);
		close(surface->fd);
		free(surface);
	}
	pthread_mutex_unlock(&g_serviceLock);
}

// Records with the service how this process uses the surface. With
// recordedOnly, the service has recorded it already.
static void serviceSetUsage(ImplData* data, uint32_t usage, bool recordedOnly)
{
	if (data->serviceID == 0)
		return;
	pthread_mutex_lock(&g_serviceLock);
	struct ServiceSurface* surface = *findServiceSurface(data->device, data->inode);
	if (surface && (surface->usage & usage) != usage)
	{
		if (recordedOnly || __darling_iosurface_set_usage(surface->serviceID, usage) == 0)
			surface->usage |= usage;
	}
	pthread_mutex_unlock(&g_serviceLock);
}

// GPU buffers.
//
// A GPU API whose driver cannot render into the surface's memory gives the
// surface a GPU buffer in the driver's own layout (_IOSurfaceGetGPUBuffer,
// gpubuffer.h) and renders into that instead. The buffer is a second store
// of the contents, next to the memory, which stays what the CPU sees. The
// IOSurface service keeps the buffer, so that every process sharing the
// surface uses the same one: GPU APIs of processes on the buffer's render
// node import it, and each process copies between the buffer and the memory
// on the GPU itself. The versions of the contents (bufferVersion for the
// buffer) say which copy is needed:
//  - Completed rendering into the buffer (_IOSurfaceGPUDidWriteBuffer)
//    raises gpuVersion and gives the buffer that version, and the seed
//    changes. If any process holding the surface may read its memory (it
//    has not imported the buffer, or uses the memory as well; the service
//    counts them in memoryProcesses), the buffer is copied into the memory
//    there and then, so that, as on macOS, the memory holds the rendering
//    once it has been flushed. Otherwise the copy waits for a lock.
//  - A lock, in any process, copies the buffer into the memory while the
//    buffer is newer, also a lock with kIOSurfaceLockAvoidSync: on macOS
//    such a lock after a flush succeeds and sees the rendering. Making the
//    Mach port, fork() and the last release of the buffer in a process do
//    the same, for processes that get the memory later.
//  - The end of a lock for writing copies the memory into the buffer in a
//    process that has the buffer. Other processes leave it to the GPU API,
//    which calls _IOSurfaceGPUWillUse before it uses the buffer.
// Each process uses the buffer through one record, found through the
// identity of the surface's memory object by every IOSurface object of the
// process for the surface; the record goes away with the last of them.

struct GPUSurface
{
	struct GPUSurface* next;
	dev_t device;
	ino_t inode;
	// IOSurface objects of this process that have found it.
	unsigned int references;
	// NULL in a child process after fork().
	_IOSurfaceGPUBo* bo;
	// The memory object mapped for the record, which outlives the objects'
	// own mappings: its shared header and the plane the buffer holds.
	void* mapping;
	size_t mappingSize;
	struct SharedHeader* shared;
	void* memory;
	size_t bytesPerRow;
	uint64_t downloads, uploads;
};

// Guards the list, the objects' links to it and the buffers, and is held
// during copies, which it serialises within the process.
static pthread_mutex_t g_gpuSurfacesLock = PTHREAD_MUTEX_INITIALIZER;
static struct GPUSurface* g_gpuSurfaces;
// The number of records, read without the lock so that surfaces skip the
// work while there are none.
static _Atomic int g_gpuSurfaceCount;

static bool mayHaveGPUSurface(ImplData* data)
{
	return data->fd >= 0 && (data->gpu != NULL || atomic_load_explicit(&g_gpuSurfaceCount, memory_order_relaxed) > 0
		|| atomic_load_explicit(&data->shared->hasGPUBuffer, memory_order_relaxed) != 0);
}

static bool bufferNewer(struct SharedHeader* shared)
{
	return atomic_load(&shared->bufferVersion) > atomic_load(&shared->cpuVersion);
}

// Whether a process holding the surface may read its memory.
static bool memoryRead(struct SharedHeader* shared)
{
	return atomic_load(&shared->memoryProcesses) != 0;
}

// The surface's record in this process, or NULL. Called with
// g_gpuSurfacesLock held.
static struct GPUSurface* findGPUSurface(ImplData* data)
{
	if (data->gpu != NULL || data->fd < 0)
		return data->gpu;
	for (struct GPUSurface* gpu = g_gpuSurfaces; gpu != NULL; gpu = gpu->next)
	{
		if (gpu->device == data->device && gpu->inode == data->inode)
		{
			gpu->references++;
			data->gpu = gpu;
			return gpu;
		}
	}
	return NULL;
}

// Makes the record for bo, the surface's buffer, and links the object to
// it. Destroys bo if it cannot. Called with g_gpuSurfacesLock held.
static struct GPUSurface* newGPUSurface(ImplData* data, _IOSurfaceGPUBo* bo)
{
	struct GPUSurface* gpu = calloc(1, sizeof(*gpu));
	void* mapping = gpu ? mmap(NULL, data->mappingSize, PROT_READ | PROT_WRITE, MAP_SHARED, data->fd, 0) : MAP_FAILED;
	if (mapping == MAP_FAILED)
	{
		free(gpu);
		_IOSurfaceGPUBoDestroy(bo);
		return NULL;
	}
	*gpu = (struct GPUSurface) {
		.next = g_gpuSurfaces,
		.device = data->device,
		.inode = data->inode,
		.references = 1,
		.bo = bo,
		.mapping = mapping,
		.mappingSize = data->mappingSize,
		.shared = mapping,
		.memory = (char*) mapping + data->dataOffset + data->surface->planes[0].memoryOffset,
		.bytesPerRow = data->surface->planes[0].bytesPerRow,
	};
	g_gpuSurfaces = gpu;
	atomic_fetch_add(&g_gpuSurfaceCount, 1);
	data->gpu = gpu;
	return gpu;
}

// Copies the buffer into the memory if it is newer. Called with
// g_gpuSurfacesLock held.
static void downloadGPUSurface(struct GPUSurface* gpu)
{
	uint64_t version = atomic_load(&gpu->shared->bufferVersion);
	if (gpu->bo == NULL || version <= atomic_load(&gpu->shared->cpuVersion))
		return;
	if (_IOSurfaceGPUBoDownload(gpu->bo, gpu->memory, gpu->bytesPerRow))
	{
		gpu->downloads++;
		raiseVersion(&gpu->shared->cpuVersion, version);
	}
}

// Copies the memory into the buffer if it is newer. Called with
// g_gpuSurfacesLock held.
static void uploadGPUSurface(struct GPUSurface* gpu)
{
	uint64_t cpu = atomic_load(&gpu->shared->cpuVersion);
	if (gpu->bo == NULL || cpu <= atomic_load(&gpu->shared->bufferVersion))
		return;
	if (_IOSurfaceGPUBoUpload(gpu->bo, gpu->memory, gpu->bytesPerRow))
	{
		gpu->uploads++;
		raiseVersion(&gpu->shared->bufferVersion, cpu);
	}
}

static void releaseGPUSurface(ImplData* data)
{
	pthread_mutex_lock(&g_gpuSurfacesLock);
	struct GPUSurface* gpu = data->gpu;
	data->gpu = NULL;
	if (gpu != NULL && --gpu->references == 0)
	{
		struct GPUSurface** entry = &g_gpuSurfaces;
		while (*entry != gpu)
			entry = &(*entry)->next;
		*entry = gpu->next;
		atomic_fetch_sub(&g_gpuSurfaceCount, 1);
		// Contents only the buffer holds stay with the surface, also if no
		// process that can copy them holds it any more.
		downloadGPUSurface(gpu);
		_IOSurfaceGPUBoDestroy(gpu->bo);
		munmap(gpu->mapping, gpu->mappingSize);
		free(gpu);
	}
	pthread_mutex_unlock(&g_gpuSurfacesLock);
}

// Imports the GPU buffer the service keeps for the surface, if it is on
// renderNode (any render node if NULL), and makes the record for it.
// Called with g_gpuSurfacesLock held.
static struct GPUSurface* importGPUSurface(ImplData* data, const char* renderNode)
{
	if (data->serviceID == 0 || !atomic_load(&data->shared->hasGPUBuffer))
		return NULL;
	dserver_iosurface_gpu_buffer_t info;
	memset(&info, 0, sizeof(info));
	int fd = -1;
	if (__darling_iosurface_copy_gpu_buffer(data->serviceID, &info, &fd) != 0)
		return NULL;
	_IOSurfaceGPUBo* bo = NULL;
	info.render_node[sizeof(info.render_node) - 1] = '\0';
	const struct _IOSurfacePlane* plane = &data->surface->planes[0];
	if ((renderNode == NULL || strcmp(renderNode, info.render_node) == 0)
		&& info.width == plane->width && info.height == plane->height)
	{
		_IOSurfaceGPUBoDescription description = {
			.fd = fd,
			.fourcc = info.fourcc,
			.modifier = info.modifier,
			.offset = info.offset,
			.stride = info.stride,
			.width = info.width,
			.height = info.height,
		};
		bo = _IOSurfaceGPUBoImport(info.render_node, &description);
	}
	close(fd);
	return bo ? newGPUSurface(data, bo) : NULL;
}

// Makes a new buffer for the surface (see _IOSurfaceGetGPUBuffer), and
// gives it to the service. If another process has given it one first, that
// one is used instead. Called with g_gpuSurfacesLock held.
static struct GPUSurface* createGPUSurface(ImplData* data, const char* renderNode, uint32_t fourcc,
	const uint64_t* modifiers, size_t modifierCount, bool* published)
{
	*published = false;
	const struct _IOSurfacePlane* plane = &data->surface->planes[0];
	// The buffer starts with the contents of the memory.
	uint64_t version = atomic_load(&data->shared->cpuVersion);
	_IOSurfaceGPUBo* bo = _IOSurfaceGPUBoCreate(renderNode, plane->width, plane->height, fourcc,
		modifiers, modifierCount);
	if (bo == NULL)
		return NULL;
	void* memory = (void*)(uintptr_t)(data->surface->address + plane->memoryOffset);
	if (!_IOSurfaceGPUBoUpload(bo, memory, plane->bytesPerRow))
	{
		_IOSurfaceGPUBoDestroy(bo);
		return NULL;
	}

	if (data->serviceID != 0)
	{
		_IOSurfaceGPUBoDescription description;
		_IOSurfaceGPUBoDescribe(bo, &description);
		dserver_iosurface_gpu_buffer_t info = {
			.modifier = description.modifier,
			.fourcc = description.fourcc,
			.width = description.width,
			.height = description.height,
			.offset = description.offset,
			.stride = description.stride,
		};
		strlcpy(info.render_node, renderNode, sizeof(info.render_node));
		int kept = -1;
		if (__darling_iosurface_publish_gpu_buffer(data->serviceID, description.fd, &info, &kept) == 0)
		{
			*published = true;
			atomic_store(&data->shared->hasGPUBuffer, 1);
			struct stat ours, theirs;
			bool same = fstat(description.fd, &ours) == 0 && fstat(kept, &theirs) == 0
				&& ours.st_dev == theirs.st_dev && ours.st_ino == theirs.st_ino;
			close(kept);
			if (!same)
			{
				// Another process's buffer won; it holds that process's
				// contents version.
				_IOSurfaceGPUBoDestroy(bo);
				return importGPUSurface(data, renderNode);
			}
		}
	}

	raiseVersion(&data->shared->bufferVersion, version);
	struct GPUSurface* gpu = newGPUSurface(data, bo);
	if (gpu)
		gpu->uploads = 1;
	return gpu;
}

// Brings the memory up to date from the buffer (toMemory), importing the
// buffer from the service if this process has none yet, or the buffer from
// the memory, in a process that has it.
static void syncGPUSurface(ImplData* data, bool toMemory)
{
	if (!mayHaveGPUSurface(data))
		return;
	if (toMemory && !bufferNewer(data->shared))
		return;
	pthread_mutex_lock(&g_gpuSurfacesLock);
	struct GPUSurface* gpu = findGPUSurface(data);
	if (gpu == NULL && toMemory)
		gpu = importGPUSurface(data, NULL);
	if (gpu != NULL)
	{
		if (toMemory)
			downloadGPUSurface(gpu);
		else
			uploadGPUSurface(gpu);
	}
	pthread_mutex_unlock(&g_gpuSurfacesLock);
}

// fork(): the child gets the memory, with the contents that only GPU
// buffers held, but does not use the buffers it inherited (gpubuffer.h).
// It holds the surfaces its parent held, which the service records, so
// that processes rendering into them keep its memory up to date.
static void beforeFork(void)
{
	pthread_mutex_lock(&g_serviceLock);
	pthread_mutex_lock(&g_gpuSurfacesLock);
	for (struct GPUSurface* gpu = g_gpuSurfaces; gpu != NULL; gpu = gpu->next)
		downloadGPUSurface(gpu);
}

static void afterForkInParent(void)
{
	pthread_mutex_unlock(&g_gpuSurfacesLock);
	pthread_mutex_unlock(&g_serviceLock);
}

static void afterForkInChild(void)
{
	for (struct GPUSurface* gpu = g_gpuSurfaces; gpu != NULL; gpu = gpu->next)
		gpu->bo = NULL;
	_IOSurfaceGPUBoForked();
	pthread_mutex_unlock(&g_gpuSurfacesLock);

	for (struct ServiceSurface* surface = g_serviceSurfaces; surface != NULL; surface = surface->next)
	{
		uint32_t serviceID = 0;
		if (__darling_iosurface_retain(surface->fd, surface->serviceID, surface->global ? DSERVER_IOSURFACE_GLOBAL : 0,
				offsetof(struct SharedHeader, memoryProcesses), &serviceID) != 0)
			serviceID = 0;
		// The service knows the surface (the parent holds it) and keeps its
		// ID; the child has not imported the buffer.
		surface->serviceID = serviceID;
		surface->usage = 0;
	}
	pthread_mutex_unlock(&g_serviceLock);
}

static void registerForkHandlers(void)
{
	pthread_atfork(beforeFork, afterForkInParent, afterForkInChild);
}

void _IOSurfaceSetGPUCallbacks(const _IOSurfaceGPUCallbacks* callbacks)
{
	if (callbacks == NULL || (callbacks->version != _IOSURFACE_GPU_CALLBACKS_VERSION
		&& callbacks->version != _IOSURFACE_GPU_CALLBACKS_VERSION_REPORTS_WRITES))
	{
		os_log_error(OS_LOG_DEFAULT, "IOSurface: ignoring GPU callbacks with an unknown version");
		return;
	}

	// Registrations are rare; the mutex only keeps two of them from taking
	// the same callbacks twice. Readers do not take it.
	pthread_mutex_lock(&g_gpuCallbacksMutex);
	int count = atomic_load_explicit(&g_gpuCallbacksCount, memory_order_relaxed);
	for (int i = 0; i < count; i++)
	{
		if (g_gpuCallbacks[i].writeback == callbacks->writeback && g_gpuCallbacks[i].forget == callbacks->forget)
		{
			pthread_mutex_unlock(&g_gpuCallbacksMutex);
			return;
		}
	}
	if (count == MAX_GPU_CALLBACKS)
	{
		pthread_mutex_unlock(&g_gpuCallbacksMutex);
		os_log_error(OS_LOG_DEFAULT, "IOSurface: too many GPU callback registrations, ignoring another one");
		return;
	}
	// A registration with the original version has no finish member.
	g_gpuCallbacks[count] = (_IOSurfaceGPUCallbacks) {
		.version = callbacks->version,
		.writeback = callbacks->writeback,
		.forget = callbacks->forget,
		.finish = callbacks->version == _IOSURFACE_GPU_CALLBACKS_VERSION_REPORTS_WRITES ? callbacks->finish : NULL,
	};
	atomic_store_explicit(&g_gpuCallbacksCount, count + 1, memory_order_release);
	pthread_mutex_unlock(&g_gpuCallbacksMutex);
}

// The registered callbacks at index, or NULL past the last one.
static const _IOSurfaceGPUCallbacks* gpuCallbacks(int index)
{
	if (index >= atomic_load_explicit(&g_gpuCallbacksCount, memory_order_acquire))
		return NULL;
	return &g_gpuCallbacks[index];
}

// Brings GPU work on the surface into its memory before the CPU or another
// process sees it, asking every registered GPU API in registration order.
// With avoidSync, APIs that report their writes (see
// _IOSURFACE_GPU_CALLBACKS_VERSION_REPORTS_WRITES) have nothing to copy, as
// the caller has checked gpuContentsPending, and are only asked to finish
// GPU work submitted into the memory. APIs that do not report their writes
// are asked to write back, as nothing tells whether they have anything to
// copy.
static void writeBackGPUContents(IOSurface* surface, struct SharedHeader* shared, bool avoidSync)
{
	if (t_inGPUWriteback)
		return;

	uint64_t pending = atomic_load(&shared->gpuVersion);
	t_inGPUWriteback = true;
	const _IOSurfaceGPUCallbacks* callbacks;
	for (int i = 0; (callbacks = gpuCallbacks(i)) != NULL; i++)
	{
		bool reportsWrites = callbacks->version == _IOSURFACE_GPU_CALLBACKS_VERSION_REPORTS_WRITES;
		void (*function)(IOSurfaceRef) = avoidSync && reportsWrites ? callbacks->finish : callbacks->writeback;
		if (function != NULL)
			function((IOSurfaceRef) surface);
	}
	t_inGPUWriteback = false;
	// The surface's GPU buffer, if rendering in it has not reached the
	// memory yet. While it is still newer (this process cannot copy it),
	// the memory is not up to date.
	syncGPUSurface((ImplData*) surface->_impl, true);
	if (!bufferNewer(shared))
		raiseVersion(&shared->cpuVersion, pending);
}

@implementation IOSurface

+ (void)initialize
{
	if (self == [IOSurface self])
	{
		CFMutableDictionaryRef matching = IOServiceNameMatching("IOSurfaceRoot");

		// IOServiceGetMatchingService() consumes the matching dictionary, so it must not be released here.
		g_surfaceService = IOServiceGetMatchingService(kIOMasterPortDefault, matching);

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

	id global = properties[(NSString*) kIOSurfaceIsGlobal];
	bool isGlobal = [global respondsToSelector: @selector(boolValue)] && [global boolValue];
	initState(idata);
	if (fd >= 0)
	{
		idata->fd = fd;
		idata->mapping = shared;
		idata->mappingSize = mappingSize;
		idata->dataOffset = shared->dataOffset;
		recordIdentity(idata);
		// Unknown until the service has counted the processes.
		atomic_store(&shared->memoryProcesses, UINT32_MAX);
		idata->serviceID = serviceRetain(fd, idata->device, idata->inode, 0, isGlobal);
	}

	surface->surfaceID = idata->serviceID != 0 ? idata->serviceID : nextUserSpaceID();
	surface->pixelFormat = (uint32_t) pixelFormat;
	surface->address = (uint64_t)(uintptr_t) memory;
	surface->planeCount = planeCount;
	memcpy(surface->planes, planes, planeCount * sizeof(struct _IOSurfacePlane));

	idata->surface = surface;
	idata->allocSize = allocSize;
	idata->userSpace = true;
	idata->planar = planeInfo.count != 0;
	id casting = properties[(NSString*) kIOSurfacePixelSizeCastingAllowed];
	if ([casting respondsToSelector: @selector(boolValue)])
		idata->allowsPixelSizeCasting = [casting boolValue];
	idata->shared = shared;
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
	idata->serviceID = serviceRetain(fd, st.st_dev, st.st_ino, header.surfaceID, (header.flags & kSharedGlobal) != 0);
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
		writeBackGPUContents(self, data->shared, false);
	if (data->fd < 0 || fileport_makeport(data->fd, &port) != 0)
		return MACH_PORT_NULL;
	return port;
}

- (nullable instancetype)initWithSurfaceID:(IOSurfaceID)surfaceID
{
	uint8_t responseBuffer[3500];
	size_t responseLength = sizeof(responseBuffer);

	// A global surface of this process, or one the IOSurface service finds:
	// a global surface of any process, or one this process holds.
	int fd = copyGlobalSurface(surfaceID);
	bool local = fd >= 0;
	if (!local && __darling_iosurface_lookup(surfaceID, &fd) != 0)
		fd = -1;
	if (fd >= 0)
	{
		self = [self _initWithSharedObject: fd];
		if (self)
		{
			ImplData* data = (ImplData*) _impl;
			if (local || ((data->shared->flags & kSharedGlobal) && data->surface->surfaceID == surfaceID))
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
		const _IOSurfaceGPUCallbacks* callbacks;
		for (int i = 0; (callbacks = gpuCallbacks(i)) != NULL; i++)
		{
			if (callbacks->forget != NULL)
				callbacks->forget((IOSurfaceRef) self);
		}

		releaseGPUSurface(data);
		serviceRelease(data);

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
	bool avoidSync = (options & kIOSurfaceLockAvoidSync) != 0;
	// The GPU buffer holds rendering that has been reported but not copied
	// into the memory yet, because no process was reading the memory. The
	// copy is made here, also for kIOSurfaceLockAvoidSync: on macOS such a
	// lock after a flush succeeds and sees the rendering.
	if (!t_inGPUWriteback)
		syncGPUSurface(data, true);
	// A GPU API holds newer contents, which a lock would copy into the
	// memory, or this process cannot copy the GPU buffer.
	if (avoidSync && !t_inGPUWriteback && gpuContentsPending(data->shared))
		return kIOReturnCannotLock;
	// CPU access sees the GPU work submitted before the lock.
	writeBackGPUContents(self, data->shared, avoidSync);

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
	// QUIRK: a lock for writing changes the seed when it is taken, and the
	// lock reports the new value; the unlock leaves it alone. The header
	// comment in IOSurfaceRef.h says the unlock increments it.
	// Evidence: black-box observation on macOS 10.14.4 (x86_64, i386) and
	// 26.7.3 (x86_64) (tier 2): each write lock, nested ones included,
	// returns the previous seed plus one, and the unlock returns that value.
	// Test: iosurface_seed_versions ("seed changes at the write lock").
	uint32_t current = (options & kIOSurfaceLockReadOnly)
		? data->shared->seed
		: atomic_fetch_add(&data->shared->seed, 1) + 1;
	if (seed)
		*seed = current;
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
	// The CPU's writes are the newest contents; copies that GPU APIs hold
	// are older, and the GPU buffer gets them. The seed changed when the
	// lock was taken.
	if (!(options & kIOSurfaceLockReadOnly))
	{
		bumpVersion(data->shared, &data->shared->cpuVersion);
		syncGPUSurface(data, false);
	}
	if (seed)
		*seed = data->shared->seed;
	return ret;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
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
	// The caller's GPU uses the memory itself.
	serviceSetUsage(data, DSERVER_IOSURFACE_USAGE_USES_MEMORY, false);

	const struct _IOSurfacePlane* plane = &data->surface->planes[planeIndex];
	out->fd = dmabuf;
	out->size = data->allocSize;
	out->offset = plane->memoryOffset;
	out->bytesPerRow = plane->bytesPerRow;
	return true;
}

void _IOSurfaceGPUDidWrite(IOSurfaceRef buffer)
{
	if (!buffer)
		return;
	ImplData* data = (ImplData*) ((IOSurface*) buffer)->_impl;
	bumpVersion(data->shared, &data->shared->gpuVersion);
}

bool _IOSurfaceIsShared(IOSurfaceRef buffer)
{
	if (!buffer)
		return false;
	ImplData* data = (ImplData*) ((IOSurface*) buffer)->_impl;
	if (data->fd < 0)
		return false;
	// The service counts this process too once it uses the memory, so more
	// than one means another process may read it (UINT32_MAX: not known).
	return atomic_load(&data->shared->memoryProcesses) > 1;
}

void _IOSurfaceGPUDidWriteMemory(IOSurfaceRef buffer)
{
	if (!buffer)
		return;
	ImplData* data = (ImplData*) ((IOSurface*) buffer)->_impl;
	bumpVersion(data->shared, &data->shared->cpuVersion);
	atomic_fetch_add(&data->shared->seed, 1);
}

static bool listedModifier(uint64_t modifier, const uint64_t* modifiers, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		if (modifiers[i] == modifier)
			return true;
	}
	return false;
}

bool _IOSurfaceGetGPUBuffer(IOSurfaceRef buffer, size_t planeIndex, const char* renderNode, uint32_t fourcc,
	const uint64_t* modifiers, size_t modifierCount, uint32_t flags, _IOSurfaceGPUBuffer* out)
{
	if (!buffer || !out || out->version != _IOSURFACE_GPU_BUFFER_VERSION || !renderNode)
		return false;
	if (fourcc != IOSURFACE_DRM_FORMAT_ARGB8888 && fourcc != IOSURFACE_DRM_FORMAT_ABGR8888)
		return false;
	ImplData* data = (ImplData*) ((IOSurface*) buffer)->_impl;
	// The buffer holds one plane at its full size; a surface in memory that
	// another process cannot map is left as it is.
	if (data->fd < 0 || data->surface->planeCount != 1 || planeIndex != 0)
		return false;
	const struct _IOSurfacePlane* plane = &data->surface->planes[0];
	if (plane->bytesPerElement != 4 || plane->bytesPerRow % 4 != 0)
		return false;

	pthread_mutex_lock(&g_gpuSurfacesLock);
	bool published = false;
	struct GPUSurface* gpu = findGPUSurface(data);
	// Another process's buffer.
	if (gpu == NULL)
		gpu = importGPUSurface(data, renderNode);
	if (gpu == NULL && (flags & _IOSURFACE_GPU_BUFFER_CREATE))
		gpu = createGPUSurface(data, renderNode, fourcc, modifiers, modifierCount, &published);

	bool found = false;
	if (gpu != NULL && gpu->bo != NULL && _IOSurfaceGPUBoOnRenderNode(gpu->bo, renderNode))
	{
		_IOSurfaceGPUBoDescription description;
		_IOSurfaceGPUBoDescribe(gpu->bo, &description);
		if (description.fourcc == fourcc && listedModifier(description.modifier, modifiers, modifierCount))
		{
			out->fd = description.fd;
			out->fourcc = description.fourcc;
			out->modifier = description.modifier;
			out->offset = description.offset;
			out->stride = description.stride;
			out->width = description.width;
			out->height = description.height;
			found = true;
		}
	}
	pthread_mutex_unlock(&g_gpuSurfacesLock);

	// The caller uses the buffer, not the memory; publishing the buffer has
	// recorded that already.
	if (found)
		serviceSetUsage(data, DSERVER_IOSURFACE_USAGE_IMPORTED_BUFFER, published);
	return found;
}

void _IOSurfaceGPUWillUse(IOSurfaceRef buffer)
{
	if (buffer)
		syncGPUSurface((ImplData*) ((IOSurface*) buffer)->_impl, false);
}

void _IOSurfaceGPUDidWriteBuffer(IOSurfaceRef buffer)
{
	if (!buffer)
		return;
	ImplData* data = (ImplData*) ((IOSurface*) buffer)->_impl;
	if (!mayHaveGPUSurface(data))
		return;

	pthread_mutex_lock(&g_gpuSurfacesLock);
	struct GPUSurface* gpu = findGPUSurface(data);
	bool wrote = gpu != NULL && gpu->bo != NULL;
	if (wrote)
	{
		raiseVersion(&gpu->shared->bufferVersion, bumpVersion(gpu->shared, &gpu->shared->gpuVersion));
		// Only for processes that read the memory; the others use the
		// buffer, and a lock copies it.
		if (memoryRead(gpu->shared))
			downloadGPUSurface(gpu);
	}
	pthread_mutex_unlock(&g_gpuSurfacesLock);
	// The contents changed; where the memory is read, they are in it
	// before the seed says so.
	if (wrote)
		atomic_fetch_add(&data->shared->seed, 1);
}

bool _IOSurfaceGetGPUBufferStatistics(IOSurfaceRef buffer, _IOSurfaceGPUBufferStatistics* statistics)
{
	if (!buffer || !statistics || statistics->version != _IOSURFACE_GPU_BUFFER_VERSION)
		return false;
	ImplData* data = (ImplData*) ((IOSurface*) buffer)->_impl;
	statistics->hasBuffer = false;
	statistics->downloads = statistics->uploads = 0;
	if (!mayHaveGPUSurface(data))
		return true;
	pthread_mutex_lock(&g_gpuSurfacesLock);
	struct GPUSurface* gpu = findGPUSurface(data);
	if (gpu != NULL)
	{
		statistics->hasBuffer = gpu->bo != NULL;
		statistics->downloads = gpu->downloads;
		statistics->uploads = gpu->uploads;
	}
	pthread_mutex_unlock(&g_gpuSurfacesLock);
	return true;
}

IOSurfaceID IOSurfaceGetID(IOSurfaceRef buffer)
{
	if (!buffer)
		return 0;
	return ((ImplData*) ((IOSurface*) buffer)->_impl)->surface->surfaceID;
}
