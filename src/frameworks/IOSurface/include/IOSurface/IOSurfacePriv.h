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

#ifndef IOSURFACE_PRIV_H
#define IOSURFACE_PRIV_H
#include <IOSurface/IOSurfaceTypes.h>
#include <IOSurface/IOSurfaceRef.h>
#include <stdbool.h>
#include <stdint.h>

// Private API definition between IOSurface.framework and iokitd

// IOSurfaceID is passed in as "input" if it's the only argument
// struct contents as passed in "inputStruct".
// See IOConnectCallMethod().

enum
{
	// in: dictionary
	// out: _IOSurfaceObjectRetval
	kIOSurfaceMethodCreate = 0,
	// in: IOSurfaceID
	kIOSurfaceMethodRelease = 1,
	// in: struct _IOSurfaceLockUnlock
	kIOSurfaceMethodLock = 2,
	// in: struct _IOSurfaceLockUnlock
	kIOSurfaceMethodUnlock = 3,
	// in: IOSurfaceID
	kIOSurfaceMethodLookupByID = 4,
	kIOSurfaceMethodSetValue = 9,
	// out: data for IOCFUnserializeWithSize()
	kIOSurfaceMethodGetValues = 10,
	kIOSurfaceMethodRemoveValue = 11,
	// in: IOSurfaceID
	kIOSurfaceMethodIncrementUseCount = 14,
	// in: IOSurfaceID
	kIOSurfaceMethodDecrementUseCount = 15,
	// in: mach_port_t as a plain value
	// out: _IOSurfaceObjectRetval
	kIOSurfaceMethodCreateByMachPort = 34,
	// in: IOSurfaceID
	// out: mach_port_t as a plain value
	kIOSurfaceMethodCreateMachPort = 35,
};

struct _IOSurfacePlane
{
	uint64_t memoryOffset;

	uint32_t bytesPerElement;
	uint32_t width;
	uint32_t height;
	uint32_t bytesPerRow;
};

struct _IOSurfaceObjectRetval
{
	IOSurfaceID surfaceID;
	uint32_t pixelFormat;
	
	// Where mapped into memory
	uint64_t address;

	uint32_t planeCount;
	struct _IOSurfacePlane planes[1];
};

struct _IOSurfaceLockUnlock
{
	IOSurfaceID surfaceID;
	uint32_t options;
};

// Private interface between IOSurface.framework and the GPU APIs that keep
// their own copies of surface contents (OpenGL.framework and Metal.framework,
// whose textures can be copies of the surface rather than views of its
// memory).
//
// writeback copies GPU contents that the CPU has not seen yet into the
// surface. IOSurface calls it before a lock is granted and before a Mach
// port for the surface is created, so that CPU access and other processes
// see completed GPU work. It must return without doing anything for a
// surface it has no pending contents for. Locks of the same surface made
// on the calling thread from inside writeback do not call it again.
//
// forget is called while a surface is being freed. The surface must not be
// retained or locked from it.
//
// Either function may be NULL. Each GPU API registers once; IOSurface calls
// the registered functions in registration order. Registering the same
// functions again, more than a few registrations, and registrations with
// an unknown version are ignored.
//
// A GPU API that registers with _IOSURFACE_GPU_CALLBACKS_VERSION_REPORTS_WRITES
// reports rendering into its copies with _IOSurfaceGPUDidWrite. A lock with
// kIOSurfaceLockAvoidSync then fails with kIOReturnCannotLock while such
// rendering has not been written back, and otherwise calls that API's
// finish instead of its writeback: finish waits for GPU work already
// submitted into the surface's memory (an imported DMA-BUF) and copies
// nothing. finish may be NULL. Callbacks registered with
// _IOSURFACE_GPU_CALLBACKS_VERSION have no finish; their writeback is
// called before every lock, kIOSurfaceLockAvoidSync or not.
#define _IOSURFACE_GPU_CALLBACKS_VERSION 1
#define _IOSURFACE_GPU_CALLBACKS_VERSION_REPORTS_WRITES 2

typedef struct
{
	uint32_t version;
	void (*writeback)(IOSurfaceRef surface);
	void (*forget)(IOSurfaceRef surface);
	// _IOSURFACE_GPU_CALLBACKS_VERSION_REPORTS_WRITES only.
	void (*finish)(IOSurfaceRef surface);
} _IOSurfaceGPUCallbacks;

// Private interface between IOSurface.framework and a GPU API that can use
// the surface's memory directly instead of keeping a copy of it.
//
// On a Linux host the memory of a surface can be exported as a DMA-BUF,
// which host GPU drivers import (EGL_EXT_image_dma_buf_import,
// VK_EXT_external_memory_dma_buf). The buffer is the memory that
// IOSurfaceGetBaseAddressOfPlane points to, in a linear layout, so GPU
// rendering into an image imported from it is what the CPU sees under the
// next lock, without a writeback copy. The writeback callback is still
// called before a lock; a GPU API that imported the buffer waits there for
// its rendering to finish.
//
// _IOSurfaceGetDMABuf fills in *buffer for plane planeIndex and returns true,
// or returns false if the surface has no DMA-BUF: the host lacks support, it
// was turned off in the org.darlinghq.iosurface defaults, or the surface's
// memory cannot be exported. The caller sets buffer->version. The
// descriptor belongs to the surface and stays valid while the caller holds
// a reference to the surface; dup() it to keep it longer. Getting the
// DMA-BUF changes nothing the public API reports: base addresses, row
// strides and plane offsets stay the same.
#define _IOSURFACE_DMABUF_VERSION 1

typedef struct
{
	uint32_t version;
	// The DMA-BUF that holds all planes of the surface, and its size.
	int fd;
	uint64_t size;
	// Where the plane starts in the DMA-BUF, and its row stride in bytes.
	uint64_t offset;
	uint32_t bytesPerRow;
} _IOSurfaceDMABuf;

// Reporting GPU writes, which every process sharing the surface sees.
//
// _IOSurfaceGPUDidWrite: the API has rendered into a copy of the surface's
// contents that it keeps, so the copy is newer than the surface's memory
// until its writeback callback copies it in (which, done under a lock for
// writing, changes the seed). Call it once the rendering is known, e.g.
// when the framebuffer it went to is flushed or unbound, and only for a
// copy that the writeback will copy in.
//
// _IOSurfaceGPUDidWriteMemory: GPU rendering into the surface's memory
// itself (an imported DMA-BUF) has been submitted, e.g. by a flush. The
// contents have changed, so the seed changes, and copies that GPU APIs
// hold are older than the memory.
//
// Private interface between IOSurface.framework and a GPU API whose driver
// can only sample the surface's memory, not render into it: on some hosts a
// DMA-BUF in the memory's linear layout is "external only" to the driver.
// Such an API gives the surface a GPU buffer, in a layout of the driver's
// own, the first time it renders into the surface. From then on the surface
// keeps its contents in the buffer as well as in its memory, and IOSurface
// copies between the two on the GPU. Every process that shares the surface
// uses the same buffer (darlingserver keeps it):
//  - GPU rendering into the buffer is reported with
//    _IOSurfaceGPUDidWriteBuffer once it has completed, and the seed
//    changes. IOSurface copies the rendering into the memory there and then
//    if a process holding the surface reads its memory (it has not imported
//    the buffer, or uses the memory through _IOSurfaceGetDMABuf), and
//    otherwise at the next lock, in whichever process takes it.
//  - CPU writes reach the buffer when the write lock ends in a process that
//    uses the buffer. Writes made elsewhere reach it when the API calls
//    _IOSurfaceGPUWillUse before it uses the buffer.
//
// _IOSurfaceGetGPUBuffer fills in *buffer for plane planeIndex and returns
// true if the surface has a buffer on the DRM render node renderNode (a host
// path) in DRM format fourcc. With _IOSURFACE_GPU_BUFFER_CREATE, a surface
// that has none gets one in one of the given layout modifiers, which the
// caller's driver must be able to render into, holding the surface's
// current contents. It returns false if the surface cannot have a buffer
// (only single-plane surfaces with four bytes per pixel can, in the formats
// ARGB8888 and ABGR8888, at the plane's size), if it has one in another
// format or on another render node, or if the host lacks the libraries.
// A buffer another process made is returned if it is on renderNode, in
// fourcc and in one of the given modifiers. Once it returns true, the
// caller counts as using the buffer rather than the memory. The caller sets
// buffer->version. The descriptor belongs to the surface and stays valid
// while the caller holds a reference to it.
//
// _IOSurfaceGetGPUBufferStatistics reports whether the surface's GPU buffer
// is in use in this process, and the copies this process made between it
// and the memory.
#define _IOSURFACE_GPU_BUFFER_VERSION 1
#define _IOSURFACE_GPU_BUFFER_CREATE 1

typedef struct
{
	uint32_t version;
	// The buffer's DMA-BUF, its DRM format and layout modifier.
	int fd;
	uint32_t fourcc;
	uint64_t modifier;
	// Where the plane starts in the DMA-BUF (always 0), its row stride in
	// bytes, and its size in pixels.
	uint32_t offset;
	uint32_t stride;
	uint32_t width, height;
} _IOSurfaceGPUBuffer;

typedef struct
{
	uint32_t version;
	bool hasBuffer;
	// Copies from the buffer into the memory, and from the memory into the
	// buffer.
	uint64_t downloads;
	uint64_t uploads;
} _IOSurfaceGPUBufferStatistics;

__BEGIN_DECLS
void _IOSurfaceSetGPUCallbacks(const _IOSurfaceGPUCallbacks *callbacks);
bool _IOSurfaceGetDMABuf(IOSurfaceRef surface, size_t planeIndex, _IOSurfaceDMABuf *buffer);
void _IOSurfaceGPUDidWrite(IOSurfaceRef surface);
void _IOSurfaceGPUDidWriteMemory(IOSurfaceRef surface);
// Whether a process other than this one holds the surface and may read its
// memory, as the IOSurface service counts them (also true while the count is
// not known). Such a reader cannot wait for this process's GPU work, so a GPU
// API that renders into the memory finishes the work before it reports the
// write; for a surface that is not shared, a lock in this process waits.
bool _IOSurfaceIsShared(IOSurfaceRef surface);
bool _IOSurfaceGetGPUBuffer(IOSurfaceRef surface, size_t planeIndex, const char *renderNode, uint32_t fourcc,
                            const uint64_t *modifiers, size_t modifierCount, uint32_t flags,
                            _IOSurfaceGPUBuffer *buffer);
void _IOSurfaceGPUWillUse(IOSurfaceRef surface);
void _IOSurfaceGPUDidWriteBuffer(IOSurfaceRef surface);
bool _IOSurfaceGetGPUBufferStatistics(IOSurfaceRef surface, _IOSurfaceGPUBufferStatistics *statistics);
__END_DECLS

#endif
