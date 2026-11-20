// Copyright (C) 2026 Lubos Dolezel
//
// NSShadow supports secure keyed coding. A keyed archive keeps the offset
// under NSShadowHoriz and NSShadowVert, the blur radius under
// NSShadowBlurRadius and the colour under NSShadowColor; zero values and the
// default colour are left out. Decoding gives back the offset, blur radius and
// colour, and the default colour when the archive has none.
// Results as on macOS 26.7.3 and 10.14.4. No display needed. No ARC.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;

static void expect(BOOL ok, const char *label) {
    printf("%s %s\n", ok ? "PASS" : "FAIL", label);
    failures += !ok;
}

static NSData *archive(NSShadow *shadow) {
    NSMutableData *data = [NSMutableData data];
    NSKeyedArchiver *archiver = [[NSKeyedArchiver alloc] initForWritingWithMutableData: data];
    [archiver encodeObject: shadow forKey: @"root"];
    [archiver finishEncoding];
    [archiver release];
    return data;
}

// The keys of the shadow's own dictionary in the archive, sorted.
static NSString *keys(NSShadow *shadow) {
    NSDictionary *plist = [NSPropertyListSerialization propertyListWithData: archive(shadow) options: 0 format: NULL error: NULL];
    NSDictionary *object = [[plist objectForKey: @"$objects"] objectAtIndex: 1];
    NSMutableArray *names = [NSMutableArray array];
    for (NSString *key in [[object allKeys] sortedArrayUsingSelector: @selector(compare:)])
        if (![key isEqualToString: @"$class"])
            [names addObject: key];
    return [names componentsJoinedByString: @","];
}

static NSShadow *unarchive(NSData *data) {
    NSKeyedUnarchiver *unarchiver = [[NSKeyedUnarchiver alloc] initForReadingWithData: data];
    [(id) unarchiver setRequiresSecureCoding: YES];
    NSShadow *shadow = nil;
    @try {
        shadow = [unarchiver decodeObjectOfClass: [NSShadow class] forKey: @"root"];
    } @catch (NSException *e) {
        printf("  raised %s: %s\n", [[e name] UTF8String], [[e reason] UTF8String]);
    }
    [unarchiver finishDecoding];
    [unarchiver release];
    return shadow;
}

static void expectKeys(NSShadow *shadow, NSString *expected, const char *label) {
    NSString *got = keys(shadow);
    printf("  keys: \"%s\"\n", [got UTF8String]);
    expect([got isEqualToString: expected], label);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    @autoreleasepool {
        expect([NSShadow conformsToProtocol: @protocol(NSSecureCoding)] &&
                       [(Class<NSSecureCoding>) [NSShadow class] supportsSecureCoding],
               "NSShadow supports secure coding");

        NSShadow *plain = [[[NSShadow alloc] init] autorelease];
        NSColor *defaultColor = [plain shadowColor];
        expectKeys(plain, @"", "a default shadow archives no values");

        NSShadow *horizontal = [[[NSShadow alloc] init] autorelease];
        [horizontal setShadowOffset: NSMakeSize(3, 0)];
        expectKeys(horizontal, @"NSShadowHoriz", "a zero vertical offset is left out");

        NSShadow *black = [[[NSShadow alloc] init] autorelease];
        [black setShadowColor: [NSColor blackColor]];
        expectKeys(black, @"NSShadowColor", "a colour other than the default is archived");

        NSColor *red = [NSColor colorWithCalibratedRed: 1 green: 0 blue: 0 alpha: 0.5];
        NSShadow *full = [[[NSShadow alloc] init] autorelease];
        [full setShadowOffset: NSMakeSize(1.5, -2)];
        [full setShadowBlurRadius: 4];
        [full setShadowColor: red];
        expectKeys(full, @"NSShadowBlurRadius,NSShadowColor,NSShadowHoriz,NSShadowVert", "all values are archived under their keys");

        NSShadow *decoded = unarchive(archive(full));
        expect(decoded != nil, "a shadow is decoded with secure coding");
        expect(NSEqualSizes([decoded shadowOffset], NSMakeSize(1.5, -2)), "the offset is decoded");
        expect([decoded shadowBlurRadius] == 4, "the blur radius is decoded");
        expect([[decoded shadowColor] isEqual: red], "the colour is decoded");

        decoded = unarchive(archive(plain));
        expect(decoded != nil && NSEqualSizes([decoded shadowOffset], NSZeroSize) && [decoded shadowBlurRadius] == 0,
               "a default shadow decodes with no offset and no blur");
        expect([[decoded shadowColor] isEqual: defaultColor], "a shadow without an archived colour has the default colour");

        NSShadow *colourless = [[[NSShadow alloc] init] autorelease];
        [colourless setShadowColor: nil];
        decoded = unarchive(archive(colourless));
        expect(decoded != nil && [decoded shadowColor] == nil, "a shadow whose colour was set to nil decodes without a colour");
    }
    printf("failures=%d\n", failures);
    return failures ? 1 : 0;
}
