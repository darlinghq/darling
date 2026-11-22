// CFLAGS: -framework Foundation -framework AVFoundation
// Copyright (C) 2026 Darling developers
//
// AVAssetExportPresetAppleM4A and AVMetadataKeySpaceCommon are exported NSString constants. The
// documentation gives no string values, so this test pins none: it checks that each constant
// exists, is a non-empty NSString and differs from its documented siblings (the other export
// presets; the metadata keys, since the only metadata key space declared is AVMetadataKeySpaceCommon
// and a key space string equal to a key would be a mistake).
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
	}
	printf(g_failures ? "FAILED\n" : "OK\n");
	return g_failures ? 1 : 0;
}
