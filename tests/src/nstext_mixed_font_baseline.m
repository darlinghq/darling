// Copyright (C) 2026 Lubos Dolezel
//
// Runs of one line that are set in different fonts share the line's baseline: NSLayoutManager reports the same
// glyph location y for every glyph of the line, and the glyphs drawn from a substituted font (the modifier
// symbols in "A<command>B") and from a second font (Helvetica next to Menlo) end on the same pixel row as the
// letters beside them.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL %s: ", #cond); printf(__VA_ARGS__); printf("\n"); } } while (0)

enum { W = 160, H = 48 };

// Draws into an RGBA bitmap (a plain CGBitmapContext, so the test needs no window server connection).
static unsigned char pixels[H * W * 4];

static void draw(NSAttributedString *str) {
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef cg = CGBitmapContextCreate(pixels, W, H, 8, W * 4, cs, kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(cs);
    CGContextSetRGBFillColor(cg, 1, 1, 1, 1);
    CGContextFillRect(cg, CGRectMake(0, 0, W, H));
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext: [NSGraphicsContext graphicsContextWithGraphicsPort: cg flipped: NO]];
    [str drawAtPoint: NSMakePoint(16, 12)];
    [NSGraphicsContext restoreGraphicsState];
    CGContextRelease(cg);
}

// Lowest dark pixel row (rows count down from the top) in the columns [x0, x1); -1 when there is none.
static int inkBottom(int x0, int x1) {
    int bottom = -1;
    for (int y = 0; y < H; y++)
        for (int x = x0; x < x1 && x < W; x++)
            if (pixels[(y * W + x) * 4] < 160)
                bottom = y;
    return bottom;
}

static NSAttributedString *piece(NSString *s, NSFont *font) {
    return [[[NSAttributedString alloc] initWithString: s attributes: @{
        NSFontAttributeName: font, NSForegroundColorAttributeName: [NSColor blackColor]}] autorelease];
}

static CGFloat width(NSAttributedString *s) { return [s size].width; }

// Ink bottoms of the three consecutive pieces, each measured in its own horizontal span.
static void inkBottoms(NSAttributedString *a, NSAttributedString *b, NSAttributedString *c, int out[3]) {
    NSMutableAttributedString *all = [[NSMutableAttributedString alloc] init];
    [all appendAttributedString: a];
    [all appendAttributedString: b];
    [all appendAttributedString: c];
    draw(all);
    int x0 = 16, x1 = 16 + (int)ceil(width(a)), x2 = x1 + (int)ceil(width(b)), x3 = x2 + (int)ceil(width(c));
    out[0] = inkBottom(x0, x1);
    out[1] = inkBottom(x1, x2);
    out[2] = inkBottom(x2, x3);
    [all release];
}

static void checkLocations(NSAttributedString *str, const char *what) {
    NSTextStorage *storage = [[NSTextStorage alloc] initWithAttributedString: str];
    NSLayoutManager *lm = [[NSLayoutManager alloc] init];
    NSTextContainer *tc = [[NSTextContainer alloc] initWithContainerSize: NSMakeSize(1000, 1000)];
    [lm addTextContainer: tc];
    [storage addLayoutManager: lm];
    (void)[lm glyphRangeForTextContainer: tc];
    NSUInteger n = [lm numberOfGlyphs];
    CHECK(n == [str length], "%s: %lu glyphs", what, (unsigned long)n);
    CGFloat y0 = [lm locationForGlyphAtIndex: 0].y;
    for (NSUInteger i = 1; i < n; i++) {
        CGFloat y = [lm locationForGlyphAtIndex: i].y;
        CHECK(y == y0, "%s: glyph %lu baseline y %.2f, first glyph %.2f", what, (unsigned long)i, y, y0);
    }
    [tc release];
    [lm release];
    [storage release];
}

int main(void) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        NSFont *menu = [NSFont menuFontOfSize: 13];
        NSFont *helv = [NSFont fontWithName: @"Helvetica" size: 13];
        NSFont *menlo = [NSFont fontWithName: @"Menlo-Regular" size: 13];
        NSString *cmd = [NSString stringWithFormat: @"%C", (unichar)0x2318];
        NSString *opt = [NSString stringWithFormat: @"%C", (unichar)0x2325];

        // Substituted font: the symbols come from a font other than the menu font.
        NSAttributedString *sym = piece([NSString stringWithFormat: @"A%@%@B", opt, cmd], menu);
        checkLocations(sym, "A option command B");
        int ink[3];
        inkBottoms(piece(@"A", menu), piece([NSString stringWithFormat: @"%@%@", opt, cmd], menu), piece(@"B", menu), ink);
        CHECK(ink[0] >= 0 && ink[1] >= 0 && ink[2] >= 0, "ink missing: %d %d %d", ink[0], ink[1], ink[2]);
        CHECK(abs(ink[1] - ink[0]) <= 1, "symbols end at row %d, letter A at row %d", ink[1], ink[0]);
        CHECK(abs(ink[1] - ink[2]) <= 1, "symbols end at row %d, letter B at row %d", ink[1], ink[2]);

        // Two named fonts of different ascent.
        if (helv != nil && menlo != nil) {
            NSMutableAttributedString *two = [[[NSMutableAttributedString alloc] init] autorelease];
            [two appendAttributedString: piece(@"HH", helv)];
            [two appendAttributedString: piece(@"HH", menlo)];
            [two appendAttributedString: piece(@"HH", helv)];
            checkLocations(two, "Helvetica Menlo Helvetica");
            inkBottoms(piece(@"HH", helv), piece(@"HH", menlo), piece(@"HH", helv), ink);
            CHECK(abs(ink[1] - ink[0]) <= 1, "Menlo ends at row %d, Helvetica at row %d", ink[1], ink[0]);
            CHECK(abs(ink[2] - ink[0]) <= 1, "Helvetica ends at row %d and row %d", ink[2], ink[0]);
        } else
            printf("skipped: Helvetica or Menlo missing\n");
    }
    if (failures == 0)
        printf("PASS\n");
    return failures != 0;
}
