// CFLAGS: -framework Foundation -framework AVFoundation
// Copyright (C) 2026 Darling developers
//
// AVAssetExportPresetAppleM4A and AVMetadataKeySpaceCommon are exported NSString constants. The
// documentation gives no string values, so this test pins the values macOS 26.7.3 and 10.14.4
// report (black-box observation: the preset's symbol name and the four-character code "comn"),
// and checks that each constant is a non-empty NSString that differs from its documented siblings
// (the other export presets; the metadata keys, since a key space string equal to a key would be a
// mistake).
// Prints OK, or the failures with a non-zero exit status.
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

static int g_failures = 0;

static void checkConstant(const char *name, NSString *value, NSArray<NSString *> *siblings)
{
	if (value == nil || ![value isKindOfClass:[NSString class]] || [value length] == 0) {
		printf("FAIL: %s is nil, not an NSString or empty\n", name);
		g_failures++;
		return;
	}
	for (NSString *sibling in siblings) {
		if ([value isEqualToString:sibling]) {
			printf("FAIL: %s has the same value as a sibling constant (%s)\n", name, [sibling UTF8String]);
			g_failures++;
		}
	}
}

int main(void)
{
	@autoreleasepool {
		checkConstant("AVAssetExportPresetAppleM4A", AVAssetExportPresetAppleM4A, @[
			AVAssetExportPresetHighestQuality,
			AVAssetExportPresetMediumQuality,
			AVAssetExportPresetLowQuality,
		]);
		checkConstant("AVMetadataKeySpaceCommon", AVMetadataKeySpaceCommon, @[
			AVMetadataCommonKeyAlbumName,
			AVMetadataCommonKeyArtist,
			AVMetadataCommonKeyArtwork,
			AVMetadataCommonKeyDescription,
			AVMetadataCommonKeyTitle,
			AVMetadataID3MetadataKeyUserURL,
		]);
		if (![AVAssetExportPresetAppleM4A isEqualToString:@"AVAssetExportPresetAppleM4A"]) {
			printf("FAIL: AVAssetExportPresetAppleM4A is '%s'\n", [AVAssetExportPresetAppleM4A UTF8String]);
			g_failures++;
		}
		if (![AVMetadataKeySpaceCommon isEqualToString:@"comn"]) {
			printf("FAIL: AVMetadataKeySpaceCommon is '%s', expected 'comn'\n", [AVMetadataKeySpaceCommon UTF8String]);
			g_failures++;
		}
	}
	printf(g_failures ? "FAILED\n" : "OK\n");
	return g_failures ? 1 : 0;
}
