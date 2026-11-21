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

#ifndef _VTPIXELTRANSFERSESSION_H_
#define _VTPIXELTRANSFERSESSION_H_

#include <VideoToolbox/VTBase.h>
#include <VideoToolbox/VTErrors.h>
#include <VideoToolbox/VTSession.h>
#include <CoreVideo/CoreVideo.h>

CF_IMPLICIT_BRIDGING_ENABLED

typedef struct CF_BRIDGED_TYPE(id) OpaqueVTPixelTransferSession *VTPixelTransferSessionRef;

VT_EXPORT OSStatus VTPixelTransferSessionCreate(CFAllocatorRef allocator,
	CF_RETURNS_RETAINED VTPixelTransferSessionRef *pixelTransferSessionOut);
VT_EXPORT void VTPixelTransferSessionInvalidate(VTPixelTransferSessionRef session);
VT_EXPORT CFTypeID VTPixelTransferSessionGetTypeID(void);
VT_EXPORT OSStatus VTPixelTransferSessionTransferImage(VTPixelTransferSessionRef session,
	CVPixelBufferRef sourceBuffer, CVPixelBufferRef destinationBuffer);

CF_IMPLICIT_BRIDGING_DISABLED

#endif
