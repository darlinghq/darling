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

#ifndef _VTCOMPRESSIONSESSION_H_
#define _VTCOMPRESSIONSESSION_H_

#include <VideoToolbox/VTBase.h>
#include <VideoToolbox/VTErrors.h>
#include <VideoToolbox/VTSession.h>
#include <CoreMedia/CMBase.h>
#include <CoreMedia/CMTime.h>
#include <CoreMedia/CMFormatDescription.h>
#include <CoreMedia/CMSampleBuffer.h>
#include <CoreVideo/CoreVideo.h>

CF_IMPLICIT_BRIDGING_ENABLED

typedef struct CF_BRIDGED_TYPE(id) OpaqueVTCompressionSession *VTCompressionSessionRef;

typedef void (*VTCompressionOutputCallback)(void *outputCallbackRefCon, void *sourceFrameRefCon, OSStatus status,
	VTEncodeInfoFlags infoFlags, CMSampleBufferRef sampleBuffer);

VT_EXPORT OSStatus VTCompressionSessionCreate(CFAllocatorRef allocator, int32_t width, int32_t height,
	CMVideoCodecType codecType, CFDictionaryRef encoderSpecification, CFDictionaryRef sourceImageBufferAttributes,
	CFAllocatorRef compressedDataAllocator, VTCompressionOutputCallback outputCallback, void *outputCallbackRefCon,
	CF_RETURNS_RETAINED VTCompressionSessionRef *compressionSessionOut);
VT_EXPORT void VTCompressionSessionInvalidate(VTCompressionSessionRef session);
VT_EXPORT CFTypeID VTCompressionSessionGetTypeID(void);
VT_EXPORT CVPixelBufferPoolRef VTCompressionSessionGetPixelBufferPool(VTCompressionSessionRef session);
VT_EXPORT OSStatus VTCompressionSessionPrepareToEncodeFrames(VTCompressionSessionRef session);
VT_EXPORT OSStatus VTCompressionSessionEncodeFrame(VTCompressionSessionRef session, CVImageBufferRef imageBuffer,
	CMTime presentationTimeStamp, CMTime duration, CFDictionaryRef frameProperties, void *sourceFrameRefcon,
	VTEncodeInfoFlags *infoFlagsOut);
VT_EXPORT OSStatus VTCompressionSessionCompleteFrames(VTCompressionSessionRef session,
	CMTime completeUntilPresentationTimeStamp);

CF_IMPLICIT_BRIDGING_DISABLED

#endif
