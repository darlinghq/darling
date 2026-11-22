// Probe 0215: CTFontCreateCopyWithAttributes on a font made from font DATA (CGFontCreateWithDataProvider), a
// font that is not installed in any font registry. Does a copy that only changes size, matrix or rendering
// attributes keep the very same face (same graphics font), and what does a copy that names another face do?
// Usage: ./probe [path-to-a-single-face-.ttf-or-.otf]   (default: the file of a stock system font)
#import "probe_common.h"
#import <CoreText/CoreText.h>
#import <CoreGraphics/CoreGraphics.h>

static NSString *describe(CTFontRef f, CGFontRef original) {
    if (f == NULL)
        return @"NULL";
    CFStringRef ps = CTFontCopyPostScriptName(f);
    CGAffineTransform m = CTFontGetMatrix(f);
    CGFontRef g = CTFontCopyGraphicsFont(f, NULL);
    UniChar ch = 'A';
    CGGlyph glyph = 0;
    CTFontGetGlyphsForCharacters(f, &ch, &glyph, 1);
    CGSize adv;
    CTFontGetAdvancesForGlyphs(f, kCTFontOrientationHorizontal, &glyph, &adv, 1);
    NSString *s = PF(@"ps=%@ size=%g matrix=[%g %g %g %g] sameCGFont=%d equalCGFont=%d glyphA=%d advanceA=%g glyphCount=%ld",
                     (__bridge NSString *)ps, (double)CTFontGetSize(f), (double)m.a, (double)m.b, (double)m.c,
                     (double)m.d, (int)(g == original), (int)(g != NULL && CFEqual(g, original)), (int)glyph,
                     (double)adv.width, (long)CTFontGetGlyphCount(f));
    if (ps) CFRelease(ps);
    if (g) CGFontRelease(g);
    return s;
}

static CTFontDescriptorRef descriptorWith(NSDictionary *attrs) {
    return CTFontDescriptorCreateWithAttributes((__bridge CFDictionaryRef)attrs);
}

int main(int argc, char **argv) {
    @autoreleasepool {
        probe_env("0215");
        NSURL *url = nil;
        if (argc > 1) {
            url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:argv[1]]];
        } else {
            CTFontRef sys = CTFontCreateWithName(CFSTR("ArialMT"), 12, NULL);
            url = (__bridge_transfer NSURL *)CTFontCopyAttribute(sys, kCTFontURLAttribute);
            CFRelease(sys);
        }
        NSData *fontData = url ? [NSData dataWithContentsOfURL:url] : nil;
        CGDataProviderRef dp = fontData ? CGDataProviderCreateWithCFData((__bridge CFDataRef)fontData) : NULL;
        CGFontRef cg = dp ? CGFontCreateWithDataProvider(dp) : NULL;
        if (cg == NULL) {
            probe_line("0215", @"font source", @"could not make a CGFont from the file; pass a .ttf path as argument");
            return 1;
        }
        CFStringRef psName = CGFontCopyPostScriptName(cg);
        probe_line("0215", @"font source", PF(@"CGFont from data provider ; ps=%@", (__bridge NSString *)psName));
        CTFontRef base = CTFontCreateWithGraphicsFont(cg, 18, NULL, NULL);
        probe_line("0215.base", @"CTFontCreateWithGraphicsFont(size 18)", describe(base, cg));

        CGAffineTransform scale2 = CGAffineTransformMakeScale(2, 2);
        CTFontRef c;

        c = CTFontCreateCopyWithAttributes(base, 24, NULL, NULL);
        probe_line("0215.copy", @"size 24, matrix NULL, attributes NULL", describe(c, cg));
        if (c) CFRelease(c);
        c = CTFontCreateCopyWithAttributes(base, 0, &scale2, NULL);
        probe_line("0215.copy", @"size 0 (keep), matrix scale 2, attributes NULL", describe(c, cg));
        if (c) CFRelease(c);

        NSArray *feat = @[@{(id)kCTFontFeatureTypeIdentifierKey: @1 /* kLigaturesType */,
                            (id)kCTFontFeatureSelectorIdentifierKey: @3 /* kCommonLigaturesOffSelector */}];
        CTFontDescriptorRef d1 = descriptorWith(@{(id)kCTFontFeatureSettingsAttribute: feat});
        c = CTFontCreateCopyWithAttributes(base, 0, NULL, d1);
        probe_line("0215.copy", @"attributes: feature settings only (common ligatures off)", describe(c, cg));
        if (c) {
            CFArrayRef f = CTFontCopyFeatureSettings(c);
            probe_line("0215.copy", @"  feature settings of the copy", PF(@"%ld entries", f ? (long)CFArrayGetCount(f) : -1L));
            if (f) CFRelease(f);
            CFRelease(c);
        }
        CFRelease(d1);

        CTFontDescriptorRef d2 = descriptorWith(@{(id)kCTFontSizeAttribute: @30});
        c = CTFontCreateCopyWithAttributes(base, 0, NULL, d2);
        probe_line("0215.copy", @"attributes: size 30 in the descriptor, size argument 0", describe(c, cg));
        if (c) CFRelease(c);
        CFRelease(d2);

        CTFontDescriptorRef d3 = descriptorWith(@{(id)kCTFontNameAttribute: @"Helvetica"});
        c = CTFontCreateCopyWithAttributes(base, 0, NULL, d3);
        probe_line("0215.copy", @"attributes: name Helvetica (another installed face)", describe(c, cg));
        if (c) CFRelease(c);
        CFRelease(d3);

        CTFontDescriptorRef d4 = descriptorWith(@{(id)kCTFontTraitsAttribute: @{(id)kCTFontSymbolicTrait: @(kCTFontBoldTrait)}});
        c = CTFontCreateCopyWithAttributes(base, 0, NULL, d4);
        probe_line("0215.copy", @"attributes: traits bold", describe(c, cg));
        if (c) CFRelease(c);
        CFRelease(d4);

        CTFontDescriptorRef own = CTFontCopyFontDescriptor(base);
        CTFontRef again = CTFontCreateWithFontDescriptor(own, 18, NULL);
        probe_line("0215.fromDescriptor", @"CTFontCreateWithFontDescriptor(descriptor of the data font, 18)", describe(again, cg));
        if (again) CFRelease(again);
        CFRelease(own);
        CFRelease(base);
        CFRelease(cg);
        CFRelease(dp);
        if (psName) CFRelease(psName);
    }
    return 0;
}
