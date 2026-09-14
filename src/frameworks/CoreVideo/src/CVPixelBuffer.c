/*
 This file is part of Darling.

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

// Minimal CPU-memory CVPixelBuffer: a single-plane, packed buffer with 16-byte row alignment.
// There is no IOSurface or pool backing. Pixel buffers are CF objects, so CFRetain, CFRelease
// and CFGetTypeID work on them (Swift and many Objective-C callers rely on that).

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <dispatch/dispatch.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef int32_t CVReturn;
typedef uint32_t OSType;
typedef uint64_t CVPixelBufferLockFlags;
typedef void (*CVPixelBufferReleaseBytesCallback)(void* releaseRefCon, const void* baseAddress);
typedef void (*CVPixelBufferReleasePlanarBytesCallback)(void* releaseRefCon, const void* dataPtr, size_t dataSize,
	size_t numberOfPlanes, const void* planeAddresses[]);

enum {
	kCVReturnSuccess = 0,
	kCVReturnInvalidArgument = -6661,
	kCVReturnAllocationFailed = -6662,
	kCVReturnInvalidPixelFormat = -6680,
};

struct __CVPixelBuffer {
	CFRuntimeBase runtimeBase;
	size_t width;
	size_t height;
	size_t bytesPerRow;
	OSType pixelFormat;
	uint8_t* base;
	Boolean ownsBase;
	CVPixelBufferReleaseBytesCallback releaseCallback;
	void* releaseRefCon;
};
typedef struct __CVPixelBuffer* CVPixelBufferRef;
typedef CFTypeRef CVBufferRef;

static void pixelBufferFinalize(CFTypeRef cf)
{
	CVPixelBufferRef buffer = (CVPixelBufferRef) cf;
	if (buffer->ownsBase)
		free(buffer->base);
	else if (buffer->releaseCallback)
		buffer->releaseCallback(buffer->releaseRefCon, buffer->base);
	buffer->base = NULL;
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
		sizeof(struct __CVPixelBuffer) - sizeof(CFRuntimeBase), NULL);
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
		case 'L008':   // kCVPixelFormatType_OneComponent8
		case 0x00000008: // kCVPixelFormatType_8Indexed
			return 1;
		case 'RGhA':   // kCVPixelFormatType_64RGBAHalf
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
	// Both the multiplication and the round-up to 16 bytes must not wrap.
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
		return kCVReturnInvalidArgument; // a row of pixels does not fit in bytesPerRow

	CVPixelBufferRef buffer = pixelBufferAllocate(allocator, width, height, bytesPerRow, pixelFormatType);
	if (buffer == NULL)
		return kCVReturnAllocationFailed;
	// The caller's memory is released through releaseCallback when the last reference goes away.
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
	// Planar buffers are not supported; fail cleanly rather than report success with no buffer.
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
	return 0; // not planar
}

Boolean CVPixelBufferIsPlanar(CVPixelBufferRef buffer)
{
	return false;
}

CVReturn CVPixelBufferLockBaseAddress(CVPixelBufferRef buffer, CVPixelBufferLockFlags flags)
{
	return buffer ? kCVReturnSuccess : kCVReturnInvalidArgument;
}

CVReturn CVPixelBufferUnlockBaseAddress(CVPixelBufferRef buffer, CVPixelBufferLockFlags flags)
{
	return buffer ? kCVReturnSuccess : kCVReturnInvalidArgument;
}
