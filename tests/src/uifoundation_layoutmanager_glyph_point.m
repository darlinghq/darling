// Copyright (C) 2026 Lubos Dolezel
//
// NSLayoutManager: -glyphIndexForPoint:inTextContainer:fractionOfDistanceThroughGlyph: accepts NULL for the
// fraction (the documented out parameter is optional) and returns the same glyph index as with a fraction
// pointer, which then lies in [0, 1]; -hyphenationFactor / -setHyphenationFactor: exist, start at 0 and read back
// what was set.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL %s: ", #cond); printf(__VA_ARGS__); printf("\n"); } } while (0)

int main(void) {
    @autoreleasepool {
        NSAttributedString *str = [[[NSAttributedString alloc] initWithString: @"Hello world, hello glyphs"
            attributes: @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica" size: 14]}] autorelease];
        NSTextStorage *storage = [[NSTextStorage alloc] initWithAttributedString: str];
        NSLayoutManager *lm = [[NSLayoutManager alloc] init];
        NSTextContainer *tc = [[NSTextContainer alloc] initWithContainerSize: NSMakeSize(400, 100)];
        [lm addTextContainer: tc];
        [storage addLayoutManager: lm];
        (void)[lm glyphRangeForTextContainer: tc];

        NSPoint points[] = { {1, 4}, {20, 6}, {60, 8}, {150, 6} };
        for (size_t i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
            CGFloat fraction = -1;
            NSUInteger withFraction = [lm glyphIndexForPoint: points[i] inTextContainer: tc fractionOfDistanceThroughGlyph: &fraction];
            NSUInteger withNull = [lm glyphIndexForPoint: points[i] inTextContainer: tc fractionOfDistanceThroughGlyph: NULL];
            CHECK(withFraction == withNull, "point %zu: index %lu with a fraction, %lu with NULL", i, (unsigned long)withFraction, (unsigned long)withNull);
            CHECK(fraction >= 0 && fraction <= 1, "point %zu: fraction %f", i, (double)fraction);
            CHECK(withNull < [lm numberOfGlyphs], "point %zu: glyph %lu of %lu", i, (unsigned long)withNull, (unsigned long)[lm numberOfGlyphs]);
        }
        CHECK([lm glyphIndexForPoint: NSMakePoint(20, 6) inTextContainer: tc] ==
              [lm glyphIndexForPoint: NSMakePoint(20, 6) inTextContainer: tc fractionOfDistanceThroughGlyph: NULL],
              "the two-argument form agrees with the NULL form");

        CHECK([lm hyphenationFactor] == 0, "default hyphenation factor %f", (double)[lm hyphenationFactor]);
        [lm setHyphenationFactor: 0.5f];
        CHECK([lm hyphenationFactor] == 0.5f, "hyphenation factor after set %f", (double)[lm hyphenationFactor]);

        [tc release];
        [lm release];
        [storage release];
    }
    if (failures) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
