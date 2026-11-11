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

#ifndef CVPIXELBUFFERPOOL_H
#define CVPIXELBUFFERPOOL_H

#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVPixelBuffer.h>

__BEGIN_DECLS

typedef struct CV_BRIDGED_TYPE(id) __CVPixelBufferPool* CVPixelBufferPoolRef;

extern const CFStringRef kCVPixelBufferPoolMaximumBufferAgeKey;
extern const CFStringRef kCVPixelBufferPoolMinimumBufferCountKey;
extern const CFStringRef kCVPixelBufferPoolAllocationThresholdKey;

CV_EXPORT CFTypeID CVPixelBufferPoolGetTypeID(void);
CV_EXPORT CVPixelBufferPoolRef CVPixelBufferPoolRetain(CVPixelBufferPoolRef pixelBufferPool);
CV_EXPORT void CVPixelBufferPoolRelease(CVPixelBufferPoolRef pixelBufferPool);

CV_EXPORT CVReturn CVPixelBufferPoolCreate(
	CFAllocatorRef allocator,
	CFDictionaryRef poolAttributes,
	CFDictionaryRef pixelBufferAttributes,
	CVPixelBufferPoolRef *poolOut
);

CV_EXPORT CVReturn CVPixelBufferPoolCreatePixelBuffer(
	CFAllocatorRef allocator,
	CVPixelBufferPoolRef pixelBufferPool,
	CVPixelBufferRef *pixelBufferOut
);

CV_EXPORT CVReturn CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(
	CFAllocatorRef allocator,
	CVPixelBufferPoolRef pixelBufferPool,
	CFDictionaryRef auxAttributes,
	CVPixelBufferRef *pixelBufferOut
);

__END_DECLS

#endif
