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

// Private interface between IOSurface.framework and a GPU API that keeps
// its own copy of surface contents (OpenGL.framework, whose textures are
// copies of the surface rather than views of its memory).
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
// Either function may be NULL. Only the first registration in a process
// takes effect; later ones, and ones with an unknown version, are ignored.
#define _IOSURFACE_GPU_CALLBACKS_VERSION 1

typedef struct
{
	uint32_t version;
	void (*writeback)(IOSurfaceRef surface);
	void (*forget)(IOSurfaceRef surface);
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

__BEGIN_DECLS
void _IOSurfaceSetGPUCallbacks(const _IOSurfaceGPUCallbacks *callbacks);
bool _IOSurfaceGetDMABuf(IOSurfaceRef surface, size_t planeIndex, _IOSurfaceDMABuf *buffer);
__END_DECLS

#endif
