/*
 This file is part of Darling.

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

// CVPixelBuffer: packed and planar pixel buffers, in memory or in an IOSurface.
// A buffer is backed by an IOSurface when its attributes contain
// kCVPixelBufferIOSurfacePropertiesKey (also with an empty dictionary) or when it is created
// with CVPixelBufferCreateWithIOSurface; the surface then holds the pixels, which are shared
// with other processes and with OpenGL through the surface. All other buffers are in memory.
// Rows start on 64-byte boundaries (or the kCVPixelBufferBytesPerRowAlignmentKey value), the
// base address is only available while the buffer is locked, and the lock of an
// IOSurface-backed buffer locks its surface.
// Pixel buffers are CF objects, so CFRetain, CFRelease and CFGetTypeID work on them.

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreVideo/CoreVideo.h>
#include <CoreGraphics/CGColorSpace.h>
#include <IOSurface/IOSurfaceRef.h>
#include <mach/kern_return.h>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "CVInternal.h"

static void poolBufferFinalized(CVPixelBufferPoolRef pool);

static void pixelBufferFinalize(CFTypeRef cf)
{
	CVPixelBufferRef buffer = (CVPixelBufferRef) cf;
	if (buffer->surface) {
		if (buffer->lockCount > 0) {
			// Released while still locked: give the surface's lock back.
			IOSurfaceUnlock(buffer->surface, (buffer->lockFlags & kCVPixelBufferLock_ReadOnly) ? kIOSurfaceLockReadOnly : 0, NULL);
		}
		CFRelease(buffer->surface);
		buffer->surface = NULL;
	} else if (buffer->ownsMemory) {
		free(buffer->memory);
	} else if (buffer->releaseCallback) {
		buffer->releaseCallback(buffer->releaseRefCon, buffer->memory);
	}
	buffer->memory = NULL;
	pthread_mutex_destroy(&buffer->lockMutex);
	if (buffer->propagatingAttachments) {
		CFRelease(buffer->propagatingAttachments);
		buffer->propagatingAttachments = NULL;
	}
	if (buffer->localAttachments) {
		CFRelease(buffer->localAttachments);
		buffer->localAttachments = NULL;
	}
	if (buffer->pool) {
		CVPixelBufferPoolRef pool = buffer->pool;
		buffer->pool = NULL;
		poolBufferFinalized(pool);
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

static CVPixelBufferRef pixelBufferAllocate(CFAllocatorRef allocator, size_t width, size_t height, OSType pixelFormatType)
{
	CVPixelBufferRef buffer = (CVPixelBufferRef) _CFRuntimeCreateInstance(allocator, CVPixelBufferGetTypeID(),
		sizeof(struct __CVBuffer) - sizeof(CFRuntimeBase), NULL);
	if (buffer == NULL)
		return NULL;
	memset((char*) buffer + sizeof(CFRuntimeBase), 0, sizeof(struct __CVBuffer) - sizeof(CFRuntimeBase));
	buffer->width = width;
	buffer->height = height;
	buffer->pixelFormat = pixelFormatType;
	pthread_mutex_init(&buffer->lockMutex, NULL);
	return buffer;
}

// The layout of the pixel formats that can be created: planes, and for each the bytes per
// element (a pixel, or for a chroma plane a Cb/Cr pair) and the subsampling.
struct __CVFormat {
	OSType type;
	size_t planeCount; // 0 for a single plane that does not make the buffer planar
	size_t bytesPerElement[kMaxPlanes];
	size_t horizontalSubsampling[kMaxPlanes];
	size_t verticalSubsampling[kMaxPlanes];
};

static const struct __CVFormat formats[] = {
	{ 0x00000020, 0, { 4 }, { 1 }, { 1 } }, // kCVPixelFormatType_32ARGB
	{ 'BGRA', 0, { 4 }, { 1 }, { 1 } },
	{ 'RGBA', 0, { 4 }, { 1 }, { 1 } },
	{ 'ABGR', 0, { 4 }, { 1 }, { 1 } },
	{ 0x00000018, 0, { 3 }, { 1 }, { 1 } }, // kCVPixelFormatType_24RGB
	{ '24BG', 0, { 3 }, { 1 }, { 1 } },
	{ 'L008', 0, { 1 }, { 1 }, { 1 } },
	{ 0x00000008, 0, { 1 }, { 1 }, { 1 } }, // kCVPixelFormatType_8Indexed
	{ 'RGhA', 0, { 8 }, { 1 }, { 1 } },
	{ '2vuy', 0, { 2 }, { 1 }, { 1 } },
	{ 'yuvs', 0, { 2 }, { 1 }, { 1 } },
	{ '420v', 2, { 1, 2 }, { 1, 2 }, { 1, 2 } },
	{ '420f', 2, { 1, 2 }, { 1, 2 }, { 1, 2 } },
	{ 'y420', 3, { 1, 1, 1 }, { 1, 2, 2 }, { 1, 2, 2 } },
};

static const struct __CVFormat* formatForType(OSType type)
{
	for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
		if (formats[i].type == type)
			return &formats[i];
	}
	return NULL;
}

static size_t alignUp(size_t value, size_t alignment)
{
	return (value + alignment - 1) / alignment * alignment;
}

static Boolean numberFromAttributes(CFDictionaryRef attributes, CFStringRef key, size_t* value)
{
	CFTypeRef number = attributes ? CFDictionaryGetValue(attributes, key) : NULL;
	int64_t read;
	if (number == NULL || CFGetTypeID(number) != CFNumberGetTypeID() || !CFNumberGetValue(number, kCFNumberSInt64Type, &read) || read <= 0)
		return false;
	*value = (size_t) read;
	return true;
}

// The properties an IOSurface needs for the buffer: its size, format and the layout of its
// planes, plus whatever the caller put under kCVPixelBufferIOSurfacePropertiesKey.
static CFDictionaryRef createSurfaceProperties(CFDictionaryRef userProperties, const struct __CVBuffer* buffer,
	const struct __CVFormat* format)
{
	CFMutableDictionaryRef properties = userProperties
		? CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, userProperties)
		: CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (properties == NULL)
		return NULL;

	int64_t width = (int64_t) buffer->width, height = (int64_t) buffer->height, pixelFormat = buffer->pixelFormat;
	CFNumberRef widthNumber = CFNumberCreate(NULL, kCFNumberSInt64Type, &width);
	CFNumberRef heightNumber = CFNumberCreate(NULL, kCFNumberSInt64Type, &height);
	CFNumberRef formatNumber = CFNumberCreate(NULL, kCFNumberSInt64Type, &pixelFormat);
	CFDictionarySetValue(properties, kIOSurfaceWidth, widthNumber);
	CFDictionarySetValue(properties, kIOSurfaceHeight, heightNumber);
	CFDictionarySetValue(properties, kIOSurfacePixelFormat, formatNumber);
	CFRelease(widthNumber);
	CFRelease(heightNumber);
	CFRelease(formatNumber);

	if (format->planeCount == 0) {
		int64_t bytesPerElement = (int64_t) format->bytesPerElement[0], bytesPerRow = (int64_t) buffer->planes[0].bytesPerRow;
		CFNumberRef elementNumber = CFNumberCreate(NULL, kCFNumberSInt64Type, &bytesPerElement);
		CFNumberRef rowNumber = CFNumberCreate(NULL, kCFNumberSInt64Type, &bytesPerRow);
		CFDictionarySetValue(properties, kIOSurfaceBytesPerElement, elementNumber);
		CFDictionarySetValue(properties, kIOSurfaceBytesPerRow, rowNumber);
		CFRelease(elementNumber);
		CFRelease(rowNumber);
	} else {
		CFMutableArrayRef planeInfo = CFArrayCreateMutable(kCFAllocatorDefault, (CFIndex) format->planeCount, &kCFTypeArrayCallBacks);
		for (size_t i = 0; i < format->planeCount; i++) {
			int64_t values[4] = { (int64_t) buffer->planes[i].width, (int64_t) buffer->planes[i].height,
				(int64_t) format->bytesPerElement[i], (int64_t) buffer->planes[i].bytesPerRow };
			CFStringRef keys[4] = { kIOSurfacePlaneWidth, kIOSurfacePlaneHeight, kIOSurfacePlaneBytesPerElement, kIOSurfacePlaneBytesPerRow };
			CFNumberRef numbers[4];
			for (int k = 0; k < 4; k++)
				numbers[k] = CFNumberCreate(NULL, kCFNumberSInt64Type, &values[k]);
			CFDictionaryRef plane = CFDictionaryCreate(kCFAllocatorDefault, (const void**) keys, (const void**) numbers, 4,
				&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			for (int k = 0; k < 4; k++)
				CFRelease(numbers[k]);
			CFArrayAppendValue(planeInfo, plane);
			CFRelease(plane);
		}
		CFDictionarySetValue(properties, kIOSurfacePlaneInfo, planeInfo);
		CFRelease(planeInfo);
	}
	return properties;
}

// Takes the layout of the planes from the surface, which decides where they are.
static void layoutFromSurface(struct __CVBuffer* buffer, const struct __CVFormat* format)
{
	size_t planeCount = IOSurfaceGetPlaneCount(buffer->surface);
	buffer->planeCount = planeCount;
	if (planeCount == 0) {
		buffer->planes[0].width = IOSurfaceGetWidth(buffer->surface);
		buffer->planes[0].height = IOSurfaceGetHeight(buffer->surface);
		buffer->planes[0].bytesPerRow = IOSurfaceGetBytesPerRow(buffer->surface);
		buffer->planes[0].offset = 0;
		return;
	}
	uint8_t* base = IOSurfaceGetBaseAddress(buffer->surface);
	for (size_t i = 0; i < planeCount && i < kMaxPlanes; i++) {
		buffer->planes[i].width = IOSurfaceGetWidthOfPlane(buffer->surface, i);
		buffer->planes[i].height = IOSurfaceGetHeightOfPlane(buffer->surface, i);
		buffer->planes[i].bytesPerRow = IOSurfaceGetBytesPerRowOfPlane(buffer->surface, i);
		buffer->planes[i].offset = base ? (size_t) ((uint8_t*) IOSurfaceGetBaseAddressOfPlane(buffer->surface, i) - base) : 0;
	}
}

CVReturn CVPixelBufferCreate(CFAllocatorRef allocator, size_t width, size_t height, OSType pixelFormatType,
	CFDictionaryRef pixelBufferAttributes, CVPixelBufferRef* pixelBufferOut)
{
	if (pixelBufferOut == NULL || width == 0 || height == 0)
		return kCVReturnInvalidArgument;
	*pixelBufferOut = NULL;

	const struct __CVFormat* format = formatForType(pixelFormatType);
	if (format == NULL)
		return kCVReturnInvalidPixelFormat;

	size_t alignment = 64;
	numberFromAttributes(pixelBufferAttributes, kCVPixelBufferBytesPerRowAlignmentKey, &alignment);

	// Lay out the planes one after the other, each starting on a 64-byte boundary.
	size_t planeCount = format->planeCount ? format->planeCount : 1;
	size_t total = 0;
	struct __CVPlane planes[kMaxPlanes] = {{ 0 }};
	for (size_t i = 0; i < planeCount; i++) {
		size_t planeWidth = (width + format->horizontalSubsampling[i] - 1) / format->horizontalSubsampling[i];
		size_t planeHeight = (height + format->verticalSubsampling[i] - 1) / format->verticalSubsampling[i];
		if (planeWidth > (SIZE_MAX - alignment) / format->bytesPerElement[i])
			return kCVReturnAllocationFailed;
		size_t bytesPerRow = alignUp(planeWidth * format->bytesPerElement[i], alignment);
		if (planeHeight > (SIZE_MAX - total - 64) / bytesPerRow)
			return kCVReturnAllocationFailed;
		planes[i] = (struct __CVPlane) { planeWidth, planeHeight, bytesPerRow, alignUp(total, 64) };
		total = planes[i].offset + bytesPerRow * planeHeight;
	}

	CVPixelBufferRef buffer = pixelBufferAllocate(allocator, width, height, pixelFormatType);
	if (buffer == NULL)
		return kCVReturnAllocationFailed;
	buffer->planeCount = format->planeCount;
	memcpy(buffer->planes, planes, sizeof(planes));
	buffer->dataSize = total;

	CFTypeRef surfaceProperties = pixelBufferAttributes ? CFDictionaryGetValue(pixelBufferAttributes, kCVPixelBufferIOSurfacePropertiesKey) : NULL;
	if (pixelBufferAttributes && CFDictionaryContainsKey(pixelBufferAttributes, kCVPixelBufferIOSurfacePropertiesKey)) {
		// The key makes the buffer IOSurface-backed, also with an empty dictionary.
		CFDictionaryRef properties = createSurfaceProperties(
			(surfaceProperties && CFGetTypeID(surfaceProperties) == CFDictionaryGetTypeID()) ? (CFDictionaryRef) surfaceProperties : NULL,
			buffer, format);
		buffer->surface = properties ? IOSurfaceCreate(properties) : NULL;
		if (properties)
			CFRelease(properties);
		if (buffer->surface == NULL) {
			CFRelease(buffer);
			return kCVReturnAllocationFailed;
		}
		layoutFromSurface(buffer, format);
	} else {
		if (posix_memalign((void**) &buffer->memory, 64, total) != 0 || buffer->memory == NULL) {
			buffer->memory = NULL;
			CFRelease(buffer);
			return kCVReturnAllocationFailed;
		}
		memset(buffer->memory, 0, total);
		buffer->ownsMemory = true;
	}

	*pixelBufferOut = buffer;
	return kCVReturnSuccess;
}

CVReturn CVPixelBufferCreateWithIOSurface(CFAllocatorRef allocator, IOSurfaceRef surface,
	CFDictionaryRef pixelBufferAttributes, CVPixelBufferRef* pixelBufferOut)
{
	if (pixelBufferOut == NULL || surface == NULL)
		return kCVReturnInvalidArgument;
	*pixelBufferOut = NULL;

	// The surface decides the size and the format; the attributes do not change them.
	OSType pixelFormat = IOSurfaceGetPixelFormat(surface);
	if (pixelFormat == 0)
		return kCVReturnInvalidPixelFormat;
	size_t planeCount = IOSurfaceGetPlaneCount(surface);
	if (planeCount > kMaxPlanes)
		return kCVReturnInvalidPixelFormat;

	CVPixelBufferRef buffer = pixelBufferAllocate(allocator, IOSurfaceGetWidth(surface), IOSurfaceGetHeight(surface), pixelFormat);
	if (buffer == NULL)
		return kCVReturnAllocationFailed;
	buffer->surface = (IOSurfaceRef) CFRetain(surface);
	layoutFromSurface(buffer, NULL);
	buffer->dataSize = IOSurfaceGetAllocSize(surface);

	*pixelBufferOut = buffer;
	return kCVReturnSuccess;
}

IOSurfaceRef CVPixelBufferGetIOSurface(CVPixelBufferRef buffer)
{
	return buffer ? buffer->surface : NULL;
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

	// A buffer around the caller's bytes is never IOSurface-backed, whatever the attributes say.
	const struct __CVFormat* format = formatForType(pixelFormatType);
	if (format == NULL || format->planeCount != 0)
		return kCVReturnInvalidPixelFormat;
	if (width > bytesPerRow / format->bytesPerElement[0])
		return kCVReturnInvalidArgument;

	CVPixelBufferRef buffer = pixelBufferAllocate(allocator, width, height, pixelFormatType);
	if (buffer == NULL)
		return kCVReturnAllocationFailed;
	buffer->planes[0] = (struct __CVPlane) { width, height, bytesPerRow, 0 };
	buffer->dataSize = bytesPerRow * height;
	buffer->memory = baseAddress;
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
	return buffer ? buffer->planes[0].bytesPerRow : 0;
}

OSType CVPixelBufferGetPixelFormatType(CVPixelBufferRef buffer)
{
	return buffer ? buffer->pixelFormat : 0;
}

size_t CVPixelBufferGetDataSize(CVPixelBufferRef buffer)
{
	return buffer ? buffer->dataSize : 0;
}

// The first byte of the buffer's pixels, or NULL when it is not locked.
static uint8_t* lockedBase(CVPixelBufferRef buffer)
{
	uint8_t* base = NULL;
	pthread_mutex_lock(&buffer->lockMutex);
	if (buffer->lockCount > 0)
		base = buffer->surface ? IOSurfaceGetBaseAddress(buffer->surface) : buffer->memory;
	pthread_mutex_unlock(&buffer->lockMutex);
	return base;
}

void* CVPixelBufferGetBaseAddress(CVPixelBufferRef buffer)
{
	if (!buffer)
		return NULL;
	uint8_t* base = lockedBase(buffer);
	return base ? base + buffer->planes[0].offset : NULL;
}

size_t CVPixelBufferGetPlaneCount(CVPixelBufferRef buffer)
{
	return buffer ? buffer->planeCount : 0;
}

Boolean CVPixelBufferIsPlanar(CVPixelBufferRef buffer)
{
	return buffer && buffer->planeCount > 0;
}

void* CVPixelBufferGetBaseAddressOfPlane(CVPixelBufferRef buffer, size_t planeIndex)
{
	if (!buffer || planeIndex >= buffer->planeCount)
		return NULL;
	uint8_t* base = lockedBase(buffer);
	return base ? base + buffer->planes[planeIndex].offset : NULL;
}

size_t CVPixelBufferGetBytesPerRowOfPlane(CVPixelBufferRef buffer, size_t planeIndex)
{
	if (!buffer || planeIndex >= buffer->planeCount)
		return 0;
	return buffer->planes[planeIndex].bytesPerRow;
}

size_t CVPixelBufferGetWidthOfPlane(CVPixelBufferRef buffer, size_t planeIndex)
{
	if (!buffer || planeIndex >= buffer->planeCount)
		return 0;
	return buffer->planes[planeIndex].width;
}

size_t CVPixelBufferGetHeightOfPlane(CVPixelBufferRef buffer, size_t planeIndex)
{
	if (!buffer || planeIndex >= buffer->planeCount)
		return 0;
	return buffer->planes[planeIndex].height;
}

CGSize CVImageBufferGetEncodedSize(CVImageBufferRef imageBuffer)
{
	if (!imageBuffer)
		return (CGSize){0, 0};
	return (CGSize){(CGFloat)imageBuffer->width, (CGFloat)imageBuffer->height};
}

// The first lock locks the IOSurface of an IOSurface-backed buffer, with the flags of that lock:
// read-only access leaves the surface's seed alone, any other lock marks the contents as
// modified when it ends. Nested locks only count, and the last unlock ends the surface lock
// with the flags of the first lock. Without a surface the locks only count.
CVReturn CVPixelBufferLockBaseAddress(CVPixelBufferRef buffer, CVPixelBufferLockFlags flags)
{
	if (!buffer)
		return kCVReturnInvalidArgument;

	CVReturn result = kCVReturnSuccess;
	pthread_mutex_lock(&buffer->lockMutex);
	if (buffer->lockCount == 0) {
		if (buffer->surface) {
			IOSurfaceLockOptions options = (flags & kCVPixelBufferLock_ReadOnly) ? kIOSurfaceLockReadOnly : 0;
			if (IOSurfaceLock(buffer->surface, options, NULL) != KERN_SUCCESS)
				result = kCVReturnError;
		}
		if (result == kCVReturnSuccess)
			buffer->lockFlags = flags;
	}
	if (result == kCVReturnSuccess)
		buffer->lockCount++;
	pthread_mutex_unlock(&buffer->lockMutex);
	return result;
}

CVReturn CVPixelBufferUnlockBaseAddress(CVPixelBufferRef buffer, CVPixelBufferLockFlags flags)
{
	if (!buffer)
		return kCVReturnInvalidArgument;

	CVReturn result = kCVReturnSuccess;
	pthread_mutex_lock(&buffer->lockMutex);
	if (buffer->lockCount == 0) {
		result = kCVReturnError;
	} else if (--buffer->lockCount == 0 && buffer->surface) {
		Boolean lockedReadOnly = (buffer->lockFlags & kCVPixelBufferLock_ReadOnly) != 0;
		if (IOSurfaceUnlock(buffer->surface, lockedReadOnly ? kIOSurfaceLockReadOnly : 0, NULL) != KERN_SUCCESS)
			result = kCVReturnError;
		// QUIRK: ending a lock with a read-only flag different from the one that took the first
		// reference fails with kCVReturnError, in both directions (the surface is unlocked all
		// the same). Not documented. Evidence: black-box observation on macOS 27.0.1 (tier 2),
		// probe vd-upstream/mac-probes/v376-cvpixelbuffer-iosurface.m and the lock/unlock flag
		// matrix recorded in MAC-EVIDENCE.md. Only buffers backed by an IOSurface do this.
		// Test: tests/src/corevideo_iosurface_pixel_buffer.c (testLocking, "QUIRK test").
		else if (lockedReadOnly != ((flags & kCVPixelBufferLock_ReadOnly) != 0))
			result = kCVReturnError;
	}
	pthread_mutex_unlock(&buffer->lockMutex);
	return result;
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
	// The number of buffers handed out that are still alive.
	size_t outstanding;
	pthread_mutex_t mutex;
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
	pthread_mutex_destroy(&pool->mutex);
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

// A buffer of the pool was released: it no longer counts, and no longer keeps the pool alive.
static void poolBufferFinalized(CVPixelBufferPoolRef pool)
{
	pthread_mutex_lock(&pool->mutex);
	if (pool->outstanding > 0)
		pool->outstanding--;
	pthread_mutex_unlock(&pool->mutex);
	CFRelease(pool);
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

CFDictionaryRef CVPixelBufferPoolGetAttributes(CVPixelBufferPoolRef pool)
{
	return pool ? pool->poolAttributes : NULL;
}

CFDictionaryRef CVPixelBufferPoolGetPixelBufferAttributes(CVPixelBufferPoolRef pool)
{
	return pool ? pool->pixelBufferAttributes : NULL;
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
	pool->outstanding = 0;
	pthread_mutex_init(&pool->mutex, NULL);

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
	// Buffers are allocated on demand and are not recycled. The pool counts the buffers it has
	// handed out that are still alive, which is what an allocation threshold in the auxiliary
	// attributes limits.
	if (!pixelBufferPool || !pixelBufferOut) return kCVReturnInvalidArgument;
	*pixelBufferOut = NULL;

	size_t threshold = 0;
	Boolean hasThreshold = numberFromAttributes(auxAttributes, kCVPixelBufferPoolAllocationThresholdKey, &threshold);

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

	pthread_mutex_lock(&pixelBufferPool->mutex);
	if (hasThreshold && pixelBufferPool->outstanding >= threshold) {
		pthread_mutex_unlock(&pixelBufferPool->mutex);
		return kCVReturnWouldExceedAllocationThreshold;
	}
	pixelBufferPool->outstanding++;
	pthread_mutex_unlock(&pixelBufferPool->mutex);

	CVReturn result = CVPixelBufferCreate(allocator, width, height, format, pixelBufferPool->pixelBufferAttributes, pixelBufferOut);
	if (result != kCVReturnSuccess) {
		pthread_mutex_lock(&pixelBufferPool->mutex);
		pixelBufferPool->outstanding--;
		pthread_mutex_unlock(&pixelBufferPool->mutex);
		return result;
	}
	(*pixelBufferOut)->pool = (CVPixelBufferPoolRef) CFRetain(pixelBufferPool);
	return kCVReturnSuccess;
}
