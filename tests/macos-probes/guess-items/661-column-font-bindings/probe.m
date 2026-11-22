// Probe 661: the fontBold and fontItalic bindings of an NSTableColumn (NSFontBoldBinding, NSFontItalicBinding)
// bound to an NSArrayController's arrangedObjects. For each row value (YES, NO, nil, NSNull, 1, 0, 2, a string,
// the three selection markers) prints the font of the prepared cell, whether bold / italic traits are present,
// and whether the font of a row depends on the rows visited before it (the data cell is shared by all rows).
// Window server needed (cells are prepared in a table in a window); alpha 0, never key, no activation.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSString *D(id o) { return o == nil ? @"nil" : [o isKindOfClass:[NSNull class]] ? @"NSNull" : PF(@"%@", o); }
static NSString *fdesc(NSFont *f) {
    if (f == nil) return @"nil";
    NSFontSymbolicTraits t = [[f fontDescriptor] symbolicTraits];
    return PF(@"%@ %g bold=%d italic=%d", [f fontName], (double)[f pointSize], (int)((t & NSFontBoldTrait) != 0), (int)((t & NSFontItalicTrait) != 0));
}

static NSWindow *gWin;
static NSTableView *gT;
static NSArrayController *gAC;

// colFont: the font to give the column's data cell before binding (nil: leave the default)
static void build(NSArray *rows, BOOL bindBold, BOOL bindItalic, NSFont *colFont, BOOL nilFont) {
    gAC = [[NSArrayController alloc] init];
    [gAC setObjectClass:[NSMutableDictionary class]];
    NSMutableArray *content = [NSMutableArray array];
    for (NSDictionary *d in rows) [content addObject:[d mutableCopy]];
    [gAC setContent:content];
    gWin = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 300, 200) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
    [gWin setReleasedWhenClosed:NO]; [gWin setAlphaValue:0];
    NSScrollView *sv = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200)];
    gT = [[NSTableView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200)];
    NSTableColumn *c = [[NSTableColumn alloc] initWithIdentifier:@"name"]; [c setWidth:200];
    if (colFont) [[c dataCell] setFont:colFont];
    if (nilFont) [[c dataCell] setFont:nil];
    [gT addTableColumn:c];
    [sv setDocumentView:gT]; [gWin setContentView:sv]; [gWin orderFront:nil];
    NSString *ex = probe_try(^{
        [c bind:NSValueBinding toObject:gAC withKeyPath:@"arrangedObjects.name" options:nil];
        if (bindBold) [c bind:NSFontBoldBinding toObject:gAC withKeyPath:@"arrangedObjects.bold" options:nil];
        if (bindItalic) [c bind:NSFontItalicBinding toObject:gAC withKeyPath:@"arrangedObjects.italic" options:nil];
    });
    if (![ex isEqualToString:@"no exception"]) probe_line("661.bind", @"bind", ex);
    [gT reloadData];
}
static NSString *fontOf(NSInteger row) { return fdesc([[gT preparedCellAtColumn:0 row:row] font]); }

int main(void) {
    @autoreleasepool {
        probe_env("661");
        probe_app();
        NSNull *nul = [NSNull null];
        struct { const char *n; id v; } vals[] = {
            {"YES", @YES}, {"NO", @NO}, {"absent key (nil)", nil}, {"NSNull", nul}, {"NSNumber 1", @1}, {"NSNumber 0", @0}, {"NSNumber 2", @2},
            {"string \"YES\"", @"YES"}, {"NSMultipleValuesMarker", NSMultipleValuesMarker}, {"NSNoSelectionMarker", NSNoSelectionMarker},
            {"NSNotApplicableMarker", NSNotApplicableMarker}};
        const int nv = 11;
        NSMutableArray *rows = [NSMutableArray array];
        for (int i = 0; i < nv; i++) {
            NSMutableDictionary *d = [NSMutableDictionary dictionaryWithObject:PF(@"row%d", i) forKey:@"name"];
            if (vals[i].v != nil) { d[@"bold"] = vals[i].v; d[@"italic"] = vals[i].v; }
            [rows addObject:d];
        }

        // Baseline: nothing bound.
        build(rows, NO, NO, nil, NO);
        probe_line("661.0", @"no font binding; default data cell font", fontOf(0));

        // Both bound; rows visited in order 0..10, then 10..0, then 0..10 again.
        build(rows, YES, YES, nil, NO);
        for (int i = 0; i < nv; i++)
            probe_line("661.1", PF(@"fontBold+fontItalic bound; value %s; visited in order 0..10 (first pass)", vals[i].n), fontOf(i));
        for (int i = nv - 1; i >= 0; i--)
            probe_line("661.2", PF(@"same table; value %s; visited in order 10..0 (second pass)", vals[i].n), fontOf(i));
        for (int i = 0; i < nv; i++)
            probe_line("661.3", PF(@"same table; value %s; third pass, order 0..10", vals[i].n), fontOf(i));
        // Fresh table, each row visited alone and first (no earlier row to leave a trait behind).
        for (int i = 0; i < nv; i++) {
            build(rows, YES, YES, nil, NO);
            probe_line("661.4", PF(@"fresh table; value %s; this row is the first one prepared", vals[i].n), fontOf(i));
        }

        // Only one of the two bound.
        build(rows, YES, NO, nil, NO);
        NSMutableArray *a = [NSMutableArray array];
        for (int i = 0; i < nv; i++) [a addObject:PF(@"%s=>%@", vals[i].n, fontOf(i))];
        probe_line("661.5", @"only fontBold bound, rows 0..10", [a componentsJoinedByString:@" ; "]);
        build(rows, NO, YES, nil, NO);
        [a removeAllObjects];
        for (int i = 0; i < nv; i++) [a addObject:PF(@"%s=>%@", vals[i].n, fontOf(i))];
        probe_line("661.6", @"only fontItalic bound, rows 0..10", [a componentsJoinedByString:@" ; "]);

        // Independent bold / italic values.
        NSArray *combo = @[@{@"name": @"a", @"bold": @YES, @"italic": @NO}, @{@"name": @"b", @"bold": @NO, @"italic": @YES},
                           @{@"name": @"c", @"bold": @YES, @"italic": @YES}, @{@"name": @"d", @"bold": @NO, @"italic": @NO},
                           @{@"name": @"e", @"bold": @YES}, @{@"name": @"f", @"italic": @YES}];
        build(combo, YES, YES, nil, NO);
        for (int i = 0; i < 6; i++) probe_line("661.7", PF(@"bold/italic combination row %c (a=Y,N b=N,Y c=Y,Y d=N,N e=Y,absent f=absent,Y)", 'a' + i), fontOf(i));
        for (int i = 5; i >= 0; i--) probe_line("661.8", PF(@"same, visited in reverse; row %c", 'a' + i), fontOf(i));

        // Column font set explicitly before binding.
        NSFont *boldF = [[NSFontManager sharedFontManager] convertFont:[NSFont systemFontOfSize:12] toHaveTrait:NSBoldFontMask];
        NSFont *italF = [[NSFontManager sharedFontManager] convertFont:[NSFont systemFontOfSize:12] toHaveTrait:NSItalicFontMask];
        NSArray *two = @[@{@"name": @"a", @"bold": @YES, @"italic": @YES}, @{@"name": @"b", @"bold": @NO, @"italic": @NO}, @{@"name": @"c"}];
        build(two, YES, YES, boldF, NO);
        for (int i = 0; i < 3; i++) probe_line("661.9", PF(@"column data cell font set to bold 12 before binding; row %c (a=Y,Y b=N,N c=absent)", 'a' + i), fontOf(i));
        build(two, YES, YES, italF, NO);
        for (int i = 0; i < 3; i++) probe_line("661.10", PF(@"column data cell font set to italic 12 before binding; row %c", 'a' + i), fontOf(i));
        build(two, YES, YES, [NSFont fontWithName:@"Courier" size:14], NO);
        for (int i = 0; i < 3; i++) probe_line("661.11", PF(@"column data cell font Courier 14 before binding; row %c", 'a' + i), fontOf(i));

        // Data cell without a font.
        build(two, YES, YES, nil, YES);
        probe_line("661.12", @"data cell font set to nil before binding; the data cell's font before any row is prepared", fdesc([[[gT tableColumns][0] dataCell] font]));
        for (int i = 0; i < 3; i++) probe_line("661.13", PF(@"data cell font nil; row %c", 'a' + i), fontOf(i));
        build(two, NO, NO, nil, YES);
        probe_line("661.14", @"data cell font nil, no font binding", fontOf(0));

        // A model change.
        build(two, YES, YES, nil, NO);
        (void)fontOf(0);
        [[gAC content][0] setValue:@NO forKey:@"bold"];
        [gT reloadData];
        probe_line("661.15", @"row a bold changed from YES to NO in the model, reloadData; row a", fontOf(0));
    }
    return 0;
}
