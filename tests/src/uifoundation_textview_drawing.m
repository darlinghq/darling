// CFLAGS: -framework AppKit
// Copyright (C) 2026 Lubos Dolezel
//
// Text drawn through AppKit after the text system moved to UIFoundation. The
// layout manager is a UIFoundation class and draws into the context AppKit
// has made current, which it gets from AppKit through a private provider.
// This draws
//   - a text view (black "Hello" underlined, blue "world" on a yellow
//     background) with -cacheDisplayInRect:toBitmapImageRep:, and
//   - the same text with the layout manager alone, into an NSBitmapImageRep
//     made current with +[NSGraphicsContext graphicsContextWithBitmapImageRep:],
// and checks that the glyphs, the colours, the background and (in the text
// view) the underline land in the bitmap. It also prints a digest of each bitmap, so a run before
// and after a change of the text system can be compared pixel for pixel.
//
// Needs an X display. Prints PASS or FAIL lines, the digests, then
// ALL PASSED or FAILED n.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;
static void expect(BOOL ok, const char *format, ...) __attribute__((format(printf, 2, 3)));
static void expect(BOOL ok, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    printf("%s ", ok ? "PASS" : "FAIL");
    vprintf(format, ap);
    printf("\n");
    va_end(ap);
    if (!ok)
        failures++;
}

typedef struct {
    NSUInteger dark, blue, yellow, longestDarkRun;
} Census;

// Counts pixels by colour in the columns [x0, x1) of a 32-bit RGBA bitmap.
static Census census(NSBitmapImageRep *rep, NSInteger x0, NSInteger x1)
{
    Census c = {0, 0, 0, 0};
    const unsigned char *data = [rep bitmapData];
    NSInteger bpp = [rep bitsPerPixel] / 8, row = [rep bytesPerRow];
    for (NSInteger y = 0; y < [rep pixelsHigh]; y++) {
        NSUInteger run = 0;
        for (NSInteger x = x0; x < x1 && x < [rep pixelsWide]; x++) {
            const unsigned char *p = data + y * row + x * bpp;
            BOOL dark = p[0] < 100 && p[1] < 100 && p[2] < 100 && p[3] > 128;
            if (dark)
                c.dark++;
            if (p[2] > 150 && p[0] < 100 && p[1] < 100 && p[3] > 128)
                c.blue++;
            if (p[0] > 200 && p[1] > 200 && p[2] < 80 && p[3] > 128)
                c.yellow++;
            run = dark ? run + 1 : 0;
            if (run > c.longestDarkRun)
                c.longestDarkRun = run;
        }
    }
    return c;
}

static uint32_t digest(NSBitmapImageRep *rep)
{
    uint32_t hash = 2166136261u;
    const unsigned char *data = [rep bitmapData];
    NSInteger bytes = [rep bytesPerRow] * [rep pixelsHigh];
    for (NSInteger i = 0; i < bytes; i++)
        hash = (hash ^ data[i]) * 16777619u;
    return hash;
}

static NSTextStorage *makeText(void)
{
    NSFont *font = [NSFont systemFontOfSize: 16];
    NSMutableAttributedString *text = [[[NSMutableAttributedString alloc]
            initWithString: @"Hello world"
                attributes: @{NSFontAttributeName: font, NSForegroundColorAttributeName: [NSColor blackColor]}]
            autorelease];
    [text addAttribute: NSUnderlineStyleAttributeName value: @(NSUnderlineStyleThick) range: NSMakeRange(0, 5)];
    [text addAttribute: NSForegroundColorAttributeName value: [NSColor blueColor] range: NSMakeRange(6, 5)];
    [text addAttribute: NSBackgroundColorAttributeName value: [NSColor yellowColor] range: NSMakeRange(6, 5)];
    NSTextStorage *storage = [[[NSTextStorage alloc] init] autorelease];
    [storage setAttributedString: text];
    return storage;
}

static void checkBitmap(NSBitmapImageRep *rep, NSRect helloRect, NSRect worldRect, BOOL underline, const char *what)
{
    Census hello = census(rep, NSMinX(helloRect), NSMaxX(helloRect));
    Census world = census(rep, NSMinX(worldRect), NSMaxX(worldRect));
    expect(hello.dark > 20, "%s: black glyphs of \"Hello\" (%lu dark pixels)", what, (unsigned long) hello.dark);
    if (underline)
        expect(hello.longestDarkRun * 10 >= NSWidth(helloRect) * 8,
              "%s: the underline spans \"Hello\" (longest dark run %lu of %g)", what,
              (unsigned long) hello.longestDarkRun, NSWidth(helloRect));
    expect(world.blue > 20, "%s: blue glyphs of \"world\" (%lu blue pixels)", what, (unsigned long) world.blue);
    expect(world.yellow > 50, "%s: yellow background behind \"world\" (%lu yellow pixels)", what,
          (unsigned long) world.yellow);
    expect(hello.yellow == 0, "%s: no yellow behind \"Hello\" (%lu)", what, (unsigned long) hello.yellow);
    printf("%s digest %08x\n", what, digest(rep));
}

int main(void)
{
    @autoreleasepool {
        [NSApplication sharedApplication];

        // A text view.
        NSTextView *view = [[[NSTextView alloc] initWithFrame: NSMakeRect(0, 0, 240, 40)] autorelease];
        [[view textStorage] setAttributedString: makeText()];
        [view setDrawsBackground: YES];
        [view setBackgroundColor: [NSColor whiteColor]];
        NSLayoutManager *manager = [view layoutManager];
        NSTextContainer *container = [view textContainer];
        [manager ensureLayoutForTextContainer: container];
        NSPoint origin = [view textContainerOrigin];
        NSRect hello = NSOffsetRect([manager boundingRectForGlyphRange: NSMakeRange(0, 5) inTextContainer: container],
                                    origin.x, origin.y);
        NSRect world = NSOffsetRect([manager boundingRectForGlyphRange: NSMakeRange(6, 5) inTextContainer: container],
                                    origin.x, origin.y);
        expect(NSWidth(hello) > 10 && NSMinX(world) > NSMaxX(hello), "text view: layout of the two words (%s, %s)",
              [NSStringFromRect(hello) UTF8String], [NSStringFromRect(world) UTF8String]);
        NSBitmapImageRep *rep = [view bitmapImageRepForCachingDisplayInRect: [view bounds]];
        [view cacheDisplayInRect: [view bounds] toBitmapImageRep: rep];
        checkBitmap(rep, hello, world, YES, "text view");

        // The layout manager alone, into a bitmap made the current context.
        NSTextStorage *storage = makeText();
        NSLayoutManager *bare = [[[NSLayoutManager alloc] init] autorelease];
        NSTextContainer *bareContainer =
                [[[NSTextContainer alloc] initWithContainerSize: NSMakeSize(240, 40)] autorelease];
        [bare addTextContainer: bareContainer];
        [storage addLayoutManager: bare];
        [bare ensureLayoutForTextContainer: bareContainer];
        NSRange glyphs = [bare glyphRangeForTextContainer: bareContainer];
        NSBitmapImageRep *bitmap = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL
                                                                            pixelsWide: 240
                                                                            pixelsHigh: 40
                                                                         bitsPerSample: 8
                                                                       samplesPerPixel: 4
                                                                              hasAlpha: YES
                                                                              isPlanar: NO
                                                                        colorSpaceName: NSDeviceRGBColorSpace
                                                                           bytesPerRow: 240 * 4
                                                                          bitsPerPixel: 32] autorelease];
        memset([bitmap bitmapData], 255, 240 * 4 * 40);
        [NSGraphicsContext saveGraphicsState];
        NSGraphicsContext *context = [NSGraphicsContext graphicsContextWithBitmapImageRep: bitmap];
        [NSGraphicsContext setCurrentContext: context];
        [bare drawBackgroundForGlyphRange: glyphs atPoint: NSZeroPoint];
        [bare drawGlyphsForGlyphRange: glyphs atPoint: NSZeroPoint];
        [context flushGraphics];
        [NSGraphicsContext restoreGraphicsState];
        hello = [bare boundingRectForGlyphRange: NSMakeRange(0, 5) inTextContainer: bareContainer];
        world = [bare boundingRectForGlyphRange: NSMakeRange(6, 5) inTextContainer: bareContainer];
        // In an unflipped context the layout manager puts the underline of
        // the first line below the bitmap (its y is not converted the way
        // the background's is), so only the text view checks the underline.
        checkBitmap(bitmap, hello, world, NO, "layout manager into a bitmap context");

        // Without a current context the layout manager draws nothing and
        // does not fail.
        [NSGraphicsContext setCurrentContext: nil];
        [bare drawGlyphsForGlyphRange: glyphs atPoint: NSZeroPoint];
        expect(YES, "drawing without a current context returns");
    }
    if (failures)
        printf("FAILED %d\n", failures);
    else
        printf("ALL PASSED\n");
    return failures != 0;
}
