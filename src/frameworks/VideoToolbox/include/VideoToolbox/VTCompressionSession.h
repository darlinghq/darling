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
#include <CoreMedia/CMTimeRange.h>
#include <CoreMedia/CMFormatDescription.h>
#include <CoreMedia/CMSampleBuffer.h>
#include <CoreVideo/CoreVideo.h>

CF_IMPLICIT_BRIDGING_ENABLED

typedef struct CF_BRIDGED_TYPE(id) OpaqueVTCompressionSession *VTCompressionSessionRef;

typedef void (*VTCompressionOutputCallback)(void *outputCallbackRefCon, void *sourceFrameRefCon, OSStatus status,
	VTEncodeInfoFlags infoFlags, CMSampleBufferRef sampleBuffer);

typedef CF_ENUM(uint32_t, VTCompressionSessionPassType) {
	kVTCompressionSessionBeginCompletePass = 0,
	kVTCompressionSessionBeginPass = 1,
	kVTCompressionSessionBeginFinalPass = 2,
};

typedef struct opaqueVTMultiPassStorage *VTMultiPassStorageRef;

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

// Multi-pass encoding. Darling's encoders are single-pass: starting a pass and
// ending one succeed, and no time ranges are advertised for another pass. This
// lets clients keep their normal analysis/final-pass control flow.
VT_EXPORT OSStatus VTCompressionSessionBeginPass(VTCompressionSessionRef session,
	VTCompressionSessionPassType passType, uint32_t flags);
VT_EXPORT OSStatus VTCompressionSessionEndPass(VTCompressionSessionRef session,
	Boolean *furtherPassesRequestedOut, uint32_t flags);
VT_EXPORT OSStatus VTCompressionSessionGetTimeRangesForNextPass(VTCompressionSessionRef session,
	CMItemCount *timeRangeCountOut, const CMTimeRange **timeRangeArrayOut);

// A multi-pass storage keeps the identifier and the time range a client gave
// it. Darling does not use it to drive a second pass, but its CFType identity
// and documented lifetime are kept so it can be set as a session property.
VT_EXPORT CFTypeID VTMultiPassStorageGetTypeID(void);
VT_EXPORT OSStatus VTMultiPassStorageCreate(CFAllocatorRef allocator, CFURLRef fileURL,
	CMTimeRange timeRange, CFDictionaryRef options, VTMultiPassStorageRef *multiPassStorageOut);
VT_EXPORT void VTMultiPassStorageClose(VTMultiPassStorageRef multiPassStorage);
VT_EXPORT OSStatus VTMultiPassStorageInvalidate(VTMultiPassStorageRef multiPassStorage);
VT_EXPORT OSStatus VTMultiPassStorageSetIdentifier(VTMultiPassStorageRef multiPassStorage, CFStringRef identifier);
VT_EXPORT OSStatus VTMultiPassStorageCopyIdentifier(VTMultiPassStorageRef multiPassStorage,
	CFStringRef *identifierOut);
VT_EXPORT OSStatus VTMultiPassStorageSetDataAtTimeStamp(VTMultiPassStorageRef multiPassStorage,
	CMTime timeStamp, CFDataRef data);
VT_EXPORT OSStatus VTMultiPassStorageCopyDataAtTimeStamp(VTMultiPassStorageRef multiPassStorage,
	CMTime timeStamp, CFAllocatorRef allocator, CFDataRef *dataOut);
VT_EXPORT OSStatus VTMultiPassStorageGetTimeStamp(VTMultiPassStorageRef multiPassStorage,
	CMTime *timeStampOut);
VT_EXPORT OSStatus VTMultiPassStorageGetTimeStampAndDuration(VTMultiPassStorageRef multiPassStorage,
	CMTime *timeStampOut, CMTime *durationOut);
VT_EXPORT OSStatus VTMultiPassStorageWriteMerged(VTMultiPassStorageRef multiPassStorage);
VT_EXPORT OSStatus VTMultiPassStorageWriteSegment(VTMultiPassStorageRef multiPassStorage,
	CMTimeRange timeRange);

#if __BLOCKS__
typedef void (^VTCompressionOutputHandler)(OSStatus status, VTEncodeInfoFlags infoFlags, CMSampleBufferRef sampleBuffer);
VT_EXPORT OSStatus VTCompressionSessionEncodeFrameWithOutputHandler(VTCompressionSessionRef session,
	CVImageBufferRef imageBuffer, CMTime presentationTimeStamp, CMTime duration, CFDictionaryRef frameProperties,
	VTEncodeInfoFlags *infoFlagsOut, VTCompressionOutputHandler outputHandler);
#endif

CF_IMPLICIT_BRIDGING_DISABLED

#endif
