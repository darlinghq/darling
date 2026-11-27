// Copy-on-scroll, opacity and catalog colour alpha of the scroll view stack.
//
// Contract (NSClipView, NSView, NSColor documentation): a clip view copies
// the already drawn pixels when it scrolls (copiesOnScroll defaults to YES)
// and only the newly exposed strip is redrawn; an opaque document view lets
// that happen; a view reports isOpaque only when it really fills its bounds
// without transparency; a colour's alphaComponent is the alpha of the colour
// it stands for, also for catalog colours.
//
// Also: a view with two small, far apart invalid rects draws those rects, not the
// rectangle around both (NSView displayIfNeeded draws the invalid rects); a clip
// view that has partial invalid rects still copies on scroll and draws only them
// plus the exposed strip; and scroll-wheel events queued for a running application
// are all delivered to the view under the pointer and leave the scroll position
// where the sum of them puts it (NSEvent only documents coalescing for mouse-moved
// and drag events, via isMouseCoalescingEnabled).
#import <AppKit/AppKit.h>
#include <stdio.h>
#include <stdlib.h>

static int failures;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

@interface CountingTable : NSTableView {
@public
    double drawnArea;
    int drawCalls;
}
@end

@implementation CountingTable
- (void)drawRect:(NSRect)r {
    drawnArea += r.size.width * r.size.height;
    drawCalls++;
    [super drawRect:r];
}
@end

@interface DS : NSObject <NSTableViewDataSource>
@end
@implementation DS
- (NSInteger)numberOfRowsInTableView:(NSTableView *)t { return 200; }
- (id)tableView:(NSTableView *)t objectValueForTableColumn:(NSTableColumn *)c row:(NSInteger)r {
    return [NSString stringWithFormat:@"row %ld", (long)r];
}
@end

// Rects a view was asked to draw, to check what the invalid rects turned into.
@interface RectView : NSView {
@public
    double drawnArea;
    int drawCalls;
    NSRect drawn[64];
}
@end
@implementation RectView
- (BOOL)isOpaque { return YES; }
- (void)drawRect:(NSRect)r {
    if (drawCalls < 64) drawn[drawCalls] = r;
    drawCalls++;
    drawnArea += r.size.width * r.size.height;
    [[NSColor whiteColor] set];
    NSRectFill(r);
}
@end

static BOOL covers(NSRect *rs, int n, NSPoint p) {
    for (int i = 0; i < n && i < 64; i++)
        if (NSPointInRect(p, rs[i])) return YES;
    return NO;
}

// An application that counts how often it brings the windows up to date.
static int updateCount;
static BOOL counting;
static NSWindow *wheelTarget; // the window a wheel event without a window of its own is meant for
@interface CountApp : NSApplication
@end
@implementation CountApp
- (void)updateWindows {
    if (counting) updateCount++;
    [super updateWindows];
}
- (void)sendEvent:(NSEvent *)e {
    if ([e type] == NSEventTypeApplicationDefined) {
        counting = NO;
        [self stop:nil];
        return;
    }
    // The test's events carry the pointer location in window coordinates but no window.
    if ([e type] == NSEventTypeScrollWheel && [e window] == nil && wheelTarget != nil) {
        [wheelTarget sendEvent:e];
        return;
    }
    [super sendEvent:e];
}
@end

// A scroll view that records the scroll-wheel events it is given.
static int wheelEvents;
static double wheelDeltaSum;
static NSPoint lastOrigin;
@interface WheelScrollView : NSScrollView
@end
@implementation WheelScrollView
- (void)scrollWheel:(NSEvent *)e {
    wheelEvents++;
    wheelDeltaSum += [e scrollingDeltaY];
    [super scrollWheel:e];
    lastOrigin = [[self contentView] bounds].origin;
}
@end

static NSEvent *wheelEventInWindow(NSWindow *w, int lines) {
    CGEventRef cg = CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitLine, 1, lines);
    NSRect f = [w frame];
    CGFloat screenH = [[NSScreen mainScreen] frame].size.height;
    CGEventSetLocation(cg, CGPointMake(NSMidX(f), screenH - NSMidY(f)));
    CGEventSetIntegerValueField(cg, kCGMouseEventWindowUnderMousePointer, [w windowNumber]);
    CGEventSetIntegerValueField(cg, kCGMouseEventWindowUnderMousePointerThatCanHandleThisEvent, [w windowNumber]);
    NSEvent *e = [NSEvent eventWithCGEvent:cg];
    CFRelease(cg);
    return e;
}

static void pump(void) {
    for (int i = 0; i < 5; i++) {
        NSEvent *e;
        while ((e = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate dateWithTimeIntervalSinceNow:0.02]
                                          inMode:NSDefaultRunLoopMode dequeue:YES]))
            [NSApp sendEvent:e];
    }
}

int main(void) {
    @autoreleasepool {
        [CountApp sharedApplication];

        // A fresh clip view copies on scroll by default.
        NSClipView *clip = [[NSClipView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
        CHECK([clip copiesOnScroll] == YES);
        [clip release];

        // Catalog colours report the alpha of the colour they resolve to.
        NSColor *cat[] = { [NSColor controlBackgroundColor], [NSColor secondaryLabelColor],
                           [NSColor tertiaryLabelColor], [NSColor gridColor], [NSColor selectedContentBackgroundColor] };
        for (size_t i = 0; i < sizeof(cat) / sizeof(cat[0]); i++) {
            NSColor *rgb = [cat[i] colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
            if (rgb != nil)
                CHECK(fabs([cat[i] alphaComponent] - [rgb alphaComponent]) < 0.01);
        }
        NSColor *dim = [NSColor tertiaryLabelColor];
        NSColor *dimRGB = [dim colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
        if (dimRGB != nil && [dimRGB alphaComponent] < 0.99)
            CHECK([dim alphaComponent] < 0.99);

        // Table with an opaque background is opaque; with a transparent one it is not.
        NSWindow *w = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, 400, 300)
                                                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskResizable
                                                    backing:NSBackingStoreBuffered defer:NO];
        NSScrollView *sv = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 400, 300)];
        CountingTable *t = [[CountingTable alloc] initWithFrame:NSMakeRect(0, 0, 400, 300)];
        NSTableColumn *col = [[NSTableColumn alloc] initWithIdentifier:@"c"];
        [col setWidth:380];
        [t addTableColumn:col];
        DS *ds = [DS new];
        [t setDataSource:ds];
        [t setUsesAlternatingRowBackgroundColors:NO];
        [t setBackgroundColor:[NSColor whiteColor]];
        CHECK([t isOpaque] == YES);
        [t setBackgroundColor:[NSColor clearColor]];
        CHECK([t isOpaque] == NO);
        [t setBackgroundColor:[NSColor whiteColor]];
        [t reloadData];
        [sv setDocumentView:t];
        [sv setHasVerticalScroller:YES];
        [[w contentView] addSubview:sv];
        [w makeKeyAndOrderFront:nil];
        pump();
        [w displayIfNeeded];
        CHECK([[sv contentView] copiesOnScroll] == YES);

        // Scrolling by a few rows redraws only the exposed strip, not the whole viewport.
        NSClipView *cv = [sv contentView];
        NSRect vis = [cv bounds];
        double full = vis.size.width * vis.size.height;
        t->drawnArea = 0;
        t->drawCalls = 0;
        int steps = 20;
        for (int i = 1; i <= steps; i++) {
            [cv scrollToPoint:NSMakePoint(0, i * 3)];
            [sv reflectScrolledClipView:cv];
            [w displayIfNeeded];
        }
        double perStep = t->drawnArea / steps;
        printf("scroll: %d steps, %.0f px^2 drawn per step (viewport %.0f px^2, %.1f%%), %d drawRect calls\n",
               steps, perStep, full, 100.0 * perStep / full, t->drawCalls);
        CHECK(perStep < full * 0.5);

        // Scrolling while a small part of the view is invalid keeps the copy: the rest of the
        // viewport is not redrawn, and the invalid part is drawn.
        {
            NSPoint o = [cv bounds].origin;
            NSRect dirty = NSMakeRect(40, o.y + 100, 60, 20);
            [cv setNeedsDisplayInRect:dirty];
            t->drawnArea = 0;
            t->drawCalls = 0;
            [cv scrollToPoint:NSMakePoint(0, o.y + 3)];
            [sv reflectScrolledClipView:cv];
            [w displayIfNeeded];
            printf("scroll with a partial invalid rect: %.0f px^2 drawn (viewport %.0f px^2, %.1f%%), %d drawRect calls\n",
                   t->drawnArea, full, 100.0 * t->drawnArea / full, t->drawCalls);
            CHECK(t->drawnArea < full * 0.5);
            CHECK(t->drawnArea >= dirty.size.width * dirty.size.height);
            CHECK([cv needsDisplay] == NO);
        }
        [w orderOut:nil];
        [w release];

        // Two small, far apart invalid rects are drawn as such, and nothing else.
        {
            NSWindow *w2 = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, 400, 300)
                                                       styleMask:NSWindowStyleMaskTitled
                                                         backing:NSBackingStoreBuffered defer:NO];
            RectView *rv = [[RectView alloc] initWithFrame:NSMakeRect(0, 0, 400, 300)];
            [[w2 contentView] addSubview:rv];
            [w2 makeKeyAndOrderFront:nil];
            pump();
            [w2 displayIfNeeded];
            NSRect a = NSMakeRect(10, 10, 20, 20), b = NSMakeRect(360, 260, 20, 20);
            rv->drawnArea = 0;
            rv->drawCalls = 0;
            [rv setNeedsDisplayInRect:a];
            [rv setNeedsDisplayInRect:b];
            [w2 displayIfNeeded];
            double fullRV = 400.0 * 300.0;
            printf("disjoint invalid rects: %.0f px^2 drawn (view %.0f px^2, %.1f%%), %d drawRect calls\n",
                   rv->drawnArea, fullRV, 100.0 * rv->drawnArea / fullRV, rv->drawCalls);
            CHECK(rv->drawnArea < fullRV * 0.05);
            CHECK(covers(rv->drawn, rv->drawCalls, NSMakePoint(NSMidX(a), NSMidY(a))));
            CHECK(covers(rv->drawn, rv->drawCalls, NSMakePoint(NSMidX(b), NSMidY(b))));
            // Overlapping invalid rects are drawn once, as the rectangle around them.
            rv->drawnArea = 0;
            rv->drawCalls = 0;
            [rv setNeedsDisplayInRect:NSMakeRect(10, 10, 50, 50)];
            [rv setNeedsDisplayInRect:NSMakeRect(40, 40, 50, 50)];
            [w2 displayIfNeeded];
            CHECK(rv->drawCalls >= 1);
            CHECK(covers(rv->drawn, rv->drawCalls, NSMakePoint(15, 15)));
            CHECK(covers(rv->drawn, rv->drawCalls, NSMakePoint(85, 85)));
            CHECK(covers(rv->drawn, rv->drawCalls, NSMakePoint(50, 50)));
            [w2 orderOut:nil];
            [w2 release];
        }

        // Scroll-wheel events queued for a running application: every one reaches the scroll view,
        // the scroll position is the sum of them, and the windows are not brought up to date after
        // each one of a burst (that is internal; the check is that it is not more than one per event).
        {
            NSWindow *w3 = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, 400, 300)
                                                       styleMask:NSWindowStyleMaskTitled
                                                         backing:NSBackingStoreBuffered defer:NO];
            WheelScrollView *sv3 = [[WheelScrollView alloc] initWithFrame:NSMakeRect(0, 0, 400, 300)];
            CountingTable *t3 = [[CountingTable alloc] initWithFrame:NSMakeRect(0, 0, 400, 300)];
            NSTableColumn *c3 = [[NSTableColumn alloc] initWithIdentifier:@"c"];
            [c3 setWidth:380];
            [t3 addTableColumn:c3];
            [t3 setDataSource:ds];
            [t3 setBackgroundColor:[NSColor whiteColor]];
            [t3 reloadData];
            [sv3 setDocumentView:t3];
            [sv3 setHasVerticalScroller:YES];
            [[w3 contentView] addSubview:sv3];
            [w3 makeKeyAndOrderFront:nil];
            pump();
            [w3 displayIfNeeded];
            NSClipView *cv3 = [sv3 contentView];
            NSPoint start = [cv3 bounds].origin;

            // One event alone gives the step of one line-counting event.
            wheelEvents = 0;
            wheelDeltaSum = 0;
            wheelTarget = w3;
            [NSApp sendEvent:wheelEventInWindow(w3, -1)];
            CHECK(wheelEvents == 1);
            double step = [cv3 bounds].origin.y - start.y;
            printf("one wheel event scrolls %.1f points\n", step);
            CHECK(step != 0);

            int N = 40;
            NSPoint before = [cv3 bounds].origin;
            wheelEvents = 0;
            wheelDeltaSum = 0;
            updateCount = 0;
            t3->drawnArea = 0;
            t3->drawCalls = 0;
            for (int i = 0; i < N; i++)
                [NSApp postEvent:wheelEventInWindow(w3, -1) atStart:NO];
            for (int i = 0; i < 2; i++)
                [NSApp postEvent:[NSEvent otherEventWithType:NSEventTypeApplicationDefined location:NSZeroPoint
                                                modifierFlags:0 timestamp:0 windowNumber:0 context:nil
                                                      subtype:1 data1:0 data2:0] atStart:NO];
            counting = YES;
            [NSApp run];
            counting = NO;
            double expect = before.y + N * step;
            double maxY = NSMaxY([[sv3 documentView] bounds]) - [cv3 bounds].size.height;
            if (expect > maxY) expect = maxY;
            printf("wheel burst: %d events delivered (sum of deltas %.1f), %d updateWindows, scrolled %.1f -> %.1f (expected %.1f), %.0f px^2 drawn\n",
                   wheelEvents, wheelDeltaSum, updateCount, before.y, [cv3 bounds].origin.y, expect, t3->drawnArea);
            CHECK(wheelEvents == N);
            CHECK(fabs(wheelDeltaSum - (-1.0 * N)) < 0.001);
            CHECK(fabs([cv3 bounds].origin.y - expect) < 0.001);
            CHECK(fabs(lastOrigin.y - [cv3 bounds].origin.y) < 0.001);
            CHECK(updateCount <= N);
            // The burst is not brought up to date once per event.
            CHECK(updateCount < N);
            [w3 orderOut:nil];
            [w3 release];
        }
    }
    if (failures) { printf("%d FAILED\n", failures); return 1; }
    puts("PASS");
    return 0;
}
