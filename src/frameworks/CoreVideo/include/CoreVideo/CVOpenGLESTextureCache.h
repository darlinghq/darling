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

#ifndef COREVIDEO_CVOPENGLESTEXTURECACHE_H
#define COREVIDEO_CVOPENGLESTEXTURECACHE_H

#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVBuffer.h>
#include <CoreVideo/CVOpenGLESTexture.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct CV_BRIDGED_TYPE(id) __CVOpenGLESTextureCache *CVOpenGLESTextureCacheRef;

#ifndef COREVIDEO_USE_EAGLCONTEXT_CLASS_IN_API
#define COREVIDEO_USE_EAGLCONTEXT_CLASS_IN_API 1
#endif

#if defined(__OBJC__) && COREVIDEO_USE_EAGLCONTEXT_CLASS_IN_API
@class EAGLContext;
typedef EAGLContext *CVEAGLContext;
#else
typedef void *CVEAGLContext;
#endif

// Cache attribute: how many seconds an unused texture stays in the cache (a CFNumber).
// The default is one second; zero turns the aging off.
CV_EXPORT const CFStringRef CV_NONNULL kCVOpenGLESTextureCacheMaximumTextureAgeKey;

CV_EXPORT CFTypeID CVOpenGLESTextureCacheGetTypeID(void);

// Makes a cache that creates its textures in an OpenGL ES 2 or 3 context.
CV_EXPORT CVReturn CVOpenGLESTextureCacheCreate(
	CFAllocatorRef CV_NULLABLE allocator,
	CFDictionaryRef CV_NULLABLE cacheAttributes,
	CVEAGLContext CV_NONNULL eaglContext,
	CFDictionaryRef CV_NULLABLE textureAttributes,
	CV_RETURNS_RETAINED_PARAMETER CVOpenGLESTextureCacheRef CV_NULLABLE * CV_NONNULL cacheOut);

// Makes (or finds in the cache) a texture holding the pixels of one plane of an image buffer.
CV_EXPORT CVReturn CVOpenGLESTextureCacheCreateTextureFromImage(
	CFAllocatorRef CV_NULLABLE allocator,
	CVOpenGLESTextureCacheRef CV_NONNULL textureCache,
	CVImageBufferRef CV_NONNULL sourceImage,
	CFDictionaryRef CV_NULLABLE textureAttributes,
	GLenum target,
	GLint internalFormat,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLenum type,
	size_t planeIndex,
	CV_RETURNS_RETAINED_PARAMETER CVOpenGLESTextureRef CV_NULLABLE * CV_NONNULL textureOut);

// Deletes the textures nobody uses any more.
CV_EXPORT void CVOpenGLESTextureCacheFlush(CVOpenGLESTextureCacheRef CV_NONNULL textureCache, CVOptionFlags options);

#if defined(__cplusplus)
}
#endif

#endif
