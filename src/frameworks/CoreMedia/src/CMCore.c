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
 * Format descriptions, block buffers and sample buffers as CF objects.
 */

#include <CoreMedia/CoreMedia.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreVideo/CoreVideo.h>
#include <dispatch/dispatch.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void unimplemented(const char *function)
{
	if (getenv("STUB_VERBOSE") != NULL)
		fprintf(stderr, "CoreMedia: %s is not implemented\n", function);
}

#pragma mark - CF classes

#define CM_CLASS(name, finalizer) \
	static CFTypeID name##TypeID = _kCFRuntimeNotATypeID; \
	static const CFRuntimeClass name##Class = { .version = 0, .className = #name, .finalize = finalizer }; \
	static CFTypeID name##GetTypeID(void) \
	{ \
		static dispatch_once_t once; \
		dispatch_once(&once, ^{ name##TypeID = _CFRuntimeRegisterClass(&name##Class); }); \
		return name##TypeID; \
	}

#define CM_NEW(type, name, allocator) \
	((type)_CFRuntimeCreateInstance(allocator, name##GetTypeID(), sizeof(*(type)NULL) - sizeof(CFRuntimeBase), NULL))

#pragma mark - Format descriptions

struct opaqueCMFormatDescription {
	CFRuntimeBase base;
	CMMediaType mediaType;
	FourCharCode subType;
	CFDictionaryRef extensions;
	CMVideoDimensions dimensions;
	AudioStreamBasicDescription asbd;
	void *layout;
	size_t layoutSize;
	void *cookie;
	size_t cookieSize;
};

static void formatDescriptionFinalize(CFTypeRef cf)
{
	struct opaqueCMFormatDescription *desc = (struct opaqueCMFormatDescription *)cf;
	if (desc->extensions != NULL)
		CFRelease(desc->extensions);
	free(desc->layout);
	free(desc->cookie);
}

CM_CLASS(CMFormatDescription_, formatDescriptionFinalize)

static struct opaqueCMFormatDescription *newFormatDescription(CFAllocatorRef allocator, CMMediaType mediaType,
	FourCharCode subType, CFDictionaryRef extensions)
{
	struct opaqueCMFormatDescription *desc = CM_NEW(struct opaqueCMFormatDescription *, CMFormatDescription_, allocator);
	if (desc == NULL)
		return NULL;
	desc->mediaType = mediaType;
	desc->subType = subType;
	desc->extensions = extensions != NULL ? CFDictionaryCreateCopy(allocator, extensions) : NULL;
	return desc;
}

CFTypeID CMFormatDescriptionGetTypeID(void) { return CMFormatDescription_GetTypeID(); }

CMMediaType CMFormatDescriptionGetMediaType(CMFormatDescriptionRef desc) { return desc ? desc->mediaType : 0; }
FourCharCode CMFormatDescriptionGetMediaSubType(CMFormatDescriptionRef desc) { return desc ? desc->subType : 0; }
CFDictionaryRef CMFormatDescriptionGetExtensions(CMFormatDescriptionRef desc) { return desc ? desc->extensions : NULL; }

CFPropertyListRef CMFormatDescriptionGetExtension(CMFormatDescriptionRef desc, CFStringRef extensionKey)
{
	if (desc == NULL || desc->extensions == NULL || extensionKey == NULL)
		return NULL;
	return CFDictionaryGetValue(desc->extensions, extensionKey);
}

OSStatus CMVideoFormatDescriptionCreate(CFAllocatorRef allocator, CMVideoCodecType codecType, int32_t width,
	int32_t height, CFDictionaryRef extensions, CMVideoFormatDescriptionRef *formatDescriptionOut)
{
	if (formatDescriptionOut == NULL)
		return kCMFormatDescriptionError_InvalidParameter;
	*formatDescriptionOut = NULL;
	struct opaqueCMFormatDescription *desc = newFormatDescription(allocator, kCMMediaType_Video, codecType, extensions);
	if (desc == NULL)
		return kCMFormatDescriptionError_AllocationFailed;
	desc->dimensions = (CMVideoDimensions){ width, height };
	*formatDescriptionOut = desc;
	return noErr;
}

/* The extensions are the buffer's propagated attachments (its color, aspect and chroma keys), as
   Apple's are. */
OSStatus CMVideoFormatDescriptionCreateForImageBuffer(CFAllocatorRef allocator, CVImageBufferRef imageBuffer,
	CMVideoFormatDescriptionRef *formatDescriptionOut)
{
	if (formatDescriptionOut != NULL)
		*formatDescriptionOut = NULL;
	if (imageBuffer == NULL || formatDescriptionOut == NULL || CFGetTypeID(imageBuffer) != CVPixelBufferGetTypeID())
		return kCMFormatDescriptionError_InvalidParameter;
	CFDictionaryRef extensions = CVBufferCopyAttachments(imageBuffer, kCVAttachmentMode_ShouldPropagate);
	OSStatus status = CMVideoFormatDescriptionCreate(allocator, CVPixelBufferGetPixelFormatType(imageBuffer),
		(int32_t)CVPixelBufferGetWidth(imageBuffer), (int32_t)CVPixelBufferGetHeight(imageBuffer), extensions,
		formatDescriptionOut);
	if (extensions != NULL)
		CFRelease(extensions);
	return status;
}

Boolean CMVideoFormatDescriptionMatchesImageBuffer(CMVideoFormatDescriptionRef desc, CVImageBufferRef imageBuffer)
{
	if (desc == NULL || imageBuffer == NULL || desc->mediaType != kCMMediaType_Video
		|| CFGetTypeID(imageBuffer) != CVPixelBufferGetTypeID())
		return false;
	return desc->subType == CVPixelBufferGetPixelFormatType(imageBuffer)
		&& desc->dimensions.width == (int32_t)CVPixelBufferGetWidth(imageBuffer)
		&& desc->dimensions.height == (int32_t)CVPixelBufferGetHeight(imageBuffer);
}

CMVideoDimensions CMVideoFormatDescriptionGetDimensions(CMVideoFormatDescriptionRef videoDesc)
{
	if (videoDesc == NULL || videoDesc->mediaType != kCMMediaType_Video)
		return (CMVideoDimensions){ 0, 0 };
	return videoDesc->dimensions;
}

/* The parameter sets of an avcC atom among the extensions' sample description atoms. */
OSStatus CMVideoFormatDescriptionGetH264ParameterSetAtIndex(CMFormatDescriptionRef videoDesc, size_t parameterSetIndex,
	const uint8_t **parameterSetPointerOut, size_t *parameterSetSizeOut, size_t *parameterSetCountOut,
	int *NALUnitHeaderLengthOut)
{
	CFDictionaryRef atoms = videoDesc ? (CFDictionaryRef)CMFormatDescriptionGetExtension(videoDesc,
		kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms) : NULL;
	if (atoms == NULL || CFGetTypeID(atoms) != CFDictionaryGetTypeID())
		return kCMFormatDescriptionError_InvalidParameter;
	CFDataRef avcC = CFDictionaryGetValue(atoms, CFSTR("avcC"));
	if (avcC == NULL || CFGetTypeID(avcC) != CFDataGetTypeID() || CFDataGetLength(avcC) < 7)
		return kCMFormatDescriptionError_InvalidParameter;
	const uint8_t *p = CFDataGetBytePtr(avcC), *end = p + CFDataGetLength(avcC);
	int nalLength = (p[4] & 3) + 1;
	const uint8_t *sets[64];
	size_t sizes[64], count = 0;
	p += 5;
	for (int kind = 0; kind < 2 && p < end; kind++) {
		unsigned n = kind == 0 ? (*p++ & 0x1f) : *p++;
		for (unsigned i = 0; i < n; i++) {
			if (end - p < 2)
				return kCMFormatDescriptionError_InvalidParameter;
			size_t size = ((size_t)p[0] << 8) | p[1];
			p += 2;
			if ((size_t)(end - p) < size || count == 64)
				return kCMFormatDescriptionError_InvalidParameter;
			sets[count] = p;
			sizes[count++] = size;
			p += size;
		}
	}
	if (parameterSetCountOut != NULL)
		*parameterSetCountOut = count;
	if (NALUnitHeaderLengthOut != NULL)
		*NALUnitHeaderLengthOut = nalLength;
	if (parameterSetPointerOut == NULL && parameterSetSizeOut == NULL)
		return noErr;
	if (parameterSetIndex >= count)
		return kCMFormatDescriptionError_InvalidParameter;
	if (parameterSetPointerOut != NULL)
		*parameterSetPointerOut = sets[parameterSetIndex];
	if (parameterSetSizeOut != NULL)
		*parameterSetSizeOut = sizes[parameterSetIndex];
	return noErr;
}

static void *copyBytes(const void *bytes, size_t size)
{
	if (bytes == NULL || size == 0)
		return NULL;
	void *copy = malloc(size);
	if (copy != NULL)
		memcpy(copy, bytes, size);
	return copy;
}

OSStatus CMAudioFormatDescriptionCreate(CFAllocatorRef allocator, const AudioStreamBasicDescription *asbd,
	size_t layoutSize, const AudioChannelLayout *layout, size_t magicCookieSize, const void *magicCookie,
	CFDictionaryRef extensions, CMAudioFormatDescriptionRef *formatDescriptionOut)
{
	if (formatDescriptionOut != NULL)
		*formatDescriptionOut = NULL;
	if (asbd == NULL || formatDescriptionOut == NULL || (layoutSize != 0 && layout == NULL)
		|| (magicCookieSize != 0 && magicCookie == NULL))
		return kCMFormatDescriptionError_InvalidParameter;
	struct opaqueCMFormatDescription *desc = newFormatDescription(allocator, kCMMediaType_Audio, asbd->mFormatID, extensions);
	if (desc == NULL)
		return kCMFormatDescriptionError_AllocationFailed;
	desc->asbd = *asbd;
	desc->layout = copyBytes(layout, layoutSize);
	desc->layoutSize = desc->layout ? layoutSize : 0;
	desc->cookie = copyBytes(magicCookie, magicCookieSize);
	desc->cookieSize = desc->cookie ? magicCookieSize : 0;
	*formatDescriptionOut = desc;
	return noErr;
}

const AudioStreamBasicDescription *CMAudioFormatDescriptionGetStreamBasicDescription(CMAudioFormatDescriptionRef desc)
{
	return desc != NULL && desc->mediaType == kCMMediaType_Audio ? &desc->asbd : NULL;
}

const AudioChannelLayout *CMAudioFormatDescriptionGetChannelLayout(CMAudioFormatDescriptionRef desc, size_t *sizeOut)
{
	if (sizeOut != NULL)
		*sizeOut = desc ? desc->layoutSize : 0;
	return desc ? desc->layout : NULL;
}

const void *CMAudioFormatDescriptionGetMagicCookie(CMAudioFormatDescriptionRef desc, size_t *sizeOut)
{
	if (sizeOut != NULL)
		*sizeOut = desc ? desc->cookieSize : 0;
	return desc ? desc->cookie : NULL;
}

#pragma mark - Block buffers

struct OpaqueCMBlockBuffer {
	CFRuntimeBase base;
	void *memory;
	size_t blockLength;
	CFAllocatorRef blockAllocator;
	CMBlockBufferCustomBlockSource source;
	Boolean hasSource;
	size_t offset;
	size_t dataLength;
};

static void blockBufferFinalize(CFTypeRef cf)
{
	struct OpaqueCMBlockBuffer *buffer = (struct OpaqueCMBlockBuffer *)cf;
	if (buffer->memory != NULL) {
		if (buffer->hasSource) {
			if (buffer->source.FreeBlock != NULL)
				buffer->source.FreeBlock(buffer->source.refCon, buffer->memory, buffer->blockLength);
		} else if (buffer->blockAllocator != kCFAllocatorNull) {
			CFAllocatorDeallocate(buffer->blockAllocator, buffer->memory);
		}
	}
	if (buffer->blockAllocator != NULL)
		CFRelease(buffer->blockAllocator);
}

CM_CLASS(CMBlockBuffer_, blockBufferFinalize)

CFTypeID CMBlockBufferGetTypeID(void) { return CMBlockBuffer_GetTypeID(); }

/* With no memory block, one of blockLength bytes is allocated, by the custom source or the block
   allocator (the default one when NULL), and freed the same way; kCFAllocatorNull frees nothing. */
OSStatus CMBlockBufferCreateWithMemoryBlock(CFAllocatorRef structureAllocator, void *memoryBlock, size_t blockLength,
	CFAllocatorRef blockAllocator, const CMBlockBufferCustomBlockSource *customBlockSource, size_t offsetToData,
	size_t dataLength, CMBlockBufferFlags flags, CMBlockBufferRef *blockBufferOut)
{
	if (blockBufferOut == NULL)
		return kCMBlockBufferBadPointerParameterErr;
	*blockBufferOut = NULL;
	if (offsetToData > blockLength || dataLength > blockLength - offsetToData)
		return kCMBlockBufferBadLengthParameterErr;
	if (customBlockSource != NULL && customBlockSource->version != kCMBlockBufferCustomBlockSourceVersion)
		return kCMBlockBufferBadCustomBlockSourceErr;
	struct OpaqueCMBlockBuffer *buffer = CM_NEW(struct OpaqueCMBlockBuffer *, CMBlockBuffer_, structureAllocator);
	if (buffer == NULL)
		return kCMBlockBufferStructureAllocationFailedErr;
	buffer->blockLength = blockLength;
	buffer->offset = offsetToData;
	buffer->dataLength = dataLength;
	if (customBlockSource != NULL) {
		buffer->source = *customBlockSource;
		buffer->hasSource = true;
	} else {
		buffer->blockAllocator = CFRetain(blockAllocator != NULL ? blockAllocator : kCFAllocatorDefault);
	}
	if (memoryBlock == NULL && blockLength > 0) {
		if (buffer->hasSource)
			memoryBlock = buffer->source.AllocateBlock ? buffer->source.AllocateBlock(buffer->source.refCon, blockLength) : NULL;
		else
			memoryBlock = CFAllocatorAllocate(buffer->blockAllocator, blockLength, 0);
		if (memoryBlock == NULL) {
			CFRelease(buffer);
			return kCMBlockBufferBlockAllocationFailedErr;
		}
	}
	buffer->memory = memoryBlock;
	*blockBufferOut = buffer;
	return kCMBlockBufferNoErr;
}

size_t CMBlockBufferGetDataLength(CMBlockBufferRef theBuffer) { return theBuffer ? theBuffer->dataLength : 0; }

OSStatus CMBlockBufferGetDataPointer(CMBlockBufferRef theBuffer, size_t offset, size_t *lengthAtOffsetOut,
	size_t *totalLengthOut, char **dataPointerOut)
{
	if (theBuffer == NULL)
		return kCMBlockBufferBadPointerParameterErr;
	if (theBuffer->dataLength == 0)
		return kCMBlockBufferEmptyBBufErr;
	if (offset >= theBuffer->dataLength)
		return kCMBlockBufferBadOffsetParameterErr;
	if (theBuffer->memory == NULL)
		return kCMBlockBufferUnallocatedBlockErr;
	if (lengthAtOffsetOut != NULL)
		*lengthAtOffsetOut = theBuffer->dataLength - offset;
	if (totalLengthOut != NULL)
		*totalLengthOut = theBuffer->dataLength;
	if (dataPointerOut != NULL)
		*dataPointerOut = (char *)theBuffer->memory + theBuffer->offset + offset;
	return kCMBlockBufferNoErr;
}

#pragma mark - Sample buffers

struct opaqueCMSampleBuffer {
	CFRuntimeBase base;
	CMBlockBufferRef dataBuffer;
	CVImageBufferRef imageBuffer;
	CMFormatDescriptionRef formatDescription;
	CMItemCount numSamples;
	CMSampleTimingInfo *timing;
	CMItemCount numTiming;
	size_t *sizes;
	CMItemCount numSizes;
	CFMutableArrayRef attachments;
	Boolean dataReady;
};

static void sampleBufferFinalize(CFTypeRef cf)
{
	struct opaqueCMSampleBuffer *sbuf = (struct opaqueCMSampleBuffer *)cf;
	if (sbuf->dataBuffer != NULL)
		CFRelease(sbuf->dataBuffer);
	if (sbuf->imageBuffer != NULL)
		CFRelease(sbuf->imageBuffer);
	if (sbuf->formatDescription != NULL)
		CFRelease(sbuf->formatDescription);
	if (sbuf->attachments != NULL)
		CFRelease(sbuf->attachments);
	free(sbuf->timing);
	free(sbuf->sizes);
}

CM_CLASS(CMSampleBuffer_, sampleBufferFinalize)

CFTypeID CMSampleBufferGetTypeID(void) { return CMSampleBuffer_GetTypeID(); }

static struct opaqueCMSampleBuffer *newSampleBuffer(CFAllocatorRef allocator, CMFormatDescriptionRef formatDescription,
	CMItemCount numSamples, CMItemCount numSampleTimingEntries, const CMSampleTimingInfo *sampleTimingArray,
	CMItemCount numSampleSizeEntries, const size_t *sampleSizeArray, OSStatus *status)
{
	if (numSamples < 0 || numSampleTimingEntries < 0 || numSampleSizeEntries < 0
		|| (numSampleTimingEntries > 0 && sampleTimingArray == NULL) || (numSampleSizeEntries > 0 && sampleSizeArray == NULL)
		|| (numSampleTimingEntries > 1 && numSampleTimingEntries != numSamples)
		|| (numSampleSizeEntries > 1 && numSampleSizeEntries != numSamples)) {
		*status = kCMSampleBufferError_InvalidEntryCount;
		return NULL;
	}
	struct opaqueCMSampleBuffer *sbuf = CM_NEW(struct opaqueCMSampleBuffer *, CMSampleBuffer_, allocator);
	if (sbuf == NULL) {
		*status = kCMSampleBufferError_AllocationFailed;
		return NULL;
	}
	sbuf->formatDescription = formatDescription ? CFRetain(formatDescription) : NULL;
	sbuf->numSamples = numSamples;
	sbuf->timing = copyBytes(sampleTimingArray, numSampleTimingEntries * sizeof(CMSampleTimingInfo));
	sbuf->numTiming = sbuf->timing ? numSampleTimingEntries : 0;
	sbuf->sizes = copyBytes(sampleSizeArray, numSampleSizeEntries * sizeof(size_t));
	sbuf->numSizes = sbuf->sizes ? numSampleSizeEntries : 0;
	*status = noErr;
	return sbuf;
}

/* A buffer that is not ready would be made ready by the callback when its data is needed; this
   CoreMedia asks for it at once. */
OSStatus CMSampleBufferCreate(CFAllocatorRef allocator, CMBlockBufferRef dataBuffer, Boolean dataReady,
	CMSampleBufferMakeDataReadyCallback makeDataReadyCallback, void *makeDataReadyRefcon,
	CMFormatDescriptionRef formatDescription, CMItemCount numSamples, CMItemCount numSampleTimingEntries,
	const CMSampleTimingInfo *sampleTimingArray, CMItemCount numSampleSizeEntries, const size_t *sampleSizeArray,
	CMSampleBufferRef *sampleBufferOut)
{
	if (sampleBufferOut == NULL)
		return kCMSampleBufferError_RequiredParameterMissing;
	*sampleBufferOut = NULL;
	OSStatus status;
	struct opaqueCMSampleBuffer *sbuf = newSampleBuffer(allocator, formatDescription, numSamples,
		numSampleTimingEntries, sampleTimingArray, numSampleSizeEntries, sampleSizeArray, &status);
	if (sbuf == NULL)
		return status;
	sbuf->dataBuffer = dataBuffer ? (CMBlockBufferRef)CFRetain(dataBuffer) : NULL;
	sbuf->dataReady = dataReady;
	if (!dataReady && makeDataReadyCallback != NULL && dataBuffer != NULL) {
		status = makeDataReadyCallback(sbuf, makeDataReadyRefcon);
		sbuf->dataReady = status == noErr;
	}
	*sampleBufferOut = sbuf;
	return noErr;
}

OSStatus CMSampleBufferCreateReady(CFAllocatorRef allocator, CMBlockBufferRef dataBuffer,
	CMFormatDescriptionRef formatDescription, CMItemCount numSamples, CMItemCount numSampleTimingEntries,
	const CMSampleTimingInfo *sampleTimingArray, CMItemCount numSampleSizeEntries, const size_t *sampleSizeArray,
	CMSampleBufferRef *sampleBufferOut)
{
	return CMSampleBufferCreate(allocator, dataBuffer, true, NULL, NULL, formatDescription, numSamples,
		numSampleTimingEntries, sampleTimingArray, numSampleSizeEntries, sampleSizeArray, sampleBufferOut);
}

OSStatus CMSampleBufferCreateForImageBuffer(CFAllocatorRef allocator, CVImageBufferRef imageBuffer,
	Boolean dataReady, CMSampleBufferMakeDataReadyCallback makeDataReadyCallback, void *makeDataReadyRefcon,
	CMVideoFormatDescriptionRef formatDescription, const CMSampleTimingInfo *sampleTiming,
	CMSampleBufferRef *sampleBufferOut)
{
	if (sampleBufferOut != NULL)
		*sampleBufferOut = NULL;
	if (imageBuffer == NULL || formatDescription == NULL || sampleTiming == NULL || sampleBufferOut == NULL)
		return kCMSampleBufferError_RequiredParameterMissing;
	if (!CMVideoFormatDescriptionMatchesImageBuffer(formatDescription, imageBuffer))
		return kCMSampleBufferError_InvalidMediaFormat;
	OSStatus status;
	struct opaqueCMSampleBuffer *sbuf = newSampleBuffer(allocator, formatDescription, 1, 1, sampleTiming, 0, NULL, &status);
	if (sbuf == NULL)
		return status;
	sbuf->imageBuffer = (CVImageBufferRef)CFRetain(imageBuffer);
	sbuf->dataReady = dataReady;
	if (!dataReady && makeDataReadyCallback != NULL)
		sbuf->dataReady = makeDataReadyCallback(sbuf, makeDataReadyRefcon) == noErr;
	*sampleBufferOut = sbuf;
	return noErr;
}

OSStatus CMSampleBufferCreateReadyWithImageBuffer(CFAllocatorRef allocator, CVImageBufferRef imageBuffer,
	CMVideoFormatDescriptionRef formatDescription, const CMSampleTimingInfo *sampleTiming,
	CMSampleBufferRef *sampleBufferOut)
{
	return CMSampleBufferCreateForImageBuffer(allocator, imageBuffer, true, NULL, NULL, formatDescription,
		sampleTiming, sampleBufferOut);
}

CVImageBufferRef CMSampleBufferGetImageBuffer(CMSampleBufferRef sbuf) { return sbuf ? sbuf->imageBuffer : NULL; }
CMBlockBufferRef CMSampleBufferGetDataBuffer(CMSampleBufferRef sbuf) { return sbuf ? sbuf->dataBuffer : NULL; }
CMFormatDescriptionRef CMSampleBufferGetFormatDescription(CMSampleBufferRef sbuf) { return sbuf ? sbuf->formatDescription : NULL; }
CMItemCount CMSampleBufferGetNumSamples(CMSampleBufferRef sbuf) { return sbuf ? sbuf->numSamples : 0; }

CMTime CMSampleBufferGetPresentationTimeStamp(CMSampleBufferRef sbuf)
{
	return sbuf && sbuf->numTiming > 0 ? sbuf->timing[0].presentationTimeStamp : kCMTimeInvalid;
}

CMTime CMSampleBufferGetDecodeTimeStamp(CMSampleBufferRef sbuf)
{
	return sbuf && sbuf->numTiming > 0 ? sbuf->timing[0].decodeTimeStamp : kCMTimeInvalid;
}

/* One timing entry for many samples gives each sample's duration. */
CMTime CMSampleBufferGetDuration(CMSampleBufferRef sbuf)
{
	if (sbuf == NULL || sbuf->numTiming == 0)
		return kCMTimeInvalid;
	if (sbuf->numTiming == 1)
		return CMTimeMultiply(sbuf->timing[0].duration, (int32_t)sbuf->numSamples);
	CMTime total = kCMTimeZero;
	for (CMItemCount i = 0; i < sbuf->numTiming; i++)
		total = CMTimeAdd(total, sbuf->timing[i].duration);
	return total;
}

CFArrayRef CMSampleBufferGetSampleAttachmentsArray(CMSampleBufferRef sbuf, Boolean createIfNecessary)
{
	if (sbuf == NULL)
		return NULL;
	if (sbuf->attachments == NULL && createIfNecessary) {
		struct opaqueCMSampleBuffer *mutable = (struct opaqueCMSampleBuffer *)sbuf;
		mutable->attachments = CFArrayCreateMutable(NULL, sbuf->numSamples, &kCFTypeArrayCallBacks);
		for (CMItemCount i = 0; i < sbuf->numSamples; i++) {
			CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
				&kCFTypeDictionaryValueCallBacks);
			CFArrayAppendValue(mutable->attachments, dict);
			CFRelease(dict);
		}
	}
	return sbuf->attachments;
}

/* Each sample in turn, as a sample buffer of its own sharing the data. */
OSStatus CMSampleBufferCallForEachSample(CMSampleBufferRef sbuf,
	OSStatus (*callback)(CMSampleBufferRef sampleBuffer, CMItemCount index, void *refcon), void *refcon)
{
	if (sbuf == NULL || callback == NULL)
		return kCMSampleBufferError_RequiredParameterMissing;
	if (sbuf->numSamples > 1 && sbuf->numSizes == 0)
		return kCMSampleBufferError_BufferHasNoSampleSizes;
	if (sbuf->numSamples <= 1)
		return sbuf->numSamples == 1 ? callback(sbuf, 0, refcon) : noErr;
	size_t offset = 0;
	for (CMItemCount i = 0; i < sbuf->numSamples; i++) {
		size_t size = sbuf->sizes[sbuf->numSizes == 1 ? 0 : i];
		CMSampleTimingInfo timing = sbuf->numTiming == 0 ? (CMSampleTimingInfo){ kCMTimeInvalid, kCMTimeInvalid, kCMTimeInvalid }
			: sbuf->timing[sbuf->numTiming == 1 ? 0 : i];
		if (sbuf->numTiming == 1 && CMTIME_IS_NUMERIC(timing.duration)) {
			CMTime step = CMTimeMultiply(timing.duration, (int32_t)i);
			timing.presentationTimeStamp = CMTimeAdd(timing.presentationTimeStamp, step);
			if (CMTIME_IS_VALID(timing.decodeTimeStamp))
				timing.decodeTimeStamp = CMTimeAdd(timing.decodeTimeStamp, step);
		}
		char *data = NULL;
		CMBlockBufferRef block = NULL;
		OSStatus status = noErr;
		if (sbuf->dataBuffer != NULL && CMBlockBufferGetDataPointer(sbuf->dataBuffer, offset, NULL, NULL, &data) == noErr)
			status = CMBlockBufferCreateWithMemoryBlock(NULL, data, size, kCFAllocatorNull, NULL, 0, size, 0, &block);
		CMSampleBufferRef sample = NULL;
		if (status == noErr)
			status = CMSampleBufferCreate(NULL, block, true, NULL, NULL, sbuf->formatDescription, 1, 1, &timing, 1,
				&size, &sample);
		if (block != NULL)
			CFRelease(block);
		if (status == noErr)
			status = callback(sample, i, refcon);
		if (sample != NULL)
			CFRelease(sample);
		if (status != noErr)
			return status;
		offset += size;
	}
	return noErr;
}
