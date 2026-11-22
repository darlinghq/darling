// Probe 0033: geometry and drawing of NSScroller: class-method widths, the rectangle of each part for legacy and
// overlay styles, and a few pixels of a rendered legacy scroller in light and dark appearance.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSString *partName(NSScrollerPart p) {
    switch (p) {
    case NSScrollerNoPart: return @"NoPart";
    case NSScrollerDecrementPage: return @"DecrementPage";
    case NSScrollerKnob: return @"Knob";
    case NSScrollerIncrementPage: return @"IncrementPage";
    case NSScrollerDecrementLine: return @"DecrementLine";
    case NSScrollerIncrementLine: return @"IncrementLine";
    case NSScrollerKnobSlot: return @"KnobSlot";
    default: return @"?";
    }
}

int main(void) {
    @autoreleasepool {
        probe_env("0033");
        probe_app();
        probe_line("0033.class", @"+preferredScrollerStyle", PF(@"%ld (0 legacy, 1 overlay)", (long)[NSScroller preferredScrollerStyle]));
        NSControlSize sizes[] = {NSControlSizeRegular, NSControlSizeSmall, NSControlSizeMini};
        const char *sn[] = {"regular", "small", "mini"};
        for (int i = 0; i < 3; i++)
            for (NSScrollerStyle st = NSScrollerStyleLegacy; st <= NSScrollerStyleOverlay; st++)
                probe_line("0033.width", PF(@"+scrollerWidthForControlSize:%s scrollerStyle:%ld", sn[i], (long)st),
                           PF(@"%g", (double)[NSScroller scrollerWidthForControlSize:sizes[i] scrollerStyle:st]));
        for (int vertical = 1; vertical >= 0; vertical--) {
            for (NSScrollerStyle st = NSScrollerStyleLegacy; st <= NSScrollerStyleOverlay; st++) {
                NSRect f = vertical ? NSMakeRect(0, 0, 15, 200) : NSMakeRect(0, 0, 200, 15);
                NSScroller *s = [[NSScroller alloc] initWithFrame:f];
                [s setScrollerStyle:st];
                [s setEnabled:YES];
                [s setKnobProportion:0.3];
                [s setDoubleValue:0.5];
                probe_line("0033.instance", PF(@"%s scroller frame %@ style %ld", vertical ? "vertical" : "horizontal", probe_rect(f), (long)st),
                           PF(@"-scrollerStyle=%ld -knobStyle=%ld -arrowsPosition=%ld -controlSize=%ld", (long)[s scrollerStyle], (long)[s knobStyle],
                              (long)[s arrowsPosition], (long)[s controlSize]));
                for (NSScrollerPart p = NSScrollerNoPart; p <= NSScrollerKnobSlot; p++)
                    probe_line("0033.rectForPart", PF(@"%s style %ld part %@", vertical ? "vertical" : "horizontal", (long)st, partName(p)),
                               probe_rect([s rectForPart:p]));
                probe_line("0033.testPart", PF(@"%s style %ld point at 50%% of length, middle of width", vertical ? "vertical" : "horizontal", (long)st),
                           PF(@"%@", partName([s testPart:vertical ? NSMakePoint(7.5, 100) : NSMakePoint(100, 7.5)])));
            }
        }
        for (int dark = 0; dark <= 1; dark++) {
            NSScroller *s = [[NSScroller alloc] initWithFrame:NSMakeRect(0, 0, 15, 200)];
            [s setScrollerStyle:NSScrollerStyleLegacy];
            [s setEnabled:YES];
            [s setKnobProportion:0.3];
            [s setDoubleValue:0.5];
            NSWindow *w = probe_window(NSMakeRect(0, 0, 15, 200), dark);
            [[w contentView] addSubview:s];
            NSBitmapImageRep *rep = probe_snapshot(s);
            NSInteger px = rep ? [rep pixelsWide] : 15, py = rep ? [rep pixelsHigh] : 200;
            double sx = px / 15.0, sy = py / 200.0;
            NSMutableArray *samples = [NSMutableArray array];
            double ys[] = {2, 20, 60, 100, 140, 180, 198};
            double xs[] = {0.5, 3, 7.5, 12, 14.5};
            for (size_t yi = 0; yi < sizeof ys / sizeof ys[0]; yi++)
                for (size_t xi = 0; xi < sizeof xs / sizeof xs[0]; xi++)
                    [samples addObject:PF(@"(%g,%g)=%@", xs[xi], ys[yi], probe_pixel(rep, (NSInteger)(xs[xi] * sx), (NSInteger)(ys[yi] * sy)))];
            probe_line("0033.pixels", PF(@"legacy vertical scroller 15x200, %s appearance, bitmap %ldx%ld, samples at (x,y from the top, in points) sRGB rrggbbaa", dark ? "dark" : "light", (long)px, (long)py),
                       [samples componentsJoinedByString:@" "]);
        }
    }
    return 0;
}
