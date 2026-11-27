// Copyright (C) 2026 Lubos Dolezel
// Screen capture access and display capture through CoreGraphics. Without a capture
// backend (or without the user's consent) CGPreflightScreenCaptureAccess is false and
// CGDisplayCreateImage returns NULL; with one, the image has the size of the display
// and non-uniform content when the screen shows something.
// Usage: screen_capture [expect-image|expect-none]
#import <Foundation/NSObject.h>
#import <Foundation/NSAutoreleasePool.h>
#include <CoreGraphics/CoreGraphics.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    int failures = 0;
    int wantImage = argc > 1 && !strcmp(argv[1], "expect-image");
    int wantNone = argc > 1 && !strcmp(argv[1], "expect-none");
    int pre = CGPreflightScreenCaptureAccess();
    printf("preflight %d\n", pre);
    CGDirectDisplayID d = CGMainDisplayID();
    CGRect b = CGDisplayBounds(d);
    printf("display %u %gx%g\n", d, b.size.width, b.size.height);
    CGImageRef img = CGDisplayCreateImage(d);
    printf("image %p\n", (void *)img);
    if (img) {
        size_t w = CGImageGetWidth(img), h = CGImageGetHeight(img);
        printf("image %zux%zu\n", w, h);
        if (!w || !h) { printf("FAIL: empty image\n"); failures++; }
        CGDataProviderRef p = CGImageGetDataProvider(img);
        CFDataRef data = p ? CGDataProviderCopyData(p) : NULL;
        if (data) {
            const unsigned char *px = CFDataGetBytePtr(data);
            CFIndex n = CFDataGetLength(data);
            int distinct = 0;
            for (CFIndex i = 4; i < n && i < 4000000; i += 4) if (memcmp(px + i, px, 3)) { distinct = 1; break; }
            printf("non-uniform %d\n", distinct);
            CFRelease(data);
        }
        CGImageRelease(img);
    }
    if (wantImage && !img) { printf("FAIL: expected an image\n"); failures++; }
    if (wantNone && (img || pre)) { printf("FAIL: expected no capture\n"); failures++; }
    [pool drain];
    printf(failures ? "FAILED\n" : "PASS\n");
    return failures != 0;
}
