// CFLAGS: -framework AppKit
// Copyright (C) 2026 Lubos Dolezel
//
// The private NSWindow -setIsMiniaturized: setter that applications call:
//   - NSWindow responds to it;
//   - it behaves like -miniaturize: / -deminiaturize: (the change is not
//     immediate; the Will/Did notifications follow, in order);
//   - setting the state a window already has changes nothing and posts nothing.
// Needs an X display with a window manager (Xvfb + xfwm4) or a Wayland
// compositor. xdg-shell does not report minimization, so on Wayland
// -isMiniaturized stays NO and only the selector and the NO cases are checked
// (the YES-dependent lines print SKIP). Exits by itself.
#import <AppKit/AppKit.h>
#import <objc/message.h>
#include <stdio.h>

static NSMutableArray *notes;
static int failures;

@interface Obs : NSObject
@end
@implementation Obs
- (void) note: (NSNotification *) n {
    [notes addObject: [n name]];
}
@end

static void expect(BOOL ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok)
        printf("  notifications seen: %s\n", [[notes description] UTF8String]);
    if (!ok)
        failures++;
}

static void spinUntil(NSWindow *w, BOOL mini) {
    NSDate *end = [NSDate dateWithTimeIntervalSinceNow: 4];
    while ([w isMiniaturized] != mini && [end timeIntervalSinceNow] > 0)
        [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.1]];
    [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.3]];
}

int main(void) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        notes = [[NSMutableArray alloc] init];
        Obs *o = [[Obs alloc] init];
        SEL set = NSSelectorFromString(@"setIsMiniaturized:");
        NSWindow *w = [[NSWindow alloc] initWithContentRect: NSMakeRect(100, 100, 300, 200)
                                                  styleMask: NSTitledWindowMask | NSMiniaturizableWindowMask
                                                    backing: NSBackingStoreBuffered
                                                      defer: NO];
        NSArray *names = @[ NSWindowWillMiniaturizeNotification, NSWindowDidMiniaturizeNotification,
                            NSWindowDidDeminiaturizeNotification ];
        for (NSString *n in names)
            [[NSNotificationCenter defaultCenter] addObserver: o selector: @selector(note:) name: n object: w];
        [w makeKeyAndOrderFront: nil];
        spinUntil(w, NO);
        [notes removeAllObjects];

        BOOL responds = [w respondsToSelector: set];
        expect(responds, "NSWindow responds to setIsMiniaturized:");
        if (!responds) {
            printf("FAILED %d\n", failures);
            return 1;
        }
        void (*setter)(id, SEL, BOOL) = (void (*)(id, SEL, BOOL))objc_msgSend;

        setter(w, set, NO);
        spinUntil(w, NO);
        expect(![w isMiniaturized] && [notes count] == 0, "NO on a normal window changes nothing");

        setter(w, set, YES);
        spinUntil(w, YES);
        if ([w isMiniaturized]) {
            expect([notes isEqualToArray: @[ NSWindowWillMiniaturizeNotification, NSWindowDidMiniaturizeNotification ]],
                  "YES miniaturizes: Will then Did notification");
            [notes removeAllObjects];
            setter(w, set, YES);
            spinUntil(w, YES);
            expect([w isMiniaturized] && [notes count] == 0, "YES on a miniaturized window changes nothing");
            setter(w, set, NO);
            spinUntil(w, NO);
            expect(![w isMiniaturized], "NO deminiaturizes");
            expect([notes isEqualToArray: @[ NSWindowDidDeminiaturizeNotification ]], "NO posts DidDeminiaturize");
        } else {
            printf("SKIP: the backend does not report minimization (Wayland)\n");
        }
        [w close];
    }
    if (failures) {
        printf("FAILED %d\n", failures);
        return 1;
    }
    printf("ALL PASSED\n");
    return 0;
}
