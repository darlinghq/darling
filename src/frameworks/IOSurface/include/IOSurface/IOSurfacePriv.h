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

__BEGIN_DECLS
void _IOSurfaceSetGPUCallbacks(const _IOSurfaceGPUCallbacks *callbacks);
__END_DECLS

#endif
