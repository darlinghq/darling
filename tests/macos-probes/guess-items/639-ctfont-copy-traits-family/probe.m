// Probe 639: CTFontCreateCopyWithSymbolicTraits and CTFontCreateCopyWithFamily on installed fonts.
// Every result is printed as: PostScript name ; family ; symbolic traits (hex) ; weight trait ; width trait ; size.
// "NULL" means the function returned NULL. Fonts that are not installed are reported and skipped.
#import "probe_common.h"
#import <CoreText/CoreText.h>

// The documented symbolic trait bit values, spelled as numbers so the probe builds on any SDK.
enum { T_ITALIC = 1 << 0, T_BOLD = 1 << 1, T_EXPANDED = 1 << 5, T_CONDENSED = 1 << 6, T_MONO = 1 << 10,
       T_VERTICAL = 1 << 11, T_UI = 1 << 12, T_COLOR = 1 << 13, T_COMPOSITE = 1 << 14, T_CLASSMASK = 0xF0000000u };

static NSString *describe(CTFontRef f) {
    if (f == NULL) return @"NULL";
    NSString *ps = (__bridge_transfer NSString *)CTFontCopyPostScriptName(f);
    NSString *fam = (__bridge_transfer NSString *)CTFontCopyFamilyName(f);
    NSDictionary *tr = (__bridge_transfer NSDictionary *)CTFontCopyTraits(f);
    NSNumber *w = tr[(id)kCTFontWeightTrait], *wd = tr[(id)kCTFontWidthTrait];
    return PF(@"%@ ; %@ ; %08x ; w=%.3f ; width=%.3f ; size=%g", ps, fam, (unsigned)CTFontGetSymbolicTraits(f),
              [w doubleValue], [wd doubleValue], (double)CTFontGetSize(f));
}

static CTFontRef make(NSString *ps, CGFloat size) {
    CTFontRef f = CTFontCreateWithName((__bridge CFStringRef)ps, size, NULL);
    NSString *got = f ? (__bridge_transfer NSString *)CTFontCopyPostScriptName(f) : nil;
    if (f && ![got isEqualToString:ps]) { CFRelease(f); return NULL; } // substituted: not installed
    return f;
}

static void traitsCase(const char *id, NSString *ps, CGFloat size, CGFloat newSize, const CGAffineTransform *m,
                       unsigned value, unsigned mask) {
    CTFontRef o = make(ps, size);
    NSString *in = PF(@"orig=%@ size=%g -> size=%g matrix=%@ value=%08x mask=%08x", ps, (double)size,
                      (double)newSize, m ? @"given" : @"NULL", value, mask);
    if (!o) { probe_line(id, in, @"original not installed"); return; }
    CTFontRef c = CTFontCreateCopyWithSymbolicTraits(o, newSize, m, value, mask);
    probe_line(id, in, PF(@"%@ ; orig=%@", describe(c), describe(o)));
    if (c) CFRelease(c);
    CFRelease(o);
}

static void familyCase(const char *id, NSString *ps, CGFloat size, CGFloat newSize, const CGAffineTransform *m,
                       NSString *family) {
    CTFontRef o = make(ps, size);
    NSString *in = PF(@"orig=%@ size=%g -> size=%g matrix=%@ family=%@", ps, (double)size, (double)newSize,
                      m ? @"given" : @"NULL", family);
    if (!o) { probe_line(id, in, @"original not installed"); return; }
    CTFontRef c = CTFontCreateCopyWithFamily(o, newSize, m, (__bridge CFStringRef)family);
    probe_line(id, in, PF(@"%@ ; orig=%@", describe(c), describe(o)));
    if (c) CFRelease(c);
    CFRelease(o);
}

// All faces of a family (PostScript names, sorted by weight then name) found through the public descriptor API.
static NSArray<NSString *> *facesOf(NSString *family) {
    CTFontDescriptorRef d = CTFontDescriptorCreateWithAttributes(
        (__bridge CFDictionaryRef)@{(id)kCTFontFamilyNameAttribute: family});
    NSArray *ds = (__bridge_transfer NSArray *)CTFontDescriptorCreateMatchingFontDescriptors(d, NULL);
    CFRelease(d);
    NSMutableArray *out = [NSMutableArray array];
    for (id x in ds) {
        NSString *ps = (__bridge_transfer NSString *)CTFontDescriptorCopyAttribute((__bridge CTFontDescriptorRef)x,
                                                                                    kCTFontNameAttribute);
        if (ps) [out addObject:ps];
    }
    [out sortUsingSelector:@selector(compare:)];
    return out;
}

int main(void) {
    @autoreleasepool {
        probe_env("639");

        // A. How value and mask combine, from regular, bold, italic and bold-italic originals.
        NSArray *origs = @[@"Helvetica", @"Helvetica-Bold", @"Helvetica-Oblique", @"Helvetica-BoldOblique"];
        unsigned values[][2] = {{T_BOLD, T_BOLD}, {0, T_BOLD}, {T_ITALIC, T_ITALIC}, {0, T_ITALIC},
                                {T_BOLD | T_ITALIC, T_BOLD | T_ITALIC}, {0, T_BOLD | T_ITALIC},
                                {T_BOLD, 0}, {0, 0}, {T_BOLD | T_ITALIC, T_BOLD}, {T_ITALIC, T_BOLD},
                                {T_BOLD, T_BOLD | T_ITALIC}, {0xFFFFFFFFu, T_BOLD}, {T_BOLD, 0xFFFFFFFFu}};
        for (NSString *o in origs)
            for (size_t i = 0; i < sizeof values / sizeof values[0]; i++)
                traitsCase("639.A.combine", o, 12, 0, NULL, values[i][0], values[i][1]);

        // B. Size: 0 keeps the original's size; explicit size; negative size; matrix passed or not.
        traitsCase("639.B.size", @"Helvetica", 17, 0, NULL, T_BOLD, T_BOLD);
        traitsCase("639.B.size", @"Helvetica", 17, 30, NULL, T_BOLD, T_BOLD);
        traitsCase("639.B.size", @"Helvetica", 17, -5, NULL, T_BOLD, T_BOLD);
        CGAffineTransform scale2 = CGAffineTransformMakeScale(2, 2);
        for (int i = 0; i < 2; i++) {
            CTFontRef o = make(@"Helvetica", 12);
            CTFontRef c = CTFontCreateCopyWithSymbolicTraits(o, 0, i ? &scale2 : NULL, T_BOLD, T_BOLD);
            CGAffineTransform t = c ? CTFontGetMatrix(c) : CGAffineTransformIdentity;
            probe_line("639.B.matrix", PF(@"Helvetica bold copy, matrix argument %@", i ? @"scale 2" : @"NULL"),
                       PF(@"%@ ; CTFontGetMatrix=[%g %g %g %g] ; ascent=%g", describe(c), t.a, t.b, t.c, t.d,
                          c ? (double)CTFontGetAscent(c) : -1.0));
            if (c) CFRelease(c);
            CFRelease(o);
        }

        // C. Which trait bits are compared (same family): width, mono, vertical, ui, color, composite, class mask.
        unsigned bits[] = {T_EXPANDED, T_CONDENSED, T_MONO, T_VERTICAL, T_UI, T_COLOR, T_COMPOSITE, T_CLASSMASK};
        const char *names[] = {"expanded", "condensed", "mono", "vertical", "ui", "color", "composite", "classmask"};
        NSArray *cOrig = @[@"Helvetica", @"HelveticaNeue-CondensedBold", @"Courier", @"Menlo-Regular", @"Avenir-Roman"];
        for (NSString *o in cOrig)
            for (size_t i = 0; i < sizeof bits / sizeof bits[0]; i++) {
                traitsCase(PF(@"639.C.set-%s", names[i]).UTF8String, o, 12, 0, NULL, bits[i], bits[i]);
                traitsCase(PF(@"639.C.clear-%s", names[i]).UTF8String, o, 12, 0, NULL, 0, bits[i]);
            }

        // D. No such face, unknown family handled in the other section; faces that cannot be bold or italic.
        for (NSString *o in @[@"Zapfino", @"AppleChancery", @"Skia", @"Optima-Regular", @"Futura-Medium", @"Symbol",
                              @"AppleColorEmoji", @"Courier-Bold", @"Menlo-BoldItalic"]) {
            traitsCase("639.D.noface", o, 14, 0, NULL, T_BOLD, T_BOLD);
            traitsCase("639.D.noface", o, 14, 0, NULL, T_ITALIC, T_ITALIC);
            traitsCase("639.D.noface", o, 14, 0, NULL, 0, T_BOLD | T_ITALIC);
        }

        // E. Choice among several faces with the wanted traits: every face of several multi-weight families
        // asked for bold on, bold off, italic on, italic off and no change (value 0 mask 0).
        for (NSString *fam in @[@"Helvetica Neue", @"Avenir Next", @"Avenir", @"Gill Sans", @"Optima", @"Futura",
                                @"Hoefler Text", @"Palatino"]) {
            NSArray *faces = facesOf(fam);
            probe_line("639.E.faces", fam, faces.count ? [faces componentsJoinedByString:@" "] : @"family not installed");
            for (NSString *ps in faces) {
                traitsCase("639.E.bold-on", ps, 12, 0, NULL, T_BOLD, T_BOLD);
                traitsCase("639.E.bold-off", ps, 12, 0, NULL, 0, T_BOLD);
                traitsCase("639.E.italic-on", ps, 12, 0, NULL, T_ITALIC, T_ITALIC);
                traitsCase("639.E.italic-off", ps, 12, 0, NULL, 0, T_ITALIC);
                traitsCase("639.E.nochange", ps, 12, 0, NULL, 0, 0);
            }
        }

        // F. CTFontCreateCopyWithFamily: originals against target families, including ones with no matching face.
        NSArray *fOrig = @[@"Helvetica", @"Helvetica-Bold", @"Helvetica-BoldOblique", @"HelveticaNeue-CondensedBold",
                           @"HelveticaNeue-Light", @"Courier-BoldOblique", @"Menlo-Bold", @"Times-Italic",
                           @"AvenirNext-Heavy", @"ArialNarrow-Bold"];
        NSArray *targets = @[@"Helvetica", @"Helvetica Neue", @"Courier", @"Menlo", @"Times", @"Times New Roman",
                             @"Arial", @"Arial Narrow", @"Avenir Next", @"Zapfino", @"NoSuchFamilyXYZ", @"",
                             @"helvetica", @"Helvetica-Bold", @"Apple Color Emoji"];
        for (NSString *o in fOrig)
            for (NSString *t in targets) familyCase("639.F.family", o, 12, 0, NULL, t);
        familyCase("639.F.size", @"Helvetica", 17, 0, NULL, @"Courier");
        familyCase("639.F.size", @"Helvetica", 17, 31, NULL, @"Courier");
        familyCase("639.F.matrix", @"Helvetica", 12, 0, &scale2, @"Courier");

        // G. State of the original that a copy may or may not carry: a cascade list, a variation, feature settings.
        {
            CTFontRef base = make(@"Helvetica", 14);
            CTFontDescriptorRef casc = CTFontDescriptorCreateWithAttributes((__bridge CFDictionaryRef)@{
                (id)kCTFontCascadeListAttribute: @[(__bridge id)CTFontDescriptorCreateWithNameAndSize(CFSTR("Courier"), 14)]});
            CTFontRef withCascade = CTFontCreateCopyWithAttributes(base, 0, NULL, casc);
            CTFontRef cp = CTFontCreateCopyWithSymbolicTraits(withCascade, 0, NULL, T_BOLD, T_BOLD);
            CTFontDescriptorRef d = cp ? CTFontCopyFontDescriptor(cp) : NULL;
            id cascAttr = d ? (__bridge_transfer id)CTFontDescriptorCopyAttribute(d, kCTFontCascadeListAttribute) : nil;
            CTFontDescriptorRef d0 = CTFontCopyFontDescriptor(withCascade);
            id cascOrig = (__bridge_transfer id)CTFontDescriptorCopyAttribute(d0, kCTFontCascadeListAttribute);
            probe_line("639.G.cascade", @"Helvetica with an explicit cascade list [Courier], then bold copy",
                       PF(@"original descriptor has cascade attribute: %@ ; bold copy descriptor has it: %@ ; copy=%@",
                          cascOrig ? @"YES" : @"NO", cascAttr ? @"YES" : @"NO", describe(cp)));

            CTFontRef sk = make(@"Skia", 20);
            if (sk) {
                CFArrayRef axes = CTFontCopyVariationAxes(sk);
                NSDictionary *axis = axes && CFArrayGetCount(axes) ? CFArrayGetValueAtIndex(axes, 0) : nil;
                NSNumber *ident = axis[(id)kCTFontVariationAxisIdentifierKey];
                NSNumber *maxv = axis[(id)kCTFontVariationAxisMaximumValueKey];
                if (ident && maxv) {
                    CTFontDescriptorRef vd = CTFontDescriptorCreateWithAttributes(
                        (__bridge CFDictionaryRef)@{(id)kCTFontVariationAttribute: @{ident: maxv}});
                    CTFontRef var = CTFontCreateCopyWithAttributes(sk, 0, NULL, vd);
                    CTFontRef c1 = CTFontCreateCopyWithSymbolicTraits(var, 0, NULL, T_ITALIC, T_ITALIC);
                    CTFontRef c2 = CTFontCreateCopyWithSymbolicTraits(var, 0, NULL, 0, 0);
                    CTFontRef c3 = CTFontCreateCopyWithFamily(var, 0, NULL, CFSTR("Skia"));
                    CFDictionaryRef v0 = CTFontCopyVariation(var);
                    CFDictionaryRef v1 = c1 ? CTFontCopyVariation(c1) : NULL;
                    CFDictionaryRef v2 = c2 ? CTFontCopyVariation(c2) : NULL;
                    CFDictionaryRef v3 = c3 ? CTFontCopyVariation(c3) : NULL;
                    probe_line("639.G.variation", @"Skia varied on its first axis to the maximum; copies",
                               PF(@"original variation=%@ ; italic copy=%@ ; no-change copy=%@ ; same-family copy=%@ ; copies: %@ | %@ | %@",
                                  v0 ? @"set" : @"none", c1 ? (v1 ? @"set" : @"none") : @"NULL",
                                  c2 ? (v2 ? @"set" : @"none") : @"NULL", c3 ? (v3 ? @"set" : @"none") : @"NULL",
                                  describe(c1), describe(c2), describe(c3)));
                } else {
                    probe_line("639.G.variation", @"Skia", @"no variation axis found");
                }
            } else {
                probe_line("639.G.variation", @"Skia", @"not installed");
            }

            NSArray *feat = @[@{(id)kCTFontFeatureTypeIdentifierKey: @1, (id)kCTFontFeatureSelectorIdentifierKey: @3}];
            CTFontDescriptorRef fd = CTFontDescriptorCreateWithAttributes(
                (__bridge CFDictionaryRef)@{(id)kCTFontFeatureSettingsAttribute: feat});
            CTFontRef withFeat = CTFontCreateCopyWithAttributes(base, 0, NULL, fd);
            CTFontRef fc = CTFontCreateCopyWithSymbolicTraits(withFeat, 0, NULL, T_BOLD, T_BOLD);
            CFArrayRef fs = fc ? CTFontCopyFeatureSettings(fc) : NULL;
            probe_line("639.G.features", @"Helvetica with feature settings (common ligatures off), then bold copy",
                       PF(@"bold copy feature settings: %ld entries ; %@", fs ? (long)CFArrayGetCount(fs) : -1L, describe(fc)));
        }
    }
    return 0;
}
