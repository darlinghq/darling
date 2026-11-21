// Copyright (C) 2026 Lubos Dolezel
//
// VideoToolbox's kVTCompressionPropertyKey_MasteringDisplayColorVolume and
// kVTProfileLevel_HEVC_Main10_AutoLevel have the values the SDK's constants
// have, and setting them through VTSessionSetProperty reports through the
// property path what the session can do: where a session can be created, a
// 24-byte CFData is accepted and a wrongly sized or typed value is a parameter
// error; where there is no encoder (no session exists), the call fails with an
// error instead of succeeding silently. The values and results are those
// observed on macOS 26.7.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreMedia/CoreMedia.h>
#include <VideoToolbox/VTCompressionProperties.h>
#include <VideoToolbox/VTCompressionSession.h>
#include <VideoToolbox/VTErrors.h>
#include <VideoToolbox/VTSession.h>
#include <stdio.h>
#include <string.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	CHECK(CFEqual(kVTCompressionPropertyKey_MasteringDisplayColorVolume, CFSTR("MasteringDisplayColorVolume")));
	CHECK(CFEqual(kVTProfileLevel_HEVC_Main10_AutoLevel, CFSTR("HEVC_Main10_AutoLevel")));
	CHECK(!CFEqual(kVTProfileLevel_HEVC_Main10_AutoLevel, kVTProfileLevel_HEVC_Main_AutoLevel));

	UInt8 volume[24];
	memset(volume, 0, sizeof(volume));
	CFDataRef data = CFDataCreate(kCFAllocatorDefault, volume, sizeof(volume));

	// Without a session the parameter is invalid.
	CHECK(VTSessionSetProperty(NULL, kVTCompressionPropertyKey_MasteringDisplayColorVolume, data) == kVTParameterErr);

	VTCompressionSessionRef session = NULL;
	OSStatus status = VTCompressionSessionCreate(kCFAllocatorDefault, 64, 64, kCMVideoCodecType_HEVC, NULL, NULL,
		kCFAllocatorDefault, NULL, NULL, &session);
	if (status == noErr && session != NULL) {
		CHECK(VTSessionSetProperty(session, kVTCompressionPropertyKey_MasteringDisplayColorVolume, data) == noErr);
		CHECK(VTSessionSetProperty(session, kVTCompressionPropertyKey_ProfileLevel, kVTProfileLevel_HEVC_Main10_AutoLevel) == noErr);
		CFDataRef shortData = CFDataCreate(kCFAllocatorDefault, volume, 5);
		CHECK(VTSessionSetProperty(session, kVTCompressionPropertyKey_MasteringDisplayColorVolume, shortData) == kVTParameterErr);
		CFRelease(shortData);
		CHECK(VTSessionSetProperty(session, kVTCompressionPropertyKey_MasteringDisplayColorVolume, CFSTR("x")) == kVTParameterErr);
		CHECK(VTSessionSetProperty(session, CFSTR("NoSuchProperty"), data) == kVTPropertyNotSupportedErr);
		VTCompressionSessionInvalidate(session);
		CFRelease(session);
	} else {
		// No encoder: creation fails, and the property cannot be accepted.
		CHECK(status != noErr);
		CHECK(session == NULL);
	}
	CFRelease(data);

	printf(g_failures ? "FAILED\n" : "PASS\n");
	return g_failures ? 1 : 0;
}
