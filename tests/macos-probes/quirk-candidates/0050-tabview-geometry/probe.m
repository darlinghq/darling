// Probe 0050: geometry of NSTabView (content rectangle, minimum size, where each tab is, by hit testing) for the
// tab view types, and a grid of pixels of a rendered tab view in light and dark appearance.
#define PROBE_APPKIT 1
#import "probe_common.h"

int main(void) {
    @autoreleasepool {
        probe_env("0050");
        probe_app();
        NSTabViewType types[] = {NSTopTabsBezelBorder, NSLeftTabsBezelBorder, NSBottomTabsBezelBorder, NSRightTabsBezelBorder, NSNoTabsBezelBorder, NSNoTabsLineBorder, NSNoTabsNoBorder};
        const char *tn[] = {"TopTabsBezelBorder", "LeftTabsBezelBorder", "BottomTabsBezelBorder", "RightTabsBezelBorder", "NoTabsBezelBorder", "NoTabsLineBorder", "NoTabsNoBorder"};
        NSControlSize sizes[] = {NSControlSizeRegular, NSControlSizeSmall, NSControlSizeMini};
        const char *sn[] = {"regular", "small", "mini"};
        for (int t = 0; t < 7; t++) {
            for (int s = 0; s < 3; s++) {
                NSTabView *tv = [[NSTabView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200)];
                [tv setTabViewType:types[t]];
                if ([tv respondsToSelector:@selector(setControlSize:)]) [tv setValue:@(sizes[s]) forKey:@"controlSize"];
                for (NSString *l in @[@"One", @"Two", @"Three"]) {
                    NSTabViewItem *it = [[NSTabViewItem alloc] initWithIdentifier:l];
                    [it setLabel:l];
                    [tv addTabViewItem:it];
                }
                probe_line("0050.geometry", PF(@"%s control size %s, frame 300x200, tabs One Two Three", tn[t], sn[s]),
                           PF(@"contentRect=%@ minimumSize=%@ intrinsicContentSize=%@", probe_rect([tv contentRect]), probe_size([tv minimumSize]),
                              probe_size([tv intrinsicContentSize])));
            }
        }
        // Where the tabs are: walk a line and report the item index under each point, as runs.
        NSTabView *tv = [[NSTabView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200)];
        for (NSString *l in @[@"One", @"Two", @"Three"]) {
            NSTabViewItem *it = [[NSTabViewItem alloc] initWithIdentifier:l];
            [it setLabel:l];
            [tv addTabViewItem:it];
        }
        for (double y = 190; y >= 150; y -= 5) {
            NSMutableArray *runs = [NSMutableArray array];
            NSInteger prev = -2, start = 0;
            for (NSInteger x = 0; x <= 300; x++) {
                NSTabViewItem *hit = [tv tabViewItemAtPoint:NSMakePoint(x, y)];
                NSInteger cur = hit ? (NSInteger)[tv indexOfTabViewItem:hit] : -1;
                if (cur != prev) {
                    if (prev != -2) [runs addObject:PF(@"x%ld..%ld=%@", (long)start, (long)x - 1, prev < 0 ? @"none" : PF(@"item%ld", (long)prev))];
                    prev = cur; start = x;
                }
            }
            [runs addObject:PF(@"x%ld..300=%@", (long)start, prev < 0 ? @"none" : PF(@"item%ld", (long)prev))];
            probe_line("0050.hit", PF(@"top tabs, view not flipped: points along y=%g (y from the bottom)", y), [runs componentsJoinedByString:@" "]);
        }
        for (int dark = 0; dark <= 1; dark++) {
            NSTabView *rv = [[NSTabView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200)];
            for (NSString *l in @[@"One", @"Two", @"Three"]) {
                NSTabViewItem *it = [[NSTabViewItem alloc] initWithIdentifier:l];
                [it setLabel:l];
                [rv addTabViewItem:it];
            }
            [rv selectTabViewItemAtIndex:1];
            NSWindow *w = probe_window(NSMakeRect(0, 0, 300, 200), dark);
            [[w contentView] addSubview:rv];
            NSBitmapImageRep *rep = probe_snapshot(rv);
            NSInteger px = rep ? [rep pixelsWide] : 300, py = rep ? [rep pixelsHigh] : 200;
            double sx = px / 300.0, sy = py / 200.0;
            // Sample rows (y from the top, points) across the whole width, every 20 points.
            for (double y = 4; y <= 196; y += 16) {
                NSMutableArray *samples = [NSMutableArray array];
                for (double x = 2; x < 300; x += 20)
                    [samples addObject:probe_pixel(rep, (NSInteger)(x * sx), (NSInteger)(y * sy))];
                probe_line("0050.pixels", PF(@"%s appearance, bitmap %ldx%ld, row y=%g points from the top, x = 2,22,42,... points, sRGB rrggbbaa, item Two selected",
                                             dark ? "dark" : "light", (long)px, (long)py, y), [samples componentsJoinedByString:@" "]);
            }
        }
    }
    return 0;
}
