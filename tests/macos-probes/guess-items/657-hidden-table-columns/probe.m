// Probe 657: hidden NSTableColumns in a cell-based table (data source, no view-based delegate): column widths,
// rectOfColumn:, columnsInRect:, columnAtPoint:, header rectangles, the table frame, sizeLastColumnToFit,
// sizeToFit and every column autoresizing style, with hidden columns before, between and after shown ones.
// Window server needed (table and header live in a window); the window is at alpha 0, never made key, the
// application is never activated.
#define PROBE_APPKIT 1
#import "probe_common.h"

@interface DS : NSObject <NSTableViewDataSource>
@end
@implementation DS
- (NSInteger)numberOfRowsInTableView:(NSTableView *)t { return 3; }
- (id)tableView:(NSTableView *)t objectValueForTableColumn:(NSTableColumn *)c row:(NSInteger)r { return PF(@"%@%ld", [c identifier], (long)r); }
@end
static DS *gDS;

static NSWindow *gWin;
static NSTableView *build(NSArray *hidden, CGFloat clipWidth) {
    static const double widths[5] = {50, 60, 70, 80, 90};
    if (gWin == nil) {
        gWin = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 600, 200) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
        [gWin setReleasedWhenClosed:NO]; [gWin setAlphaValue:0];
    }
    [gWin setContentSize:NSMakeSize(clipWidth, 200)];
    NSScrollView *sv = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, clipWidth, 200)];
    [sv setHasHorizontalScroller:NO]; [sv setHasVerticalScroller:NO];
    [sv setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    NSTableView *t = [[NSTableView alloc] initWithFrame:NSMakeRect(0, 0, clipWidth, 200)];
    [t setDataSource:gDS];
    for (int i = 0; i < 5; i++) {
        NSTableColumn *c = [[NSTableColumn alloc] initWithIdentifier:PF(@"c%d", i)];
        [c setWidth:widths[i]]; [c setMinWidth:20]; [c setMaxWidth:1000];
        [[c headerCell] setStringValue:PF(@"H%d", i)];
        [t addTableColumn:c];
    }
    for (NSNumber *h in hidden) [[[t tableColumns] objectAtIndex:[h integerValue]] setHidden:YES];
    [sv setDocumentView:t]; [gWin setContentView:sv]; [gWin orderFront:nil];
    [t reloadData]; [t layoutSubtreeIfNeeded];
    return t;
}

static NSString *widths(NSTableView *t) {
    NSMutableArray *a = [NSMutableArray array];
    for (NSTableColumn *c in [t tableColumns]) [a addObject:PF(@"%g%@", [c width], [c isHidden] ? @"h" : @"")];
    return [a componentsJoinedByString:@","];
}
static NSString *rng(NSRange r) { return r.length == 0 ? PF(@"empty(loc=%lu)", (unsigned long)r.location) : PF(@"%lu..%lu", (unsigned long)r.location, (unsigned long)(NSMaxRange(r) - 1)); }

static void geometry(const char *id, NSString *input, NSTableView *t) {
    NSMutableArray *rc = [NSMutableArray array], *hc = [NSMutableArray array], *pt = [NSMutableArray array];
    NSTableHeaderView *hv = (NSTableHeaderView *)[t headerView];
    for (NSInteger i = 0; i < [t numberOfColumns]; i++) {
        NSRect r = [t rectOfColumn:i];
        [rc addObject:PF(@"%g+%g", r.origin.x, r.size.width)];
        NSRect h = hv ? [hv headerRectOfColumn:i] : NSZeroRect;
        [hc addObject:PF(@"%g+%g", h.origin.x, h.size.width)];
        // column found at the center of the rect, and 1pt inside its left edge
        [pt addObject:PF(@"%ld/%ld", (long)[t columnAtPoint:NSMakePoint(NSMidX(r), 5)], (long)[t columnAtPoint:NSMakePoint(r.origin.x + 1, 5)])];
    }
    NSRect full = NSMakeRect(0, 0, NSWidth([t frame]), 100);
    probe_line(id, input, PF(@"widths=[%@] ; rectOfColumn x+w=[%@] ; headerRect x+w=[%@] ; columnAtPoint center/left+1=[%@] ; columnsInRect(full)=%@ ; table.frame.w=%g bounds.w=%g spacing.w=%g header.frame=%@ ; clip.w=%g",
        widths(t), [rc componentsJoinedByString:@" "], [hc componentsJoinedByString:@" "], [pt componentsJoinedByString:@" "],
        rng([t columnsInRect:full]), NSWidth([t frame]), NSWidth([t bounds]), [t intercellSpacing].width,
        hv ? probe_rect([hv frame]) : @"noheader", NSWidth([[[t enclosingScrollView] contentView] frame])));
}

int main(void) {
    @autoreleasepool {
        probe_env("657");
        probe_app();
        gDS = [DS new];
        NSArray *pats = @[@[], @[@1], @[@0], @[@4], @[@1, @3], @[@1, @2], @[@0, @4], @[@0, @1, @3, @4], @[@0, @1, @2, @3, @4], @[@2]];
        const char *names[] = {"none", "1", "0", "4", "1,3", "1,2", "0,4", "all but 2", "all", "2"};
        for (int p = 0; p < 10; p++) {
            NSTableView *t = build(pats[p], 600);
            NSString *tag = PF(@"hidden=%s, 5 columns widths 50,60,70,80,90, clip 600, intercell %g", names[p], [t intercellSpacing].width);
            geometry("657.1", tag, t);
            // partial rectangles: a range that covers the first shown/hidden boundary
            NSMutableArray *parts = [NSMutableArray array];
            double xs[][2] = {{0, 1}, {55, 10}, {100, 30}, {0, 180}, {120, 150}, {400, 50}, {600, 20}};
            for (int i = 0; i < 7; i++)
                [parts addObject:PF(@"x%g+%g:%@", xs[i][0], xs[i][1], rng([t columnsInRect:NSMakeRect(xs[i][0], 0, xs[i][1], 50)]))];
            probe_line("657.2", [tag stringByAppendingString:@" ; columnsInRect"], [parts componentsJoinedByString:@" "]);
            // cell frames
            NSMutableArray *cf = [NSMutableArray array];
            for (NSInteger i = 0; i < [t numberOfColumns]; i++) { NSRect f = [t frameOfCellAtColumn:i row:1]; [cf addObject:PF(@"%g+%g", f.origin.x, f.size.width)]; }
            probe_line("657.3", [tag stringByAppendingString:@" ; frameOfCellAtColumn:row:1"], [cf componentsJoinedByString:@" "]);
        }

        // sizeLastColumnToFit with clip wider than the columns, and narrower
        double clips[] = {700, 300};
        for (int ci = 0; ci < 2; ci++)
            for (int p = 0; p < 10; p++) {
                NSTableView *t = build(pats[p], clips[ci]);
                [t sizeLastColumnToFit]; [t layoutSubtreeIfNeeded];
                geometry("657.4", PF(@"hidden=%s, clip %g, after sizeLastColumnToFit", names[p], clips[ci]), t);
            }
        for (int p = 0; p < 10; p++) {
            NSTableView *t = build(pats[p], 700);
            [t sizeToFit]; [t layoutSubtreeIfNeeded];
            geometry("657.5", PF(@"hidden=%s, clip 700, after sizeToFit", names[p]), t);
        }

        // Autoresizing styles: build at 600, then change the clip width to 800 and 400.
        struct { NSTableViewColumnAutoresizingStyle s; const char *n; } styles[] = {
            {NSTableViewNoColumnAutoresizing, "None"}, {NSTableViewUniformColumnAutoresizingStyle, "Uniform"},
            {NSTableViewSequentialColumnAutoresizingStyle, "Sequential"}, {NSTableViewReverseSequentialColumnAutoresizingStyle, "ReverseSequential"},
            {NSTableViewLastColumnOnlyAutoresizingStyle, "LastOnly"}, {NSTableViewFirstColumnOnlyAutoresizingStyle, "FirstOnly"}};
        double newW[] = {800, 400};
        for (int si = 0; si < 6; si++)
            for (int p = 0; p < 10; p += 1) {
                if (p != 0 && p != 1 && p != 4 && p != 3 && p != 2 && p != 7) continue;
                for (int wi = 0; wi < 2; wi++) {
                    NSTableView *t = build(pats[p], 600);
                    [t setColumnAutoresizingStyle:styles[si].s];
                    [gWin setContentSize:NSMakeSize(newW[wi], 200)];
                    probe_spin(0.05); [t layoutSubtreeIfNeeded];
                    geometry("657.6", PF(@"hidden=%s, style %s, clip 600 -> %g", names[p], styles[si].n, newW[wi]), t);
                }
            }
        // Setting a column hidden after the table was laid out, and showing it again
        NSTableView *t = build(@[], 600);
        [[[t tableColumns] objectAtIndex:1] setHidden:YES]; [t layoutSubtreeIfNeeded];
        geometry("657.7", @"column 1 hidden after layout (no hidden before)", t);
        [[[t tableColumns] objectAtIndex:1] setHidden:NO]; [t layoutSubtreeIfNeeded];
        geometry("657.8", @"column 1 shown again", t);
        // The header hit test and a click-sized header rect for hidden columns
        t = build(@[@1, @3], 600);
        NSMutableArray *hh = [NSMutableArray array];
        for (double x = 0; x <= 400; x += 25) [hh addObject:PF(@"%g:%ld", x, (long)[(NSTableHeaderView *)[t headerView] columnAtPoint:NSMakePoint(x, 5)])];
        probe_line("657.9", @"hidden=1,3; headerView columnAtPoint at x=0..400 step 25 / tableView columnAtPoint", PF(@"%@ ; %@", [hh componentsJoinedByString:@" "], ({ NSMutableArray *tt = [NSMutableArray array]; for (double x = 0; x <= 400; x += 25) [tt addObject:PF(@"%g:%ld", x, (long)[t columnAtPoint:NSMakePoint(x, 5)])]; [tt componentsJoinedByString:@" "]; })));
        probe_line("657.10", @"hidden=1,3; numberOfColumns / selection of a hidden column", PF(@"n=%ld ; isHidden(1)=%d ; selectColumnIndexes{1} then selectedColumnIndexes=%@", (long)[t numberOfColumns], (int)[[[t tableColumns] objectAtIndex:1] isHidden], ({ [t setAllowsColumnSelection:YES]; [t selectColumnIndexes:[NSIndexSet indexSetWithIndex:1] byExtendingSelection:NO]; [t selectedColumnIndexes]; })));
    }
    return 0;
}
