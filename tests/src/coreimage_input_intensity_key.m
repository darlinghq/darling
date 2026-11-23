// CFLAGS: -framework Foundation -framework CoreImage
// Copyright (C) 2026 Darling developers
//
// kCIInputIntensityKey is an exported NSString constant. The documentation gives no string value,
// so this test pins the value macOS 26.7.3 and 10.14.4 report, "inputIntensity" (black-box
// observation), and checks that the constant differs from the other CIFilter input keys (they are
// used as dictionary keys, so a collision would be a bug).
// Prints OK, or the failures with a non-zero exit status.
#import <CoreImage/CoreImage.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

int main(void)
{
	int failures = 0;
	@autoreleasepool {
		NSString *value = kCIInputIntensityKey;
		if (value == nil || ![value isKindOfClass:[NSString class]] || [value length] == 0) {
			printf("FAIL: kCIInputIntensityKey is nil, not an NSString or empty\n");
			failures++;
		} else {
			if (![value isEqualToString:@"inputIntensity"]) {
				printf("FAIL: kCIInputIntensityKey is '%s', expected 'inputIntensity'\n", [value UTF8String]);
				failures++;
			}
			NSArray<NSString *> *siblings = @[
				kCIInputAngleKey,
				kCIInputBackgroundImageKey,
				kCIInputBrightnessKey,
				kCIInputColorKey,
				kCIInputContrastKey,
				kCIInputEVKey,
				kCIInputExtentKey,
				kCIInputImageKey,
				kCIInputRadiusKey,
				kCIInputSaturationKey,
			];
			for (NSString *sibling in siblings) {
				if ([value isEqualToString:sibling]) {
					printf("FAIL: kCIInputIntensityKey has the same value as another input key (%s)\n", [sibling UTF8String]);
					failures++;
				}
			}
		}
	}
	printf(failures ? "FAILED\n" : "OK\n");
	return failures ? 1 : 0;
}
