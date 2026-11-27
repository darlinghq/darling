// Copy-on-scroll, opacity and catalog colour alpha of the scroll view stack.
//
// Contract (NSClipView, NSView, NSColor documentation): a clip view copies
// the already drawn pixels when it scrolls (copiesOnScroll defaults to YES)
// and only the newly exposed strip is redrawn; an opaque document view lets
// that happen; a view reports isOpaque only when it really fills its bounds
// without transparency; a colour's alphaComponent is the alpha of the colour
// it stands for, also for catalog colours.
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
        [NSApplication sharedApplication];

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
        [w orderOut:nil];
        [w release];
    }
    if (failures) { printf("%d FAILED\n", failures); return 1; }
    puts("PASS");
    return 0;
}
