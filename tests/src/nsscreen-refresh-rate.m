// Copyright (C) 2026 Lubos Dolezel
// NSScreen refresh-rate API; GUI test, run under Xvfb.
#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#include <math.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

int main(void)
{
    @autoreleasepool {
        [NSApplication sharedApplication];
        NSScreen *screen = [[NSScreen screens] objectAtIndex:0];
        CHECK(screen != nil);

        NSTimeInterval min = [screen minimumRefreshInterval];
        NSTimeInterval max = [screen maximumRefreshInterval];
        NSInteger fps = [screen maximumFramesPerSecond];
        NSTimeInterval granularity = [screen displayUpdateGranularity];

        CHECK(fps > 0);
        CHECK(min > 0 && max > 0 && min <= max);
        CHECK(granularity > 0 && granularity <= max);
        CHECK(fps == (NSInteger)lround(1.0 / min));

        // Consistent with CoreGraphics, which reports the same mode's rate (60 when the backend has none).
        CGDirectDisplayID display = [[[screen deviceDescription] objectForKey:@"NSScreenNumber"] unsignedIntValue];
        CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display);
        CHECK(mode != NULL);
        if (mode) {
            double rate = CGDisplayModeGetRefreshRate(mode);
            if (rate > 0) CHECK(fabs(1.0 / min - rate) < 0.01);
            else CHECK(fps == 60);
            CFRelease(mode);
        }
        printf("fps=%ld min=%.6f max=%.6f granularity=%.6f\n", (long)fps, min, max, granularity);
    }
    if (failures) return 1;
    printf("PASS\n");
    return 0;
}
