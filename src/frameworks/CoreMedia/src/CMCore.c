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

/*
 * The CoreMedia that codec code (VideoToolbox clients such as VLC) calls. Format
 * descriptions, block buffers and sample buffers cannot be made yet, which fails the way a missing
 * codec does: callers fall back to software decoding.
 */

#include <CoreMedia/CoreMedia.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void unimplemented(const char *function)
{
	if (getenv("STUB_VERBOSE") != NULL)
		fprintf(stderr, "CoreMedia: %s is not implemented\n", function);
}

#pragma mark - Format descriptions

CFTypeID CMFormatDescriptionGetTypeID(void) { return 0; /* _kCFRuntimeNotATypeID: not a CF type yet */ }
CMMediaType CMFormatDescriptionGetMediaType(CMFormatDescriptionRef desc) { return 0; }
FourCharCode CMFormatDescriptionGetMediaSubType(CMFormatDescriptionRef desc) { return 0; }
CFDictionaryRef CMFormatDescriptionGetExtensions(CMFormatDescriptionRef desc) { return NULL; }
CFPropertyListRef CMFormatDescriptionGetExtension(CMFormatDescriptionRef desc, CFStringRef extensionKey) { return NULL; }

OSStatus CMVideoFormatDescriptionCreate(CFAllocatorRef allocator, CMVideoCodecType codecType, int32_t width,
	int32_t height, CFDictionaryRef extensions, CMVideoFormatDescriptionRef *formatDescriptionOut)
{
	unimplemented(__func__);
	if (formatDescriptionOut != NULL)
		*formatDescriptionOut = NULL;
	return kCMFormatDescriptionError_AllocationFailed;
}

CMVideoDimensions CMVideoFormatDescriptionGetDimensions(CMVideoFormatDescriptionRef videoDesc)
{
	CMVideoDimensions dimensions = { 0, 0 };
	return dimensions;
}

OSStatus CMVideoFormatDescriptionGetH264ParameterSetAtIndex(CMFormatDescriptionRef videoDesc, size_t parameterSetIndex,
	const uint8_t **parameterSetPointerOut, size_t *parameterSetSizeOut, size_t *parameterSetCountOut,
	int *NALUnitHeaderLengthOut)
{
	return kCMFormatDescriptionError_InvalidParameter;
}

OSStatus CMAudioFormatDescriptionCreate(CFAllocatorRef allocator, const AudioStreamBasicDescription *asbd,
	size_t layoutSize, const AudioChannelLayout *layout, size_t magicCookieSize, const void *magicCookie,
	CFDictionaryRef extensions, CMAudioFormatDescriptionRef *formatDescriptionOut)
{
	unimplemented(__func__);
	if (formatDescriptionOut != NULL)
		*formatDescriptionOut = NULL;
	if (asbd == NULL || formatDescriptionOut == NULL)
		return kCMFormatDescriptionError_InvalidParameter;
	return kCMFormatDescriptionError_AllocationFailed;
}

const AudioStreamBasicDescription *CMAudioFormatDescriptionGetStreamBasicDescription(CMAudioFormatDescriptionRef desc)
{
	return NULL;
}

const AudioChannelLayout *CMAudioFormatDescriptionGetChannelLayout(CMAudioFormatDescriptionRef desc, size_t *sizeOut)
{
	if (sizeOut != NULL)
		*sizeOut = 0;
	return NULL;
}

const void *CMAudioFormatDescriptionGetMagicCookie(CMAudioFormatDescriptionRef desc, size_t *sizeOut)
{
	if (sizeOut != NULL)
		*sizeOut = 0;
	return NULL;
}

#pragma mark - Block buffers

CFTypeID CMBlockBufferGetTypeID(void) { return 0; /* _kCFRuntimeNotATypeID: not a CF type yet */ }

OSStatus CMBlockBufferCreateWithMemoryBlock(CFAllocatorRef structureAllocator, void *memoryBlock, size_t blockLength,
	CFAllocatorRef blockAllocator, const CMBlockBufferCustomBlockSource *customBlockSource, size_t offsetToData,
	size_t dataLength, CMBlockBufferFlags flags, CMBlockBufferRef *blockBufferOut)
{
	unimplemented(__func__);
	if (blockBufferOut != NULL)
		*blockBufferOut = NULL;
	return kCMBlockBufferStructureAllocationFailedErr;
}

size_t CMBlockBufferGetDataLength(CMBlockBufferRef theBuffer) { return 0; }

OSStatus CMBlockBufferGetDataPointer(CMBlockBufferRef theBuffer, size_t offset, size_t *lengthAtOffsetOut,
	size_t *totalLengthOut, char **dataPointerOut)
{
	return theBuffer == NULL ? kCMBlockBufferBadPointerParameterErr : kCMBlockBufferEmptyBBufErr;
}

#pragma mark - Sample buffers

CFTypeID CMSampleBufferGetTypeID(void) { return 0; /* _kCFRuntimeNotATypeID: not a CF type yet */ }

OSStatus CMSampleBufferCreate(CFAllocatorRef allocator, CMBlockBufferRef dataBuffer, Boolean dataReady,
	CMSampleBufferMakeDataReadyCallback makeDataReadyCallback, void *makeDataReadyRefcon,
	CMFormatDescriptionRef formatDescription, CMItemCount numSamples, CMItemCount numSampleTimingEntries,
	const CMSampleTimingInfo *sampleTimingArray, CMItemCount numSampleSizeEntries, const size_t *sampleSizeArray,
	CMSampleBufferRef *sampleBufferOut)
{
	unimplemented(__func__);
	if (sampleBufferOut != NULL)
		*sampleBufferOut = NULL;
	return kCMSampleBufferError_AllocationFailed;
}

OSStatus CMSampleBufferCreateReady(CFAllocatorRef allocator, CMBlockBufferRef dataBuffer,
	CMFormatDescriptionRef formatDescription, CMItemCount numSamples, CMItemCount numSampleTimingEntries,
	const CMSampleTimingInfo *sampleTimingArray, CMItemCount numSampleSizeEntries, const size_t *sampleSizeArray,
	CMSampleBufferRef *sampleBufferOut)
{
	return CMSampleBufferCreate(allocator, dataBuffer, true, NULL, NULL, formatDescription, numSamples,
		numSampleTimingEntries, sampleTimingArray, numSampleSizeEntries, sampleSizeArray, sampleBufferOut);
}

CMBlockBufferRef CMSampleBufferGetDataBuffer(CMSampleBufferRef sbuf) { return NULL; }
CMTime CMSampleBufferGetDecodeTimeStamp(CMSampleBufferRef sbuf) { return kCMTimeInvalid; }
CMTime CMSampleBufferGetPresentationTimeStamp(CMSampleBufferRef sbuf) { return kCMTimeInvalid; }
CMTime CMSampleBufferGetDuration(CMSampleBufferRef sbuf) { return kCMTimeInvalid; }
CMFormatDescriptionRef CMSampleBufferGetFormatDescription(CMSampleBufferRef sbuf) { return NULL; }
CFArrayRef CMSampleBufferGetSampleAttachmentsArray(CMSampleBufferRef sbuf, Boolean createIfNecessary) { return NULL; }
CMItemCount CMSampleBufferGetNumSamples(CMSampleBufferRef sbuf) { return 0; }

OSStatus CMSampleBufferCallForEachSample(CMSampleBufferRef sbuf,
	OSStatus (*callback)(CMSampleBufferRef sampleBuffer, CMItemCount index, void *refcon), void *refcon)
{
	return sbuf == NULL ? kCMSampleBufferError_RequiredParameterMissing : kCMSampleBufferError_BufferHasNoSampleSizes;
}
