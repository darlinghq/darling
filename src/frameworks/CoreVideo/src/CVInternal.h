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

#ifndef COREVIDEO_INTERNAL_H
#define COREVIDEO_INTERNAL_H

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreVideo/CoreVideo.h>
#include <IOSurface/IOSurfaceRef.h>
#include <pthread.h>
#include <stdint.h>

// The object behind every CVBuffer (and so CVImageBuffer, CVPixelBuffer and CVOpenGLTexture):
// the buffer-wide state comes first so that other buffer classes can start with it and share
// the attachment functions.
#define kMaxPlanes 3

struct __CVPlane {
	size_t width;
	size_t height;
	size_t bytesPerRow;
	size_t offset; // from the start of the memory (or of the IOSurface) to the plane's first row
};

struct __CVBuffer {
	CFRuntimeBase runtimeBase;
	size_t width;
	size_t height;
	OSType pixelFormat;
	size_t planeCount; // 0 for a buffer that is not planar; planes[0] still describes its pixels
	struct __CVPlane planes[kMaxPlanes];
	size_t dataSize;
	uint8_t* memory; // the pixels of a buffer that is not backed by an IOSurface
	Boolean ownsMemory;
	CVPixelBufferReleaseBytesCallback releaseCallback;
	void* releaseRefCon;
	IOSurfaceRef surface; // the buffer's pixels live in this surface when there is one; retained
	pthread_mutex_t lockMutex;
	size_t lockCount;
	CVPixelBufferLockFlags lockFlags; // the flags of the lock that took the first reference
	CVPixelBufferPoolRef pool; // retained while the buffer is outstanding
	CFMutableDictionaryRef propagatingAttachments;
	CFMutableDictionaryRef localAttachments;
};


#endif
