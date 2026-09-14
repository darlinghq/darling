/*
 This file is part of Darling.

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

// Minimal CPU-memory CVPixelBuffer: a single-plane, packed buffer with 16-byte row alignment.
// There is no IOSurface or pool backing.

#include <CoreFoundation/CoreFoundation.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef int32_t CVReturn;
typedef uint32_t OSType;
typedef uint64_t CVPixelBufferLockFlags;

enum {
	kCVReturnSuccess = 0,
	kCVReturnInvalidArgument = -6661,
	kCVReturnAllocationFailed = -6662,
	kCVReturnInvalidPixelFormat = -6680,
};

struct __CVPixelBuffer {
	atomic_long refcount;
	size_t width;
	size_t height;
	size_t bytesPerRow;
	OSType pixelFormat;
	uint8_t* base;
};
typedef struct __CVPixelBuffer* CVPixelBufferRef;
typedef CVPixelBufferRef CVBufferRef;

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
	if (width > SIZE_MAX / bpp)
		return kCVReturnAllocationFailed;

	size_t bytesPerRow = (width * bpp + 15) & ~(size_t)15;
	if (height > SIZE_MAX / bytesPerRow)
		return kCVReturnAllocationFailed;

	CVPixelBufferRef buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL)
		return kCVReturnAllocationFailed;
	buffer->base = calloc(height, bytesPerRow);
	if (buffer->base == NULL) {
		free(buffer);
		return kCVReturnAllocationFailed;
	}

	atomic_init(&buffer->refcount, 1);
	buffer->width = width;
	buffer->height = height;
	buffer->bytesPerRow = bytesPerRow;
	buffer->pixelFormat = pixelFormatType;
	*pixelBufferOut = buffer;
	return kCVReturnSuccess;
}

CVPixelBufferRef CVPixelBufferRetain(CVPixelBufferRef buffer)
{
	if (buffer)
		atomic_fetch_add(&buffer->refcount, 1);
	return buffer;
}

void CVPixelBufferRelease(CVPixelBufferRef buffer)
{
	if (buffer && atomic_fetch_sub(&buffer->refcount, 1) == 1) {
		free(buffer->base);
		free(buffer);
	}
}

CVBufferRef CVBufferRetain(CVBufferRef buffer)
{
	return CVPixelBufferRetain(buffer);
}

void CVBufferRelease(CVBufferRef buffer)
{
	CVPixelBufferRelease(buffer);
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

CFTypeID CVPixelBufferGetTypeID(void)
{
	return 0;
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
