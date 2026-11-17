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

#ifndef _VTDECOMPRESSIONSESSION_H_
#define _VTDECOMPRESSIONSESSION_H_

#include <VideoToolbox/VTBase.h>
#include <VideoToolbox/VTErrors.h>
#include <VideoToolbox/VTSession.h>
#include <CoreMedia/CMBase.h>
#include <CoreMedia/CMTime.h>
#include <CoreMedia/CMFormatDescription.h>
#include <CoreMedia/CMSampleBuffer.h>
#include <CoreVideo/CoreVideo.h>

CF_IMPLICIT_BRIDGING_ENABLED

typedef struct CF_BRIDGED_TYPE(id) OpaqueVTDecompressionSession *VTDecompressionSessionRef;

typedef void (*VTDecompressionOutputCallback)(void *decompressionOutputRefCon, void *sourceFrameRefCon,
	OSStatus status, VTDecodeInfoFlags infoFlags, CVImageBufferRef imageBuffer, CMTime presentationTimeStamp,
	CMTime presentationDuration);

typedef struct {
	VTDecompressionOutputCallback decompressionOutputCallback;
	void *decompressionOutputRefCon;
} VTDecompressionOutputCallbackRecord;

VT_EXPORT OSStatus VTDecompressionSessionCreate(CFAllocatorRef allocator,
	CMVideoFormatDescriptionRef videoFormatDescription, CFDictionaryRef videoDecoderSpecification,
	CFDictionaryRef destinationImageBufferAttributes, const VTDecompressionOutputCallbackRecord *outputCallback,
	CF_RETURNS_RETAINED VTDecompressionSessionRef *decompressionSessionOut);
VT_EXPORT void VTDecompressionSessionInvalidate(VTDecompressionSessionRef session);
VT_EXPORT CFTypeID VTDecompressionSessionGetTypeID(void);
VT_EXPORT OSStatus VTDecompressionSessionDecodeFrame(VTDecompressionSessionRef session, CMSampleBufferRef sampleBuffer,
	VTDecodeFrameFlags decodeFlags, void *sourceFrameRefCon, VTDecodeInfoFlags *infoFlagsOut);
VT_EXPORT OSStatus VTDecompressionSessionFinishDelayedFrames(VTDecompressionSessionRef session);
VT_EXPORT Boolean VTDecompressionSessionCanAcceptFormatDescription(VTDecompressionSessionRef session,
	CMFormatDescriptionRef newFormatDesc);
VT_EXPORT OSStatus VTDecompressionSessionWaitForAsynchronousFrames(VTDecompressionSessionRef session);
VT_EXPORT OSStatus VTDecompressionSessionCopyBlackPixelBuffer(VTDecompressionSessionRef session,
	CF_RETURNS_RETAINED CVPixelBufferRef *pixelBufferOut);
VT_EXPORT Boolean VTIsHardwareDecodeSupported(CMVideoCodecType codecType);
VT_EXPORT void VTRegisterSupplementalVideoDecoderIfAvailable(CMVideoCodecType codecType);

CF_IMPLICIT_BRIDGING_DISABLED

#endif
