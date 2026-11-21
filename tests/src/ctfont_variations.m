/* Copyright (C) 2026 Lubos Dolezel
 * Variable font axes and settings through CoreText: CTFontCopyVariationAxes,
 * CTFontCopyVariation and the kCTFontVariationAttribute of descriptors.
 * Usage: ctfont_variations ctfont-variations-fixtures/DarlingVariableTest.ttf ctfont-variations-fixtures/DarlingStaticTest.ttf
 * Fixtures: see ctfont-variations-fixtures/make-fixture.py (SIL OFL 1.1, OFL.txt). */
#import <Foundation/Foundation.h>
#import <CoreText/CoreText.h>
#import <CoreGraphics/CoreGraphics.h>

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL line %d: %s ", __LINE__, #cond); fputs([[NSString stringWithFormat: __VA_ARGS__] UTF8String], stdout); printf("\n"); } } while (0)

static CTFontRef fontFromFile(const char *path, CGFloat size) {
    NSData *data = [NSData dataWithContentsOfFile: [NSString stringWithUTF8String: path]];
    CTFontDescriptorRef d = CTFontManagerCreateFontDescriptorFromData((CFDataRef) data);
    return d ? CTFontCreateWithFontDescriptor(d, size, NULL) : NULL;
}

static CGFloat advanceOfA(CTFontRef f) {
    UniChar c = 'A'; CGGlyph g = 0; CGSize adv = CGSizeZero;
    CTFontGetGlyphsForCharacters(f, &c, &g, 1);
    CTFontGetAdvancesForGlyphs(f, kCTFontOrientationHorizontal, &g, &adv, 1);
    return adv.width;
}

int main(int argc, char **argv) {
    @autoreleasepool {
        const uint32_t wght = 'wght', hide = 'HIDE';
        CTFontRef vf = fontFromFile(argv[1], 20);
        CTFontRef sf = fontFromFile(argv[2], 20);
        CHECK(vf != NULL && sf != NULL, @"fonts load");
        if (vf == NULL || sf == NULL) return 1;

        // Axes: wght (visible) then HIDE (hidden), as in the fvar table.
        NSArray *axes = (NSArray *) CTFontCopyVariationAxes(vf);
        CHECK([axes count] == 2, @"axes %@", axes);
        NSDictionary *w = [axes count] > 0 ? axes[0] : nil, *h = [axes count] > 1 ? axes[1] : nil;
        CHECK([w[(id) kCTFontVariationAxisIdentifierKey] unsignedIntValue] == wght, @"id %@", w);
        CHECK([w[(id) kCTFontVariationAxisMinimumValueKey] doubleValue] == 100, @"min");
        CHECK([w[(id) kCTFontVariationAxisMaximumValueKey] doubleValue] == 900, @"max");
        CHECK([w[(id) kCTFontVariationAxisDefaultValueKey] doubleValue] == 400, @"default");
        CHECK([w[(id) kCTFontVariationAxisNameKey] isEqual: @"Weight"], @"name %@", w[(id) kCTFontVariationAxisNameKey]);
        CHECK(w[(id) kCTFontVariationAxisHiddenKey] == nil, @"visible axis has no Hidden key");
        CHECK([h[(id) kCTFontVariationAxisIdentifierKey] unsignedIntValue] == hide, @"hid id");
        CHECK([h[(id) kCTFontVariationAxisNameKey] isEqual: @"Hidden Axis"], @"hid name");
        CHECK([h[(id) kCTFontVariationAxisHiddenKey] boolValue], @"hidden axis has Hidden = true");
        CHECK([w count] == 5 && [h count] == 6, @"key sets");
        [axes release];

        // A variable font at its defaults: an empty dictionary, not NULL.
        NSDictionary *none = (NSDictionary *) CTFontCopyVariation(vf);
        CHECK(none != nil && [none count] == 0, @"default variation %@", none);
        [none release];

        // A font without variations: NULL from both.
        CHECK(CTFontCopyVariationAxes(sf) == NULL, @"static axes");
        CHECK(CTFontCopyVariation(sf) == NULL, @"static variation");

        // Applying settings through a descriptor: unknown tags are dropped.
        NSDictionary *var = @{ @(wght): @700, @(hide): @1, @(0x7a7a7a7a): @5 };
        CTFontDescriptorRef vd = CTFontDescriptorCreateWithAttributes(
                (CFDictionaryRef) @{ (id) kCTFontVariationAttribute: var });
        CTFontRef c = CTFontCreateCopyWithAttributes(vf, 20, NULL, vd);
        NSDictionary *set = (NSDictionary *) CTFontCopyVariation(c);
        NSDictionary *want = @{ @(wght): @700, @(hide): @1 };
        CHECK([set isEqual: want], @"copy variation %@", set);
        [set release];
        CHECK(advanceOfA(vf) > 11.99 && advanceOfA(vf) < 12.01, @"default advance %g", advanceOfA(vf));
        CHECK(advanceOfA(c) > 15.59 && advanceOfA(c) < 15.61, @"weight 700 advance %g", advanceOfA(c));
        CTFontRef sc = CTFontCreateCopyWithAttributes(sf, 20, NULL, vd);
        CHECK(CTFontCopyVariation(sc) == NULL, @"static font stays without variation");

        // Values are clamped to the axis range.
        CTFontDescriptorRef vd3 = CTFontDescriptorCreateWithAttributes(
                (CFDictionaryRef) @{ (id) kCTFontVariationAttribute: @{ @(wght): @5000 } });
        CTFontRef c3 = CTFontCreateCopyWithAttributes(vf, 20, NULL, vd3);
        NSDictionary *set3 = (NSDictionary *) CTFontCopyVariation(c3);
        CHECK([set3 isEqual: @{ @(wght): @900 }], @"clamped %@", set3);
        [set3 release];

        // A font's descriptor carries its settings; the size change keeps them.
        CTFontDescriptorRef cd = CTFontCopyFontDescriptor(c);
        NSDictionary *attr = (NSDictionary *) CTFontDescriptorCopyAttribute(cd, kCTFontVariationAttribute);
        CHECK([attr[@(wght)] isEqual: @700] && [attr[@(hide)] isEqual: @1], @"descriptor variation %@", attr);
        CTFontRef big = CTFontCreateCopyWithAttributes(c, 40, NULL, NULL);
        CHECK(advanceOfA(big) > 31.19 && advanceOfA(big) < 31.21, @"size copy keeps variation %g", advanceOfA(big));

        // CTFontDescriptorCreateCopyWithVariation applies on font creation.
        NSData *data = [NSData dataWithContentsOfFile: [NSString stringWithUTF8String: argv[1]]];
        CTFontDescriptorRef base = CTFontManagerCreateFontDescriptorFromData((CFDataRef) data);
        CTFontDescriptorRef vd2 = CTFontDescriptorCreateCopyWithVariation(base, (CFNumberRef) @(wght), 900);
        CTFontRef c2 = CTFontCreateWithFontDescriptor(vd2, 20, NULL);
        NSDictionary *set2 = (NSDictionary *) CTFontCopyVariation(c2);
        CHECK([set2 isEqual: @{ @(wght): @900 }], @"descriptor-made variation %@", set2);
        CHECK(advanceOfA(c2) > 17.99 && advanceOfA(c2) < 18.01, @"advance at 900 %g", advanceOfA(c2));
        [set2 release];
    }
    printf(failures ? "FAILED\n" : "PASS\n");
    return failures != 0;
}
