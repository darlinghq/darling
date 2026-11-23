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

// The CMAttachment functions (CMSetAttachment and friends), on a CMBlockBuffer, a CMSampleBuffer and a
// CVPixelBuffer. Expected values follow the documented behaviour: one mode per key, retain on set and
// release on removal, a snapshot from CMCopyDictionaryOfAttachments, propagation of the propagating
// attachments only, and equivalence with the CVBuffer functions on a CVBuffer. Prints PASS when
// everything holds; the exit code is the number of failed checks.

#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

// `storeBacked` is true for the objects that keep their own attachment store (CMBlockBuffer, CMSampleBuffer).
static void test_bearer(CMAttachmentBearerRef a, CMAttachmentBearerRef b, bool storeBacked)
{
	CFStringRef keyA = CFSTR("test.attachment.a"), keyB = CFSTR("test.attachment.b");
	CFMutableArrayRef value = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	CMAttachmentMode mode = 99;

	CHECK(kCMAttachmentMode_ShouldNotPropagate != kCMAttachmentMode_ShouldPropagate);
	CHECK(CMGetAttachment(a, keyA, &mode) == NULL);
	// QUIRK test: no attachments gives NULL on a store-backed bearer (macOS 26.7.3), an empty dictionary on a CVBuffer
	CFDictionaryRef none = CMCopyDictionaryOfAttachments(NULL, a, kCMAttachmentMode_ShouldPropagate);
	CHECK(storeBacked ? none == NULL : (none != NULL && CFDictionaryGetCount(none) == 0));
	if (none != NULL)
		CFRelease(none);

	// QUIRK test: an invalid mode sets nothing, on every bearer (macOS 26.7.3)
	CMSetAttachment(a, keyA, value, 2);
	CMSetAttachment(a, keyA, value, -1);
	CHECK(CMGetAttachment(a, keyA, &mode) == NULL && mode == 99);

	// set retains; get reports the mode
	CFIndex base = CFGetRetainCount(value);
	CMSetAttachment(a, keyA, value, kCMAttachmentMode_ShouldPropagate);
	CHECK(CFGetRetainCount(value) == base + 1);
	CHECK(CMGetAttachment(a, keyA, &mode) == value && mode == kCMAttachmentMode_ShouldPropagate);
	CHECK(CMGetAttachment(a, keyA, NULL) == value);

	// a key exists in one mode at a time
	CMSetAttachment(a, keyA, value, kCMAttachmentMode_ShouldNotPropagate);
	CHECK(CMGetAttachment(a, keyA, &mode) == value && mode == kCMAttachmentMode_ShouldNotPropagate);
	CFDictionaryRef propagating = CMCopyDictionaryOfAttachments(NULL, a, kCMAttachmentMode_ShouldPropagate);
	CFDictionaryRef local = CMCopyDictionaryOfAttachments(NULL, a, kCMAttachmentMode_ShouldNotPropagate);
	CHECK(propagating != NULL && CFDictionaryGetCount(propagating) == 0);
	CHECK(local != NULL && CFDictionaryGetCount(local) == 1 && CFDictionaryGetValue(local, keyA) == value);
	CFRelease(propagating);
	CFRelease(local);

	// propagate copies the propagating attachments only
	CMSetAttachment(a, keyB, value, kCMAttachmentMode_ShouldPropagate);
	CMPropagateAttachments(a, b);
	CHECK(CMGetAttachment(b, keyB, &mode) == value && mode == kCMAttachmentMode_ShouldPropagate);
	CHECK(CMGetAttachment(b, keyA, NULL) == NULL);

	// a dictionary sets one attachment per pair
	CFDictionaryRef pairs = CFDictionaryCreate(NULL, (const void **)&keyA, (const void **)&value, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CMSetAttachments(b, pairs, kCMAttachmentMode_ShouldNotPropagate);
	CHECK(CMGetAttachment(b, keyA, &mode) == value && mode == kCMAttachmentMode_ShouldNotPropagate);
	CFRelease(pairs);

	// removal releases
	CMRemoveAttachment(a, keyA);
	CHECK(CMGetAttachment(a, keyA, NULL) == NULL && CMGetAttachment(a, keyB, NULL) == value);
	CMRemoveAllAttachments(a);
	CMRemoveAllAttachments(b);
	CHECK(CMGetAttachment(a, keyB, NULL) == NULL && CMGetAttachment(b, keyA, NULL) == NULL);
	CFDictionaryRef afterRemoveAll = CMCopyDictionaryOfAttachments(NULL, a, kCMAttachmentMode_ShouldPropagate);
	CHECK(storeBacked ? afterRemoveAll == NULL : (afterRemoveAll != NULL && CFDictionaryGetCount(afterRemoveAll) == 0));
	if (afterRemoveAll != NULL)
		CFRelease(afterRemoveAll);
	CHECK(CFGetRetainCount(value) == base);

	// a value the caller does not own (what CMGetAttachment returned) can be set in the other mode: the
	// move must not free it first
	CFMutableArrayRef owned = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	CMSetAttachment(a, keyA, owned, kCMAttachmentMode_ShouldPropagate);
	CFRelease(owned); // the bearer is now the only owner
	CFTypeRef borrowed = CMGetAttachment(a, keyA, NULL);
	CMSetAttachment(a, keyA, borrowed, kCMAttachmentMode_ShouldNotPropagate);
	CHECK(CMGetAttachment(a, keyA, &mode) == borrowed && mode == kCMAttachmentMode_ShouldNotPropagate);
	CHECK(CFGetRetainCount(borrowed) == 1);
	CMRemoveAllAttachments(a);

	CFRelease(value);
}

int main(void)
{
	CMBlockBufferRef block1 = NULL, block2 = NULL;
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 16, NULL, NULL, 0, 16, 0, &block1) == noErr);
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 16, NULL, NULL, 0, 16, 0, &block2) == noErr);
	test_bearer(block1, block2, true);

	CMSampleBufferRef sample1 = NULL, sample2 = NULL;
	CHECK(CMSampleBufferCreateReady(NULL, block1, NULL, 0, 0, NULL, 0, NULL, &sample1) == noErr);
	CHECK(CMSampleBufferCreateReady(NULL, block2, NULL, 0, 0, NULL, 0, NULL, &sample2) == noErr);
	test_bearer(sample1, sample2, true);

	// on a CVBuffer they are the CVBuffer attachment functions
	CVPixelBufferRef pixels1 = NULL, pixels2 = NULL;
	CHECK(CVPixelBufferCreate(NULL, 4, 4, kCVPixelFormatType_32BGRA, NULL, &pixels1) == kCVReturnSuccess);
	CHECK(CVPixelBufferCreate(NULL, 4, 4, kCVPixelFormatType_32BGRA, NULL, &pixels2) == kCVReturnSuccess);
	test_bearer(pixels1, pixels2, false);
	CMSetAttachment(pixels1, CFSTR("test.attachment.cv"), CFSTR("value"), kCMAttachmentMode_ShouldPropagate);
	CFTypeRef found = CVBufferGetAttachment(pixels1, CFSTR("test.attachment.cv"), NULL);
	CHECK(found != NULL && CFEqual(found, CFSTR("value")));
	CVBufferSetAttachment(pixels2, CFSTR("test.attachment.cv2"), CFSTR("value2"), kCVAttachmentMode_ShouldNotPropagate);
	found = CMGetAttachment(pixels2, CFSTR("test.attachment.cv2"), NULL);
	CHECK(found != NULL && CFEqual(found, CFSTR("value2")));

	// an invalid mode has no attachments to copy; a NULL value and an object that bears nothing change nothing
	CHECK(CMCopyDictionaryOfAttachments(NULL, block1, 2) == NULL);
	CMSetAttachment(block1, CFSTR("test.attachment.null"), NULL, kCMAttachmentMode_ShouldPropagate);
	CHECK(CMGetAttachment(block1, CFSTR("test.attachment.null"), NULL) == NULL);
	CFStringRef notBearer = CFStringCreateWithCString(NULL, "text", kCFStringEncodingUTF8);
	CMSetAttachment(notBearer, CFSTR("test.attachment.a"), notBearer, kCMAttachmentMode_ShouldPropagate);
	CHECK(CMGetAttachment(notBearer, CFSTR("test.attachment.a"), NULL) == NULL);
	CHECK(CMCopyDictionaryOfAttachments(NULL, notBearer, kCMAttachmentMode_ShouldPropagate) == NULL);
	CFRelease(notBearer);

	CFRelease(pixels1);
	CFRelease(pixels2);
	CFRelease(sample1);
	CFRelease(sample2);
	CFRelease(block1);
	CFRelease(block2);

	printf(failures ? "FAILED\n" : "PASS\n");
	return failures;
}
