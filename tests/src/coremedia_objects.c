/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// CoreMedia's CF objects: format descriptions, block buffers and sample buffers. Expected values
// follow the documented contracts; the status codes and edge cases the documentation does not
// spell out were observed on macOS 10.14.4 and 26.7.3 (identical on both). The file also builds
// against the real SDK (clang -framework CoreMedia -framework CoreVideo -framework AudioToolbox).
// Prints PASS when everything holds; the exit code is the number of failed checks.

#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

static int allocations, frees, readyCalls;

static void *countingAlloc(void *refCon, size_t size) { allocations++; return calloc(1, size); }
static void countingFree(void *refCon, void *memory, size_t size) { frees++; free(memory); }
static OSStatus makeReady(CMSampleBufferRef sbuf, void *refCon) { readyCalls++; return noErr; }

static CFDictionaryRef avcExtensions(void)
{
	// avcC: profile bytes, a 4-byte NAL length, one 3-byte SPS (0x67 ..) and one 2-byte PPS (0x68 ..)
	static const uint8_t avcC[] = { 1, 0x64, 0, 0x1f, 0xff, 0xe1, 0, 3, 0x67, 1, 2, 1, 0, 2, 0x68, 9 };
	CFDataRef data = CFDataCreate(NULL, avcC, sizeof(avcC));
	CFStringRef atomKey = CFSTR("avcC");
	CFDictionaryRef atoms = CFDictionaryCreate(NULL, (const void **)&atomKey, (const void **)&data, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFStringRef key = kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms;
	CFDictionaryRef ext = CFDictionaryCreate(NULL, (const void **)&key, (const void **)&atoms, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFRelease(data);
	CFRelease(atoms);
	return ext;
}

static void test_video_format_description(void)
{
	CFDictionaryRef ext = avcExtensions();
	CMVideoFormatDescriptionRef a = NULL, b = NULL, c = NULL, plain = NULL;

	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 640, 480, ext, NULL)
		== kCMFormatDescriptionError_InvalidParameter);
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 640, 480, ext, &a) == noErr);
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 640, 480, ext, &b) == noErr);
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 640, 481, ext, &c) == noErr);
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 16, 16, NULL, &plain) == noErr);
	CHECK(a != NULL && b != NULL && c != NULL && plain != NULL);

	CHECK(CFGetTypeID(a) == CMFormatDescriptionGetTypeID());
	CHECK(CMFormatDescriptionGetMediaType(a) == kCMMediaType_Video);
	CHECK(CMFormatDescriptionGetMediaSubType(a) == kCMVideoCodecType_H264);
	CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions(a);
	CHECK(dims.width == 640 && dims.height == 480);

	// equality: the same parameters and extensions are equal; one differing dimension is not
	CHECK(CFEqual(a, a));
	CHECK(CFEqual(a, b));
	CHECK(!CFEqual(a, c));
	CHECK(!CFEqual(a, plain));
	CHECK(CMFormatDescriptionEqual(a, b));
	CHECK(!CMFormatDescriptionEqual(a, c));

	// extensions
	CHECK(CFDictionaryGetCount(CMFormatDescriptionGetExtensions(a)) == 1);
	CHECK(CFEqual(CMFormatDescriptionGetExtension(a, kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms),
		CFDictionaryGetValue(ext, kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms)));
	CHECK(CMFormatDescriptionGetExtension(a, CFSTR("NoSuchExtension")) == NULL);
	CHECK(CMFormatDescriptionGetExtensions(plain) == NULL);

	// the description owns a copy of the extensions
	CFRelease(ext);
	CHECK(CMFormatDescriptionGetExtension(a, kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms) != NULL);

	// H.264 parameter sets out of the avcC atom
	const uint8_t *set = NULL;
	size_t size = 0, count = 0;
	int nalHeader = 0;
	CHECK(CMVideoFormatDescriptionGetH264ParameterSetAtIndex(a, 0, &set, &size, &count, &nalHeader) == noErr);
	CHECK(size == 3 && count == 2 && nalHeader == 4 && set != NULL && set[0] == 0x67);
	CHECK(CMVideoFormatDescriptionGetH264ParameterSetAtIndex(a, 1, &set, &size, &count, &nalHeader) == noErr);
	CHECK(size == 2 && set != NULL && set[0] == 0x68);
	set = NULL;
	size = 77;
	count = 0;
	nalHeader = 0;
	CHECK(CMVideoFormatDescriptionGetH264ParameterSetAtIndex(a, 2, &set, &size, &count, &nalHeader)
		== kCMFormatDescriptionBridgeError_InvalidParameter);
	CHECK(set == NULL && size == 0 && count == 2 && nalHeader == 4);
	count = 0;
	nalHeader = 0;
	CHECK(CMVideoFormatDescriptionGetH264ParameterSetAtIndex(a, 0, NULL, NULL, &count, &nalHeader) == noErr);
	CHECK(count == 2 && nalHeader == 4);
	CHECK(CMVideoFormatDescriptionGetH264ParameterSetAtIndex(plain, 0, &set, &size, &count, &nalHeader)
		== kCMFormatDescriptionError_InvalidParameter);

	CFRelease(a);
	CFRelease(b);
	CFRelease(c);
	CFRelease(plain);
}

static void test_audio_format_description(void)
{
	AudioStreamBasicDescription asbd = { 44100, kAudioFormatLinearPCM,
		kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 8, 1, 8, 2, 32, 0 };
	AudioStreamBasicDescription other = asbd;
	other.mSampleRate = 48000;
	const uint8_t cookie[4] = { 1, 2, 3, 4 };
	CMAudioFormatDescriptionRef a = NULL, b = NULL, c = NULL;

	CHECK(CMAudioFormatDescriptionCreate(NULL, NULL, 0, NULL, 0, NULL, NULL, &a)
		== kCMFormatDescriptionError_InvalidParameter);
	CHECK(CMAudioFormatDescriptionCreate(NULL, &asbd, 0, NULL, sizeof(cookie), cookie, NULL, &a) == noErr);
	CHECK(CMAudioFormatDescriptionCreate(NULL, &asbd, 0, NULL, sizeof(cookie), cookie, NULL, &b) == noErr);
	CHECK(CMAudioFormatDescriptionCreate(NULL, &other, 0, NULL, 0, NULL, NULL, &c) == noErr);

	CHECK(CFGetTypeID(a) == CMFormatDescriptionGetTypeID());
	CHECK(CMFormatDescriptionGetMediaType(a) == kCMMediaType_Audio);
	CHECK(CMFormatDescriptionGetMediaSubType(a) == kAudioFormatLinearPCM);
	const AudioStreamBasicDescription *got = CMAudioFormatDescriptionGetStreamBasicDescription(a);
	CHECK(got != NULL && got->mSampleRate == 44100 && got->mChannelsPerFrame == 2);

	size_t size = 9;
	const void *magic = CMAudioFormatDescriptionGetMagicCookie(a, &size);
	CHECK(size == sizeof(cookie) && magic != NULL && magic != cookie && memcmp(magic, cookie, sizeof(cookie)) == 0);
	size = 9;
	CHECK(CMAudioFormatDescriptionGetChannelLayout(a, &size) == NULL && size == 0);

	CHECK(CFEqual(a, b));
	CHECK(!CFEqual(a, c));
	CHECK(CMFormatDescriptionEqual(a, b) && !CMFormatDescriptionEqual(a, c));

	// a video description has no stream description
	CMVideoFormatDescriptionRef video = NULL;
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 4, 4, NULL, &video) == noErr);
	CHECK(CMAudioFormatDescriptionGetStreamBasicDescription(video) == NULL);
	CHECK(!CFEqual(a, video));

	CFRelease(a);
	CFRelease(b);
	CFRelease(c);
	CFRelease(video);
}

static void test_quirk_audio_description_null_layout_with_size(void)
{
	// QUIRK pinned by CMAudioFormatDescriptionCreate: a layout size with a NULL layout is accepted.
	AudioStreamBasicDescription asbd = { 44100, kAudioFormatLinearPCM,
		kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 8, 1, 8, 2, 32, 0 };
	CMAudioFormatDescriptionRef a = NULL;
	CHECK(CMAudioFormatDescriptionCreate(NULL, &asbd, 8, NULL, 0, NULL, NULL, &a) == noErr);
	CHECK(a != NULL);
	if (a != NULL)
		CFRelease(a);
}

static void test_block_buffer(void)
{
	CMBlockBufferRef b = NULL;
	size_t lengthAtOffset = 99, total = 99;
	char *data = NULL;

	// documented argument errors
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 16, NULL, NULL, 0, 16, 0, NULL)
		== kCMBlockBufferBadPointerParameterErr);
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 16, NULL, NULL, 0, 17, 0, &b) == kCMBlockBufferBadLengthParameterErr);
	CHECK(b == NULL);
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 16, NULL, NULL, 17, 0, 0, &b) == kCMBlockBufferBadLengthParameterErr);
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 0, NULL, NULL, 0, 0, 0, &b) == kCMBlockBufferBadLengthParameterErr);
	CMBlockBufferCustomBlockSource bad = { 7, countingAlloc, countingFree, NULL };
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 16, NULL, &bad, 0, 16, 0, &b) == kCMBlockBufferBadCustomBlockSourceErr);
	CHECK(b == NULL);

	// memory that is not asked for is not allocated: the data cannot be reached yet
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 16, NULL, NULL, 2, 10, 0, &b) == noErr);
	CHECK(CFGetTypeID(b) == CMBlockBufferGetTypeID());
	CHECK(CMBlockBufferGetDataLength(b) == 10);
	CHECK(CMBlockBufferGetDataPointer(b, 0, &lengthAtOffset, &total, &data) == kCMBlockBufferUnallocatedBlockErr);
	CHECK(lengthAtOffset == 0 && total == 10 && data == NULL);
	CHECK(CMBlockBufferGetDataPointer(b, 10, NULL, NULL, &data) == kCMBlockBufferBadOffsetParameterErr);
	CHECK(!CMBlockBufferIsRangeContiguous(b, 0, 0));
	CHECK(CMBlockBufferAssureBlockMemory(b) == noErr);
	CHECK(CMBlockBufferGetDataPointer(b, 0, &lengthAtOffset, &total, &data) == noErr);
	CHECK(lengthAtOffset == 10 && total == 10 && data != NULL);
	char *start = data;
	CFRelease(b);

	// allocation at creation, data access with offsets
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 16, NULL, NULL, 2, 10, kCMBlockBufferAssureMemoryNowFlag, &b) == noErr);
	CHECK(CMBlockBufferGetDataPointer(b, 0, &lengthAtOffset, &total, &data) == noErr);
	CHECK(lengthAtOffset == 10 && total == 10 && data != NULL);
	start = data;
	CHECK(CMBlockBufferGetDataPointer(b, 4, &lengthAtOffset, &total, &data) == noErr);
	CHECK(lengthAtOffset == 6 && total == 10 && data == start + 4);
	CHECK(CMBlockBufferGetDataPointer(b, 10, &lengthAtOffset, &total, &data) == kCMBlockBufferBadOffsetParameterErr);
	CHECK(CMBlockBufferGetDataPointer(b, 11, &lengthAtOffset, &total, &data) == kCMBlockBufferBadOffsetParameterErr);
	CHECK(CMBlockBufferGetDataPointer(b, 0, NULL, NULL, NULL) == noErr);

	// the contiguous check: ranges inside the data
	CHECK(CMBlockBufferIsRangeContiguous(b, 0, 10));
	CHECK(CMBlockBufferIsRangeContiguous(b, 0, 0));
	CHECK(CMBlockBufferIsRangeContiguous(b, 4, 6));
	CHECK(!CMBlockBufferIsRangeContiguous(b, 0, 11));
	CHECK(!CMBlockBufferIsRangeContiguous(b, 10, 1));
	CHECK(!CMBlockBufferIsEmpty(b));

	// copy and fill, bounded by the data length (the 2 bytes before the data are not part of it)
	memset(start, 7, 10);
	char copy[10] = { 0 };
	CHECK(CMBlockBufferCopyDataBytes(b, 2, 4, copy) == noErr && copy[0] == 7 && copy[3] == 7);
	CHECK(CMBlockBufferCopyDataBytes(b, 8, 4, copy) == kCMBlockBufferBadLengthParameterErr);
	CHECK(CMBlockBufferFillDataBytes(0x55, b, 2, 4) == noErr);
	CHECK(start[1] == 7 && start[2] == 0x55 && start[5] == 0x55 && start[6] == 7);
	CHECK(CMBlockBufferFillDataBytes(0x55, b, 8, 4) == kCMBlockBufferBadLengthParameterErr);
	CFRelease(b);

	// a custom source allocates lazily and frees the block exactly once, when the buffer goes away
	CMBlockBufferCustomBlockSource source = { kCMBlockBufferCustomBlockSourceVersion, countingAlloc, countingFree, NULL };
	allocations = frees = 0;
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 32, NULL, &source, 0, 32, 0, &b) == noErr);
	CHECK(allocations == 0 && frees == 0);
	CHECK(CMBlockBufferGetDataPointer(b, 0, NULL, NULL, &data) == kCMBlockBufferUnallocatedBlockErr);
	CHECK(CMBlockBufferAssureBlockMemory(b) == noErr && allocations == 1);
	CHECK(CMBlockBufferAssureBlockMemory(b) == noErr && allocations == 1);
	CFRetain(b);
	CFRelease(b);
	CHECK(frees == 0);
	CFRelease(b);
	CHECK(frees == 1);

	// memory the caller supplies is used in place and freed by the block allocator
	char *mem = calloc(1, 8);
	mem[3] = 'x';
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, mem, 8, kCFAllocatorMalloc, NULL, 0, 8, 0, &b) == noErr);
	CHECK(CMBlockBufferGetDataPointer(b, 0, NULL, NULL, &data) == noErr && data == mem);
	CFRelease(b);

	// memory the caller keeps: kCFAllocatorNull frees nothing
	char own[8] = { 0 };
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, own, 8, kCFAllocatorNull, NULL, 0, 8, 0, &b) == noErr);
	CFRelease(b);

	CHECK(CMBlockBufferGetDataPointer(NULL, 0, NULL, NULL, &data) == kCMBlockBufferBadPointerParameterErr);
}

static CMSampleTimingInfo timing(int64_t index)
{
	CMSampleTimingInfo t = { CMTimeMake(1, 30), CMTimeMake(index, 30), CMTimeMake(index - 1, 30) };
	return t;
}

static void test_sample_buffer(void)
{
	CMVideoFormatDescriptionRef format = NULL;
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 64, 32, NULL, &format) == noErr);
	CMBlockBufferRef block = NULL;
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 8, NULL, NULL, 0, 8, kCMBlockBufferAssureMemoryNowFlag, &block) == noErr);
	CMSampleTimingInfo times[3] = { timing(0), timing(1), timing(2) };
	size_t sizes[3] = { 2, 3, 3 };
	CMSampleBufferRef sb = NULL;

	CHECK(CMSampleBufferCreate(NULL, block, true, NULL, NULL, format, 3, 3, times, 3, sizes, NULL)
		== kCMSampleBufferError_RequiredParameterMissing);
	CHECK(CMSampleBufferCreate(NULL, block, true, NULL, NULL, format, 3, 3, times, 3, sizes, &sb) == noErr);
	CHECK(CFGetTypeID(sb) == CMSampleBufferGetTypeID());
	CHECK(CMSampleBufferGetNumSamples(sb) == 3);
	CHECK(CMSampleBufferGetDataBuffer(sb) == block);
	CHECK(CMSampleBufferGetFormatDescription(sb) == format);
	CHECK(CMSampleBufferDataIsReady(sb));
	CMTime t = CMSampleBufferGetPresentationTimeStamp(sb);
	CHECK(t.value == 0 && t.timescale == 30 && CMTIME_IS_VALID(t));
	t = CMSampleBufferGetDecodeTimeStamp(sb);
	CHECK(t.value == -1 && t.timescale == 30);
	t = CMSampleBufferGetDuration(sb);
	CHECK(CMTIME_IS_VALID(t) && CMTimeCompare(t, CMTimeMake(3, 30)) == 0);

	// the buffer holds the block and the format description; releasing ours keeps them alive
	CFRelease(block);
	CFRelease(format);
	CHECK(CMBlockBufferGetDataLength(CMSampleBufferGetDataBuffer(sb)) == 8);
	CHECK(CMVideoFormatDescriptionGetDimensions(CMSampleBufferGetFormatDescription(sb)).width == 64);

	// attachments: none until asked for, then one dictionary per sample, and the same array afterwards
	CHECK(CMSampleBufferGetSampleAttachmentsArray(sb, false) == NULL);
	CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sb, true);
	CHECK(attachments != NULL && CFArrayGetCount(attachments) == 3);
	CHECK(CMSampleBufferGetSampleAttachmentsArray(sb, false) == attachments);
	CFMutableDictionaryRef second = (CFMutableDictionaryRef)CFArrayGetValueAtIndex(attachments, 1);
	CFDictionarySetValue(second, kCMSampleAttachmentKey_NotSync, kCFBooleanTrue);
	CHECK(CFDictionaryGetCount(second) == 1);
	CHECK(CFDictionaryGetCount(CFArrayGetValueAtIndex(attachments, 0)) == 0);

	CFRelease(sb);
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 8, NULL, NULL, 0, 8, kCMBlockBufferAssureMemoryNowFlag, &block) == noErr);
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 64, 32, NULL, &format) == noErr);

	// one timing entry for all samples: the duration is the entry's times the sample count
	CHECK(CMSampleBufferCreate(NULL, block, true, NULL, NULL, format, 3, 1, times, 3, sizes, &sb) == noErr);
	t = CMSampleBufferGetDuration(sb);
	CHECK(CMTimeCompare(t, CMTimeMake(3, 30)) == 0);
	t = CMSampleBufferGetPresentationTimeStamp(sb);
	CHECK(t.value == 0 && t.timescale == 30);
	CFRelease(sb);

	// no timing: invalid presentation time, zero duration
	CHECK(CMSampleBufferCreate(NULL, block, true, NULL, NULL, format, 1, 0, NULL, 1, sizes, &sb) == noErr);
	CHECK(CMTIME_IS_INVALID(CMSampleBufferGetPresentationTimeStamp(sb)));
	t = CMSampleBufferGetDuration(sb);
	CHECK(CMTIME_IS_VALID(t) && t.value == 0);
	CFRelease(sb);

	// entry counts must be 1 or the sample count
	sb = NULL;
	CHECK(CMSampleBufferCreate(NULL, block, true, NULL, NULL, format, 3, 2, times, 3, sizes, &sb) == kCMSampleBufferError_InvalidEntryCount);
	CHECK(CMSampleBufferCreate(NULL, block, true, NULL, NULL, format, 3, 1, times, 2, sizes, &sb) == kCMSampleBufferError_InvalidEntryCount);
	CHECK(sb == NULL);

	// a buffer that is not ready is not asked at creation; MakeDataReady asks once
	readyCalls = 0;
	CHECK(CMSampleBufferCreate(NULL, block, false, makeReady, NULL, format, 1, 1, times, 1, sizes, &sb) == noErr);
	CHECK(readyCalls == 0 && !CMSampleBufferDataIsReady(sb));
	CHECK(CMSampleBufferMakeDataReady(sb) == noErr);
	CHECK(readyCalls == 1 && CMSampleBufferDataIsReady(sb));
	CHECK(CMSampleBufferMakeDataReady(sb) == noErr && readyCalls == 1);
	CFRelease(sb);

	CFRelease(block);
	CFRelease(format);
}

static OSStatus collectSample(CMSampleBufferRef sample, CMItemCount index, void *refcon)
{
	CMTime *seen = refcon;
	seen[index] = CMSampleBufferGetPresentationTimeStamp(sample);
	CHECK(CMSampleBufferGetNumSamples(sample) == 1);
	CHECK(CMBlockBufferGetDataLength(CMSampleBufferGetDataBuffer(sample)) == (index == 0 ? 2 : 3));
	return noErr;
}

static void test_sample_buffer_for_each_sample(void)
{
	CMBlockBufferRef block = NULL;
	CHECK(CMBlockBufferCreateWithMemoryBlock(NULL, NULL, 8, NULL, NULL, 0, 8, kCMBlockBufferAssureMemoryNowFlag, &block) == noErr);
	CMSampleTimingInfo one = timing(10);
	size_t sizes[3] = { 2, 3, 3 };
	CMSampleBufferRef sb = NULL;
	CHECK(CMSampleBufferCreate(NULL, block, true, NULL, NULL, NULL, 3, 1, &one, 3, sizes, &sb) == noErr);
	CFRelease(block);
	CMTime seen[3];
	memset(seen, 0, sizeof(seen));
	CHECK(CMSampleBufferCallForEachSample(sb, collectSample, seen) == noErr);
	CHECK(seen[0].value == 10 && seen[1].value == 11 && seen[2].value == 12 && seen[2].timescale == 30);
	CHECK(CMSampleBufferCallForEachSample(NULL, collectSample, seen) == kCMSampleBufferError_RequiredParameterMissing);
	CFRelease(sb);
}

static void test_image_buffer(void)
{
	CVPixelBufferRef pixels = NULL;
	CHECK(CVPixelBufferCreate(NULL, 64, 32, kCVPixelFormatType_32BGRA, NULL, &pixels) == kCVReturnSuccess);
	CMVideoFormatDescriptionRef format = NULL;
	CHECK(CMVideoFormatDescriptionCreateForImageBuffer(NULL, pixels, &format) == noErr);
	CHECK(CMFormatDescriptionGetMediaSubType(format) == kCVPixelFormatType_32BGRA);
	CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions(format);
	CHECK(dims.width == 64 && dims.height == 32);
	CHECK(CMVideoFormatDescriptionMatchesImageBuffer(format, pixels));

	CMVideoFormatDescriptionRef other = NULL;
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 64, 32, NULL, &other) == noErr);
	CHECK(!CMVideoFormatDescriptionMatchesImageBuffer(other, pixels));

	CMSampleTimingInfo one = { CMTimeMake(1, 30), CMTimeMake(5, 30), kCMTimeInvalid };
	CMSampleBufferRef sb = NULL, rejected = NULL;
	CFIndex before = CFGetRetainCount(pixels);
	CHECK(CMSampleBufferCreateReadyWithImageBuffer(NULL, pixels, format, &one, &sb) == noErr);
	CHECK(CMSampleBufferGetImageBuffer(sb) == pixels);
	CHECK(CMSampleBufferGetNumSamples(sb) == 1);
	CHECK(CMSampleBufferGetDataBuffer(sb) == NULL);
	CHECK(CMSampleBufferDataIsReady(sb));
	CHECK(CMTimeCompare(CMSampleBufferGetPresentationTimeStamp(sb), CMTimeMake(5, 30)) == 0);
	CHECK(CFGetRetainCount(pixels) == before + 1);

	CHECK(CMSampleBufferCreateReadyWithImageBuffer(NULL, pixels, other, &one, &rejected) == kCMSampleBufferError_InvalidMediaFormat);
	CHECK(CMSampleBufferCreateReadyWithImageBuffer(NULL, pixels, format, NULL, &rejected) == kCMSampleBufferError_RequiredParameterMissing);
	CHECK(CMSampleBufferCreateReadyWithImageBuffer(NULL, NULL, format, &one, &rejected) == kCMSampleBufferError_RequiredParameterMissing);
	CHECK(rejected == NULL);

	CFRelease(sb);
	CHECK(CFGetRetainCount(pixels) == before);
	CFRelease(other);
	CFRelease(format);
	CFRelease(pixels);
}

int main(void)
{
	test_video_format_description();
	test_audio_format_description();
	test_quirk_audio_description_null_layout_with_size();
	test_block_buffer();
	test_sample_buffer();
	test_sample_buffer_for_each_sample();
	test_image_buffer();
	if (failures == 0)
		printf("PASS\n");
	return failures;
}
