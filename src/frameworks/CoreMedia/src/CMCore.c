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
#include "CMAttachmentStore.h"
#include <CoreFoundation/CFRuntime.h>
#include <CoreVideo/CoreVideo.h>
#include <dispatch/dispatch.h>
#include <string.h>
#include <stdlib.h>

#pragma mark - CF classes

#define CM_CLASS(name, cname, ...) \
	static CFTypeID name##TypeID = _kCFRuntimeNotATypeID; \
	static const CFRuntimeClass name##Class = { .version = 0, .className = cname, __VA_ARGS__ }; \
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

static Boolean sameBytes(const void *a, size_t aSize, const void *b, size_t bSize)
{
	return aSize == bSize && (aSize == 0 || memcmp(a, b, aSize) == 0);
}

/* Descriptions of the same media type and subtype whose parameters and extensions are equal are
   equal. */
static Boolean formatDescriptionEqual(CFTypeRef cf1, CFTypeRef cf2)
{
	const struct opaqueCMFormatDescription *a = cf1, *b = cf2;
	if (a->mediaType != b->mediaType || a->subType != b->subType)
		return false;
	if (a->extensions != b->extensions && (a->extensions == NULL || b->extensions == NULL
		|| !CFEqual(a->extensions, b->extensions)))
		return false;
	if (a->mediaType == kCMMediaType_Video)
		return a->dimensions.width == b->dimensions.width && a->dimensions.height == b->dimensions.height;
	if (a->mediaType == kCMMediaType_Audio)
		return memcmp(&a->asbd, &b->asbd, sizeof(a->asbd)) == 0
			&& sameBytes(a->layout, a->layoutSize, b->layout, b->layoutSize)
			&& sameBytes(a->cookie, a->cookieSize, b->cookie, b->cookieSize);
	return true;
}

static CFHashCode formatDescriptionHash(CFTypeRef cf)
{
	const struct opaqueCMFormatDescription *desc = cf;
	return (CFHashCode)desc->mediaType ^ ((CFHashCode)desc->subType << 1);
}

CM_CLASS(CMFormatDescription_, "CMFormatDescription", .finalize = formatDescriptionFinalize, .equal = formatDescriptionEqual, .hash = formatDescriptionHash)

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

Boolean CMFormatDescriptionEqual(CMFormatDescriptionRef desc1, CMFormatDescriptionRef desc2)
{
	if (desc1 == NULL || desc2 == NULL)
		return desc1 == desc2;
	return CFEqual(desc1, desc2);
}

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
	if (parameterSetIndex >= count) {
		if (parameterSetPointerOut != NULL)
			*parameterSetPointerOut = NULL;
		if (parameterSetSizeOut != NULL)
			*parameterSetSizeOut = 0;
		return kCMFormatDescriptionBridgeError_InvalidParameter;
	}
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
	/* QUIRK: a non-zero layout or cookie size with a NULL pointer is not an error; the description is made
	   without that layout or cookie. Observed on macOS 10.14.4 and 26.7.3 (black-box, tier 2).
	   Test: test_quirk_audio_description_null_layout_with_size in coremedia_objects.c */
	if (asbd == NULL || formatDescriptionOut == NULL)
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
	CMAttachmentStore bearerAttachments;
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
	CMAttachmentStoreRelease(&buffer->bearerAttachments);
}

CM_CLASS(CMBlockBuffer_, "CMBlockBuffer", .finalize = blockBufferFinalize)

CFTypeID CMBlockBufferGetTypeID(void) { return CMBlockBuffer_GetTypeID(); }

/* Memory is taken from the custom source, or the block allocator (the default one when NULL), and freed
   the same way; kCFAllocatorNull frees nothing. Without a memory block, the block is only allocated
   when asked for (kCMBlockBufferAssureMemoryNowFlag or CMBlockBufferAssureBlockMemory), and one cannot
   be described without a length. */
static OSStatus assureMemory(struct OpaqueCMBlockBuffer *buffer)
{
	if (buffer->memory != NULL || buffer->blockLength == 0)
		return kCMBlockBufferNoErr;
	if (buffer->hasSource)
		buffer->memory = buffer->source.AllocateBlock != NULL
			? buffer->source.AllocateBlock(buffer->source.refCon, buffer->blockLength) : NULL;
	else
		buffer->memory = CFAllocatorAllocate(buffer->blockAllocator, buffer->blockLength, 0);
	return buffer->memory != NULL ? kCMBlockBufferNoErr : kCMBlockBufferBlockAllocationFailedErr;
}

OSStatus CMBlockBufferCreateWithMemoryBlock(CFAllocatorRef structureAllocator, void *memoryBlock, size_t blockLength,
	CFAllocatorRef blockAllocator, const CMBlockBufferCustomBlockSource *customBlockSource, size_t offsetToData,
	size_t dataLength, CMBlockBufferFlags flags, CMBlockBufferRef *blockBufferOut)
{
	if (blockBufferOut == NULL)
		return kCMBlockBufferBadPointerParameterErr;
	*blockBufferOut = NULL;
	if ((memoryBlock == NULL && blockLength == 0) || offsetToData > blockLength || dataLength > blockLength - offsetToData)
		return kCMBlockBufferBadLengthParameterErr;
	if (customBlockSource != NULL && customBlockSource->version != kCMBlockBufferCustomBlockSourceVersion)
		return kCMBlockBufferBadCustomBlockSourceErr;
	struct OpaqueCMBlockBuffer *buffer = CM_NEW(struct OpaqueCMBlockBuffer *, CMBlockBuffer_, structureAllocator);
	if (buffer == NULL)
		return kCMBlockBufferStructureAllocationFailedErr;
	buffer->memory = memoryBlock;
	buffer->blockLength = blockLength;
	buffer->offset = offsetToData;
	buffer->dataLength = dataLength;
	if (customBlockSource != NULL) {
		buffer->source = *customBlockSource;
		buffer->hasSource = true;
	} else {
		buffer->blockAllocator = CFRetain(blockAllocator != NULL ? blockAllocator : CFAllocatorGetDefault());
	}
	if ((flags & kCMBlockBufferAssureMemoryNowFlag) != 0) {
		OSStatus status = assureMemory(buffer);
		if (status != kCMBlockBufferNoErr) {
			CFRelease(buffer);
			return status;
		}
	}
	*blockBufferOut = buffer;
	return kCMBlockBufferNoErr;
}

OSStatus CMBlockBufferAssureBlockMemory(CMBlockBufferRef theBuffer)
{
	return theBuffer == NULL ? kCMBlockBufferBadPointerParameterErr : assureMemory(theBuffer);
}

size_t CMBlockBufferGetDataLength(CMBlockBufferRef theBuffer) { return theBuffer ? theBuffer->dataLength : 0; }

Boolean CMBlockBufferIsEmpty(CMBlockBufferRef theBuffer) { return theBuffer == NULL || theBuffer->dataLength == 0; }

/* The data of one memory block is contiguous when it is allocated and the range lies inside it. */
Boolean CMBlockBufferIsRangeContiguous(CMBlockBufferRef theBuffer, size_t offset, size_t length)
{
	return theBuffer != NULL && theBuffer->memory != NULL && offset <= theBuffer->dataLength
		&& length <= theBuffer->dataLength - offset;
}

OSStatus CMBlockBufferGetDataPointer(CMBlockBufferRef theBuffer, size_t offset, size_t *lengthAtOffsetOut,
	size_t *totalLengthOut, char **dataPointerOut)
{
	if (theBuffer == NULL)
		return kCMBlockBufferBadPointerParameterErr;
	if (lengthAtOffsetOut != NULL)
		*lengthAtOffsetOut = 0;
	if (totalLengthOut != NULL)
		*totalLengthOut = theBuffer->dataLength;
	if (dataPointerOut != NULL)
		*dataPointerOut = NULL;
	if (offset >= theBuffer->dataLength)
		return theBuffer->dataLength == 0 ? kCMBlockBufferEmptyBBufErr : kCMBlockBufferBadOffsetParameterErr;
	if (theBuffer->memory == NULL)
		return kCMBlockBufferUnallocatedBlockErr;
	if (lengthAtOffsetOut != NULL)
		*lengthAtOffsetOut = theBuffer->dataLength - offset;
	if (dataPointerOut != NULL)
		*dataPointerOut = (char *)theBuffer->memory + theBuffer->offset + offset;
	return kCMBlockBufferNoErr;
}

OSStatus CMBlockBufferCopyDataBytes(CMBlockBufferRef theSourceBuffer, size_t offsetToData, size_t dataLength,
	void *destination)
{
	if (theSourceBuffer == NULL || destination == NULL)
		return kCMBlockBufferBadPointerParameterErr;
	if (offsetToData > theSourceBuffer->dataLength || dataLength > theSourceBuffer->dataLength - offsetToData)
		return kCMBlockBufferBadLengthParameterErr;
	if (dataLength == 0)
		return kCMBlockBufferNoErr;
	if (theSourceBuffer->memory == NULL)
		return kCMBlockBufferUnallocatedBlockErr;
	memcpy(destination, (const char *)theSourceBuffer->memory + theSourceBuffer->offset + offsetToData, dataLength);
	return kCMBlockBufferNoErr;
}

OSStatus CMBlockBufferFillDataBytes(char fillByte, CMBlockBufferRef destinationBuffer, size_t offsetIntoDestination,
	size_t dataLength)
{
	if (destinationBuffer == NULL)
		return kCMBlockBufferBadPointerParameterErr;
	if (offsetIntoDestination > destinationBuffer->dataLength
		|| dataLength > destinationBuffer->dataLength - offsetIntoDestination)
		return kCMBlockBufferBadLengthParameterErr;
	if (dataLength == 0)
		return kCMBlockBufferNoErr;
	OSStatus status = assureMemory(destinationBuffer);
	if (status != kCMBlockBufferNoErr)
		return status;
	memset((char *)destinationBuffer->memory + destinationBuffer->offset + offsetIntoDestination, fillByte, dataLength);
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
	CMSampleBufferMakeDataReadyCallback makeDataReady;
	void *makeDataReadyRefcon;
	CMAttachmentStore bearerAttachments;
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
	CMAttachmentStoreRelease(&sbuf->bearerAttachments);
}

CM_CLASS(CMSampleBuffer_, "CMSampleBuffer", .finalize = sampleBufferFinalize)

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

/* A buffer that is not ready is made ready by the callback when CMSampleBufferMakeDataReady asks for
   it, not at creation. */
static void setDataReady(struct opaqueCMSampleBuffer *sbuf, Boolean dataReady,
	CMSampleBufferMakeDataReadyCallback callback, void *refcon)
{
	sbuf->dataReady = dataReady;
	if (!dataReady) {
		sbuf->makeDataReady = callback;
		sbuf->makeDataReadyRefcon = refcon;
	}
}

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
	setDataReady(sbuf, dataReady, makeDataReadyCallback, makeDataReadyRefcon);
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
	setDataReady(sbuf, dataReady, makeDataReadyCallback, makeDataReadyRefcon);
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

Boolean CMSampleBufferDataIsReady(CMSampleBufferRef sbuf) { return sbuf != NULL && sbuf->dataReady; }

OSStatus CMSampleBufferMakeDataReady(CMSampleBufferRef sbuf)
{
	if (sbuf == NULL)
		return kCMSampleBufferError_RequiredParameterMissing;
	struct opaqueCMSampleBuffer *mutable = (struct opaqueCMSampleBuffer *)sbuf;
	if (mutable->dataReady)
		return noErr;
	if (mutable->makeDataReady == NULL)
		return kCMSampleBufferError_BufferNotReady;
	OSStatus status = mutable->makeDataReady(sbuf, mutable->makeDataReadyRefcon);
	if (status == noErr)
		mutable->dataReady = true;
	return status;
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
	if (sbuf == NULL)
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

static void releaseParentBlock(void *refcon, void *doomedMemoryBlock, size_t sizeInBytes)
{
	CFRelease((CMBlockBufferRef)refcon);
}

/* Each sample in turn, as a sample buffer of its own sharing the data; its block keeps the parent's
   block alive. */
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
		if (sbuf->dataBuffer != NULL && CMBlockBufferGetDataPointer(sbuf->dataBuffer, offset, NULL, NULL, &data) == noErr) {
			CMBlockBufferCustomBlockSource source = { kCMBlockBufferCustomBlockSourceVersion, NULL, releaseParentBlock,
				(void *)CFRetain(sbuf->dataBuffer) };
			status = CMBlockBufferCreateWithMemoryBlock(NULL, data, size, NULL, &source, 0, size, 0, &block);
			if (status != noErr)
				CFRelease(sbuf->dataBuffer);
		}
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

#pragma mark - Attachment bearers

CMAttachmentStore *CMCoreGetAttachmentStore(CFTypeRef object)
{
	CFTypeID type = CFGetTypeID(object);
	if (type == CMSampleBuffer_GetTypeID())
		return &((struct opaqueCMSampleBuffer *)object)->bearerAttachments;
	if (type == CMBlockBuffer_GetTypeID())
		return &((struct OpaqueCMBlockBuffer *)object)->bearerAttachments;
	return NULL;
}

void CMAttachmentStoreRelease(CMAttachmentStore *store)
{
	if (store->propagating != NULL)
		CFRelease(store->propagating);
	if (store->local != NULL)
		CFRelease(store->local);
	store->propagating = store->local = NULL;
}
