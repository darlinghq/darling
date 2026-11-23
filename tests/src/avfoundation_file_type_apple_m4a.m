// CFLAGS: -framework Foundation -framework AVFoundation
// Copyright (C) 2026 Lubos Dolezel
//
// AVFileTypeAppleM4A is the AVFileType UTI for the Apple m4a audio file format. The documentation of its
// Swift spelling, AVFileType.m4a, gives the value as com.apple.m4a-audio, so that value is pinned. It must also
// be non-empty and differ from the other file types declared here.
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		AVFileType m4a = AVFileTypeAppleM4A;
		CHECK(m4a != nil);
		CHECK([m4a isKindOfClass:[NSString class]]);
		CHECK([m4a length] > 0);
		CHECK([m4a isEqualToString:@"com.apple.m4a-audio"]);
		CHECK(![m4a isEqualToString:AVFileTypeMPEG4]);
		CHECK(![m4a isEqualToString:AVFileTypeQuickTimeMovie]);
	}

	printf(g_failures == 0 ? "PASS\n" : "FAIL\n");
	return g_failures != 0;
}
