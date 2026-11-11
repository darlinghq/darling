// CFLAGS: -framework CoreFoundation -framework VideoToolbox
// Copyright (C) 2026 Lubos Dolezel
//
// VideoToolbox public constants and the pixel rotation entry points. The
// expected strings are the values printed by the public constants on macOS 27
// (arm64), a black-box observation; the H.264 profile/level strings are the
// constant names without the kVTProfileLevel_ prefix. The pixel rotation
// functions use the documented prototypes; no session can be created here, so
// creation must fail with kVTCouldNotCreateInstanceErr and a NULL session.
// Prints PASS or FAIL lines, then ALL PASSED or FAILED n.
#include <CoreFoundation/CoreFoundation.h>
#include <VideoToolbox/VTCompressionProperties.h>
#include <VideoToolbox/VTErrors.h>
#include <VideoToolbox/VTPixelRotationProperties.h>
#include <VideoToolbox/VTPixelRotationSession.h>
#include <VideoToolbox/VTPixelTransferProperties.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check_string(const char *name, CFStringRef value, const char *expected)
{
	char buf[128] = "(null)";
	if (value != NULL)
		CFStringGetCString(value, buf, sizeof buf, kCFStringEncodingUTF8);
	if (strcmp(buf, expected) == 0) {
		printf("PASS: %s\n", name);
	} else {
		printf("FAIL: %s is \"%s\", expected \"%s\"\n", name, buf, expected);
		failures++;
	}
}

#define CHECK(c, name) check_string(#c, c, name)
#define CHECK_LEVEL(c) check_string(#c, c, &#c[sizeof("kVTProfileLevel_") - 1])

int main(void)
{
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_1_3);
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_3_0);
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_3_1);
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_3_2);
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_4_0);
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_4_1);
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_4_2);
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_5_0);
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_5_1);
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_5_2);
	CHECK_LEVEL(kVTProfileLevel_H264_Extended_5_0);
	CHECK_LEVEL(kVTProfileLevel_H264_High_3_0);
	CHECK_LEVEL(kVTProfileLevel_H264_High_3_1);
	CHECK_LEVEL(kVTProfileLevel_H264_High_3_2);
	CHECK_LEVEL(kVTProfileLevel_H264_High_4_0);
	CHECK_LEVEL(kVTProfileLevel_H264_High_4_1);
	CHECK_LEVEL(kVTProfileLevel_H264_High_4_2);
	CHECK_LEVEL(kVTProfileLevel_H264_High_5_0);
	CHECK_LEVEL(kVTProfileLevel_H264_High_5_1);
	CHECK_LEVEL(kVTProfileLevel_H264_High_5_2);
	CHECK_LEVEL(kVTProfileLevel_H264_Main_3_0);
	CHECK_LEVEL(kVTProfileLevel_H264_Main_3_1);
	CHECK_LEVEL(kVTProfileLevel_H264_Main_3_2);
	CHECK_LEVEL(kVTProfileLevel_H264_Main_4_0);
	CHECK_LEVEL(kVTProfileLevel_H264_Main_4_1);
	CHECK_LEVEL(kVTProfileLevel_H264_Main_4_2);
	CHECK_LEVEL(kVTProfileLevel_H264_Main_5_0);
	CHECK_LEVEL(kVTProfileLevel_H264_Main_5_1);
	CHECK_LEVEL(kVTProfileLevel_H264_Main_5_2);
	CHECK_LEVEL(kVTProfileLevel_H264_Baseline_AutoLevel);
	CHECK_LEVEL(kVTProfileLevel_H264_Main_AutoLevel);
	CHECK_LEVEL(kVTProfileLevel_H264_High_AutoLevel);
	CHECK(kVTEncodeFrameOptionKey_ForceKeyFrame, "EncoderForceKeyframe");
	CHECK(kVTCompressionPropertyKey_Quality, "Quality");
	CHECK(kVTCompressionPropertyKey_PixelAspectRatio, "PixelAspectRatio");
	CHECK(kVTCompressionPropertyKey_MoreFramesAfterEnd, "MoreFramesAfterEnd");
	CHECK(kVTCompressionPropertyKey_MoreFramesBeforeStart, "MoreFramesBeforeStart");
	CHECK(kVTCompressionPropertyKey_MaxH264SliceBytes, "MaxH264SliceBytes");

	CHECK(kVTRotation_0, "Rotation0");
	CHECK(kVTRotation_180, "Rotation180");
	CHECK(kVTRotation_CCW90, "RotationCCW90");
	CHECK(kVTRotation_CW90, "RotationCW90");
	CHECK(kVTPixelRotationPropertyKey_Rotation, "Rotation");
	CHECK(kVTPixelRotationPropertyKey_FlipHorizontalOrientation, "FlipHorizontalOrientation");
	CHECK(kVTPixelRotationPropertyKey_FlipVerticalOrientation, "FlipVerticalOrientation");

	CHECK(kVTPixelTransferPropertyKey_ScalingMode, "ScalingMode");
	CHECK(kVTScalingMode_Normal, "Normal");
	CHECK(kVTScalingMode_CropSourceToCleanAperture, "CropSourceToCleanAperture");
	CHECK(kVTScalingMode_Letterbox, "Letterbox");
	CHECK(kVTScalingMode_Trim, "Trim");
	CHECK(kVTPixelTransferPropertyKey_DestinationCleanAperture, "DestinationCleanAperture");
	CHECK(kVTPixelTransferPropertyKey_DestinationPixelAspectRatio, "DestinationPixelAspectRatio");
	CHECK(kVTPixelTransferPropertyKey_DownsamplingMode, "DownsamplingMode");
	CHECK(kVTDownsamplingMode_Decimate, "Decimate");
	CHECK(kVTDownsamplingMode_Average, "Average");
	CHECK(kVTPixelTransferPropertyKey_DestinationColorPrimaries, "DestinationColorPrimaries");
	CHECK(kVTPixelTransferPropertyKey_DestinationTransferFunction, "DestinationTransferFunction");
	CHECK(kVTPixelTransferPropertyKey_DestinationICCProfile, "DestinationICCProfile");
	CHECK(kVTPixelTransferPropertyKey_DestinationYCbCrMatrix, "DestinationYCbCrMatrix");
	CHECK(kVTPixelTransferPropertyKey_RealTime, "RealTime");

	if (kVTPixelRotationNotSupportedErr == -12914 && kVTInvalidSessionErr == -12903
	    && kVTParameterErr == -12902 && kVTCouldNotCreateInstanceErr == -12907) {
		printf("PASS: error code values\n");
	} else {
		printf("FAIL: error code values\n");
		failures++;
	}

	VTPixelRotationSessionRef session = (VTPixelRotationSessionRef) 1;
	OSStatus status = VTPixelRotationSessionCreate(NULL, &session);
	if (status == kVTCouldNotCreateInstanceErr && session == NULL) {
		printf("PASS: session creation fails cleanly\n");
	} else {
		printf("FAIL: VTPixelRotationSessionCreate returned %d, session %p\n", (int) status, (void *) session);
		failures++;
	}
	if (VTPixelRotationSessionCreate(NULL, NULL) == kVTParameterErr) {
		printf("PASS: NULL output pointer is a parameter error\n");
	} else {
		printf("FAIL: NULL output pointer\n");
		failures++;
	}
	if (VTPixelRotationSessionRotateImage(NULL, NULL, NULL) == kVTInvalidSessionErr) {
		printf("PASS: rotating without a session is an invalid session error\n");
	} else {
		printf("FAIL: rotating without a session\n");
		failures++;
	}
	VTPixelRotationSessionInvalidate(NULL);

	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
