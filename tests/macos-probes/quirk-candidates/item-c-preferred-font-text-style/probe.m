// Item (c): the fonts +[NSFont preferredFontForTextStyle:options:] answers for each text style, and the
// related system font sizes, by public names. Prints point size, name, weight and metrics.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSString *weightOf(NSFont *f) {
    NSDictionary *traits = [[f fontDescriptor] objectForKey:NSFontTraitsAttribute];
    NSNumber *w = traits[NSFontWeightTrait];
    return w ? PF(@"%g", [w doubleValue]) : @"n/a";
}

static NSString *describe(NSFont *f) {
    if (f == nil) return @"nil";
    return PF(@"fontName=%@ family=%@ pointSize=%g weightTrait=%@ symbolicTraits=0x%x ascender=%g descender=%g leading=%g capHeight=%g xHeight=%g boundingRectForFont=%@",
              [f fontName], [f familyName], (double)[f pointSize], weightOf(f), (unsigned)[[f fontDescriptor] symbolicTraits],
              (double)[f ascender], (double)[f descender], (double)[f leading], (double)[f capHeight], (double)[f xHeight], probe_rect([f boundingRectForFont]));
}

int main(void) {
    @autoreleasepool {
        probe_env("item-c");
        probe_app();
        probe_line("item-c.system", @"+systemFontSize", PF(@"%g", (double)[NSFont systemFontSize]));
        probe_line("item-c.system", @"+smallSystemFontSize", PF(@"%g", (double)[NSFont smallSystemFontSize]));
        probe_line("item-c.system", @"+labelFontSize", PF(@"%g", (double)[NSFont labelFontSize]));
        probe_line("item-c.system", @"+systemFontSizeForControlSize: regular/small/mini", PF(@"%g %g %g",
                   (double)[NSFont systemFontSizeForControlSize:NSControlSizeRegular], (double)[NSFont systemFontSizeForControlSize:NSControlSizeSmall],
                   (double)[NSFont systemFontSizeForControlSize:NSControlSizeMini]));
        probe_line("item-c.system", @"+messageFontOfSize:0", describe([NSFont messageFontOfSize:0]));
        probe_line("item-c.system", @"+menuFontOfSize:0", describe([NSFont menuFontOfSize:0]));
        probe_line("item-c.system", @"+titleBarFontOfSize:0", describe([NSFont titleBarFontOfSize:0]));
        probe_line("item-c.system", @"+toolTipsFontOfSize:0", describe([NSFont toolTipsFontOfSize:0]));
        probe_line("item-c.system", @"+controlContentFontOfSize:0", describe([NSFont controlContentFontOfSize:0]));
        probe_line("item-c.system", @"+systemFontOfSize:0", describe([NSFont systemFontOfSize:0]));
        probe_line("item-c.system", @"+boldSystemFontOfSize:0", describe([NSFont boldSystemFontOfSize:0]));
#if PROBE_SDK_AT_LEAST(110000)
        NSArray *values = @[NSFontTextStyleLargeTitle, NSFontTextStyleTitle1, NSFontTextStyleTitle2, NSFontTextStyleTitle3,
                            NSFontTextStyleHeadline, NSFontTextStyleSubheadline, NSFontTextStyleBody, NSFontTextStyleCallout,
                            NSFontTextStyleFootnote, NSFontTextStyleCaption1, NSFontTextStyleCaption2];
        NSArray *names = @[@"NSFontTextStyleLargeTitle", @"NSFontTextStyleTitle1", @"NSFontTextStyleTitle2", @"NSFontTextStyleTitle3",
                           @"NSFontTextStyleHeadline", @"NSFontTextStyleSubheadline", @"NSFontTextStyleBody", @"NSFontTextStyleCallout",
                           @"NSFontTextStyleFootnote", @"NSFontTextStyleCaption1", @"NSFontTextStyleCaption2"];
        for (NSUInteger i = 0; i < [values count]; i++) {
            NSString *style = values[i];
            NSString *nm = names[i];
            probe_line("item-c.constant", nm, PF(@"\"%@\"", style));
            probe_line("item-c.preferredFont", PF(@"+preferredFontForTextStyle:%@ options:@{}", nm), describe([NSFont preferredFontForTextStyle:style options:@{}]));
            NSFontDescriptor *d = [NSFontDescriptor preferredFontDescriptorForTextStyle:style options:@{}];
            probe_line("item-c.descriptor", PF(@"+[NSFontDescriptor preferredFontDescriptorForTextStyle:%@ options:@{}]", nm),
                       d ? PF(@"pointSize=%g postscriptName=%@", (double)[[d objectForKey:NSFontSizeAttribute] doubleValue], [d postscriptName]) : @"nil");
        }
        probe_line("item-c.preferredFont", @"+preferredFontForTextStyle:@\"not-a-style\" options:@{}", describe([NSFont preferredFontForTextStyle:@"not-a-style" options:@{}]));
#else
        probe_line("item-c.preferredFont", @"+preferredFontForTextStyle:options:", @"SKIPPED: the SDK this probe was built with predates macOS 11");
#endif
    }
    return 0;
}
