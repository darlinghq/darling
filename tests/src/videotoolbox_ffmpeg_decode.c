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

#include <VideoToolbox/VideoToolbox.h>
#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		failures++; \
		printf("FAIL line %d: %s\n", __LINE__, #cond); \
	} \
} while (0)

#define UNKNOWN_CODEC 0x78787878 /* 'xxxx' */

static const uint8_t avcc_header[] = {
	0x01, 0x42, 0xc0, 0x0a, 0xff, 0xe1,
	0x00, 0x17, // SPS len 23
	0x67, 0x42, 0xc0, 0x0a, 0xd9, 0x04, 0x26, 0xc0, 0x44, 0x00, 0x00, 0x03,
	0x00, 0x04, 0x00, 0x00, 0x03, 0x00, 0xc8, 0x3c, 0x48, 0x99, 0x20,
	0x01, // 1 PPS
	0x00, 0x05, // PPS len 5
	0x68, 0xcb, 0x83, 0xcb, 0x20
};

static const uint8_t sample_h264_slice[] = {
	0x00, 0x00, 0x00, 0x1f, // 31 bytes length prefix
	0x65, 0x88, 0x84, 0x0a, 0xf1, 0x18, 0xa0, 0x00, 0x27, 0xa7, 0x1c, 0x00,
	0x04, 0xbe, 0x23, 0x80, 0x00, 0x82, 0xec, 0x9c, 0x9c, 0x9d, 0x75, 0xd7,
	0x5d, 0x75, 0xd7, 0x5d, 0x75, 0xd7, 0x5e
};

typedef struct CallbackState {
	int callCount;
	OSStatus lastStatus;
	VTDecodeInfoFlags lastFlags;
	void *lastRefCon;
	size_t lastWidth;
	size_t lastHeight;
	OSType lastPixelFormat;
	CMTime lastPTS;
} CallbackState;

static void testDecompressionCallback(void *refCon, void *frameRefCon, OSStatus status,
	VTDecodeInfoFlags flags, CVImageBufferRef image, CMTime pts, CMTime duration)
{
	CallbackState *state = (CallbackState *)refCon;
	if (!state) return;

	state->callCount++;
	state->lastStatus = status;
	state->lastFlags = flags;
	state->lastRefCon = frameRefCon;
	state->lastPTS = pts;

	if (image) {
		CVPixelBufferRef pixbuf = (CVPixelBufferRef)image;
		state->lastWidth = CVPixelBufferGetWidth(pixbuf);
		state->lastHeight = CVPixelBufferGetHeight(pixbuf);
		state->lastPixelFormat = CVPixelBufferGetPixelFormatType(pixbuf);
	} else {
		state->lastWidth = 0;
		state->lastHeight = 0;
		state->lastPixelFormat = 0;
	}
}

static VTDecompressionSessionRef gReSession;
static int gReCalls;

static void reentrantCallback(void *refCon, void *frameRefCon, OSStatus status,
	VTDecodeInfoFlags flags, CVImageBufferRef image, CMTime pts, CMTime duration)
{
	CFTypeRef value = NULL;
	if (VTSessionCopyProperty(gReSession, kVTDecompressionPropertyKey_RealTime, NULL, &value) == noErr) {
		gReCalls++;
		if (value)
			CFRelease(value);
	}
}

static CMSampleBufferRef createH264SampleBuffer(CMVideoFormatDescriptionRef format, const uint8_t *data, size_t size, CMTime pts)
{
	CMBlockBufferRef blockBuffer = NULL;
	OSStatus err = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault, (void *)data, size,
		kCFAllocatorNull, NULL, 0, size, 0, &blockBuffer);
	if (err != noErr || !blockBuffer)
		return NULL;

	CMSampleTimingInfo timing = {
		.duration = CMTimeMake(1, 25),
		.presentationTimeStamp = pts,
		.decodeTimeStamp = pts,
	};
	CMSampleBufferRef sampleBuffer = NULL;
	size_t sampleSize = size;
	err = CMSampleBufferCreate(kCFAllocatorDefault, blockBuffer, true, NULL, NULL, format, 1, 1, &timing, 1, &sampleSize, &sampleBuffer);
	CFRelease(blockBuffer);
	return (err == noErr) ? sampleBuffer : NULL;
}

int main(void)
{
	setbuf(stdout, NULL);
	printf("--- Running VideoToolbox ffmpeg decode tests ---\n");

	// Build format description with avcC extradata
	CFDataRef avccData = CFDataCreate(NULL, avcc_header, sizeof(avcc_header));
	const void *atomKeys[] = { CFSTR("avcC") };
	const void *atomVals[] = { avccData };
	CFDictionaryRef atoms = CFDictionaryCreate(NULL, atomKeys, atomVals, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	const void *extKeys[] = { kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms };
	const void *extVals[] = { atoms };
	CFDictionaryRef extensions = CFDictionaryCreate(NULL, extKeys, extVals, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CMVideoFormatDescriptionRef format64 = NULL;
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 64, 64, extensions, &format64) == noErr);
	CHECK(format64 != NULL);

	CFRelease(extensions);
	CFRelease(atoms);
	CFRelease(avccData);

	CallbackState cbState = { 0 };
	VTDecompressionOutputCallbackRecord callbackRecord = {
		.decompressionOutputCallback = testDecompressionCallback,
		.decompressionOutputRefCon = &cbState,
	};

	VTDecompressionSessionRef session = NULL;
	OSStatus createStatus = VTDecompressionSessionCreate(NULL, format64, NULL, NULL, &callbackRecord, &session);
	if (createStatus == kVTCouldNotFindVideoDecoderErr) {
		// The decoders come from the host's libavcodec, which is optional (and has no 32-bit wrapper
		// without a 32-bit host FFmpeg): without it a session is refused the way it is without a codec.
		printf("SKIP: no host libavcodec, sessions are refused with kVTCouldNotFindVideoDecoderErr\n");
		CFRelease(format64);
		return failures;
	}
	CHECK(createStatus == noErr);
	CHECK(session != NULL);
	CHECK(CFGetTypeID(session) == VTDecompressionSessionGetTypeID());

	// Error cases for creation
	VTDecompressionSessionRef dummySession = NULL;
	CHECK(VTDecompressionSessionCreate(NULL, NULL, NULL, NULL, &callbackRecord, &dummySession) == kVTParameterErr);
	CHECK(VTDecompressionSessionCreate(NULL, format64, NULL, NULL, &callbackRecord, NULL) == kVTParameterErr);

	CMVideoFormatDescriptionRef formatUnknown = NULL;
	CHECK(CMVideoFormatDescriptionCreate(NULL, UNKNOWN_CODEC, 64, 64, NULL, &formatUnknown) == noErr);
	CHECK(VTDecompressionSessionCreate(NULL, formatUnknown, NULL, NULL, &callbackRecord, &dummySession) == kVTCouldNotFindVideoDecoderErr);
	CFRelease(formatUnknown);

	// 2. CanAcceptFormatDescription
	CHECK(VTDecompressionSessionCanAcceptFormatDescription(session, format64) == true);
	CMVideoFormatDescriptionRef formatDiffRes = NULL;
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_H264, 128, 128, NULL, &formatDiffRes) == noErr);
	CHECK(VTDecompressionSessionCanAcceptFormatDescription(session, formatDiffRes) == false);
	CFRelease(formatDiffRes);

	CMVideoFormatDescriptionRef formatHevc = NULL;
	CHECK(CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_HEVC, 64, 64, NULL, &formatHevc) == noErr);
	CHECK(VTDecompressionSessionCanAcceptFormatDescription(session, formatHevc) == false);
	CFRelease(formatHevc);

	CHECK(VTDecompressionSessionCanAcceptFormatDescription(session, NULL) == false);
	CHECK(VTDecompressionSessionCanAcceptFormatDescription(NULL, format64) == false);

	// 3. Black pixel buffer
	CVPixelBufferRef blackBuf = NULL;
	CHECK(VTDecompressionSessionCopyBlackPixelBuffer(session, &blackBuf) == noErr);
	CHECK(blackBuf != NULL);
	if (blackBuf) {
		CHECK(CVPixelBufferGetWidth(blackBuf) == 64);
		CHECK(CVPixelBufferGetHeight(blackBuf) == 64);
		CHECK(CVPixelBufferGetPixelFormatType(blackBuf) == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
		CFRelease(blackBuf);
	}
	CHECK(VTDecompressionSessionCopyBlackPixelBuffer(session, NULL) == kVTParameterErr);

	// 4. Properties
	CFBooleanRef realTimeVal = NULL;
	CHECK(VTSessionCopyProperty(session, kVTDecompressionPropertyKey_RealTime, NULL, &realTimeVal) == noErr);
	CHECK(realTimeVal != NULL && CFBooleanGetValue(realTimeVal) == true);
	if (realTimeVal) CFRelease(realTimeVal);

	CHECK(VTSessionSetProperty(session, kVTDecompressionPropertyKey_RealTime, kCFBooleanFalse) == noErr);
	CHECK(VTSessionCopyProperty(session, kVTDecompressionPropertyKey_RealTime, NULL, &realTimeVal) == noErr);
	CHECK(realTimeVal != NULL && CFBooleanGetValue(realTimeVal) == false);
	if (realTimeVal) CFRelease(realTimeVal);

	CFBooleanRef hwVal = NULL;
	OSStatus hwErr = VTSessionCopyProperty(session, kVTDecompressionPropertyKey_UsingHardwareAcceleratedVideoDecoder, NULL, &hwVal);
	CHECK(hwErr == noErr || hwErr == kVTPropertyNotSupportedErr);
	if (hwErr == noErr && hwVal) {
		CFRelease(hwVal);
	}

	int threads = 2;
	CFNumberRef threadNum = CFNumberCreate(NULL, kCFNumberIntType, &threads);
	OSStatus tcErr = VTSessionSetProperty(session, kVTDecompressionPropertyKey_ThreadCount, threadNum);
	CHECK(tcErr == noErr || tcErr == kVTPropertyNotSupportedErr);
	CFRelease(threadNum);

	CFDictionaryRef supportedProps = NULL;
	CHECK(VTSessionCopySupportedPropertyDictionary(session, &supportedProps) == noErr);
	CHECK(supportedProps != NULL);
	if (supportedProps) {
		CHECK(CFDictionaryContainsKey(supportedProps, kVTDecompressionPropertyKey_RealTime));
		CFRelease(supportedProps);
	}

	// 5. Synchronous frame decoding
	CMSampleBufferRef sbuf1 = createH264SampleBuffer(format64, sample_h264_slice, sizeof(sample_h264_slice), CMTimeMake(10, 25));
	CHECK(sbuf1 != NULL);

	cbState.callCount = 0;
	VTDecodeInfoFlags infoFlags = 0;
	void *testRefCon = (void *)0x12345;
	CHECK(VTDecompressionSessionDecodeFrame(session, sbuf1, 0, testRefCon, &infoFlags) == noErr);
	CHECK(VTDecompressionSessionFinishDelayedFrames(session) == noErr);
	CHECK(cbState.callCount == 1);
	CHECK(cbState.lastStatus == noErr);
	CHECK(cbState.lastRefCon == testRefCon);
	CHECK(cbState.lastWidth == 64);
	CHECK(cbState.lastHeight == 64);
	CHECK(cbState.lastPTS.value == 10);

	// 6. Asynchronous frame decoding
	CMSampleBufferRef sbuf2 = createH264SampleBuffer(format64, sample_h264_slice, sizeof(sample_h264_slice), CMTimeMake(20, 25));
	CHECK(sbuf2 != NULL);

	cbState.callCount = 0;
	infoFlags = 0;
	void *testRefConAsync = (void *)0x6789a;
	CHECK(VTDecompressionSessionDecodeFrame(session, sbuf2, kVTDecodeFrame_EnableAsynchronousDecompression, testRefConAsync, &infoFlags) == noErr);

	CHECK(VTDecompressionSessionWaitForAsynchronousFrames(session) == noErr);
	CHECK(cbState.callCount == 1);
	CHECK(cbState.lastStatus == noErr);
	CHECK(cbState.lastRefCon == testRefConAsync);
	CHECK(cbState.lastPTS.value == 20);
	CFRelease(sbuf2);

	// 7. DecodeFrameWithOutputHandler
	CMSampleBufferRef sbuf3 = createH264SampleBuffer(format64, sample_h264_slice, sizeof(sample_h264_slice), CMTimeMake(30, 25));
	CHECK(sbuf3 != NULL);

	// Verify calling DecodeFrameWithOutputHandler on session with callback record returns kVTParameterErr
	CHECK(VTDecompressionSessionDecodeFrameWithOutputHandler(session, sbuf3, 0, &infoFlags,
		^(OSStatus status, VTDecodeInfoFlags flags, CVImageBufferRef imageBuffer, CMTime pts, CMTime duration) {}) == kVTParameterErr);

	// Create session without callback record for block handler decoding
	VTDecompressionSessionRef sessionBlock = NULL;
	CHECK(VTDecompressionSessionCreate(NULL, format64, NULL, NULL, NULL, &sessionBlock) == noErr);
	CHECK(sessionBlock != NULL);

	// Calling DecodeFrame on sessionBlock (without callback record) returns kVTParameterErr
	CHECK(VTDecompressionSessionDecodeFrame(sessionBlock, sbuf3, 0, NULL, &infoFlags) == kVTParameterErr);

	__block int blockCalled = 0;
	__block OSStatus blockStatus = -1;
	__block size_t blockW = 0;
	__block size_t blockH = 0;
	__block CMTime blockPTS = kCMTimeInvalid;

	CHECK(VTDecompressionSessionDecodeFrameWithOutputHandler(sessionBlock, sbuf3, 0, &infoFlags,
		^(OSStatus status, VTDecodeInfoFlags flags, CVImageBufferRef imageBuffer, CMTime pts, CMTime duration) {
			blockCalled++;
			blockStatus = status;
			blockPTS = pts;
			if (imageBuffer) {
				blockW = CVPixelBufferGetWidth((CVPixelBufferRef)imageBuffer);
				blockH = CVPixelBufferGetHeight((CVPixelBufferRef)imageBuffer);
			}
		}) == noErr);

	CHECK(VTDecompressionSessionFinishDelayedFrames(sessionBlock) == noErr);
	CHECK(blockCalled == 1);
	CHECK(blockStatus == noErr);
	CHECK(blockW == 64 && blockH == 64);
	CHECK(blockPTS.value == 30);
	CFRelease(sbuf3);

	VTDecompressionSessionInvalidate(sessionBlock);
	CFRelease(sessionBlock);

	// 8. Dropped frame request (kVTDecodeFrame_DoNotOutputFrame)
	CMSampleBufferRef sbuf4 = createH264SampleBuffer(format64, sample_h264_slice, sizeof(sample_h264_slice), CMTimeMake(40, 25));
	CHECK(sbuf4 != NULL);

	cbState.callCount = 0;
	infoFlags = 0;
	CHECK(VTDecompressionSessionDecodeFrame(session, sbuf4, kVTDecodeFrame_DoNotOutputFrame, NULL, &infoFlags) == noErr);
	CHECK(VTDecompressionSessionFinishDelayedFrames(session) == noErr);
	CHECK(cbState.callCount == 1);
	CHECK(cbState.lastStatus == noErr);
	CHECK(cbState.lastWidth == 0); // No image buffer emitted
	CFRelease(sbuf4);

	// 9. Error handling: corrupted data
	uint8_t badData[] = { 0x00, 0x00, 0x00, 0x05, 0x65, 0xde, 0xad, 0xbe, 0xef };
	CMSampleBufferRef sbufBad = createH264SampleBuffer(format64, badData, sizeof(badData), CMTimeMake(50, 25));
	CHECK(sbufBad != NULL);

	cbState.callCount = 0;
	CHECK(VTDecompressionSessionDecodeFrame(session, sbufBad, 0, NULL, &infoFlags) == noErr);
	CHECK(VTDecompressionSessionFinishDelayedFrames(session) == noErr);
	CHECK(cbState.callCount == 1);
	CHECK(cbState.lastStatus != noErr); // bad data status
	CHECK(cbState.lastWidth == 0);
	CFRelease(sbufBad);

	// NULL sample buffer error
	CHECK(VTDecompressionSessionDecodeFrame(session, NULL, 0, NULL, NULL) == kVTParameterErr);

	// Invalidation
	VTDecompressionSessionInvalidate(session);
	CHECK(VTDecompressionSessionDecodeFrame(session, sbuf1, 0, NULL, NULL) == kVTInvalidSessionErr);
	CHECK(VTDecompressionSessionFinishDelayedFrames(session) == kVTInvalidSessionErr);
	CHECK(VTDecompressionSessionWaitForAsynchronousFrames(session) == kVTInvalidSessionErr);
	CFRelease(sbuf1);

	CFRelease(session);

	// 10. Output callbacks may call back into the session.
	{
		VTDecompressionSessionRef reSession = NULL;
		VTDecompressionOutputCallbackRecord reRecord = {
			.decompressionOutputCallback = reentrantCallback,
			.decompressionOutputRefCon = NULL,
		};
		CHECK(VTDecompressionSessionCreate(NULL, format64, NULL, NULL, &reRecord, &reSession) == noErr);
		if (reSession) {
			gReSession = reSession;
			gReCalls = 0;
			CMSampleBufferRef sb = createH264SampleBuffer(format64, sample_h264_slice, sizeof(sample_h264_slice), CMTimeMake(1, 25));
			CHECK(VTDecompressionSessionDecodeFrame(reSession, sb, 0, NULL, NULL) == noErr);
			CHECK(VTDecompressionSessionFinishDelayedFrames(reSession) == noErr);
			CHECK(gReCalls == 1);
			CFRelease(sb);
			VTDecompressionSessionInvalidate(reSession);
			CFRelease(reSession);
		}
	}

	// 11. Releasing a session while frames are still queued for asynchronous decoding is safe.
	{
		VTDecompressionSessionRef relSession = NULL;
		CallbackState relState = { 0 };
		VTDecompressionOutputCallbackRecord relRecord = {
			.decompressionOutputCallback = testDecompressionCallback,
			.decompressionOutputRefCon = &relState,
		};
		CHECK(VTDecompressionSessionCreate(NULL, format64, NULL, NULL, &relRecord, &relSession) == noErr);
		if (relSession) {
			for (int i = 0; i < 4; i++) {
				CMSampleBufferRef sb = createH264SampleBuffer(format64, sample_h264_slice, sizeof(sample_h264_slice), CMTimeMake(i, 25));
				CHECK(VTDecompressionSessionDecodeFrame(relSession, sb, kVTDecodeFrame_EnableAsynchronousDecompression, NULL, NULL) == noErr);
				CFRelease(sb);
			}
			CFRelease(relSession);
			usleep(300000);
		}
	}

	// 12. Destination pixel formats: a list is taken in order of preference, one that cannot be produced is refused,
	// and packed output is filled in.
	{
		int unsupported = 0x32767579; /* '2vuy' */
		int bgra = kCVPixelFormatType_32BGRA;
		CFNumberRef nUnsupported = CFNumberCreate(NULL, kCFNumberIntType, &unsupported);
		CFNumberRef nBgra = CFNumberCreate(NULL, kCFNumberIntType, &bgra);

		const void *k[] = { kCVPixelBufferPixelFormatTypeKey };
		const void *v1[] = { nUnsupported };
		CFDictionaryRef onlyUnsupported = CFDictionaryCreate(NULL, k, v1, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		VTDecompressionSessionRef fmtSession = NULL;
		CHECK(VTDecompressionSessionCreate(NULL, format64, NULL, onlyUnsupported, &callbackRecord, &fmtSession) == kVTPixelTransferNotSupportedErr);
		CHECK(fmtSession == NULL);
		CFRelease(onlyUnsupported);

		const void *list[] = { nUnsupported, nBgra };
		CFArrayRef formats = CFArrayCreate(NULL, list, 2, &kCFTypeArrayCallBacks);
		const void *v2[] = { formats };
		CFDictionaryRef preference = CFDictionaryCreate(NULL, k, v2, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CallbackState fmtState = { 0 };
		VTDecompressionOutputCallbackRecord fmtRecord = {
			.decompressionOutputCallback = testDecompressionCallback,
			.decompressionOutputRefCon = &fmtState,
		};
		CHECK(VTDecompressionSessionCreate(NULL, format64, NULL, preference, &fmtRecord, &fmtSession) == noErr);
		if (fmtSession) {
			CMSampleBufferRef sb = createH264SampleBuffer(format64, sample_h264_slice, sizeof(sample_h264_slice), CMTimeMake(1, 25));
			CHECK(VTDecompressionSessionDecodeFrame(fmtSession, sb, 0, NULL, NULL) == noErr);
			CHECK(VTDecompressionSessionFinishDelayedFrames(fmtSession) == noErr);
			CHECK(fmtState.callCount == 1);
			CHECK(fmtState.lastStatus == noErr);
			CHECK(fmtState.lastPixelFormat == kCVPixelFormatType_32BGRA);
			CFRelease(sb);
			VTDecompressionSessionInvalidate(fmtSession);
			CFRelease(fmtSession);
		}
		CFRelease(preference);
		CFRelease(formats);
		CFRelease(nUnsupported);
		CFRelease(nBgra);
	}

	CFRelease(format64);

	if (failures == 0) {
		printf("ALL TESTS PASSED!\n");
	} else {
		printf("%d TESTS FAILED!\n", failures);
	}
	return failures;
}
