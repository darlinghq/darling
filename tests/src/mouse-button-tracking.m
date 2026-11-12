/* Copyright (C) 2026 Lubos Dolezel
 *
 * Mouse-up, drag and down events for one button go to the view that received
 * that button's mouse-down, whatever the other buttons do in between.
 * Events are fed through -[NSWindow sendEvent:]. Needs a window server
 * connection (Xvfb :99 on Darling). Prints a log of deliveries and
 * PASS or FAIL lines.
 */
#import <Cocoa/Cocoa.h>
#include <signal.h>
#include <unistd.h>

static NSMutableArray *log_;
static int failures;

@interface LView : NSView
@property (copy) NSString *name;
@end
@implementation LView
- (BOOL) isFlipped { return NO; }
- (BOOL) acceptsFirstMouse: (NSEvent *) e { return YES; }
#define LOGM(sel, tag) - (void) sel: (NSEvent *) e { [log_ addObject: [NSString stringWithFormat: @"%@:" tag, self.name]]; }
LOGM(mouseDown, @"down")
LOGM(mouseUp, @"up")
LOGM(mouseDragged, @"drag")
LOGM(rightMouseDown, @"rdown")
LOGM(rightMouseUp, @"rup")
LOGM(rightMouseDragged, @"rdrag")
LOGM(otherMouseDown, @"odown")
LOGM(otherMouseUp, @"oup")
LOGM(otherMouseDragged, @"odrag")
@end

static NSWindow *win;
static LView *A, *B;
static NSPoint pa, pb;

static void feed(NSEventType t, NSPoint p) {
    NSEvent *e = [NSEvent mouseEventWithType: t location: p modifierFlags: 0 timestamp: 1
        windowNumber: [win windowNumber] context: nil eventNumber: 1 clickCount: 1 pressure: 1];
    [win sendEvent: e];
}

static void expect(NSString *what, NSArray *want) {
    if (![log_ isEqualToArray: want]) {
        printf("FAIL: %s\n  got:  %s\n  want: %s\n", [what UTF8String],
               [[log_ componentsJoinedByString: @" "] UTF8String],
               [[want componentsJoinedByString: @" "] UTF8String]);
        failures++;
    } else {
        printf("ok: %s: %s\n", [what UTF8String], [[log_ componentsJoinedByString: @" "] UTF8String]);
    }
    [log_ removeAllObjects];
}

static void watchdog(int s) { write(1, "FAIL: timeout\n", 14); _exit(1); }

int main(void) {
    signal(SIGALRM, watchdog);
    alarm(60);
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];
        log_ = [NSMutableArray new];
        win = [[NSWindow alloc] initWithContentRect: NSMakeRect(100, 100, 400, 200)
            styleMask: NSWindowStyleMaskTitled backing: NSBackingStoreBuffered defer: NO];
        A = [[LView alloc] initWithFrame: NSMakeRect(0, 0, 200, 200)]; A.name = @"A";
        B = [[LView alloc] initWithFrame: NSMakeRect(200, 0, 200, 200)]; B.name = @"B";
        [[win contentView] addSubview: A];
        [[win contentView] addSubview: B];
        [win makeKeyAndOrderFront: nil];
        [NSApp activateIgnoringOtherApps: YES];
        [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.5]];
        pa = NSMakePoint(50, 50);
        pb = NSMakePoint(250, 50);

        /* 1. right down on A, left click on B, right up elsewhere (over B) */
        feed(NSEventTypeRightMouseDown, pa);
        feed(NSEventTypeRightMouseDragged, pb);
        feed(NSEventTypeLeftMouseDown, pb);
        feed(NSEventTypeLeftMouseDragged, pa);
        feed(NSEventTypeRightMouseDragged, pb);
        feed(NSEventTypeLeftMouseUp, pa);
        feed(NSEventTypeRightMouseDragged, pb);
        feed(NSEventTypeRightMouseUp, pb);
        expect(@"rightDown(A) leftDown(B) leftUp rightUp", @[@"A:rdown", @"A:rdrag", @"B:down", @"B:drag", @"A:rdrag", @"B:up", @"A:rdrag", @"A:rup"]);

        /* 2. the reverse order: left down on A, right down on B, right up, left up */
        feed(NSEventTypeLeftMouseDown, pa);
        feed(NSEventTypeRightMouseDown, pb);
        feed(NSEventTypeRightMouseUp, pa);
        feed(NSEventTypeLeftMouseUp, pb);
        expect(@"leftDown(A) rightDown(B) rightUp leftUp", @[@"A:down", @"B:rdown", @"B:rup", @"A:up"]);

        /* other buttons */
        feed(NSEventTypeLeftMouseDown, pa);
        feed(NSEventTypeOtherMouseDown, pb);
        feed(NSEventTypeOtherMouseDragged, pa);
        feed(NSEventTypeOtherMouseUp, pa);
        feed(NSEventTypeLeftMouseUp, pb);
        expect(@"leftDown(A) otherDown(B) otherUp leftUp", @[@"A:down", @"B:odown", @"B:odrag", @"B:oup", @"A:up"]);

        /* 3. a plain click still works afterwards */
        feed(NSEventTypeLeftMouseDown, pb);
        feed(NSEventTypeLeftMouseUp, pb);
        feed(NSEventTypeRightMouseDown, pa);
        feed(NSEventTypeRightMouseUp, pa);
        expect(@"plain clicks", @[@"B:down", @"B:up", @"A:rdown", @"A:rup"]);

        /* 4. the target view moves during the drag: up still reaches it */
        feed(NSEventTypeLeftMouseDown, pa);
        [A setFrameOrigin: NSMakePoint(200, 0)];
        [B setFrameOrigin: NSMakePoint(0, 0)];
        feed(NSEventTypeLeftMouseDragged, pa);
        feed(NSEventTypeLeftMouseUp, pa);
        expect(@"target moves during the drag", @[@"A:down", @"A:drag", @"A:up"]);
        [A setFrameOrigin: NSMakePoint(0, 0)];
        [B setFrameOrigin: NSMakePoint(200, 0)];

        /* 5. the target view leaves the window during the drag: nothing more
           is delivered for that button (observed on macOS), later clicks work */
        [A retain];
        feed(NSEventTypeLeftMouseDown, pa);
        [A removeFromSuperview];
        feed(NSEventTypeLeftMouseDragged, pa);
        feed(NSEventTypeLeftMouseUp, pa);
        expect(@"target removed during the drag", @[@"A:down"]);
        [[win contentView] addSubview: A];
        [A release];
        feed(NSEventTypeLeftMouseDown, pa);
        feed(NSEventTypeLeftMouseUp, pa);
        expect(@"click after the removed target", @[@"A:down", @"A:up"]);
    }
    if (!failures) puts("PASS: mouse buttons track their own mouse-down view");
    return failures ? 1 : 0;
}
