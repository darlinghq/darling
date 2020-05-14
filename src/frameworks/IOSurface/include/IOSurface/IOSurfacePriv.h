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

#endif
