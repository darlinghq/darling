// Copyright (C) 2026 Lubos Dolezel
//
// The AVAudioTimePitchAlgorithm constants of AVAudioProcessingSettings.h are
// exported by AVFoundation with the macOS values (observed on macOS 26.7.3).
// A binary that imports them weakly and copies the string pointer at load
// time crashes on a null dereference when they are missing.
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#include <dlfcn.h>
#include <stdio.h>

static int g_failures = 0;

static void check(const char *name, NSString *expected)
{
	NSString *__unsafe_unretained *p = (NSString *__unsafe_unretained *) dlsym(RTLD_DEFAULT, name);
	if (p == NULL || *p == nil || ![*p isEqualToString:expected]) {
		printf("FAIL: %s = %s\n", name, p && *p ? [*p UTF8String] : "(missing)");
		g_failures++;
	}
}

int main(void)
{
	@autoreleasepool {
		check("AVAudioTimePitchAlgorithmTimeDomain", @"TimeDomain");
		check("AVAudioTimePitchAlgorithmSpectral", @"Spectral");
		check("AVAudioTimePitchAlgorithmVarispeed", @"Varispeed");
		if (![AVAudioTimePitchAlgorithmSpectral isEqualToString:@"Spectral"]) {
			printf("FAIL: header constant\n");
			g_failures++;
		}
	}
	printf(g_failures ? "FAILED\n" : "ALL PASSED\n");
	return g_failures ? 1 : 0;
}
