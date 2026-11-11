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
#include <unistd.h>

static CGImageSourceRef createSource(NSURL *url, NSDictionary *options) {
	return CGImageSourceCreateWithURL((__bridge CFURLRef)url, (__bridge CFDictionaryRef)options);
}

int main(void) {
	// A minimal PNG file: the signature and an IHDR chunk are enough to identify the image type.
	static const unsigned char png[] = {
		0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a, 0, 0, 0, 0x0d, 'I', 'H', 'D', 'R',
		0, 0, 0, 1, 0, 0, 0, 1, 8, 6, 0, 0, 0, 0x1f, 0x15, 0xc4, 0x89,
		0, 0, 0, 0, 'I', 'E', 'N', 'D', 0xae, 0x42, 0x60, 0x82
	};
	NSString *path = [NSTemporaryDirectory() stringByAppendingPathComponent:
		[NSString stringWithFormat:@"imageio-arc-bridging-%d.png", getpid()]];
	[[NSData dataWithBytes:png length:sizeof(png)] writeToFile:path atomically:NO];
	CGImageSourceRef source = createSource([NSURL fileURLWithPath:path], @{});
	[[NSFileManager defaultManager] removeItemAtPath:path error:NULL];
	if (!source) {
		printf("FAIL: no source was created for a PNG file\n");
		return 1;
	}
	CFRelease(source);
	printf("PASS: image-source declarations bridge under ARC\n");
	return 0;
}
