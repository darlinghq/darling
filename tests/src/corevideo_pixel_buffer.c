// Copyright (C) 2026 Lubos Dolezel
//
// CVPixelBuffer is a Core Foundation type: CFRetain, CFRelease and CFGetTypeID work on it.
// Buffers created by CVPixelBufferCreate have 16-byte aligned rows, can be locked and
// written, and carry attachments that propagate or not as their mode says. A buffer
// pool hands out buffers of the configured size and format.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <CoreVideo/CoreVideo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static int g_released;
static void* g_releasedBase;

static void releaseBytes(void* refCon, const void* baseAddress)
{
	g_released++;
	g_releasedBase = (void*) baseAddress;
}

static CFNumberRef number(int64_t value)
{
	return CFNumberCreate(NULL, kCFNumberSInt64Type, &value);
}

int main(void)
{
	CVPixelBufferRef buffer = NULL;

	CHECK(CVPixelBufferGetTypeID() != 0);
	CHECK(CVPixelBufferCreate(NULL, 10, 4, kCVPixelFormatType_32BGRA, NULL, &buffer) == kCVReturnSuccess);
	CHECK(buffer != NULL);
	CHECK(CFGetTypeID(buffer) == CVPixelBufferGetTypeID());
	CHECK(CVPixelBufferGetWidth(buffer) == 10);
	CHECK(CVPixelBufferGetHeight(buffer) == 4);
	CHECK(CVPixelBufferGetPixelFormatType(buffer) == kCVPixelFormatType_32BGRA);
	// A row of 10 pixels is 40 bytes; rows are padded to a multiple of 16
	CHECK(CVPixelBufferGetBytesPerRow(buffer) >= 40 && CVPixelBufferGetBytesPerRow(buffer) % 16 == 0);
	CHECK(CVPixelBufferGetDataSize(buffer) >= CVPixelBufferGetBytesPerRow(buffer) * 4);
	CHECK(!CVPixelBufferIsPlanar(buffer));
	CHECK(CVPixelBufferGetPlaneCount(buffer) == 0);

	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	uint8_t* base = CVPixelBufferGetBaseAddress(buffer);
	CHECK(base != NULL);
	CHECK(((uintptr_t) base & 15) == 0);
	memset(base, 0xAB, CVPixelBufferGetDataSize(buffer));
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);

	// Reference counting through Core Foundation and through CoreVideo agree
	CFRetain(buffer);
	CHECK(CFGetRetainCount(buffer) == 2);
	CVPixelBufferRelease(buffer);
	CHECK(CFGetRetainCount(buffer) == 1);

	// Attachments: mode, propagation and removal
	CFStringRef key1 = CFSTR("test.attachment.propagating");
	CFStringRef key2 = CFSTR("test.attachment.local");
	CVBufferSetAttachment(buffer, key1, CFSTR("one"), kCVAttachmentMode_ShouldPropagate);
	CVBufferSetAttachment(buffer, key2, CFSTR("two"), kCVAttachmentMode_ShouldNotPropagate);

	CVAttachmentMode mode = (CVAttachmentMode) 99;
	CHECK(CFEqual(CVBufferGetAttachment(buffer, key1, &mode), CFSTR("one")));
	CHECK(mode == kCVAttachmentMode_ShouldPropagate);
	CHECK(CFEqual(CVBufferGetAttachment(buffer, key2, &mode), CFSTR("two")));
	CHECK(mode == kCVAttachmentMode_ShouldNotPropagate);
	CHECK(CVBufferGetAttachment(buffer, CFSTR("test.attachment.missing"), NULL) == NULL);

	CFTypeRef copied = CVBufferCopyAttachment(buffer, key1, NULL);
	CHECK(copied != NULL && CFEqual(copied, CFSTR("one")));
	if (copied)
		CFRelease(copied);

	CVPixelBufferRef other = NULL;
	CHECK(CVPixelBufferCreate(NULL, 2, 2, kCVPixelFormatType_32BGRA, NULL, &other) == kCVReturnSuccess);
	CVBufferPropagateAttachments(buffer, other);
	CHECK(CFEqual(CVBufferGetAttachment(other, key1, &mode), CFSTR("one")));
	CHECK(mode == kCVAttachmentMode_ShouldPropagate);
	CHECK(CVBufferGetAttachment(other, key2, NULL) == NULL);

	// Setting a key again in the other mode moves it
	CVBufferSetAttachment(buffer, key1, CFSTR("one"), kCVAttachmentMode_ShouldNotPropagate);
	CHECK(CVBufferGetAttachment(buffer, key1, &mode) != NULL && mode == kCVAttachmentMode_ShouldNotPropagate);

	CFDictionaryRef all = CVBufferCopyAttachments(buffer, kCVAttachmentMode_ShouldNotPropagate);
	CHECK(all != NULL && CFDictionaryGetCount(all) == 2);
	if (all)
		CFRelease(all);

	CVBufferRemoveAttachment(buffer, key1);
	CHECK(CVBufferGetAttachment(buffer, key1, NULL) == NULL);
	CVBufferRemoveAllAttachments(buffer);
	CHECK(CVBufferGetAttachment(buffer, key2, NULL) == NULL);

	CVPixelBufferRelease(other);
	CVPixelBufferRelease(buffer);

	// Caller-owned memory: the release callback runs once, when the last reference goes
	size_t rowBytes = 64;
	void* bytes = calloc(rowBytes, 8);
	CHECK(CVPixelBufferCreateWithBytes(NULL, 8, 8, kCVPixelFormatType_32BGRA, bytes, rowBytes, releaseBytes, NULL, NULL, &buffer)
		== kCVReturnSuccess);
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CHECK(CVPixelBufferGetBaseAddress(buffer) == bytes);
	CHECK(CVPixelBufferGetBytesPerRow(buffer) == rowBytes);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);
	CFRetain(buffer);
	CFRelease(buffer);
	CHECK(g_released == 0);
	CFRelease(buffer);
	CHECK(g_released == 1 && g_releasedBase == bytes);
	free(bytes);

	// Invalid arguments
	CHECK(CVPixelBufferCreate(NULL, 0, 4, kCVPixelFormatType_32BGRA, NULL, &buffer) == kCVReturnInvalidArgument);
	CHECK(CVPixelBufferCreate(NULL, 4, 0, kCVPixelFormatType_32BGRA, NULL, &buffer) == kCVReturnInvalidArgument);
	CHECK(CVPixelBufferCreate(NULL, 4, 4, kCVPixelFormatType_32BGRA, NULL, NULL) == kCVReturnInvalidArgument);
	CHECK(CVPixelBufferCreate(NULL, 4, 4, 'xxxx', NULL, &buffer) == kCVReturnInvalidPixelFormat);
	CHECK(buffer == NULL);
	CHECK(CVPixelBufferLockBaseAddress(NULL, 0) == kCVReturnInvalidArgument);

	// Pools
	const void* poolKeys[] = { kCVPixelBufferWidthKey, kCVPixelBufferHeightKey, kCVPixelBufferPixelFormatTypeKey };
	CFNumberRef width = number(16), height = number(8), format = number(kCVPixelFormatType_32BGRA);
	const void* poolValues[] = { width, height, format };
	CFDictionaryRef attributes = CFDictionaryCreate(NULL, poolKeys, poolValues, 3, &kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	CVPixelBufferPoolRef pool = NULL;
	CHECK(CVPixelBufferPoolCreate(NULL, NULL, attributes, &pool) == kCVReturnSuccess);
	CHECK(pool != NULL);
	CHECK(CFGetTypeID(pool) == CVPixelBufferPoolGetTypeID());
	CHECK(CVPixelBufferPoolCreatePixelBuffer(NULL, pool, &buffer) == kCVReturnSuccess);
	CHECK(CVPixelBufferGetWidth(buffer) == 16 && CVPixelBufferGetHeight(buffer) == 8);
	CHECK(CVPixelBufferGetPixelFormatType(buffer) == kCVPixelFormatType_32BGRA);
	CVPixelBufferRelease(buffer);
	CVPixelBufferPoolRelease(pool);
	CFRelease(attributes);
	CFRelease(width);
	CFRelease(height);
	CFRelease(format);

	if (g_failures)
	{
		printf("%d failures\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
