// Probe 640: NSImageSymbolConfiguration factories and configurationByApplyingConfiguration:.
// Configurations have no public getters, so every result is observed through the symbol image it produces:
// size, number of inked pixels and the dominant colours of a rendering into a bitmap (light appearance).
// No window is shown. Needs a window-server connection only for symbol image drawing. Selectors are called by
// name at run time so the probe builds on any SDK; "n/a" means the OS does not answer the selector.
#define PROBE_APPKIT 1
#import "probe_common.h"
#import <AppKit/AppKit.h>
#import <objc/message.h>

extern NSString *const NSFontTextStyleLargeTitle, *const NSFontTextStyleTitle1, *const NSFontTextStyleTitle2,
    *const NSFontTextStyleTitle3, *const NSFontTextStyleHeadline, *const NSFontTextStyleSubheadline,
    *const NSFontTextStyleBody, *const NSFontTextStyleCallout, *const NSFontTextStyleFootnote,
    *const NSFontTextStyleCaption1, *const NSFontTextStyleCaption2;

static Class gCfg;
static NSString *const kSymbol = @"cloud.sun.rain.fill"; // multicolor, hierarchical layers, palette layers

static id call0(id t, const char *sel) {
    SEL s = sel_registerName(sel);
    return [t respondsToSelector:s] ? ((id(*)(id, SEL))objc_msgSend)(t, s) : nil;
}
static id call1(id t, const char *sel, id a) {
    SEL s = sel_registerName(sel);
    return [t respondsToSelector:s] ? ((id(*)(id, SEL, id))objc_msgSend)(t, s, a) : nil;
}
static id callInt(id t, const char *sel, NSInteger a) {
    SEL s = sel_registerName(sel);
    return [t respondsToSelector:s] ? ((id(*)(id, SEL, NSInteger))objc_msgSend)(t, s, a) : nil;
}
static id callTextScale(id t, NSString *style, NSInteger scale) {
    SEL s = sel_registerName("configurationWithTextStyle:scale:");
    return [t respondsToSelector:s] ? ((id(*)(id, SEL, id, NSInteger))objc_msgSend)(t, s, style, scale) : nil;
}
static id callSize(id t, double pt, double weight, NSInteger scale) {
    SEL s = sel_registerName("configurationWithPointSize:weight:scale:");
    return [t respondsToSelector:s] ? ((id(*)(id, SEL, double, double, NSInteger))objc_msgSend)(t, s, pt, weight, scale) : nil;
}
static id callWeight(id t, double w) {
    SEL s = sel_registerName("configurationWithWeight:");
    return [t respondsToSelector:s] ? ((id(*)(id, SEL, double))objc_msgSend)(t, s, w) : nil;
}

// Observe a symbol image: size, inked pixels (alpha >= 128), and the top colours (3 bits per channel).
static NSString *signature(NSImage *img) {
    if (img == nil) return @"image nil";
    NSSize sz = img.size;
    int w = (int)ceil(sz.width) + 2, h = (int)ceil(sz.height) + 2;
    if (w < 1 || h < 1 || w > 4000 || h > 4000) return PF(@"size=%@ (not rendered)", probe_size(sz));
    NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:w pixelsHigh:h
        bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace
        bytesPerRow:0 bitsPerPixel:32];
    NSGraphicsContext *ctx = [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:ctx];
    NSAppearance *aq = probe_appearance(NO) ?: [NSAppearance appearanceNamed:NSAppearanceNameAqua];
    void (^draw)(void) = ^{ [img drawAtPoint:NSMakePoint(1, 1) fromRect:NSZeroRect operation:(NSCompositingOperation)2 /* source-over */ fraction:1]; };
    if ([aq respondsToSelector:@selector(performAsCurrentDrawingAppearance:)]) [aq performAsCurrentDrawingAppearance:draw];
    else draw();
    [NSGraphicsContext restoreGraphicsState];
    NSMutableDictionary<NSString *, NSNumber *> *hist = [NSMutableDictionary dictionary];
    long ink = 0;
    unsigned char *d = [rep bitmapData];
    NSInteger bpr = [rep bytesPerRow];
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            unsigned char *p = d + y * bpr + x * 4; // RGBA, non-premultiplied
            if (p[3] < 128) continue;
            ink++;
            NSString *k = PF(@"%d%d%d", p[0] >> 5, p[1] >> 5, p[2] >> 5);
            hist[k] = @([hist[k] longValue] + 1);
        }
    NSArray *keys = [hist keysSortedByValueUsingComparator:^NSComparisonResult(NSNumber *a, NSNumber *b) { return [b compare:a]; }];
    NSMutableArray *top = [NSMutableArray array];
    for (NSUInteger i = 0; i < keys.count && i < 4; i++)
        [top addObject:PF(@"rgb%@:%ld%%", keys[i], ink ? 100 * [hist[keys[i]] longValue] / ink : 0L)];
    return PF(@"size=%@ ink=%ld colours=%lu top=[%@]", probe_size(sz), ink, (unsigned long)hist.count,
              [top componentsJoinedByString:@" "]);
}

static NSImage *symbolImage(id config) {
    SEL s = sel_registerName("imageWithSystemSymbolName:accessibilityDescription:");
    if (![NSImage respondsToSelector:s]) return nil;
    NSImage *i = ((id(*)(id, SEL, id, id))objc_msgSend)([NSImage class], s, kSymbol, nil);
    if (i == nil || config == nil) return i;
    return call1(i, "imageWithSymbolConfiguration:", config);
}

int main(void) {
    @autoreleasepool {
        probe_env("640");
        probe_app();
        gCfg = NSClassFromString(@"NSImageSymbolConfiguration");
        if (gCfg == nil || symbolImage(nil) == nil) {
            probe_line("640", @"NSImageSymbolConfiguration / symbol images", @"not available on this OS");
            return 0;
        }
        probe_line("640.0.plain", PF(@"symbol %@ with no configuration", kSymbol), signature(symbolImage(nil)));

        NSString *styles[] = {NSFontTextStyleLargeTitle, NSFontTextStyleTitle1, NSFontTextStyleTitle2, NSFontTextStyleTitle3,
                              NSFontTextStyleHeadline, NSFontTextStyleSubheadline, NSFontTextStyleBody, NSFontTextStyleCallout,
                              NSFontTextStyleFootnote, NSFontTextStyleCaption1, NSFontTextStyleCaption2};
        NSString *snames[] = {@"largeTitle", @"title1", @"title2", @"title3", @"headline", @"subheadline", @"body",
                              @"callout", @"footnote", @"caption1", @"caption2"};

        // A. Text-style factories: size and ink per style, and with the three scales.
        for (int i = 0; i < 11; i++) {
            id c = ((id(*)(id, SEL, id))objc_msgSend)(gCfg, sel_registerName("configurationWithTextStyle:"), styles[i]);
            probe_line("640.A.textStyle", PF(@"configurationWithTextStyle:%@", snames[i]), c ? signature(symbolImage(c)) : @"factory returned nil");
        }
        for (NSInteger sc = 1; sc <= 3; sc++) {
            id c = callTextScale(gCfg, NSFontTextStyleBody, sc);
            probe_line("640.A.textStyleScale", PF(@"configurationWithTextStyle:body scale:%ld (1 small, 2 medium, 3 large)", (long)sc),
                       c ? signature(symbolImage(c)) : @"n/a");
        }

        // B. Rendering-preference factories on their own (after a point size, so the symbol is large enough to read).
        id size64 = callSize(gCfg, 64, 0, 2);
        probe_line("640.B.alone", @"size 64 only", signature(symbolImage(size64)));
        id mono = call0(gCfg, "configurationPreferringMonochrome");
        id hier = call0(gCfg, "configurationPreferringHierarchical");
        id multi = call0(gCfg, "configurationPreferringMulticolor");
        id pal = call1(gCfg, "configurationWithPaletteColors:", @[[NSColor redColor], [NSColor greenColor], [NSColor blueColor]]);
        id palB = call1(gCfg, "configurationWithPaletteColors:", @[[NSColor purpleColor]]);
        NSArray *names = @[@"size64", @"mono", @"hier", @"multi", @"palette(r,g,b)", @"palette(purple)"];
        NSArray *cfgs = @[size64 ?: [NSNull null], mono ?: [NSNull null], hier ?: [NSNull null], multi ?: [NSNull null],
                          pal ?: [NSNull null], palB ?: [NSNull null]];
        for (NSUInteger i = 0; i < names.count; i++)
            probe_line("640.B.alone", PF(@"%@ then size 64 applied", names[i]), cfgs[i] == [NSNull null] ? @"n/a" :
                       signature(symbolImage(i == 0 ? cfgs[i] : call1(cfgs[i], "configurationByApplyingConfiguration:", size64))));

        // C. Receiver x argument. Each result compared by the reader against the rows of section B and A.
        // First the rendering preference (all with size 64 applied last so size is not the variable).
        for (NSUInteger r = 1; r < names.count; r++)
            for (NSUInteger a = 1; a < names.count; a++) {
                if (cfgs[r] == [NSNull null] || cfgs[a] == [NSNull null]) continue;
                id res = call1(cfgs[r], "configurationByApplyingConfiguration:", cfgs[a]);
                res = call1(res, "configurationByApplyingConfiguration:", size64);
                probe_line("640.C.preference", PF(@"receiver %@ . applying %@ (then size 64)", names[r], names[a]),
                           res ? signature(symbolImage(res)) : @"result nil");
            }
        // Does applying a preference-less configuration keep the receiver's preference?
        for (NSUInteger r = 1; r < names.count; r++) {
            if (cfgs[r] == [NSNull null]) continue;
            id res = call1(cfgs[r], "configurationByApplyingConfiguration:", size64);
            probe_line("640.C.keeps-preference", PF(@"receiver %@ . applying size 64 only", names[r]), res ? signature(symbolImage(res)) : @"result nil");
            res = call1(size64, "configurationByApplyingConfiguration:", cfgs[r]);
            probe_line("640.C.adds-preference", PF(@"receiver size 64 . applying %@", names[r]), res ? signature(symbolImage(res)) : @"result nil");
        }

        // D. Size, weight and text style: who replaces whom.
        id ps40b = callSize(gCfg, 40, 0.4, 2), ps20l = callSize(gCfg, 20, -0.4, 2);
        id t1 = ((id(*)(id, SEL, id))objc_msgSend)(gCfg, sel_registerName("configurationWithTextStyle:"), NSFontTextStyleTitle1);
        id body = ((id(*)(id, SEL, id))objc_msgSend)(gCfg, sel_registerName("configurationWithTextStyle:"), NSFontTextStyleBody);
        id cap2L = callTextScale(gCfg, NSFontTextStyleCaption2, 3);
        id scaleL = callInt(gCfg, "configurationWithScale:", 3);
        id weightH = callWeight(gCfg, 0.56);
        NSArray *sn = @[@"ps40-bold-medium", @"ps20-light-medium", @"title1", @"body", @"caption2+largeScale", @"scale-large-only", @"weight-heavy-only"];
        NSArray *sc = @[ps40b ?: [NSNull null], ps20l ?: [NSNull null], t1 ?: [NSNull null], body ?: [NSNull null],
                        cap2L ?: [NSNull null], scaleL ?: [NSNull null], weightH ?: [NSNull null]];
        for (NSUInteger i = 0; i < sn.count; i++)
            probe_line("640.D.alone", PF(@"%@ alone", sn[i]), sc[i] == [NSNull null] ? @"n/a" : signature(symbolImage(sc[i])));
        for (NSUInteger r = 0; r < sn.count; r++)
            for (NSUInteger a = 0; a < sn.count; a++) {
                if (sc[r] == [NSNull null] || sc[a] == [NSNull null]) continue;
                id res = call1(sc[r], "configurationByApplyingConfiguration:", sc[a]);
                probe_line("640.D.size-weight-style", PF(@"receiver %@ . applying %@", sn[r], sn[a]),
                           res ? signature(symbolImage(res)) : @"result nil");
            }

        // E. Applying to nil, to itself; equality and identity of results.
        probe_line("640.E.nil-argument", @"size64 . applying nil", ({ NSString *r = probe_try(^{ call1(size64, "configurationByApplyingConfiguration:", nil); }); r; }));
        id self2 = call1(size64, "configurationByApplyingConfiguration:", size64);
        probe_line("640.E.identity", @"size64 . applying size64", PF(@"returned new object: %@ ; isEqual to size64: %@ ; isEqual to ps20l: %@",
                   self2 != size64 ? @"YES" : @"NO", [self2 isEqual:size64] ? @"YES" : @"NO", [self2 isEqual:ps20l] ? @"YES" : @"NO"));
        probe_line("640.E.equality", @"two configurationPreferringMonochrome objects isEqual / same object",
                   PF(@"%@ / %@", [mono isEqual:call0(gCfg, "configurationPreferringMonochrome")] ? @"YES" : @"NO",
                      mono == call0(gCfg, "configurationPreferringMonochrome") ? @"YES" : @"NO"));
        probe_line("640.E.equality", @"mono isEqual hier ; multi isEqual hier",
                   PF(@"%@ ; %@", [mono isEqual:hier] ? @"YES" : @"NO", [multi isEqual:hier] ? @"YES" : @"NO"));

        // F. Secure-coding round trip: does the state survive? (Archive contents are not read, only the result.)
        probe_line("640.F.secure-coding", @"supportsSecureCoding", [gCfg respondsToSelector:@selector(supportsSecureCoding)] ?
                   ([gCfg supportsSecureCoding] ? @"YES" : @"NO") : @"n/a");
        NSArray *rt = @[ps40b ?: [NSNull null], t1 ?: [NSNull null], hier ?: [NSNull null], mono ?: [NSNull null],
                        multi ?: [NSNull null], pal ?: [NSNull null],
                        (ps40b && hier) ? call1(ps40b, "configurationByApplyingConfiguration:", hier) : [NSNull null]];
        NSArray *rn = @[@"ps40-bold", @"title1", @"hierarchical", @"monochrome", @"multicolor", @"palette(r,g,b)", @"ps40-bold . hierarchical"];
        for (NSUInteger i = 0; i < rt.count; i++) {
            if (rt[i] == [NSNull null]) continue;
            NSError *err = nil;
            NSData *data = [NSKeyedArchiver archivedDataWithRootObject:rt[i] requiringSecureCoding:YES error:&err];
            if (!data) { probe_line("640.F.round-trip", rn[i], PF(@"archive failed: %@/%ld", err.domain, (long)err.code)); continue; }
            NSError *e2 = nil;
            id back = [NSKeyedUnarchiver unarchivedObjectOfClass:gCfg fromData:data error:&e2];
            probe_line("640.F.round-trip", rn[i], back ? PF(@"decoded ; before: %@ ; after: %@ ; isEqual %@", signature(symbolImage(rt[i])),
                                                           signature(symbolImage(back)), [back isEqual:rt[i]] ? @"YES" : @"NO")
                                                       : PF(@"decode failed: %@/%ld", e2.domain, (long)e2.code));
        }

        // G. The enum-valued factories newer than the documented four (names read from the Mac's own header by the
        // human; here the raw values 0..3 are tried so the effect of each value is visible).
        for (NSInteger v = 0; v <= 3; v++) {
            for (int k = 0; k < 2; k++) {
                const char *sel = k ? "configurationWithVariableValueMode:" : "configurationWithColorRenderingMode:";
                __block NSString *out = nil;
                NSString *exc = probe_try(^{
                    id c = callInt(gCfg, sel, v);
                    out = c ? signature(symbolImage(call1(c, "configurationByApplyingConfiguration:", size64))) : @"selector n/a or nil";
                });
                probe_line(k ? "640.G.variableValueMode" : "640.G.colorRenderingMode", PF(@"raw value %ld", (long)v),
                           out ? out : exc);
            }
        }
    }
    return 0;
}
