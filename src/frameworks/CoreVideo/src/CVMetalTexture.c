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

// CVMetalTexture and CVMetalTextureCache. Texture sharing with Metal is not
// implemented: creating a cache or a texture fails with kCVReturnUnsupported
// (invalid arguments are reported as kCVReturnInvalidArgument first), so no
// object of either type ever exists and the other entry points only have to
// be callable.

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <dispatch/dispatch.h>
#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVBuffer.h>

const CFStringRef kCVMetalTextureUsage = CFSTR("MetalTextureUsage");
const CFStringRef kCVMetalTextureStorageMode = CFSTR("MetalTextureStorageMode");
const CFStringRef kCVMetalTextureCacheMaximumTextureAgeKey = CFSTR("MaximumMetalTextureAge");

// The class carries no state; it exists so the type IDs are distinct and
// valid for CFGetTypeID comparisons.
static const CFRuntimeClass metalTextureClass = {
	.version = 0,
	.className = "CVMetalTexture",
};

static const CFRuntimeClass metalTextureCacheClass = {
	.version = 0,
	.className = "CVMetalTextureCache",
};

CFTypeID CVMetalTextureGetTypeID(void)
{
	static CFTypeID typeID;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		typeID = _CFRuntimeRegisterClass(&metalTextureClass);
	});
	return typeID;
}

CFTypeID CVMetalTextureCacheGetTypeID(void)
{
	static CFTypeID typeID;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		typeID = _CFRuntimeRegisterClass(&metalTextureCacheClass);
	});
	return typeID;
}

void* CVMetalTextureGetTexture(CFTypeRef image)
{
	return NULL;
}

CVReturn CVMetalTextureCacheCreate(CFAllocatorRef allocator, CFDictionaryRef cacheAttributes,
	void* metalDevice, CFDictionaryRef textureAttributes, void** cacheOut)
{
	if (cacheOut == NULL || metalDevice == NULL)
		return kCVReturnInvalidArgument;
	*cacheOut = NULL;
	return kCVReturnUnsupported;
}

CVReturn CVMetalTextureCacheCreateTextureFromImage(CFAllocatorRef allocator, void* textureCache,
	CFTypeRef sourceImage, CFDictionaryRef textureAttributes, unsigned long pixelFormat,
	size_t width, size_t height, size_t planeIndex, void** textureOut)
{
	if (textureOut == NULL || textureCache == NULL || sourceImage == NULL)
		return kCVReturnInvalidArgument;
	*textureOut = NULL;
	return kCVReturnUnsupported;
}

void CVMetalTextureCacheFlush(void* textureCache, uint64_t options)
{
}
