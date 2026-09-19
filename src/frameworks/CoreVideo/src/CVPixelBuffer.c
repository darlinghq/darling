/*
 This file is part of Darling.

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

// Minimal CPU-memory CVPixelBuffer: a single-plane, packed buffer with 16-byte row alignment.
// Pixel buffers are CF objects, so CFRetain, CFRelease and CFGetTypeID work on them.

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreVideo/CoreVideo.h>
#include <CoreGraphics/CGColorSpace.h>
#include <dispatch/dispatch.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct __CVBuffer {
	CFRuntimeBase runtimeBase;
	size_t width;
	size_t height;
	size_t bytesPerRow;
	OSType pixelFormat;
	uint8_t* base;
	Boolean ownsBase;
	CVPixelBufferReleaseBytesCallback releaseCallback;
	void* releaseRefCon;
	CFMutableDictionaryRef attachments;
};

typedef struct __CVBuffer __CVPixelBuffer;

static void pixelBufferFinalize(CFTypeRef cf)
{
	CVPixelBufferRef buffer = (CVPixelBufferRef) cf;
	if (buffer->ownsBase)
		free(buffer->base);
	else if (buffer->releaseCallback)
		buffer->releaseCallback(buffer->releaseRefCon, buffer->base);
	buffer->base = NULL;
	if (buffer->attachments) {
		CFRelease(buffer->attachments);
		buffer->attachments = NULL;
	}
}

static const CFRuntimeClass pixelBufferClass = {
	.version = 0,
	.className = "CVPixelBuffer",
	.finalize = pixelBufferFinalize,
};

static CFTypeID pixelBufferTypeID = _kCFRuntimeNotATypeID;

CFTypeID CVPixelBufferGetTypeID(void)
{
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		pixelBufferTypeID = _CFRuntimeRegisterClass(&pixelBufferClass);
	});
	return pixelBufferTypeID;
}

static CVPixelBufferRef pixelBufferAllocate(CFAllocatorRef allocator, size_t width, size_t height, size_t bytesPerRow,
	OSType pixelFormatType)
{
	CVPixelBufferRef buffer = (CVPixelBufferRef) _CFRuntimeCreateInstance(allocator, CVPixelBufferGetTypeID(),
		sizeof(struct __CVBuffer) - sizeof(CFRuntimeBase), NULL);
	if (buffer == NULL)
		return NULL;
	buffer->width = width;
	buffer->height = height;
	buffer->bytesPerRow = bytesPerRow;
	buffer->pixelFormat = pixelFormatType;
	buffer->base = NULL;
	buffer->ownsBase = false;
	buffer->releaseCallback = NULL;
	buffer->releaseRefCon = NULL;
	buffer->attachments = NULL;
	return buffer;
}

// Bytes per pixel for packed formats commonly used for CPU drawing; 0 means unsupported.
static size_t bytesPerPixelForFormat(OSType format)
{
	switch (format) {
		case 0x00000020: // kCVPixelFormatType_32ARGB
		case 'BGRA':     // kCVPixelFormatType_32BGRA
		case 'RGBA':     // kCVPixelFormatType_32RGBA
		case 'ABGR':     // kCVPixelFormatType_32ABGR
			return 4;
		case 0x00000018: // kCVPixelFormatType_24RGB
		case '24BG':     // kCVPixelFormatType_24BGR
			return 3;
		case 'L008':     // kCVPixelFormatType_OneComponent8
		case 0x00000008: // kCVPixelFormatType_8Indexed
			return 1;
		case 'RGhA':     // kCVPixelFormatType_64RGBAHalf
			return 8;
		default:
			return 0;
	}
}

CVReturn CVPixelBufferCreate(CFAllocatorRef allocator, size_t width, size_t height, OSType pixelFormatType,
	CFDictionaryRef pixelBufferAttributes, CVPixelBufferRef* pixelBufferOut)
{
	if (pixelBufferOut == NULL || width == 0 || height == 0)
		return kCVReturnInvalidArgument;
	*pixelBufferOut = NULL;

	size_t bpp = bytesPerPixelForFormat(pixelFormatType);
	if (bpp == 0)
		return kCVReturnInvalidPixelFormat;
	if (width > (SIZE_MAX - 15) / bpp)
		return kCVReturnAllocationFailed;

	size_t bytesPerRow = (width * bpp + 15) & ~(size_t)15;
	if (height > SIZE_MAX / bytesPerRow)
		return kCVReturnAllocationFailed;

	CVPixelBufferRef buffer = pixelBufferAllocate(allocator, width, height, bytesPerRow, pixelFormatType);
	if (buffer == NULL)
		return kCVReturnAllocationFailed;
	buffer->base = calloc(height, bytesPerRow);
	if (buffer->base == NULL) {
		CFRelease(buffer);
		return kCVReturnAllocationFailed;
	}
	buffer->ownsBase = true;

	*pixelBufferOut = buffer;
	return kCVReturnSuccess;
}

CVReturn CVPixelBufferCreateWithBytes(CFAllocatorRef allocator, size_t width, size_t height, OSType pixelFormatType,
	void* baseAddress, size_t bytesPerRow, CVPixelBufferReleaseBytesCallback releaseCallback, void* releaseRefCon,
	CFDictionaryRef pixelBufferAttributes, CVPixelBufferRef* pixelBufferOut)
{
	if (pixelBufferOut == NULL || baseAddress == NULL || width == 0 || height == 0 || bytesPerRow == 0)
		return kCVReturnInvalidArgument;
	*pixelBufferOut = NULL;
	if (height > SIZE_MAX / bytesPerRow)
		return kCVReturnInvalidArgument;

	size_t bpp = bytesPerPixelForFormat(pixelFormatType);
	if (bpp == 0)
		return kCVReturnInvalidPixelFormat;
	if (width > bytesPerRow / bpp)
		return kCVReturnInvalidArgument;

	CVPixelBufferRef buffer = pixelBufferAllocate(allocator, width, height, bytesPerRow, pixelFormatType);
	if (buffer == NULL)
		return kCVReturnAllocationFailed;
	buffer->base = baseAddress;
	buffer->releaseCallback = releaseCallback;
	buffer->releaseRefCon = releaseRefCon;

	*pixelBufferOut = buffer;
	return kCVReturnSuccess;
}

CVReturn CVPixelBufferCreateWithPlanarBytes(CFAllocatorRef allocator, size_t width, size_t height,
	OSType pixelFormatType, void* dataPtr, size_t dataSize, size_t numberOfPlanes, void* planeBaseAddress[],
	size_t planeWidth[], size_t planeHeight[], size_t planeBytesPerRow[],
	CVPixelBufferReleasePlanarBytesCallback releaseCallback, void* releaseRefCon,
	CFDictionaryRef pixelBufferAttributes, CVPixelBufferRef* pixelBufferOut)
{
	if (pixelBufferOut != NULL)
		*pixelBufferOut = NULL;
	return kCVReturnInvalidPixelFormat;
}

CVPixelBufferRef CVPixelBufferRetain(CVPixelBufferRef buffer)
{
	if (buffer)
		CFRetain(buffer);
	return buffer;
}

void CVPixelBufferRelease(CVPixelBufferRef buffer)
{
	if (buffer)
		CFRelease(buffer);
}

CVBufferRef CVBufferRetain(CVBufferRef buffer)
{
	if (buffer)
		CFRetain(buffer);
	return buffer;
}

void CVBufferRelease(CVBufferRef buffer)
{
	if (buffer)
		CFRelease(buffer);
}

size_t CVPixelBufferGetWidth(CVPixelBufferRef buffer)
{
	return buffer ? buffer->width : 0;
}

size_t CVPixelBufferGetHeight(CVPixelBufferRef buffer)
{
	return buffer ? buffer->height : 0;
}

size_t CVPixelBufferGetBytesPerRow(CVPixelBufferRef buffer)
{
	return buffer ? buffer->bytesPerRow : 0;
}

OSType CVPixelBufferGetPixelFormatType(CVPixelBufferRef buffer)
{
	return buffer ? buffer->pixelFormat : 0;
}

size_t CVPixelBufferGetDataSize(CVPixelBufferRef buffer)
{
	return buffer ? buffer->bytesPerRow * buffer->height : 0;
}

void* CVPixelBufferGetBaseAddress(CVPixelBufferRef buffer)
{
	return buffer ? buffer->base : NULL;
}

size_t CVPixelBufferGetPlaneCount(CVPixelBufferRef buffer)
{
	return 0;
}

Boolean CVPixelBufferIsPlanar(CVPixelBufferRef buffer)
{
	return false;
}

void* CVPixelBufferGetBaseAddressOfPlane(CVPixelBufferRef buffer, size_t planeIndex)
{
	if (!buffer || planeIndex > 0)
		return NULL;
	return buffer->base;
}

size_t CVPixelBufferGetBytesPerRowOfPlane(CVPixelBufferRef buffer, size_t planeIndex)
{
	if (!buffer || planeIndex > 0)
		return 0;
	return buffer->bytesPerRow;
}

size_t CVPixelBufferGetWidthOfPlane(CVPixelBufferRef buffer, size_t planeIndex)
{
	if (!buffer || planeIndex > 0)
		return 0;
	return buffer->width;
}

size_t CVPixelBufferGetHeightOfPlane(CVPixelBufferRef buffer, size_t planeIndex)
{
	if (!buffer || planeIndex > 0)
		return 0;
	return buffer->height;
}

CGSize CVImageBufferGetEncodedSize(CVImageBufferRef imageBuffer)
{
	if (!imageBuffer)
		return (CGSize){0, 0};
	return (CGSize){(CGFloat)imageBuffer->width, (CGFloat)imageBuffer->height};
}

CVReturn CVMetalTextureCacheCreate(
	CFAllocatorRef allocator,
	CFDictionaryRef cacheAttributes,
	void *metalDevice,
	CFDictionaryRef textureAttributes,
	void *cacheOut)
{
	if (cacheOut) *(void**)cacheOut = NULL;
	return kCVReturnAllocationFailed;
}

CVReturn CVMetalTextureCacheCreateTextureFromImage(
	CFAllocatorRef allocator,
	void *textureCache,
	CVImageBufferRef sourceImage,
	CFDictionaryRef textureAttributes,
	int pixelFormat,
	size_t width,
	size_t height,
	size_t planeIndex,
	void *textureOut)
{
	if (textureOut) *(void**)textureOut = NULL;
	return kCVReturnAllocationFailed;
}

void* CVMetalTextureGetTexture(void *image)
{
	return NULL;
}

CVReturn CVPixelBufferLockBaseAddress(CVPixelBufferRef buffer, CVPixelBufferLockFlags flags)
{
	return buffer ? kCVReturnSuccess : kCVReturnInvalidArgument;
}

CVReturn CVPixelBufferUnlockBaseAddress(CVPixelBufferRef buffer, CVPixelBufferLockFlags flags)
{
	return buffer ? kCVReturnSuccess : kCVReturnInvalidArgument;
}

// ── CVBuffer Attachment Management ──

void CVBufferSetAttachment(CVBufferRef buffer, CFStringRef key, CFTypeRef value, CVAttachmentMode attachmentMode)
{
	if (!buffer || !key)
		return;
	if (!buffer->attachments) {
		buffer->attachments = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (!buffer->attachments)
			return;
	}
	if (value)
		CFDictionarySetValue(buffer->attachments, key, value);
	else
		CFDictionaryRemoveValue(buffer->attachments, key);
}

CFTypeRef CVBufferGetAttachment(CVBufferRef buffer, CFStringRef key, CVAttachmentMode *attachmentMode)
{
	if (!buffer || !buffer->attachments || !key)
		return NULL;
	if (attachmentMode)
		*attachmentMode = kCVAttachmentMode_ShouldPropagate;
	return CFDictionaryGetValue(buffer->attachments, key);
}

CFTypeRef CVBufferCopyAttachment(CVBufferRef buffer, CFStringRef key, CVAttachmentMode *attachmentMode)
{
	CFTypeRef val = CVBufferGetAttachment(buffer, key, attachmentMode);
	if (val)
		CFRetain(val);
	return val;
}

void CVBufferRemoveAttachment(CVBufferRef buffer, CFStringRef key)
{
	if (!buffer || !buffer->attachments || !key)
		return;
	CFDictionaryRemoveValue(buffer->attachments, key);
}

void CVBufferRemoveAllAttachments(CVBufferRef buffer)
{
	if (!buffer || !buffer->attachments)
		return;
	CFDictionaryRemoveAllValues(buffer->attachments);
}

CFDictionaryRef CVBufferGetAttachments(CVBufferRef buffer, CVAttachmentMode attachmentMode)
{
	if (!buffer)
		return NULL;
	return buffer->attachments;
}

CFDictionaryRef CVBufferCopyAttachments(CVBufferRef buffer, CVAttachmentMode attachmentMode)
{
	if (!buffer)
		return NULL;
	if (!buffer->attachments)
		return CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	return CFDictionaryCreateCopy(kCFAllocatorDefault, buffer->attachments);
}

void CVBufferSetAttachments(CVBufferRef buffer, CFDictionaryRef theAttachments, CVAttachmentMode attachmentMode)
{
	if (!buffer || !theAttachments)
		return;
	if (!buffer->attachments) {
		buffer->attachments = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, theAttachments);
		return;
	}
	CFIndex count = CFDictionaryGetCount(theAttachments);
	if (count > 0) {
		const void** keys = (const void**) malloc(sizeof(void*) * count);
		const void** values = (const void**) malloc(sizeof(void*) * count);
		if (keys && values) {
			CFDictionaryGetKeysAndValues(theAttachments, keys, values);
			for (CFIndex i = 0; i < count; i++) {
				CFDictionarySetValue(buffer->attachments, keys[i], values[i]);
			}
		}
		free((void*)keys);
		free((void*)values);
	}
}

void CVBufferPropagateAttachments(CVBufferRef sourceBuffer, CVBufferRef destinationBuffer)
{
	if (sourceBuffer && destinationBuffer && sourceBuffer->attachments) {
		CVBufferSetAttachments(destinationBuffer, sourceBuffer->attachments, kCVAttachmentMode_ShouldPropagate);
	}
}

// ── Color Space and Code Point Utilities ──

CGColorSpaceRef CVImageBufferCreateColorSpaceFromAttachments(CFDictionaryRef attachments)
{
	if (!attachments)
		return NULL;

	CGColorSpaceRef cs = (CGColorSpaceRef) CFDictionaryGetValue(attachments, kCVImageBufferCGColorSpaceKey);
	if (cs) {
		CGColorSpaceRetain(cs);
		return cs;
	}

	CFStringRef primaries = (CFStringRef) CFDictionaryGetValue(attachments, kCVImageBufferColorPrimariesKey);
	if (primaries) {
		if (CFEqual(primaries, kCVImageBufferColorPrimaries_ITU_R_709_2))
			return CGColorSpaceCreateWithName(kCGColorSpaceITUR_709);
		if (CFEqual(primaries, kCVImageBufferColorPrimaries_ITU_R_2020))
			return CGColorSpaceCreateWithName(kCGColorSpaceITUR_2020);
	}

	return CGColorSpaceCreateDeviceRGB();
}

CFStringRef CVColorPrimariesGetStringForIntegerCodePoint(int32_t codePoint)
{
	switch (codePoint) {
		case 1:
			return kCVImageBufferColorPrimaries_ITU_R_709_2;
		case 4:
			return kCVImageBufferColorPrimaries_EBU_3213;
		case 5:
		case 6:
			return kCVImageBufferColorPrimaries_SMPTE_C;
		case 9:
			return kCVImageBufferColorPrimaries_ITU_R_2020;
		default:
			return NULL;
	}
}

int32_t CVColorPrimariesGetIntegerCodePointForString(CFStringRef colorPrimariesString)
{
	if (!colorPrimariesString) return 2;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_ITU_R_709_2)) return 1;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_EBU_3213)) return 4;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_SMPTE_C)) return 6;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_ITU_R_2020)) return 9;
	return 2;
}

CFStringRef CVTransferFunctionGetStringForIntegerCodePoint(int32_t codePoint)
{
	switch (codePoint) {
		case 1:
			return kCVImageBufferTransferFunction_ITU_R_709_2;
		case 6:
		case 7:
			return kCVImageBufferTransferFunction_SMPTE_240M_1995;
		case 8:
			return kCVImageBufferTransferFunction_Linear;
		case 14:
			return kCVImageBufferTransferFunction_ITU_R_2020;
		case 16:
			return kCVImageBufferTransferFunction_SMPTE_ST_2084_PQ;
		case 17:
			return kCVImageBufferTransferFunction_SMPTE_ST_428_1;
		case 18:
			return kCVImageBufferTransferFunction_ITU_R_2100_HLG;
		default:
			return NULL;
	}
}

int32_t CVTransferFunctionGetIntegerCodePointForString(CFStringRef transferFunctionString)
{
	if (!transferFunctionString) return 2;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_ITU_R_709_2)) return 1;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_SMPTE_240M_1995)) return 6;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_Linear)) return 8;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_ITU_R_2020)) return 14;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_SMPTE_ST_2084_PQ)) return 16;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_SMPTE_ST_428_1)) return 17;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_ITU_R_2100_HLG)) return 18;
	return 2;
}

CFStringRef CVYCbCrMatrixGetStringForIntegerCodePoint(int32_t codePoint)
{
	switch (codePoint) {
		case 1:
			return kCVImageBufferYCbCrMatrix_ITU_R_709_2;
		case 5:
		case 6:
			return kCVImageBufferYCbCrMatrix_ITU_R_601_4;
		case 7:
			return kCVImageBufferYCbCrMatrix_SMPTE_240M_1995;
		case 9:
			return kCVImageBufferYCbCrMatrix_ITU_R_2020;
		default:
			return NULL;
	}
}

int32_t CVYCbCrMatrixGetIntegerCodePointForString(CFStringRef yCbCrMatrixString)
{
	if (!yCbCrMatrixString) return 2;
	if (CFEqual(yCbCrMatrixString, kCVImageBufferYCbCrMatrix_ITU_R_709_2)) return 1;
	if (CFEqual(yCbCrMatrixString, kCVImageBufferYCbCrMatrix_ITU_R_601_4)) return 6;
	if (CFEqual(yCbCrMatrixString, kCVImageBufferYCbCrMatrix_SMPTE_240M_1995)) return 7;
	if (CFEqual(yCbCrMatrixString, kCVImageBufferYCbCrMatrix_ITU_R_2020)) return 9;
	return 2;
}

// ── CVPixelBufferPool ──

struct __CVPixelBufferPool {
	CFRuntimeBase runtimeBase;
	CFDictionaryRef poolAttributes;
	CFDictionaryRef pixelBufferAttributes;
};

static void pixelBufferPoolFinalize(CFTypeRef cf)
{
	CVPixelBufferPoolRef pool = (CVPixelBufferPoolRef) cf;
	if (pool->poolAttributes)
		CFRelease(pool->poolAttributes);
	if (pool->pixelBufferAttributes)
		CFRelease(pool->pixelBufferAttributes);
	pool->poolAttributes = NULL;
	pool->pixelBufferAttributes = NULL;
}

static const CFRuntimeClass pixelBufferPoolClass = {
	.version = 0,
	.className = "CVPixelBufferPool",
	.finalize = pixelBufferPoolFinalize,
};

static CFTypeID pixelBufferPoolTypeID = _kCFRuntimeNotATypeID;

CFTypeID CVPixelBufferPoolGetTypeID(void)
{
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		pixelBufferPoolTypeID = _CFRuntimeRegisterClass(&pixelBufferPoolClass);
	});
	return pixelBufferPoolTypeID;
}

CVPixelBufferPoolRef CVPixelBufferPoolRetain(CVPixelBufferPoolRef pixelBufferPool)
{
	if (pixelBufferPool)
		CFRetain(pixelBufferPool);
	return pixelBufferPool;
}

void CVPixelBufferPoolRelease(CVPixelBufferPoolRef pixelBufferPool)
{
	if (pixelBufferPool)
		CFRelease(pixelBufferPool);
}

CVReturn CVPixelBufferPoolCreate(
	CFAllocatorRef allocator,
	CFDictionaryRef poolAttributes,
	CFDictionaryRef pixelBufferAttributes,
	CVPixelBufferPoolRef *poolOut)
{
	if (!poolOut) return kCVReturnInvalidArgument;
	*poolOut = NULL;

	CVPixelBufferPoolRef pool = (CVPixelBufferPoolRef) _CFRuntimeCreateInstance(allocator, CVPixelBufferPoolGetTypeID(),
		sizeof(struct __CVPixelBufferPool) - sizeof(CFRuntimeBase), NULL);
	if (!pool) return kCVReturnAllocationFailed;

	pool->poolAttributes = poolAttributes ? CFDictionaryCreateCopy(allocator, poolAttributes) : NULL;
	pool->pixelBufferAttributes = pixelBufferAttributes ? CFDictionaryCreateCopy(allocator, pixelBufferAttributes) : NULL;

	*poolOut = pool;
	return kCVReturnSuccess;
}

CVReturn CVPixelBufferPoolCreatePixelBuffer(
	CFAllocatorRef allocator,
	CVPixelBufferPoolRef pixelBufferPool,
	CVPixelBufferRef *pixelBufferOut)
{
	return CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(allocator, pixelBufferPool, NULL, pixelBufferOut);
}

CVReturn CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(
	CFAllocatorRef allocator,
	CVPixelBufferPoolRef pixelBufferPool,
	CFDictionaryRef auxAttributes,
	CVPixelBufferRef *pixelBufferOut)
{
	// TODO: respect kCVPixelBufferPoolAllocationThresholdKey in auxAttributes
	if (!pixelBufferPool || !pixelBufferOut) return kCVReturnInvalidArgument;
	*pixelBufferOut = NULL;

	size_t width = 0, height = 0;
	OSType format = 0;

	if (pixelBufferPool->pixelBufferAttributes) {
		CFNumberRef numWidth = (CFNumberRef) CFDictionaryGetValue(pixelBufferPool->pixelBufferAttributes, kCVPixelBufferWidthKey);
		if (numWidth) CFNumberGetValue(numWidth, kCFNumberSInt64Type, &width);
		CFNumberRef numHeight = (CFNumberRef) CFDictionaryGetValue(pixelBufferPool->pixelBufferAttributes, kCVPixelBufferHeightKey);
		if (numHeight) CFNumberGetValue(numHeight, kCFNumberSInt64Type, &height);
		CFNumberRef numFormat = (CFNumberRef) CFDictionaryGetValue(pixelBufferPool->pixelBufferAttributes, kCVPixelBufferPixelFormatTypeKey);
		if (numFormat) CFNumberGetValue(numFormat, kCFNumberSInt32Type, &format);
	}

	if (width == 0 || height == 0 || format == 0)
		return kCVReturnInvalidPixelBufferAttributes;

	return CVPixelBufferCreate(allocator, width, height, format, pixelBufferPool->pixelBufferAttributes, pixelBufferOut);
}
