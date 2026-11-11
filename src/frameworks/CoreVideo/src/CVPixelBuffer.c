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
#include <pthread.h>
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
	CFMutableDictionaryRef propagatingAttachments;
	CFMutableDictionaryRef localAttachments;
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
	if (buffer->propagatingAttachments) {
		CFRelease(buffer->propagatingAttachments);
		buffer->propagatingAttachments = NULL;
	}
	if (buffer->localAttachments) {
		CFRelease(buffer->localAttachments);
		buffer->localAttachments = NULL;
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
	buffer->propagatingAttachments = NULL;
	buffer->localAttachments = NULL;
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

CVReturn CVPixelBufferLockBaseAddress(CVPixelBufferRef buffer, CVPixelBufferLockFlags flags)
{
	return buffer ? kCVReturnSuccess : kCVReturnInvalidArgument;
}

CVReturn CVPixelBufferUnlockBaseAddress(CVPixelBufferRef buffer, CVPixelBufferLockFlags flags)
{
	return buffer ? kCVReturnSuccess : kCVReturnInvalidArgument;
}

// ── CVBuffer Attachment Management ──

// Attachments are kept in two dictionaries, by propagation mode. One lock covers every
// buffer's attachments; they are touched rarely and briefly.
static pthread_mutex_t attachmentLock = PTHREAD_MUTEX_INITIALIZER;

static CFMutableDictionaryRef attachmentDictionary(CVBufferRef buffer, CVAttachmentMode mode, bool create)
{
	CFMutableDictionaryRef* slot = (mode == kCVAttachmentMode_ShouldPropagate)
		? &buffer->propagatingAttachments : &buffer->localAttachments;
	if (*slot == NULL && create) {
		*slot = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	}
	return *slot;
}

static void setAttachmentLocked(CVBufferRef buffer, CFStringRef key, CFTypeRef value, CVAttachmentMode mode)
{
	CVAttachmentMode other = (mode == kCVAttachmentMode_ShouldPropagate)
		? kCVAttachmentMode_ShouldNotPropagate : kCVAttachmentMode_ShouldPropagate;
	CFMutableDictionaryRef otherDictionary = attachmentDictionary(buffer, other, false);
	if (otherDictionary)
		CFDictionaryRemoveValue(otherDictionary, key);

	CFMutableDictionaryRef dictionary = attachmentDictionary(buffer, mode, value != NULL);
	if (dictionary == NULL)
		return;
	if (value)
		CFDictionarySetValue(dictionary, key, value);
	else
		CFDictionaryRemoveValue(dictionary, key);
}

void CVBufferSetAttachment(CVBufferRef buffer, CFStringRef key, CFTypeRef value, CVAttachmentMode attachmentMode)
{
	if (!buffer || !key)
		return;
	pthread_mutex_lock(&attachmentLock);
	setAttachmentLocked(buffer, key, value, attachmentMode);
	pthread_mutex_unlock(&attachmentLock);
}

static CFTypeRef findAttachmentLocked(CVBufferRef buffer, CFStringRef key, CVAttachmentMode* attachmentMode)
{
	CFTypeRef value = NULL;
	if (buffer->propagatingAttachments && (value = CFDictionaryGetValue(buffer->propagatingAttachments, key))) {
		if (attachmentMode)
			*attachmentMode = kCVAttachmentMode_ShouldPropagate;
	} else if (buffer->localAttachments && (value = CFDictionaryGetValue(buffer->localAttachments, key))) {
		if (attachmentMode)
			*attachmentMode = kCVAttachmentMode_ShouldNotPropagate;
	}
	return value;
}

CFTypeRef CVBufferGetAttachment(CVBufferRef buffer, CFStringRef key, CVAttachmentMode* attachmentMode)
{
	if (!buffer || !key)
		return NULL;
	pthread_mutex_lock(&attachmentLock);
	CFTypeRef value = findAttachmentLocked(buffer, key, attachmentMode);
	pthread_mutex_unlock(&attachmentLock);
	return value;
}

CFTypeRef CVBufferCopyAttachment(CVBufferRef buffer, CFStringRef key, CVAttachmentMode* attachmentMode)
{
	if (!buffer || !key)
		return NULL;
	pthread_mutex_lock(&attachmentLock);
	CFTypeRef value = findAttachmentLocked(buffer, key, attachmentMode);
	if (value)
		CFRetain(value);
	pthread_mutex_unlock(&attachmentLock);
	return value;
}

void CVBufferRemoveAttachment(CVBufferRef buffer, CFStringRef key)
{
	if (!buffer || !key)
		return;
	pthread_mutex_lock(&attachmentLock);
	if (buffer->propagatingAttachments)
		CFDictionaryRemoveValue(buffer->propagatingAttachments, key);
	if (buffer->localAttachments)
		CFDictionaryRemoveValue(buffer->localAttachments, key);
	pthread_mutex_unlock(&attachmentLock);
}

void CVBufferRemoveAllAttachments(CVBufferRef buffer)
{
	if (!buffer)
		return;
	pthread_mutex_lock(&attachmentLock);
	if (buffer->propagatingAttachments)
		CFDictionaryRemoveAllValues(buffer->propagatingAttachments);
	if (buffer->localAttachments)
		CFDictionaryRemoveAllValues(buffer->localAttachments);
	pthread_mutex_unlock(&attachmentLock);
}

CFDictionaryRef CVBufferGetAttachments(CVBufferRef buffer, CVAttachmentMode attachmentMode)
{
	if (!buffer)
		return NULL;
	pthread_mutex_lock(&attachmentLock);
	CFDictionaryRef dictionary = attachmentDictionary(buffer, attachmentMode, false);
	pthread_mutex_unlock(&attachmentLock);
	return dictionary;
}

CFDictionaryRef CVBufferCopyAttachments(CVBufferRef buffer, CVAttachmentMode attachmentMode)
{
	if (!buffer)
		return NULL;
	pthread_mutex_lock(&attachmentLock);
	CFDictionaryRef dictionary = attachmentDictionary(buffer, attachmentMode, false);
	CFDictionaryRef copy;
	if (dictionary)
		copy = CFDictionaryCreateCopy(kCFAllocatorDefault, dictionary);
	else
		copy = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	pthread_mutex_unlock(&attachmentLock);
	return copy;
}

static void setAttachmentFromDictionary(const void* key, const void* value, void* context)
{
	void** args = (void**) context;
	setAttachmentLocked((CVBufferRef) args[0], (CFStringRef) key, (CFTypeRef) value, *(CVAttachmentMode*) args[1]);
}

void CVBufferSetAttachments(CVBufferRef buffer, CFDictionaryRef theAttachments, CVAttachmentMode attachmentMode)
{
	if (!buffer || !theAttachments)
		return;
	void* args[2] = { buffer, &attachmentMode };
	pthread_mutex_lock(&attachmentLock);
	CFDictionaryApplyFunction(theAttachments, setAttachmentFromDictionary, args);
	pthread_mutex_unlock(&attachmentLock);
}

void CVBufferPropagateAttachments(CVBufferRef sourceBuffer, CVBufferRef destinationBuffer)
{
	if (!sourceBuffer || !destinationBuffer || sourceBuffer == destinationBuffer)
		return;
	CFDictionaryRef propagating = CVBufferCopyAttachments(sourceBuffer, kCVAttachmentMode_ShouldPropagate);
	CVBufferSetAttachments(destinationBuffer, propagating, kCVAttachmentMode_ShouldPropagate);
	CFRelease(propagating);
}

// ── Color space ──

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

// ── Code point lookups ──
//
// The integer code points are those of ITU-T H.273. The string for a code point that has
// no named constant is synthesised, see synthesizedString().

// QUIRK: a code point with no named constant (other than 0 and 2) maps to a string made
// of a kind prefix and the number, such as "ColorPrimaries#3", rather than to NULL.
// Undocumented. Evidence: black-box observation on macOS 27 (tier 2), probe
// vd-upstream/mac-probes/cvcodepoints.c. Test: tests/src/corevideo_code_points.c
// (quirk_unnamed_code_point_gets_synthesized_string).
// The strings live for the life of the process, as the lookup functions follow the Get rule.
static CFStringRef synthesizedString(const char* prefix, int32_t codePoint)
{
	static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	static CFMutableDictionaryRef cache;
	CFStringRef result = NULL;

	pthread_mutex_lock(&lock);
	if (cache == NULL)
		cache = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks);
	CFStringRef name = cache ? CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s#%d"), prefix, (int) codePoint) : NULL;
	if (name) {
		result = CFDictionaryGetValue(cache, name);
		if (result == NULL) {
			CFDictionarySetValue(cache, name, name);
			result = name;
		}
		CFRelease(name);
	}
	pthread_mutex_unlock(&lock);
	return result;
}

CFStringRef CVColorPrimariesGetStringForIntegerCodePoint(int32_t codePoint)
{
	switch (codePoint) {
		case 0:
		case 2:
			return NULL;
		case 1:
			return kCVImageBufferColorPrimaries_ITU_R_709_2;
		case 5:
			return kCVImageBufferColorPrimaries_EBU_3213;
		case 6:
			return kCVImageBufferColorPrimaries_SMPTE_C;
		case 9:
			return kCVImageBufferColorPrimaries_ITU_R_2020;
		case 11:
			return kCVImageBufferColorPrimaries_DCI_P3;
		case 12:
			return kCVImageBufferColorPrimaries_P3_D65;
		case 22:
			return kCVImageBufferColorPrimaries_P22;
		default:
			return synthesizedString("ColorPrimaries", codePoint);
	}
}

// QUIRK: a missing or unrecognised string maps to code point 2 (unspecified).
// Undocumented. Evidence: black-box observation on macOS 27 (tier 2), probe
// vd-upstream/mac-probes/cvcodepoints.c. Test: tests/src/corevideo_code_points.c.
int32_t CVColorPrimariesGetIntegerCodePointForString(CFStringRef colorPrimariesString)
{
	if (!colorPrimariesString) return 2;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_ITU_R_709_2)) return 1;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_EBU_3213)) return 5;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_SMPTE_C)) return 6;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_ITU_R_2020)) return 9;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_DCI_P3)) return 11;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_P3_D65)) return 12;
	if (CFEqual(colorPrimariesString, kCVImageBufferColorPrimaries_P22)) return 22;
	return 2;
}

CFStringRef CVTransferFunctionGetStringForIntegerCodePoint(int32_t codePoint)
{
	switch (codePoint) {
		case 0:
		case 2:
			return NULL;
		case 1:
		case 6:
		case 14:
		case 15:
			return kCVImageBufferTransferFunction_ITU_R_709_2;
		case 7:
			return kCVImageBufferTransferFunction_SMPTE_240M_1995;
		case 8:
			return kCVImageBufferTransferFunction_Linear;
		case 13:
			return kCVImageBufferTransferFunction_sRGB;
		case 16:
			return kCVImageBufferTransferFunction_SMPTE_ST_2084_PQ;
		case 17:
			return kCVImageBufferTransferFunction_SMPTE_ST_428_1;
		case 18:
			return kCVImageBufferTransferFunction_ITU_R_2100_HLG;
		default:
			return synthesizedString("TransferFunction", codePoint);
	}
}

// QUIRK: the ITU_R_2020 transfer function string maps back to code point 1, the same as
// ITU_R_709_2, and an unrecognised string maps to 2. Undocumented. Evidence: black-box
// observation on macOS 27 (tier 2), probe vd-upstream/mac-probes/cvcodepoints.c.
// Test: tests/src/corevideo_code_points.c.
int32_t CVTransferFunctionGetIntegerCodePointForString(CFStringRef transferFunctionString)
{
	if (!transferFunctionString) return 2;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_ITU_R_709_2)) return 1;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_ITU_R_2020)) return 1;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_SMPTE_240M_1995)) return 7;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_Linear)) return 8;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_sRGB)) return 13;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_SMPTE_ST_2084_PQ)) return 16;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_SMPTE_ST_428_1)) return 17;
	if (CFEqual(transferFunctionString, kCVImageBufferTransferFunction_ITU_R_2100_HLG)) return 18;
	return 2;
}

CFStringRef CVYCbCrMatrixGetStringForIntegerCodePoint(int32_t codePoint)
{
	switch (codePoint) {
		case 0:
		case 2:
			return NULL;
		case 1:
			return kCVImageBufferYCbCrMatrix_ITU_R_709_2;
		case 6:
			return kCVImageBufferYCbCrMatrix_ITU_R_601_4;
		case 7:
			return kCVImageBufferYCbCrMatrix_SMPTE_240M_1995;
		case 9:
			return kCVImageBufferYCbCrMatrix_ITU_R_2020;
		case 14:
			return CFSTR("ITU_R_2100_ICtCp");
		case 15:
			return CFSTR("IPT_C2");
		default:
			return synthesizedString("YCbCrMatrix", codePoint);
	}
}

int32_t CVYCbCrMatrixGetIntegerCodePointForString(CFStringRef yCbCrMatrixString)
{
	if (!yCbCrMatrixString) return 2;
	if (CFEqual(yCbCrMatrixString, kCVImageBufferYCbCrMatrix_ITU_R_709_2)) return 1;
	if (CFEqual(yCbCrMatrixString, kCVImageBufferYCbCrMatrix_ITU_R_601_4)) return 6;
	if (CFEqual(yCbCrMatrixString, kCVImageBufferYCbCrMatrix_SMPTE_240M_1995)) return 7;
	if (CFEqual(yCbCrMatrixString, kCVImageBufferYCbCrMatrix_ITU_R_2020)) return 9;
	if (CFEqual(yCbCrMatrixString, CFSTR("ITU_R_2100_ICtCp"))) return 14;
	if (CFEqual(yCbCrMatrixString, CFSTR("IPT_C2"))) return 15;
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
	// Buffers are allocated on demand and are not recycled; auxAttributes is not consulted.
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
