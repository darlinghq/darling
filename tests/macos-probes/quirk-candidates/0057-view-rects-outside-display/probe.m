// Probe 0057: what -getRectsBeingDrawn:count: and -needsToDrawRect: answer when an application calls -drawRect:
// itself (no display pass), for views with and without a window, drawn into an image or a bitmap context, and
// during a real display pass for comparison.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSMutableArray *gLog;

@interface ProbeView : NSView
@end
@implementation ProbeView
- (void)drawRect:(NSRect)dirty {
    const NSRect *rects = NULL;
    NSInteger count = -1;
    [self getRectsBeingDrawn:&rects count:&count];
    NSMutableArray *rs = [NSMutableArray array];
    for (NSInteger i = 0; i < count && rects != NULL; i++)
        [rs addObject:probe_rect(rects[i])];
    NSRect inside = NSInsetRect([self bounds], 2, 2);
    NSRect outside = NSMakeRect(NSMaxX([self bounds]) + 50, NSMaxY([self bounds]) + 50, 5, 5);
    [gLog addObject:PF(@"drawRect:%@ ; getRectsBeingDrawn count=%ld rects=%@ ; needsToDrawRect(inside bounds)=%d needsToDrawRect(outside bounds)=%d ; visibleRect=%@ window=%@ ctx=%@",
                       probe_rect(dirty), (long)count, rects == NULL ? @"NULL" : [rs componentsJoinedByString:@","],
                       (int)[self needsToDrawRect:inside], (int)[self needsToDrawRect:outside],
                       probe_rect([self visibleRect]), [self window] ? @"yes" : @"no",
                       [NSGraphicsContext currentContext] ? @"yes" : @"no")];
}
@end

static void flush(const char *id, NSString *input) {
    probe_line(id, input, [gLog count] ? [gLog componentsJoinedByString:@" || "] : @"drawRect: was not called");
    [gLog removeAllObjects];
}

int main(void) {
    @autoreleasepool {
        probe_env("0057");
        probe_app();
        gLog = [NSMutableArray array];
        NSRect frame = NSMakeRect(7, 9, 100, 60);

        // 1. No window, no context.
        ProbeView *v1 = [[ProbeView alloc] initWithFrame:frame];
        [v1 drawRect:[v1 bounds]];
        flush("0057.1", @"view without window; the application calls -drawRect:(bounds) with no graphics context");

        // 2. No window, drawing into an image.
        ProbeView *v2 = [[ProbeView alloc] initWithFrame:frame];
        NSImage *img = [[NSImage alloc] initWithSize:frame.size];
        [img lockFocus];
        [v2 drawRect:[v2 bounds]];
        [img unlockFocus];
        flush("0057.2", @"view without window; -drawRect:(bounds) between -lockFocus and -unlockFocus of an NSImage");

        // 3. No window, partial dirty rect passed by the caller.
        ProbeView *v3 = [[ProbeView alloc] initWithFrame:frame];
        [img lockFocus];
        [v3 drawRect:NSMakeRect(10, 10, 20, 20)];
        [img unlockFocus];
        flush("0057.3", @"view without window; -drawRect:{10,10,20,20} (a partial rect) in an image");

        // 4. No window, after -setNeedsDisplayInRect: recorded a dirty rect.
        ProbeView *v4 = [[ProbeView alloc] initWithFrame:frame];
        [v4 setNeedsDisplayInRect:NSMakeRect(1, 2, 3, 4)];
        [img lockFocus];
        [v4 drawRect:[v4 bounds]];
        [img unlockFocus];
        flush("0057.4", @"view without window after -setNeedsDisplayInRect:{1,2,3,4}; -drawRect:(bounds) in an image");

        // 5. Subview of a windowless superview.
        ProbeView *sup = [[ProbeView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200)];
        ProbeView *sub = [[ProbeView alloc] initWithFrame:NSMakeRect(20, 30, 100, 60)];
        [sup addSubview:sub];
        [img lockFocus];
        [sub drawRect:[sub bounds]];
        [img unlockFocus];
        flush("0057.5", @"subview in a windowless superview; the application calls the subview's -drawRect:(bounds)");

        // 6. Bounds origin moved.
        ProbeView *v6 = [[ProbeView alloc] initWithFrame:frame];
        [v6 setBounds:NSMakeRect(-20, -30, 100, 60)];
        [img lockFocus];
        [v6 drawRect:[v6 bounds]];
        [img unlockFocus];
        flush("0057.6", @"view without window, bounds origin {-20,-30}; -drawRect:(bounds) in an image");

        // 7. View in a window that was never shown; direct call.
        NSWindow *w = probe_window(NSMakeRect(0, 0, 200, 120), NO);
        ProbeView *v7 = [[ProbeView alloc] initWithFrame:NSMakeRect(10, 10, 100, 60)];
        [[w contentView] addSubview:v7];
        [img lockFocus];
        [v7 drawRect:[v7 bounds]];
        [img unlockFocus];
        flush("0057.7", @"view in a window never shown; the application calls -drawRect:(bounds) in an image");

        // 8. Real display passes.
        [[w contentView] displayIfNeeded];
        flush("0057.8a", @"view in a window never shown; -displayIfNeeded on the content view (nothing marked dirty)");
        [v7 setNeedsDisplay:YES];
        [v7 displayIfNeeded];
        flush("0057.8b", @"same view after -setNeedsDisplay:YES and -displayIfNeeded");
        [v7 displayRect:NSMakeRect(5, 5, 30, 20)];
        flush("0057.8c", @"-displayRect:{5,5,30,20}");
        NSBitmapImageRep *rep = [v7 bitmapImageRepForCachingDisplayInRect:[v7 bounds]];
        [v7 cacheDisplayInRect:[v7 bounds] toBitmapImageRep:rep];
        flush("0057.8d", @"-cacheDisplayInRect:(bounds) toBitmapImageRep:");

        // 9. Windowless view through the same API.
        ProbeView *v9 = [[ProbeView alloc] initWithFrame:frame];
        NSBitmapImageRep *rep9 = [v9 bitmapImageRepForCachingDisplayInRect:[v9 bounds]];
        [v9 cacheDisplayInRect:[v9 bounds] toBitmapImageRep:rep9];
        flush("0057.9a", @"view without window; -cacheDisplayInRect:(bounds) toBitmapImageRep:");
        [v9 displayRectIgnoringOpacity:[v9 bounds] inContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:rep9]];
        flush("0057.9b", @"view without window; -displayRectIgnoringOpacity:inContext: with a bitmap context");

        // 10. A bitmap context made current by the application, direct call.
        ProbeView *v10 = [[ProbeView alloc] initWithFrame:frame];
        NSBitmapImageRep *rep10 = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:100 pixelsHigh:60 bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace bytesPerRow:0 bitsPerPixel:0];
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:rep10]];
        [v10 drawRect:[v10 bounds]];
        [NSGraphicsContext restoreGraphicsState];
        flush("0057.10", @"view without window; -drawRect:(bounds) with a bitmap context set current by the application");
    }
    return 0;
}
