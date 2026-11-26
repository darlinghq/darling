// Copyright (C) 2026 Lubos Dolezel
//
// The font and foregroundColor properties of NSTextStorage (its scripting
// properties, https://developer.apple.com/documentation/appkit/nstextstorage).
// Setting one gives every character the attribute; reading one gives the
// first character's, which is the attribute's documented default (Helvetica
// 12, black) when the character has none, and nil when there are no
// characters. Observed on macOS 26.7.3 (x86_64) with the same inputs.
#import <AppKit/AppKit.h>

static int failures;

static void report(const char *name, BOOL ok)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
    if (!ok)
        failures++;
}

int main(void)
{
    @autoreleasepool {
        NSTextStorage *storage = [[[NSTextStorage alloc] init] autorelease];
        report("no characters: no color", storage.foregroundColor == nil);
        report("no characters: no font", storage.font == nil);

        [storage.mutableString setString:@"abcd"];
        report("no attribute: black", [storage.foregroundColor isEqual:[NSColor blackColor]]);
        report("no attribute: Helvetica 12", [storage.font.fontName isEqualToString:@"Helvetica"]
               && storage.font.pointSize == 12);

        storage.foregroundColor = [NSColor redColor];
        report("set: the property", [storage.foregroundColor isEqual:[NSColor redColor]]);
        report("set: every character",
               [[storage attribute:NSForegroundColorAttributeName atIndex:0 effectiveRange:NULL] isEqual:[NSColor redColor]]
               && [[storage attribute:NSForegroundColorAttributeName atIndex:3 effectiveRange:NULL] isEqual:[NSColor redColor]]);

        [storage addAttribute:NSForegroundColorAttributeName value:[NSColor blueColor] range:NSMakeRange(2, 2)];
        report("mixed: the first character's", [storage.foregroundColor isEqual:[NSColor redColor]]);
        [storage removeAttribute:NSForegroundColorAttributeName range:NSMakeRange(0, 1)];
        report("first character without one: black", [storage.foregroundColor isEqual:[NSColor blackColor]]);

        NSFont *font = [NSFont systemFontOfSize:20];
        storage.font = font;
        report("font: set and read", [storage.font isEqual:font]
               && [[storage attribute:NSFontAttributeName atIndex:3 effectiveRange:NULL] isEqual:font]);
    }
    return failures != 0;
}
