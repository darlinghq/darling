/* Copyright (C) 2026 Lubos Dolezel
 * -[NSWindow colorSpace] / -setColorSpace: round-trip. No color management is
 * applied; the setting is stored. Pass anything as argv[1] to skip the stderr
 * check (not needed on macOS). */
#import <Cocoa/Cocoa.h>
#include <unistd.h>
#include <fcntl.h>

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL line %d: %s ", __LINE__, #cond); fputs([[NSString stringWithFormat: __VA_ARGS__] UTF8String], stdout); printf("\n"); } } while (0)

int main(int argc, char **argv) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        NSWindow *w = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, 100, 100)
                                                  styleMask: NSWindowStyleMaskTitled
                                                    backing: NSBackingStoreBuffered
                                                      defer: YES];
        __block int notifications = 0;
        id obs = [[NSNotificationCenter defaultCenter] addObserverForName: nil object: w queue: nil
                                                               usingBlock: ^(NSNotification *n) { notifications++; }];

        // Everything the unimplemented stub logged went to stderr.
        char path[] = "/tmp/nswindow_colorspace.XXXXXX";
        int saved = dup(2), log = mkstemp(path);
        dup2(log, 2);

        // A window that has none of its own is in its screen's color space.
        NSColorSpace *initial = w.colorSpace;
        CHECK(initial != nil, @"initial color space");
        if (w.screen != nil)
            CHECK([initial isEqual: w.screen.colorSpace], @"initial %@ vs screen %@", initial, w.screen.colorSpace);

        NSColorSpace *spaces[] = { [NSColorSpace genericGrayColorSpace], [NSColorSpace sRGBColorSpace],
                                   [NSColorSpace displayP3ColorSpace], [NSColorSpace genericRGBColorSpace],
                                   [NSColorSpace deviceRGBColorSpace] };
        for (int i = 0; i < 5; i++) {
            w.colorSpace = spaces[i];
            CHECK(w.colorSpace == spaces[i], @"round trip %d: %@", i, w.colorSpace);
        }

        // nil goes back to the default.
        w.colorSpace = nil;
        CHECK(w.colorSpace != nil && [w.colorSpace isEqual: initial], @"after nil: %@", w.colorSpace);

        // Setting it does not post notifications.
        CHECK(notifications == 0, @"%d notifications", notifications);

        fflush(stderr);
        dup2(saved, 2);
        close(saved);
        off_t size = lseek(log, 0, SEEK_END);
        CHECK(size == 0 || argc > 1, @"stderr output of %lld bytes", (long long) size);
        close(log);
        unlink(path);
        [[NSNotificationCenter defaultCenter] removeObserver: obs];
        [w release];
    }
    printf(failures ? "FAILED\n" : "PASS\n");
    return failures != 0;
}
