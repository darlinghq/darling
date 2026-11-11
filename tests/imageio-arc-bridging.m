// Copyright (C) 2026 Lubos Dolezel
//
// ImageIO image-source declarations are audited for ARC bridging: borrowed
// NSURL and NSDictionary values pass through their CF types without explicit
// ownership annotations, and Create/Copy results are owned by the caller.
//   clang -fobjc-arc -framework Foundation -framework ImageIO tests/imageio-arc-bridging.m -o /tmp/imageio-arc-bridging
// Compilation failing under ARC is the failure mode.
#import <Foundation/Foundation.h>
#import <ImageIO/ImageIO.h>
#include <stdio.h>

static CGImageSourceRef createSource(NSURL *url, NSDictionary *options) {
	return CGImageSourceCreateWithURL((__bridge CFURLRef)url, (__bridge CFDictionaryRef)options);
}

int main(void) {
	CGImageSourceRef source = createSource([NSURL fileURLWithPath:@"/nonexistent/imageio-arc-bridging.png"], @{});
	if (source) {
		printf("FAIL: a source was created for a missing file\n");
		CFRelease(source);
		return 1;
	}
	printf("PASS: image-source declarations bridge under ARC\n");
	return 0;
}
