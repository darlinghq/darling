// CFLAGS: -framework AppKit
/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Checks the NSStringDrawing rect-based methods on NSString and
 * NSAttributedString (drawWithRect:options:..., boundingRectWithSize:options:...)
 * and NSStringDrawingContext. Geometry is checked as relations against the
 * font's own metrics rather than absolute widths, so it holds with any font.
 */
#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#include <stdio.h>
#include <stdlib.h>

#pragma clang diagnostic ignored "-Wmethod-signatures"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"
#pragma clang diagnostic ignored "-Wobjc-method-access"

// Declared here so the test also compiles against headers that predate them.
@interface NSString (DrawWithRectTestDecl)
- (NSRect) boundingRectWithSize: (NSSize) size options: (NSStringDrawingOptions) o attributes: (NSDictionary *) a context: (id) c;
- (void) drawWithRect: (NSRect) r options: (NSStringDrawingOptions) o attributes: (NSDictionary *) a;
- (void) drawWithRect: (NSRect) r options: (NSStringDrawingOptions) o attributes: (NSDictionary *) a context: (id) c;
@end
@interface NSAttributedString (DrawWithRectTestDecl)
- (NSRect) boundingRectWithSize: (NSSize) size options: (NSStringDrawingOptions) o;
- (NSRect) boundingRectWithSize: (NSSize) size options: (NSStringDrawingOptions) o context: (id) c;
- (void) drawWithRect: (NSRect) r options: (NSStringDrawingOptions) o context: (id) c;
@end

static int failures;
#define CHECK(cond, ...) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL line %d: ", __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
            fputc('\n', stderr); \
            failures++; \
        } \
    } while (0)

enum { W = 240, H = 240 };
static const NSStringDrawingOptions LFO = NSStringDrawingUsesLineFragmentOrigin;

typedef struct { int minx, maxx, miny, maxy; } Ink;   // pixel rows, row 0 = top

// Draws into a W x H bitmap whose current NSGraphicsContext is flipped or not,
// then returns the extent of the inked pixels in bitmap row/column terms.
static Ink drawInk(BOOL flipped, void (^draw)(void)) {
    unsigned char *pixels = calloc(W * H, 4);
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef cg = CGBitmapContextCreate(pixels, W, H, 8, W * 4, cs, kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(cs);
    if (flipped) {
        CGContextTranslateCTM(cg, 0, H);
        CGContextScaleCTM(cg, 1, -1);
    }
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext: [NSGraphicsContext graphicsContextWithGraphicsPort: cg flipped: flipped]];
    draw();
    [NSGraphicsContext restoreGraphicsState];
    CGContextFlush(cg);

    Ink ink = { 9999, -1, 9999, -1 };
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            if (pixels[(y * W + x) * 4 + 3] > 64) {
                if (x < ink.minx) ink.minx = x;
                if (x > ink.maxx) ink.maxx = x;
                if (y < ink.miny) ink.miny = y;
                if (y > ink.maxy) ink.maxy = y;
            }
    CGContextRelease(cg);
    free(pixels);
    return ink;
}

static BOOL near(CGFloat a, CGFloat b, CGFloat tol) { return fabs(a - b) <= tol; }

int main(void) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        NSFont *font = [NSFont fontWithName: @"Helvetica" size: 24] ?: [NSFont systemFontOfSize: 24];
        NSDictionary *attrs = @{NSFontAttributeName: font,
                                NSForegroundColorAttributeName: [NSColor blackColor]};
        CGFloat descent = ceil(-[font descender]);
        CGFloat lineH = ceil([[[[NSLayoutManager alloc] init] autorelease] defaultLineHeightForFont: font]);
        NSSize big = NSMakeSize(1000, 1000);

        CHECK([NSString instancesRespondToSelector: @selector(drawWithRect:options:attributes:)],
              "NSString -drawWithRect:options:attributes: missing");
        CHECK([NSString instancesRespondToSelector: @selector(drawWithRect:options:attributes:context:)],
              "NSString -drawWithRect:options:attributes:context: missing");
        CHECK([NSString instancesRespondToSelector: @selector(boundingRectWithSize:options:attributes:context:)],
              "NSString -boundingRectWithSize:options:attributes:context: missing");
        CHECK([NSAttributedString instancesRespondToSelector: @selector(boundingRectWithSize:options:)],
              "NSAttributedString -boundingRectWithSize:options: missing");
        CHECK([NSAttributedString instancesRespondToSelector: @selector(boundingRectWithSize:options:context:)],
              "NSAttributedString -boundingRectWithSize:options:context: missing");
        CHECK([NSAttributedString instancesRespondToSelector: @selector(drawWithRect:options:context:)],
              "NSAttributedString -drawWithRect:options:context: missing");
        if (failures) return 1;

        // --- bounding rects -------------------------------------------------
        NSString *s = @"Hxg";
        NSRect lfo = [s boundingRectWithSize: big options: LFO attributes: attrs];
        NSRect one = [s boundingRectWithSize: big options: 0 attributes: attrs];
        CHECK(lfo.origin.x == 0 && lfo.origin.y == 0, "LFO origin %g,%g", lfo.origin.x, lfo.origin.y);
        CHECK(near(lfo.size.height, lineH, 1), "LFO height %g, line height %g", lfo.size.height, lineH);
        CHECK(lfo.size.width > 10 && lfo.size.width < 200, "LFO width %g", lfo.size.width);
        CHECK(near(one.origin.y, -descent, 0.01), "single line origin.y %g, expected %g", one.origin.y, -descent);
        CHECK(near(one.size.height, lfo.size.height, 0.01) && near(one.size.width, lfo.size.width, 0.01),
              "single line size %gx%g vs %gx%g", one.size.width, one.size.height, lfo.size.width, lfo.size.height);

        NSRect zero = [s boundingRectWithSize: NSZeroSize options: 0 attributes: attrs];
        CHECK(near(zero.size.width, lfo.size.width, 0.01), "zero-size width %g vs %g", zero.size.width, lfo.size.width);
        NSSize sw = [s sizeWithAttributes: attrs];
        CHECK(near(sw.width, lfo.size.width, 0.01), "sizeWithAttributes width");

        NSRect empty = [@"" boundingRectWithSize: big options: LFO attributes: attrs];
        CHECK(empty.size.width == 0, "empty string width %g", empty.size.width);

        // Without LFO a long string is one line clipped to the proposed width.
        NSString *m = @"Hxg Hxg Hxg Hxg Hxg Hxg Hxg Hxg";
        NSRect wide = [m boundingRectWithSize: big options: LFO attributes: attrs];
        CGFloat limit = floor(wide.size.width / 2.5);
        NSRect clipped = [m boundingRectWithSize: NSMakeSize(limit, 1000) options: 0 attributes: attrs];
        CHECK(near(clipped.size.width, limit, 0.01) && near(clipped.size.height, lineH, 1),
              "no-LFO long string %gx%g, limit %g", clipped.size.width, clipped.size.height, limit);

        // With LFO it wraps: more than one line, within the width.
        NSRect wrapped = [m boundingRectWithSize: NSMakeSize(limit, 1000) options: LFO attributes: attrs];
        CHECK(wrapped.size.width <= limit + 0.01, "wrapped width %g > %g", wrapped.size.width, limit);
        CHECK(wrapped.size.height >= lineH * 2 - 1, "wrapped height %g", wrapped.size.height);
        NSRect unlimitedHeight = [m boundingRectWithSize: NSMakeSize(limit, 0) options: LFO attributes: attrs];
        CHECK(near(unlimitedHeight.size.height, wrapped.size.height, 0.01), "zero height is unlimited");

        // A height that holds one line but not two lays out one line.
        NSRect oneLine = [m boundingRectWithSize: NSMakeSize(limit, lineH * 1.5) options: LFO attributes: attrs];
        CHECK(near(oneLine.size.height, lineH, 1), "height-limited %g, line height %g", oneLine.size.height, lineH);

        // UsesDeviceMetrics reports the ink, which is tighter than the line box.
        NSRect ink = [s boundingRectWithSize: big options: LFO | NSStringDrawingUsesDeviceMetrics attributes: attrs];
        CHECK(ink.origin.y == 0 && ink.size.height > 5 && ink.size.height < lfo.size.height,
              "device metrics rect %g,%g %gx%g", ink.origin.x, ink.origin.y, ink.size.width, ink.size.height);
        CHECK(ink.size.width <= lfo.size.width + 0.01, "device metrics width %g > %g", ink.size.width, lfo.size.width);

        // NSAttributedString entry points agree with NSString ones.
        NSAttributedString *as = [[[NSAttributedString alloc] initWithString: s attributes: attrs] autorelease];
        NSRect alfo = [as boundingRectWithSize: big options: LFO];
        NSRect aone = [as boundingRectWithSize: big options: 0];
        CHECK(NSEqualRects(alfo, lfo), "attributed LFO rect differs");
        CHECK(NSEqualRects(aone, one), "attributed single-line rect differs");

        // --- NSStringDrawingContext ----------------------------------------
        Class ctxClass = NSClassFromString(@"NSStringDrawingContext");
        CHECK(ctxClass != Nil, "NSStringDrawingContext missing");
        if (ctxClass == Nil) return 1;
        id ctx = [[[ctxClass alloc] init] autorelease];
        NSRect cb = [m boundingRectWithSize: NSMakeSize(limit, 1000) options: LFO attributes: attrs context: ctx];
        NSRect total = [[ctx valueForKey: @"totalBounds"] rectValue];
        CHECK(NSEqualRects(cb, wrapped), "context bounding rect differs");
        CHECK(NSEqualRects(total, wrapped), "totalBounds %g,%g %gx%g != bounds", total.origin.x, total.origin.y,
              total.size.width, total.size.height);
        CHECK([[ctx valueForKey: @"actualScaleFactor"] doubleValue] == 1.0, "actualScaleFactor");
        CHECK(NSEqualRects([m boundingRectWithSize: NSMakeSize(limit, 1000) options: LFO attributes: attrs context: nil],
                           wrapped), "nil context");
        id ctx2 = [[[ctxClass alloc] init] autorelease];
        NSRect acb = [as boundingRectWithSize: big options: LFO context: ctx2];
        CHECK(NSEqualRects(acb, lfo) && NSEqualRects([[ctx2 valueForKey: @"totalBounds"] rectValue], lfo),
              "attributed context bounds");

        // --- drawing ---------------------------------------------------------
        for (int flipped = 0; flipped < 2; flipped++) {
            NSRect r = NSMakeRect(40, 100, 150, 60);
            NSString *h = @"H";
            CGFloat top = flipped ? r.origin.y : H - NSMaxY(r);       // rect top, in bitmap rows
            CGFloat base = flipped ? r.origin.y : H - r.origin.y;    // rect origin, in bitmap rows

            // Single line: the rect origin is the baseline; the H sits above it.
            Ink a = drawInk(flipped, ^{ [h drawWithRect: r options: 0 attributes: attrs]; });
            CHECK(a.maxx >= 0, "flipped=%d: nothing drawn without LFO", flipped);
            CHECK(near(a.maxy + 1, base, 1.5), "flipped=%d: no-LFO H bottom row %d, baseline row %g", flipped, a.maxy + 1, base);
            CHECK(a.miny < base - 8, "flipped=%d: no-LFO H does not rise above the baseline", flipped);
            CHECK(a.minx >= r.origin.x - 1 && a.minx <= r.origin.x + 6, "flipped=%d: no-LFO H x %d", flipped, a.minx);

            // LFO: the rect top is the top of the line, the baseline lies
            // (line height - descent) below it.
            Ink b = drawInk(flipped, ^{ [h drawWithRect: r options: LFO attributes: attrs]; });
            CHECK(b.maxx >= 0, "flipped=%d: nothing drawn with LFO", flipped);
            CHECK(near(b.maxy + 1, top + lineH - descent, 1.5),
                  "flipped=%d: LFO H bottom row %d, expected %g", flipped, b.maxy + 1, top + lineH - descent);
            CHECK(b.miny >= top - 1, "flipped=%d: LFO ink above the rect top", flipped);

            // The attributed string draws identically.
            NSAttributedString *ah = [[[NSAttributedString alloc] initWithString: h attributes: attrs] autorelease];
            Ink c = drawInk(flipped, ^{ [ah drawWithRect: r options: LFO context: nil]; });
            CHECK(c.miny == b.miny && c.maxy == b.maxy && c.minx == b.minx && c.maxx == b.maxx,
                  "flipped=%d: attributed LFO ink differs", flipped);
            Ink d = drawInk(flipped, ^{ [ah drawWithRect: r options: 0]; });
            CHECK(d.miny == a.miny && d.maxy == a.maxy, "flipped=%d: attributed no-LFO ink differs", flipped);

            // Single-line draws are clipped to the rect width.
            NSRect narrow = NSMakeRect(40, 100, 30, 60);
            Ink e = drawInk(flipped, ^{ [m drawWithRect: narrow options: 0 attributes: attrs]; });
            CHECK(e.maxx < NSMaxX(narrow) + 1, "flipped=%d: no-LFO ink at column %d past rect edge %g", flipped, e.maxx, NSMaxX(narrow));

            // Drawing reports the total bounds through the context.
            id dctx = [[[ctxClass alloc] init] autorelease];
            drawInk(flipped, ^{ [m drawWithRect: NSMakeRect(20, 20, limit, 200) options: LFO attributes: attrs context: dctx]; });
            NSRect dt = [[dctx valueForKey: @"totalBounds"] rectValue];
            CHECK(near(dt.size.height, wrapped.size.height, 0.01) && dt.origin.x == 0 && dt.origin.y == 0,
                  "flipped=%d: draw totalBounds %g,%g %gx%g", flipped, dt.origin.x, dt.origin.y, dt.size.width, dt.size.height);

            // Truncation: with room for one line only, the last visible line is cut and ends in an
            // ellipsis, so no second line is drawn and the ink stays inside the rect.
            NSRect tr = NSMakeRect(20, 20, limit, lineH + 4);
            CGFloat trTop = flipped ? tr.origin.y : H - NSMaxY(tr);
            Ink t = drawInk(flipped, ^{ [m drawWithRect: tr options: LFO | NSStringDrawingTruncatesLastVisibleLine attributes: attrs]; });
            CHECK(t.maxx >= 0 && t.maxx <= NSMaxX(tr), "flipped=%d: truncated ink right edge %d", flipped, t.maxx);
            CHECK(t.maxy <= trTop + lineH, "flipped=%d: truncated text beyond the first line (row %d)", flipped, t.maxy);
            NSRect tb = [m boundingRectWithSize: tr.size options: LFO | NSStringDrawingTruncatesLastVisibleLine attributes: attrs];
            CHECK(near(tb.size.height, lineH, 1) && tb.size.width <= limit + 0.01, "truncated bounds %gx%g", tb.size.width, tb.size.height);
            NSRect nt = [m boundingRectWithSize: tr.size options: LFO attributes: attrs];
            CHECK(tb.size.width >= nt.size.width - 0.01, "truncated bounds narrower than untruncated (%g < %g)", tb.size.width, nt.size.width);
        }
    }
    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    printf("OK\n");
    return 0;
}
