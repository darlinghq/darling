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

#ifndef CVMETALTEXTURECACHE_H
#define CVMETALTEXTURECACHE_H

#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVBuffer.h>
#include <CoreVideo/CVMetalTexture.h>

__BEGIN_DECLS

CV_EXPORT const CFStringRef CV_NONNULL kCVMetalTextureCacheMaximumTextureAgeKey;

CV_EXPORT CFTypeID CVMetalTextureCacheGetTypeID(void);

#if defined(__OBJC__)
@protocol MTLDevice;

typedef struct __CVMetalTextureCache *CVMetalTextureCacheRef;

// Metal texture interop is not available: the create calls validate their
// arguments and then fail with kCVReturnUnsupported, leaving no object behind.
CV_EXPORT CVReturn CVMetalTextureCacheCreate(CFAllocatorRef CV_NULLABLE allocator,
	CFDictionaryRef CV_NULLABLE cacheAttributes,
	id <MTLDevice> CV_NONNULL metalDevice,
	CFDictionaryRef CV_NULLABLE textureAttributes,
	CVMetalTextureCacheRef CV_NULLABLE * CV_NONNULL cacheOut);

// pixelFormat is an MTLPixelFormat.
CV_EXPORT CVReturn CVMetalTextureCacheCreateTextureFromImage(CFAllocatorRef CV_NULLABLE allocator,
	CVMetalTextureCacheRef CV_NONNULL textureCache,
	CVImageBufferRef CV_NONNULL sourceImage,
	CFDictionaryRef CV_NULLABLE textureAttributes,
	unsigned long pixelFormat,
	size_t width,
	size_t height,
	size_t planeIndex,
	CVMetalTextureRef CV_NULLABLE * CV_NONNULL textureOut);

CV_EXPORT void CVMetalTextureCacheFlush(CVMetalTextureCacheRef CV_NONNULL textureCache, CVOptionFlags options);
#endif

__END_DECLS

#endif
