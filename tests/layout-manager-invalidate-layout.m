// Copyright (C) 2026 Lubos Dolezel
//
// -[NSLayoutManager invalidateLayoutForCharacterRange:actualCharacterRange:]:
// the layout of the given characters is discarded and the actual range reported
// covers the paragraphs they are in; an empty range is reported unchanged, and
// a NULL actual range pointer is allowed. Layout made afterwards is the same as
// before. Expected ranges are what macOS 26.7.3 reports for the same text.
// No display needed. MRC.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

static BOOL invalidates(NSLayoutManager *lm, NSRange range, NSRange expected)
{
    NSRange actual = NSMakeRange(99, 99);
    [lm invalidateLayoutForCharacterRange:range actualCharacterRange:&actual];
    if (!NSEqualRanges(actual, expected)) {
        printf("  {%lu,%lu} -> {%lu,%lu}, want {%lu,%lu}\n", (unsigned long)range.location, (unsigned long)range.length,
               (unsigned long)actual.location, (unsigned long)actual.length, (unsigned long)expected.location, (unsigned long)expected.length);
        return NO;
    }
    return YES;
}

int main(void)
{
    @autoreleasepool {
        NSTextStorage *ts = [[[NSTextStorage alloc] initWithString:@"hello world\nsecond line here\nthird"] autorelease];
        NSLayoutManager *lm = [[[NSLayoutManager alloc] init] autorelease];
        NSTextContainer *tc = [[[NSTextContainer alloc] initWithContainerSize:NSMakeSize(300, 1e7)] autorelease];
        [lm addTextContainer:tc];
        [ts addLayoutManager:lm];
        [lm ensureLayoutForTextContainer:tc];
        NSRect used = [lm usedRectForTextContainer:tc];

        CHECK(invalidates(lm, NSMakeRange(2, 1), NSMakeRange(0, 12)));
        CHECK(invalidates(lm, NSMakeRange(13, 3), NSMakeRange(12, 17)));
        CHECK(invalidates(lm, NSMakeRange(0, 0), NSMakeRange(0, 0)));
        CHECK(invalidates(lm, NSMakeRange(0, 34), NSMakeRange(0, 34)));
        CHECK(invalidates(lm, NSMakeRange(30, 4), NSMakeRange(29, 5)));
        [lm invalidateLayoutForCharacterRange:NSMakeRange(2, 1) actualCharacterRange:NULL];

        [lm ensureLayoutForTextContainer:tc];
        CHECK(NSEqualRects([lm usedRectForTextContainer:tc], used));
    }
    if (failures == 0)
        printf("PASS layout-manager-invalidate-layout\n");
    return failures != 0;
}
