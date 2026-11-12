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

#ifndef CVOPENGLTEXTURECACHE_H
#define CVOPENGLTEXTURECACHE_H

#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVImageBuffer.h>
#include <CoreVideo/CVOpenGLTexture.h>
#include <OpenGL/OpenGL.h>

__BEGIN_DECLS

typedef struct CV_BRIDGED_TYPE(id) __CVOpenGLTextureCache* CVOpenGLTextureCacheRef;

CV_EXPORT CFTypeID CVOpenGLTextureCacheGetTypeID(void);

// A cache of the textures made for one OpenGL context. The cache retains the context and the
// pixel format.
CV_EXPORT CVReturn CVOpenGLTextureCacheCreate(CFAllocatorRef CV_NULLABLE allocator,
	CFDictionaryRef CV_NULLABLE cacheAttributes, CGLContextObj CV_NONNULL cglContext,
	CGLPixelFormatObj CV_NULLABLE cglPixelFormat, CFDictionaryRef CV_NULLABLE textureAttributes,
	CVOpenGLTextureCacheRef CV_NULLABLE * CV_NONNULL cacheOut);

// Makes a texture of a pixel buffer: a pixel buffer backed by an IOSurface is bound to the
// texture with CGLTexImageIOSurface2D, the pixels of any other packed buffer are copied. The
// texture is a GL_TEXTURE_RECTANGLE texture. A planar buffer is kCVReturnPixelBufferNotOpenGLCompatible.
// The same buffer gets the same texture name as long as the cache keeps it.
CV_EXPORT CVReturn CVOpenGLTextureCacheCreateTextureFromImage(CFAllocatorRef CV_NULLABLE allocator,
	CVOpenGLTextureCacheRef CV_NONNULL textureCache, CVImageBufferRef CV_NONNULL sourceImage,
	CFDictionaryRef CV_NULLABLE attributes, CVOpenGLTextureRef CV_NULLABLE * CV_NONNULL textureOut);

CV_EXPORT CVOpenGLTextureCacheRef CV_NULLABLE CVOpenGLTextureCacheRetain(CVOpenGLTextureCacheRef CV_NULLABLE textureCache);
CV_EXPORT void CVOpenGLTextureCacheRelease(CVOpenGLTextureCacheRef CV_NULLABLE textureCache);

// Deletes the texture names no texture object uses any more.
CV_EXPORT void CVOpenGLTextureCacheFlush(CVOpenGLTextureCacheRef CV_NONNULL textureCache, CVOptionFlags options);

__END_DECLS

#endif
