/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

#ifndef _CMSAMPLEBUFFER_H_
#define _CMSAMPLEBUFFER_H_

#include <CoreMedia/CMBase.h>
#include <CoreMedia/CMTime.h>
#include <CoreMedia/CMFormatDescription.h>
#include <CoreMedia/CMBlockBuffer.h>

CF_IMPLICIT_BRIDGING_ENABLED

enum {
	kCMSampleBufferError_AllocationFailed = -12730,
	kCMSampleBufferError_RequiredParameterMissing = -12731,
	kCMSampleBufferError_AlreadyHasDataBuffer = -12732,
	kCMSampleBufferError_BufferNotReady = -12733,
	kCMSampleBufferError_SampleIndexOutOfRange = -12734,
	kCMSampleBufferError_BufferHasNoSampleSizes = -12735,
	kCMSampleBufferError_BufferHasNoSampleTimingInfo = -12736,
	kCMSampleBufferError_ArrayTooSmall = -12737,
	kCMSampleBufferError_InvalidEntryCount = -12738,
	kCMSampleBufferError_CannotSubdivide = -12739,
	kCMSampleBufferError_SampleTimingInfoInvalid = -12740,
	kCMSampleBufferError_InvalidMediaTypeForOperation = -12741,
	kCMSampleBufferError_InvalidSampleData = -12742,
	kCMSampleBufferError_InvalidMediaFormat = -12743,
	kCMSampleBufferError_Invalidated = -12744,
	kCMSampleBufferError_DataFailed = -16750,
	kCMSampleBufferError_DataCanceled = -16751,
};

typedef struct CF_BRIDGED_TYPE(id) opaqueCMSampleBuffer *CMSampleBufferRef;

#pragma pack(push, 4)
typedef struct {
	CMTime duration;
	CMTime presentationTimeStamp;
	CMTime decodeTimeStamp;
} CMSampleTimingInfo;
#pragma pack(pop)

CM_EXPORT const CMSampleTimingInfo kCMTimingInfoInvalid;

typedef OSStatus (*CMSampleBufferMakeDataReadyCallback)(CMSampleBufferRef sbuf, void *makeDataReadyRefcon);

CM_EXPORT const CFStringRef kCMSampleAttachmentKey_NotSync;
CM_EXPORT const CFStringRef kCMSampleAttachmentKey_DisplayImmediately;
CM_EXPORT const CFStringRef kCMSampleAttachmentKey_IsDependedOnByOthers;
CM_EXPORT const CFStringRef kCMSampleBufferAttachmentKey_ResetDecoderBeforeDecoding;

CM_EXPORT CFTypeID CMSampleBufferGetTypeID(void);
CM_EXPORT OSStatus CMSampleBufferCreate(CFAllocatorRef allocator, CMBlockBufferRef dataBuffer, Boolean dataReady,
	CMSampleBufferMakeDataReadyCallback makeDataReadyCallback, void *makeDataReadyRefcon,
	CMFormatDescriptionRef formatDescription, CMItemCount numSamples, CMItemCount numSampleTimingEntries,
	const CMSampleTimingInfo *sampleTimingArray, CMItemCount numSampleSizeEntries,
	const size_t *sampleSizeArray, CF_RETURNS_RETAINED CMSampleBufferRef *sampleBufferOut);
CM_EXPORT OSStatus CMSampleBufferCreateReady(CFAllocatorRef allocator, CMBlockBufferRef dataBuffer,
	CMFormatDescriptionRef formatDescription, CMItemCount numSamples, CMItemCount numSampleTimingEntries,
	const CMSampleTimingInfo *sampleTimingArray, CMItemCount numSampleSizeEntries, const size_t *sampleSizeArray,
	CF_RETURNS_RETAINED CMSampleBufferRef *sampleBufferOut);
CM_EXPORT OSStatus CMSampleBufferCreateForImageBuffer(CFAllocatorRef allocator, CVImageBufferRef imageBuffer,
	Boolean dataReady, CMSampleBufferMakeDataReadyCallback makeDataReadyCallback, void *makeDataReadyRefcon,
	CMVideoFormatDescriptionRef formatDescription, const CMSampleTimingInfo *sampleTiming,
	CF_RETURNS_RETAINED CMSampleBufferRef *sampleBufferOut);
CM_EXPORT OSStatus CMSampleBufferCreateReadyWithImageBuffer(CFAllocatorRef allocator, CVImageBufferRef imageBuffer,
	CMVideoFormatDescriptionRef formatDescription, const CMSampleTimingInfo *sampleTiming,
	CF_RETURNS_RETAINED CMSampleBufferRef *sampleBufferOut);
CM_EXPORT CVImageBufferRef CMSampleBufferGetImageBuffer(CMSampleBufferRef sbuf);
CM_EXPORT Boolean CMSampleBufferDataIsReady(CMSampleBufferRef sbuf);
CM_EXPORT OSStatus CMSampleBufferMakeDataReady(CMSampleBufferRef sbuf);
CM_EXPORT CMBlockBufferRef CMSampleBufferGetDataBuffer(CMSampleBufferRef sbuf);
CM_EXPORT CMTime CMSampleBufferGetDecodeTimeStamp(CMSampleBufferRef sbuf);
CM_EXPORT CMTime CMSampleBufferGetPresentationTimeStamp(CMSampleBufferRef sbuf);
CM_EXPORT CMTime CMSampleBufferGetDuration(CMSampleBufferRef sbuf);
CM_EXPORT CMFormatDescriptionRef CMSampleBufferGetFormatDescription(CMSampleBufferRef sbuf);
CM_EXPORT CFArrayRef CMSampleBufferGetSampleAttachmentsArray(CMSampleBufferRef sbuf, Boolean createIfNecessary);
CM_EXPORT CMItemCount CMSampleBufferGetNumSamples(CMSampleBufferRef sbuf);
CM_EXPORT OSStatus CMSampleBufferCallForEachSample(CMSampleBufferRef sbuf,
	OSStatus (*callback)(CMSampleBufferRef sampleBuffer, CMItemCount index, void *refcon), void *refcon);

CF_IMPLICIT_BRIDGING_DISABLED

#endif
