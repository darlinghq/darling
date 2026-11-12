// Copyright (C) 2026 Lubos Dolezel
//
// IOSurface-backed CVPixelBuffers, as observed on macOS 27.0.1 (arm64 and x86_64): a buffer is
// IOSurface-backed when its attributes contain kCVPixelBufferIOSurfacePropertiesKey (also with
// an empty dictionary) and for CVPixelBufferCreateWithIOSurface, and is in memory otherwise,
// also when created around the caller's bytes. The surface has the buffer's size, format,
// bytes per row and planes. The base address is only available while the buffer is locked;
// the first lock locks the surface with that lock's flags, nested locks only count, and the
// seed of the surface moves when the lock was not read-only. Pools pass their attributes on
// and can limit the buffers they have outstanding. Passes unchanged on macOS.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CoreVideo.h>
#include <IOSurface/IOSurface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static CFNumberRef number(int64_t value)
{
	return CFNumberCreate(NULL, kCFNumberSInt64Type, &value);
}

// A dictionary with one entry, or an IOSurfaceProperties entry holding another dictionary.
static CFDictionaryRef attributes(CFStringRef key, CFTypeRef value)
{
	return CFDictionaryCreate(NULL, (const void**) &key, &value, 1, &kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
}

static CFDictionaryRef emptyDictionary(void)
{
	return CFDictionaryCreate(NULL, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
}

static CFDictionaryRef surfaceAttributes(void)
{
	CFDictionaryRef empty = emptyDictionary();
	CFDictionaryRef result = attributes(kCVPixelBufferIOSurfacePropertiesKey, empty);
	CFRelease(empty);
	return result;
}

static CVPixelBufferRef create(size_t width, size_t height, OSType format, CFDictionaryRef attrs)
{
	CVPixelBufferRef buffer = NULL;
	CHECK(CVPixelBufferCreate(NULL, width, height, format, attrs, &buffer) == kCVReturnSuccess);
	return buffer;
}

static void testBacking(void)
{
	CFDictionaryRef withSurface = surfaceAttributes();
	CFDictionaryRef empty = emptyDictionary();

	// Without the key a buffer is in memory, with or without other attributes.
	CVPixelBufferRef buffer = create(33, 17, kCVPixelFormatType_32BGRA, NULL);
	CHECK(buffer && CVPixelBufferGetIOSurface(buffer) == NULL);
	CVPixelBufferRelease(buffer);
	buffer = create(33, 17, kCVPixelFormatType_32BGRA, empty);
	CHECK(buffer && CVPixelBufferGetIOSurface(buffer) == NULL);
	CVPixelBufferRelease(buffer);

	// With the key, also with an empty dictionary, the pixels are in a surface of the same geometry.
	buffer = create(33, 17, kCVPixelFormatType_32BGRA, withSurface);
	IOSurfaceRef surface = buffer ? CVPixelBufferGetIOSurface(buffer) : NULL;
	CHECK(surface != NULL);
	if (surface) {
		CHECK(IOSurfaceGetWidth(surface) == 33 && IOSurfaceGetHeight(surface) == 17);
		CHECK(IOSurfaceGetPixelFormat(surface) == kCVPixelFormatType_32BGRA);
		CHECK(IOSurfaceGetBytesPerElement(surface) == 4);
		CHECK(IOSurfaceGetBytesPerRow(surface) == CVPixelBufferGetBytesPerRow(buffer));
		CHECK(IOSurfaceGetPlaneCount(surface) == 0);
		// Rows are padded for the hardware (64 bytes on Apple silicon), and the data is the rows.
		CHECK(CVPixelBufferGetBytesPerRow(buffer) >= 33 * 4 && CVPixelBufferGetBytesPerRow(buffer) < 33 * 4 + 64);
		CHECK(CVPixelBufferGetBytesPerRow(buffer) % 16 == 0);
		CHECK(CVPixelBufferGetDataSize(buffer) == CVPixelBufferGetBytesPerRow(buffer) * 17);
	}
	CVPixelBufferRelease(buffer);

	// A buffer in memory has the same row size as one in a surface, whatever the width.
	size_t widths[] = { 1, 15, 16, 17, 20, 37 };
	for (size_t i = 0; i < sizeof(widths) / sizeof(widths[0]); i++) {
		CVPixelBufferRef inMemory = create(widths[i], 4, kCVPixelFormatType_32BGRA, NULL);
		buffer = create(widths[i], 4, kCVPixelFormatType_32BGRA, withSurface);
		CHECK(inMemory && buffer && CVPixelBufferGetBytesPerRow(inMemory) == CVPixelBufferGetBytesPerRow(buffer));
		CHECK(buffer && CVPixelBufferGetBytesPerRow(buffer) >= widths[i] * 4);
		CVPixelBufferRelease(inMemory);
		CVPixelBufferRelease(buffer);
	}

	// kCVPixelBufferBytesPerRowAlignmentKey changes the row size of both kinds.
	CFNumberRef alignment = number(256);
	CFMutableDictionaryRef aligned = CFDictionaryCreateMutableCopy(NULL, 0, withSurface);
	CFDictionarySetValue(aligned, kCVPixelBufferBytesPerRowAlignmentKey, alignment);
	buffer = create(33, 17, kCVPixelFormatType_32BGRA, aligned);
	CHECK(buffer && CVPixelBufferGetBytesPerRow(buffer) == 256);
	CHECK(buffer && CVPixelBufferGetIOSurface(buffer) && IOSurfaceGetBytesPerRow(CVPixelBufferGetIOSurface(buffer)) == 256);
	CVPixelBufferRelease(buffer);
	CFDictionarySetValue(aligned, kCVPixelBufferIOSurfacePropertiesKey, empty);
	CFDictionaryRemoveValue(aligned, kCVPixelBufferIOSurfacePropertiesKey);
	buffer = create(33, 17, kCVPixelFormatType_32BGRA, aligned);
	CHECK(buffer && CVPixelBufferGetBytesPerRow(buffer) == 256 && CVPixelBufferGetIOSurface(buffer) == NULL);
	CVPixelBufferRelease(buffer);
	CFRelease(aligned);
	CFRelease(alignment);

	// Other single-plane formats.
	OSType formats[] = { kCVPixelFormatType_32ARGB, kCVPixelFormatType_24RGB, kCVPixelFormatType_OneComponent8 };
	size_t bytesPerPixel[] = { 4, 3, 1 };
	for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
		buffer = create(33, 17, formats[i], withSurface);
		surface = buffer ? CVPixelBufferGetIOSurface(buffer) : NULL;
		CHECK(surface != NULL && IOSurfaceGetPixelFormat(surface) == formats[i]);
		CHECK(surface != NULL && IOSurfaceGetBytesPerElement(surface) == bytesPerPixel[i]);
		CVPixelBufferRelease(buffer);
	}

	// A bi-planar format: luma at full size, interleaved chroma at half size rounded up.
	buffer = create(33, 17, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange, withSurface);
	surface = buffer ? CVPixelBufferGetIOSurface(buffer) : NULL;
	CHECK(buffer && CVPixelBufferIsPlanar(buffer) && CVPixelBufferGetPlaneCount(buffer) == 2);
	CHECK(surface != NULL && IOSurfaceGetPlaneCount(surface) == 2);
	if (buffer && surface) {
		CHECK(CVPixelBufferGetWidthOfPlane(buffer, 0) == 33 && CVPixelBufferGetHeightOfPlane(buffer, 0) == 17);
		CHECK(CVPixelBufferGetWidthOfPlane(buffer, 1) == 17 && CVPixelBufferGetHeightOfPlane(buffer, 1) == 9);
		CHECK(CVPixelBufferGetBytesPerRowOfPlane(buffer, 0) >= 33 && CVPixelBufferGetBytesPerRowOfPlane(buffer, 1) >= 34);
		for (size_t plane = 0; plane < 2; plane++) {
			CHECK(IOSurfaceGetWidthOfPlane(surface, plane) == CVPixelBufferGetWidthOfPlane(buffer, plane));
			CHECK(IOSurfaceGetHeightOfPlane(surface, plane) == CVPixelBufferGetHeightOfPlane(buffer, plane));
			CHECK(IOSurfaceGetBytesPerRowOfPlane(surface, plane) == CVPixelBufferGetBytesPerRowOfPlane(buffer, plane));
		}
		CHECK(IOSurfaceGetBytesPerElementOfPlane(surface, 0) == 1 && IOSurfaceGetBytesPerElementOfPlane(surface, 1) == 2);
		CHECK(CVPixelBufferGetBaseAddressOfPlane(buffer, 0) == NULL);
		CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
		CHECK(CVPixelBufferGetBaseAddressOfPlane(buffer, 0) == IOSurfaceGetBaseAddressOfPlane(surface, 0));
		CHECK(CVPixelBufferGetBaseAddressOfPlane(buffer, 1) == IOSurfaceGetBaseAddressOfPlane(surface, 1));
		CHECK(CVPixelBufferGetBaseAddressOfPlane(buffer, 2) == NULL);
		CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);
	}
	CVPixelBufferRelease(buffer);

	// A buffer around the caller's bytes is never IOSurface-backed.
	static unsigned char bytes[64 * 8];
	buffer = NULL;
	CHECK(CVPixelBufferCreateWithBytes(NULL, 16, 8, kCVPixelFormatType_32BGRA, bytes, 64, NULL, NULL, withSurface, &buffer)
		== kCVReturnSuccess);
	CHECK(buffer && CVPixelBufferGetIOSurface(buffer) == NULL);
	CHECK(CVPixelBufferGetIOSurface(NULL) == NULL);
	CVPixelBufferRelease(buffer);

	CFRelease(withSurface);
	CFRelease(empty);
}

static uint32_t seedOf(CVPixelBufferRef buffer)
{
	return IOSurfaceGetSeed(CVPixelBufferGetIOSurface(buffer));
}

static void testLocking(void)
{
	CFDictionaryRef withSurface = surfaceAttributes();
	CVPixelBufferRef buffer = create(32, 16, kCVPixelFormatType_32BGRA, withSurface);
	IOSurfaceRef surface = buffer ? CVPixelBufferGetIOSurface(buffer) : NULL;
	if (!surface) {
		CHECK(surface != NULL);
		CFRelease(withSurface);
		return;
	}

	// The base address exists only while the buffer is locked.
	CHECK(CVPixelBufferGetBaseAddress(buffer) == NULL);
	uint32_t seed = seedOf(buffer);
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferGetBaseAddress(buffer) != NULL);
	CHECK(CVPixelBufferGetBaseAddress(buffer) == IOSurfaceGetBaseAddress(surface));
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferGetBaseAddress(buffer) == NULL);
	// A lock for writing moves the seed once; a read-only lock does not.
	CHECK(seedOf(buffer) == seed + 1);
	seed = seedOf(buffer);
	CHECK(CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	CHECK(CVPixelBufferGetBaseAddress(buffer) != NULL);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	CHECK(seedOf(buffer) == seed);

	// Nested locks count; the seed moves once for the pair.
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferGetBaseAddress(buffer) != NULL);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferGetBaseAddress(buffer) == NULL);
	CHECK(seedOf(buffer) == seed + 1);
	seed = seedOf(buffer);

	// The flags of the first lock decide, not those of nested locks: the seed moves for a first
	// lock that is not read-only, and the last unlock has to pass the flags of the first lock.
	CHECK(CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	CHECK(seedOf(buffer) == seed);
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(seedOf(buffer) == seed + 1);
	seed = seedOf(buffer);

	// QUIRK test: the last unlock with other flags than the first lock fails on a buffer in a
	// surface, in both directions, and leaves the buffer unlocked.
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnError);
	CHECK(CVPixelBufferGetBaseAddress(buffer) == NULL);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnError);
	CHECK(seedOf(buffer) == seed + 1);
	// (The state of a buffer after a failed unlock is not specified: another one for the other direction.)
	CVPixelBufferRef other = create(32, 16, kCVPixelFormatType_32BGRA, withSurface);
	if (other) {
		uint32_t otherSeed = seedOf(other);
		CHECK(CVPixelBufferLockBaseAddress(other, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
		CHECK(CVPixelBufferUnlockBaseAddress(other, 0) == kCVReturnError);
		CHECK(CVPixelBufferGetBaseAddress(other) == NULL);
		CHECK(CVPixelBufferUnlockBaseAddress(other, 0) == kCVReturnError);
		CHECK(seedOf(other) == otherSeed);
		CVPixelBufferRelease(other);
	}
	seed = seedOf(buffer);

	// An unlock without a lock is an error.
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnError);
	CHECK(CVPixelBufferLockBaseAddress(NULL, 0) == kCVReturnInvalidArgument);
	CHECK(CVPixelBufferUnlockBaseAddress(NULL, 0) == kCVReturnInvalidArgument);

	// A buffer in memory has no base address while unlocked either, and counts its locks.
	CVPixelBufferRef memory = create(32, 16, kCVPixelFormatType_32BGRA, NULL);
	CHECK(memory && CVPixelBufferGetBaseAddress(memory) == NULL);
	CHECK(CVPixelBufferLockBaseAddress(memory, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	CHECK(CVPixelBufferGetBaseAddress(memory) != NULL && ((uintptr_t) CVPixelBufferGetBaseAddress(memory) & 63) == 0);
	CHECK(CVPixelBufferUnlockBaseAddress(memory, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferGetBaseAddress(memory) == NULL);
	CHECK(CVPixelBufferLockBaseAddress(memory, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferUnlockBaseAddress(memory, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	CHECK(CVPixelBufferUnlockBaseAddress(memory, 0) == kCVReturnError);
	CVPixelBufferRelease(memory);

	// A buffer released while locked gives its surface lock back.
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CFRetain(surface);
	CVPixelBufferRelease(buffer);
	CHECK(IOSurfaceLock(surface, 0, NULL) == kIOReturnSuccess);
	CHECK(IOSurfaceUnlock(surface, 0, NULL) == kIOReturnSuccess);
	CFRelease(surface);
	CFRelease(withSurface);
}

static void testRoundTrip(void)
{
	CFDictionaryRef withSurface = surfaceAttributes();
	CVPixelBufferRef buffer = create(20, 10, kCVPixelFormatType_32BGRA, withSurface);
	IOSurfaceRef surface = buffer ? CVPixelBufferGetIOSurface(buffer) : NULL;
	if (!surface) {
		CHECK(surface != NULL);
		CFRelease(withSurface);
		return;
	}

	// Written through the buffer, read through the surface, and through another buffer on it.
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	uint8_t* base = CVPixelBufferGetBaseAddress(buffer);
	size_t bytesPerRow = CVPixelBufferGetBytesPerRow(buffer);
	for (size_t y = 0; y < 10; y++)
		for (size_t x = 0; x < 20; x++)
			base[y * bytesPerRow + x * 4] = (uint8_t) (x * 10 + y);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);

	CHECK(IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
	const uint8_t* viaSurface = IOSurfaceGetBaseAddress(surface);
	CHECK(viaSurface[3 * bytesPerRow + 7 * 4] == 7 * 10 + 3);
	CHECK(IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);

	CVPixelBufferRef second = NULL;
	CHECK(CVPixelBufferCreateWithIOSurface(NULL, surface, NULL, &second) == kCVReturnSuccess);
	CHECK(second != NULL && second != buffer && CVPixelBufferGetIOSurface(second) == surface);
	CHECK(CVPixelBufferLockBaseAddress(second, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	const uint8_t* viaSecond = CVPixelBufferGetBaseAddress(second);
	CHECK(viaSecond != NULL && viaSecond[9 * bytesPerRow + 19 * 4] == 19 * 10 + 9);
	CHECK(CVPixelBufferUnlockBaseAddress(second, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	CVPixelBufferRelease(second);
	CVPixelBufferRelease(buffer);
	CFRelease(withSurface);
}

static void testCreateWithIOSurface(void)
{
	CFNumberRef width = number(20), height = number(10), bytesPerElement = number(4), format = number(kCVPixelFormatType_32BGRA);
	CFStringRef keys[] = { kIOSurfaceWidth, kIOSurfaceHeight, kIOSurfaceBytesPerElement, kIOSurfacePixelFormat };
	CFTypeRef values[] = { width, height, bytesPerElement, format };
	CFDictionaryRef properties = CFDictionaryCreate(NULL, (const void**) keys, (const void**) values, 4,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	IOSurfaceRef surface = IOSurfaceCreate(properties);
	CHECK(surface != NULL);
	if (!surface)
		return;

	// The surface decides size and format; the buffer retains the surface and shares its row size.
	CFNumberRef otherWidth = number(999);
	CFDictionaryRef attrs = attributes(kCVPixelBufferWidthKey, otherWidth);
	CFIndex retains = CFGetRetainCount(surface);
	CVPixelBufferRef buffer = NULL;
	CHECK(CVPixelBufferCreateWithIOSurface(NULL, surface, attrs, &buffer) == kCVReturnSuccess);
	CHECK(buffer != NULL && CVPixelBufferGetIOSurface(buffer) == surface);
	CHECK(buffer && CVPixelBufferGetWidth(buffer) == 20 && CVPixelBufferGetHeight(buffer) == 10);
	CHECK(buffer && CVPixelBufferGetPixelFormatType(buffer) == kCVPixelFormatType_32BGRA);
	CHECK(buffer && CVPixelBufferGetBytesPerRow(buffer) == IOSurfaceGetBytesPerRow(surface));
	CHECK(buffer && CVPixelBufferGetDataSize(buffer) == IOSurfaceGetAllocSize(surface));
	CHECK(buffer && !CVPixelBufferIsPlanar(buffer));
	CHECK(CFGetRetainCount(surface) == retains + 1);
	CVPixelBufferRelease(buffer);
	CHECK(CFGetRetainCount(surface) == retains);
	CFRelease(attrs);
	CFRelease(otherWidth);
	CFRelease(surface);

	// A surface without a pixel format, and missing arguments.
	CFDictionaryRef noFormat = attributes(kIOSurfaceWidth, width);
	CFMutableDictionaryRef mutableProperties = CFDictionaryCreateMutableCopy(NULL, 0, properties);
	CFDictionaryRemoveValue(mutableProperties, kIOSurfacePixelFormat);
	IOSurfaceRef plain = IOSurfaceCreate(mutableProperties);
	CHECK(plain != NULL);
	buffer = NULL;
	CHECK(plain && CVPixelBufferCreateWithIOSurface(NULL, plain, NULL, &buffer) == kCVReturnInvalidPixelFormat);
	CHECK(buffer == NULL);
	CHECK(CVPixelBufferCreateWithIOSurface(NULL, NULL, NULL, &buffer) == kCVReturnInvalidArgument);
	if (plain) {
		CHECK(CVPixelBufferCreateWithIOSurface(NULL, plain, NULL, NULL) == kCVReturnInvalidArgument);
		CFRelease(plain);
	}
	CFRelease(mutableProperties);
	CFRelease(noFormat);
	CFRelease(properties);
	CFRelease(width);
	CFRelease(height);
	CFRelease(bytesPerElement);
	CFRelease(format);
}

static void testPools(void)
{
	CFNumberRef width = number(64), height = number(32), format = number(kCVPixelFormatType_32BGRA);
	CFStringRef keys[] = { kCVPixelBufferWidthKey, kCVPixelBufferHeightKey, kCVPixelBufferPixelFormatTypeKey,
		kCVPixelBufferIOSurfacePropertiesKey };
	CFDictionaryRef empty = emptyDictionary();
	CFTypeRef values[] = { width, height, format, empty };
	CFDictionaryRef plain = CFDictionaryCreate(NULL, (const void**) keys, (const void**) values, 3,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionaryRef backed = CFDictionaryCreate(NULL, (const void**) keys, (const void**) values, 4,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	// The attributes of the pool decide whether its buffers are in a surface.
	CVPixelBufferPoolRef pool = NULL;
	CHECK(CVPixelBufferPoolCreate(NULL, NULL, plain, &pool) == kCVReturnSuccess);
	CVPixelBufferRef buffer = NULL;
	CHECK(CVPixelBufferPoolCreatePixelBuffer(NULL, pool, &buffer) == kCVReturnSuccess);
	CHECK(buffer && CVPixelBufferGetIOSurface(buffer) == NULL && CVPixelBufferGetBytesPerRow(buffer) >= 64 * 4);
	CHECK(CVPixelBufferPoolGetPixelBufferAttributes(pool) != NULL
		&& !CFDictionaryContainsKey(CVPixelBufferPoolGetPixelBufferAttributes(pool), kCVPixelBufferIOSurfacePropertiesKey));
	CVPixelBufferRelease(buffer);
	CVPixelBufferPoolRelease(pool);

	pool = NULL;
	CHECK(CVPixelBufferPoolCreate(NULL, NULL, backed, &pool) == kCVReturnSuccess);
	CHECK(CVPixelBufferPoolGetPixelBufferAttributes(pool) != NULL
		&& CFDictionaryContainsKey(CVPixelBufferPoolGetPixelBufferAttributes(pool), kCVPixelBufferIOSurfacePropertiesKey));
	buffer = NULL;
	CHECK(CVPixelBufferPoolCreatePixelBuffer(NULL, pool, &buffer) == kCVReturnSuccess);
	IOSurfaceRef surface = buffer ? CVPixelBufferGetIOSurface(buffer) : NULL;
	CHECK(surface != NULL);
	CHECK(surface && IOSurfaceGetWidth(surface) == 64 && IOSurfaceGetHeight(surface) == 32);
	CHECK(buffer && CVPixelBufferGetBytesPerRow(buffer) >= 64 * 4);
	CVPixelBufferRelease(buffer);
	CVPixelBufferPoolRelease(pool);

	// An allocation threshold in the auxiliary attributes limits the buffers outstanding.
	CFNumberRef two = number(2);
	CFDictionaryRef poolAttributes = attributes(kCVPixelBufferPoolMinimumBufferCountKey, two);
	CFDictionaryRef threshold = attributes(kCVPixelBufferPoolAllocationThresholdKey, two);
	pool = NULL;
	CHECK(CVPixelBufferPoolCreate(NULL, poolAttributes, backed, &pool) == kCVReturnSuccess);
	CHECK(CVPixelBufferPoolGetAttributes(pool) != NULL
		&& CFDictionaryContainsKey(CVPixelBufferPoolGetAttributes(pool), kCVPixelBufferPoolMinimumBufferCountKey));
	CVPixelBufferRef first = NULL, secondBuffer = NULL, third = NULL;
	CHECK(CVPixelBufferPoolCreatePixelBuffer(NULL, pool, &first) == kCVReturnSuccess);
	CHECK(CVPixelBufferPoolCreatePixelBuffer(NULL, pool, &secondBuffer) == kCVReturnSuccess);
	CHECK(CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(NULL, pool, threshold, &third) == kCVReturnWouldExceedAllocationThreshold);
	CHECK(third == NULL);
	CHECK(CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(NULL, pool, NULL, &third) == kCVReturnSuccess);
	CVPixelBufferRelease(third);
	CVPixelBufferRelease(first);
	CVPixelBufferRelease(secondBuffer);
	CVPixelBufferPoolRelease(pool);

	CFRelease(threshold);
	CFRelease(poolAttributes);
	CFRelease(two);
	CFRelease(backed);
	CFRelease(plain);
	CFRelease(empty);
	CFRelease(width);
	CFRelease(height);
	CFRelease(format);
}

int main(void)
{
	testBacking();
	testLocking();
	testRoundTrip();
	testCreateWithIOSurface();
	testPools();

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
