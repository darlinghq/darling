/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef CVPIXELBUFFERIOSURFACE_H
#define CVPIXELBUFFERIOSURFACE_H

#include <sys/cdefs.h>
#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <IOSurface/IOSurfaceRef.h>

__BEGIN_DECLS

extern const CFStringRef kCVPixelBufferIOSurfaceOpenGLTextureCompatibilityKey;

// The IOSurface that holds the pixels of the buffer, or NULL for a buffer that is not
// IOSurface-backed. The surface is not retained for the caller.
CV_EXPORT IOSurfaceRef CV_NULLABLE CVPixelBufferGetIOSurface(CVPixelBufferRef pixelBuffer);

// A pixel buffer around an existing IOSurface, which it retains. The size and the pixel format
// are the surface's, and the attributes do not change them; a surface without a pixel format
// gives kCVReturnInvalidPixelFormat.
CV_EXPORT CVReturn CVPixelBufferCreateWithIOSurface(CFAllocatorRef CV_NULLABLE allocator, IOSurfaceRef surface,
	CFDictionaryRef CV_NULLABLE pixelBufferAttributes, CVPixelBufferRef CV_NULLABLE * CV_NONNULL pixelBufferOut);

__END_DECLS

#endif
