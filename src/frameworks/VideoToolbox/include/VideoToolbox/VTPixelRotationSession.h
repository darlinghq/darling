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

#ifndef _VTPIXELROTATIONSESSION_H_
#define _VTPIXELROTATIONSESSION_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <VideoToolbox/VTPixelRotationProperties.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpaqueVTPixelRotationSession *VTPixelRotationSessionRef;

OSStatus VTPixelRotationSessionCreate(CFAllocatorRef allocator,
	VTPixelRotationSessionRef *pixelRotationSessionOut);
void VTPixelRotationSessionInvalidate(VTPixelRotationSessionRef session);
CFTypeID VTPixelRotationSessionGetTypeID(void);
OSStatus VTPixelRotationSessionRotateImage(VTPixelRotationSessionRef session,
	CVPixelBufferRef sourceBuffer, CVPixelBufferRef destinationBuffer);

#ifdef __cplusplus
}
#endif

#endif
