// Probe 656: NSTableView drop validation. A data source with and without tableView:validateDrop:..., what the
// table proposes (row and DropOn / DropAbove) for pointer positions inside a row, in the gap, below the last row
// and beside the columns, what setDropRow:dropOperation: does to the drag operation the table returns, and what
// acceptDrop receives. Driven through the public NSDraggingDestination methods of the table with a fake
// NSDraggingInfo; no user input, no drag session. Window server needed (a table lives in a window); the window is
// at alpha 0, never made key, the application is never activated.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSMutableArray *gLog;
static NSString *opname(NSUInteger op) {
    if (op == NSDragOperationNone) return @"None";
    NSMutableArray *a = [NSMutableArray array];
    if (op == NSDragOperationEvery) return @"Every";
    if (op & NSDragOperationCopy) [a addObject:@"Copy"];
    if (op & NSDragOperationLink) [a addObject:@"Link"];
    if (op & NSDragOperationGeneric) [a addObject:@"Generic"];
    if (op & NSDragOperationPrivate) [a addObject:@"Private"];
    if (op & NSDragOperationMove) [a addObject:@"Move"];
    if (op & NSDragOperationDelete) [a addObject:@"Delete"];
    return [a count] ? [a componentsJoinedByString:@"+"] : PF(@"0x%lx", (unsigned long)op);
}
static NSString *tvop(NSTableViewDropOperation o) { return o == NSTableViewDropOn ? @"On" : o == NSTableViewDropAbove ? @"Above" : PF(@"%ld", (long)o); }

#define NO_RETARGET (-1000)
@interface DS : NSObject <NSTableViewDataSource>
@property BOOL hasValidate, hasAccept;
@property NSInteger retargetRow;                 // NO_RETARGET: do not call setDropRow
@property NSTableViewDropOperation retargetOp;
@property NSUInteger returnOp;
@end
@implementation DS
- (NSInteger)numberOfRowsInTableView:(NSTableView *)t { return 5; }
- (id)tableView:(NSTableView *)t objectValueForTableColumn:(NSTableColumn *)c row:(NSInteger)r { return PF(@"r%ld", (long)r); }
- (BOOL)respondsToSelector:(SEL)s {
    if (s == @selector(tableView:validateDrop:proposedRow:proposedDropOperation:)) return self.hasValidate;
    if (s == @selector(tableView:acceptDrop:row:dropOperation:)) return self.hasAccept;
    return [super respondsToSelector:s];
}
- (NSDragOperation)tableView:(NSTableView *)t validateDrop:(id<NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)op {
    [gLog addObject:PF(@"validate(row=%ld,%@)", (long)row, tvop(op))];
    if (self.retargetRow != NO_RETARGET) {
        [t setDropRow:self.retargetRow dropOperation:self.retargetOp];
        [gLog addObject:PF(@"setDropRow(%ld,%@)", (long)self.retargetRow, tvop(self.retargetOp))];
    }
    return self.returnOp;
}
- (BOOL)tableView:(NSTableView *)t acceptDrop:(id<NSDraggingInfo>)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)op {
    [gLog addObject:PF(@"accept(row=%ld,%@)", (long)row, tvop(op))];
    return YES;
}
@end

@interface Info : NSObject <NSDraggingInfo>
@property NSWindow *win;
@property NSPoint loc;
@property NSDragOperation mask;
@property NSPasteboard *pb;
@end
@implementation Info
- (NSWindow *)draggingDestinationWindow { return self.win; }
- (NSDragOperation)draggingSourceOperationMask { return self.mask; }
- (NSPoint)draggingLocation { return self.loc; }
- (NSPoint)draggedImageLocation { return self.loc; }
- (NSImage *)draggedImage { return nil; }
- (NSPasteboard *)draggingPasteboard { return self.pb; }
- (id)draggingSource { return nil; }
- (NSInteger)draggingSequenceNumber { return 1; }
- (void)slideDraggedImageTo:(NSPoint)p {}
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)u { return nil; }
- (NSDraggingFormation)draggingFormation { return NSDraggingFormationDefault; }
- (void)setDraggingFormation:(NSDraggingFormation)f {}
- (BOOL)animatesToDestination { return NO; }
- (void)setAnimatesToDestination:(BOOL)b {}
- (NSInteger)numberOfValidItemsForDrop { return 1; }
- (void)setNumberOfValidItemsForDrop:(NSInteger)n {}
- (void)enumerateDraggingItemsWithOptions:(NSDraggingItemEnumerationOptions)o forView:(NSView *)v classes:(NSArray *)c searchOptions:(NSDictionary *)s usingBlock:(void (^)(NSDraggingItem *, NSInteger, BOOL *))b {}
- (NSSpringLoadingHighlight)springLoadingHighlight { return NSSpringLoadingHighlightNone; }
- (void)resetSpringLoading {}
- (NSMethodSignature *)methodSignatureForSelector:(SEL)s {
    return [super methodSignatureForSelector:s] ?: [NSMethodSignature signatureWithObjCTypes:"q@:"];
}
- (void)forwardInvocation:(NSInvocation *)i { [gLog addObject:PF(@"info-selector-missing %@", NSStringFromSelector([i selector]))]; }
@end

@interface Rig : NSObject
@property NSTableView *t;
@property NSWindow *w;
@property DS *ds;
@end
@implementation Rig
@end
static Rig *rig(BOOL validate, BOOL accept, NSInteger rrow, NSTableViewDropOperation rop, NSUInteger retOp) {
    NSWindow *w = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 400, 300) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
    [w setReleasedWhenClosed:NO]; [w setAlphaValue:0];
    NSScrollView *sv = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 400, 300)];
    NSTableView *t = [[NSTableView alloc] initWithFrame:NSMakeRect(0, 0, 400, 300)];
    NSTableColumn *c = [[NSTableColumn alloc] initWithIdentifier:@"c"]; [c setWidth:150];
    [t addTableColumn:c]; [t setHeaderView:nil]; [t setRowHeight:20];
    [t registerForDraggedTypes:@[NSPasteboardTypeString]];
    DS *ds = [DS new]; ds.hasValidate = validate; ds.hasAccept = accept; ds.retargetRow = rrow; ds.retargetOp = rop; ds.returnOp = retOp;
    [t setDataSource:ds];
    [sv setDocumentView:t]; [w setContentView:sv]; [w orderFront:nil]; // alpha 0, no activation
    [t reloadData];
    Rig *r = [Rig new]; r.t = t; r.w = w; r.ds = ds;
    return r;
}

static NSString *drag(Rig *r, NSPoint tablePoint, NSDragOperation mask, BOOL full) {
    [gLog removeAllObjects];
    NSPasteboard *pb = [NSPasteboard pasteboardWithUniqueName];
    [pb declareTypes:@[NSPasteboardTypeString] owner:nil]; [pb setString:@"x" forType:NSPasteboardTypeString];
    Info *info = [Info new]; info.win = r.w; info.mask = mask; info.pb = pb;
    info.loc = [r.t convertPoint:tablePoint toView:nil];
    id<NSDraggingDestination> d = (id)r.t;
    NSMutableArray *out = [NSMutableArray array];
    __block NSDragOperation e = 0, u = 0; __block BOOL prep = NO, perf = NO;
    NSString *ex = probe_try(^{
        e = [d draggingEntered:info];
        u = [d draggingUpdated:info];
        if (full) {
            prep = [d prepareForDragOperation:info];
            perf = [d performDragOperation:info];
            [d concludeDragOperation:info];
        } else [d draggingExited:info];
    });
    [out addObject:PF(@"entered=%@ updated=%@", opname(e), opname(u))];
    if (full) [out addObject:PF(@"prepare=%d perform=%d", prep, perf)];
    [out addObject:PF(@"calls=[%@]", [gLog componentsJoinedByString:@" "])];
    if (![ex isEqualToString:@"no exception"]) [out addObject:ex];
    [pb releaseGlobally];
    return [out componentsJoinedByString:@" ; "];
}

int main(void) {
    @autoreleasepool {
        probe_env("656");
        probe_app();
        gLog = [NSMutableArray array];
        Rig *r0 = rig(YES, YES, NO_RETARGET, 0, NSDragOperationMove);
        NSRect r2 = [r0.t rectOfRow:2], r4 = [r0.t rectOfRow:4];
        probe_line("656.0", @"geometry", PF(@"rows=%ld rowHeight=%g spacing=%@ row2=%@ row4=%@ table.frame=%@ clip=%@",
            (long)[r0.t numberOfRows], [r0.t rowHeight], probe_size([r0.t intercellSpacing]), probe_rect(r2), probe_rect(r4),
            probe_rect([r0.t frame]), probe_rect([[[r0.t enclosingScrollView] contentView] frame])));

        // Pointer positions: x in the column, y fraction through row 2, then gap, below rows, beside the column.
        double fr[] = {0.02, 0.1, 0.25, 0.4, 0.5, 0.6, 0.75, 0.9, 0.98};
        NSDragOperation every = NSDragOperationEvery;

        // A. No validateDrop in the data source.
        Rig *a = rig(NO, YES, NO_RETARGET, 0, 0);
        for (int i = 0; i < 9; i++)
            probe_line("656.A", PF(@"no validateDrop (acceptDrop present); pointer at row2 +%g of height; full drop sequence", fr[i]),
                       drag(a, NSMakePoint(50, r2.origin.y + fr[i] * r2.size.height), every, YES));
        a = rig(NO, NO, NO_RETARGET, 0, 0);
        probe_line("656.A2", @"neither validateDrop nor acceptDrop; pointer mid row 2", drag(a, NSMakePoint(50, NSMidY(r2)), every, YES));

        // B. validateDrop returns Move without retargeting: what the table proposes.
        Rig *b = rig(YES, YES, NO_RETARGET, 0, NSDragOperationMove);
        for (int i = 0; i < 9; i++)
            probe_line("656.B", PF(@"validate returns Move, no retarget; pointer at row2 +%g of height; full drop", fr[i]),
                       drag(b, NSMakePoint(50, r2.origin.y + fr[i] * r2.size.height), every, YES));
        struct { const char *n; double y; } spots[] = {
            {"row 0 top edge +0.5", 0.5}, {"gap between row 2 and 3 (maxY of row 2 + spacing/2)", NSMaxY(r2) + [r0.t intercellSpacing].height / 2},
            {"just below row 4 (maxY + 1)", NSMaxY(r4) + 1}, {"well below the rows (maxY + 40)", NSMaxY(r4) + 40}, {"bottom of the table frame - 1", NSHeight([r0.t frame]) - 1}};
        for (int i = 0; i < 5; i++)
            probe_line("656.B2", PF(@"validate returns Move, no retarget; pointer at %s", spots[i].n),
                       drag(b, NSMakePoint(50, spots[i].y), every, YES));
        probe_line("656.B3", @"validate returns Move; pointer mid row 2 but x=380 (right of every column)", drag(b, NSMakePoint(380, NSMidY(r2)), every, YES));

        // B4. Draggingdestination feedback style (10.6+/10.11): does the proposal change?
        NSInteger styles[] = {-1, 0, 1, 2};
        for (int s = 0; s < 4; s++) {
            Rig *f = rig(YES, YES, NO_RETARGET, 0, NSDragOperationMove);
            NSString *ex = @"n/a";
            if ([f.t respondsToSelector:@selector(setDraggingDestinationFeedbackStyle:)]) {
                [f.t setDraggingDestinationFeedbackStyle:(NSTableViewDraggingDestinationFeedbackStyle)styles[s]];
                ex = PF(@"%ld", (long)styles[s]);
            }
            for (int i = 0; i < 9; i += 2)
                probe_line("656.B4", PF(@"feedback style %@; row2 +%g; update only", ex, fr[i]),
                           drag(f, NSMakePoint(50, r2.origin.y + fr[i] * r2.size.height), every, NO));
        }

        // C. Retargeting through setDropRow:dropOperation:, pointer in the middle of row 0.
        struct { NSInteger row; NSTableViewDropOperation op; NSUInteger ret; const char *n; } cases[] = {
            {2, NSTableViewDropOn, NSDragOperationMove, "setDropRow(2,On), return Move"},
            {1, NSTableViewDropAbove, NSDragOperationMove, "setDropRow(1,Above), return Move"},
            {-1, NSTableViewDropOn, NSDragOperationMove, "setDropRow(-1,On), return Move"},
            {-1, NSTableViewDropAbove, NSDragOperationMove, "setDropRow(-1,Above), return Move"},
            {5, NSTableViewDropAbove, NSDragOperationMove, "setDropRow(5=numberOfRows,Above), return Move"},
            {5, NSTableViewDropOn, NSDragOperationMove, "setDropRow(5=numberOfRows,On), return Move"},
            {99, NSTableViewDropAbove, NSDragOperationMove, "setDropRow(99,Above), return Move"},
            {99, NSTableViewDropOn, NSDragOperationMove, "setDropRow(99,On), return Move"},
            {-5, NSTableViewDropAbove, NSDragOperationMove, "setDropRow(-5,Above), return Move"},
            {2, NSTableViewDropOn, NSDragOperationNone, "setDropRow(2,On), return None"},
            {2, NSTableViewDropAbove, NSDragOperationCopy, "setDropRow(2,Above), return Copy"},
            {NO_RETARGET, 0, NSDragOperationNone, "no retarget, return None"},
            {NO_RETARGET, 0, NSDragOperationCopy, "no retarget, return Copy"},
            {NO_RETARGET, 0, NSDragOperationGeneric, "no retarget, return Generic"},
            {NO_RETARGET, 0, NSDragOperationEvery, "no retarget, return Every"},
            {NO_RETARGET, 0, NSDragOperationDelete, "no retarget, return Delete"},
        };
        for (int i = 0; i < 16; i++) {
            Rig *c = rig(YES, YES, cases[i].row, cases[i].op, cases[i].ret);
            probe_line("656.C", PF(@"%s; pointer mid row 0; source mask Every; full drop", cases[i].n), drag(c, NSMakePoint(50, [c.t rectOfRow:0].origin.y + 10), every, YES));
        }
        // C2. Source mask that does not include what the data source returns.
        Rig *c2 = rig(YES, YES, NO_RETARGET, 0, NSDragOperationMove);
        probe_line("656.C2", @"validate returns Move; source mask Copy only; mid row 0", drag(c2, NSMakePoint(50, 10), NSDragOperationCopy, YES));
        probe_line("656.C3", @"validate returns Move; source mask None; mid row 0", drag(c2, NSMakePoint(50, 10), NSDragOperationNone, YES));

        // D. The target at drop time: pointer moves between the last validation and the drop.
        {
            Rig *d = rig(YES, YES, NO_RETARGET, 0, NSDragOperationMove);
            [gLog removeAllObjects];
            NSPasteboard *pb = [NSPasteboard pasteboardWithUniqueName];
            [pb declareTypes:@[NSPasteboardTypeString] owner:nil]; [pb setString:@"x" forType:NSPasteboardTypeString];
            Info *i1 = [Info new]; i1.win = d.w; i1.mask = every; i1.pb = pb;
            id<NSDraggingDestination> dd = (id)d.t;
            i1.loc = [d.t convertPoint:NSMakePoint(50, [d.t rectOfRow:1].origin.y + 10) toView:nil];
            [dd draggingEntered:i1];
            [dd draggingUpdated:i1];
            i1.loc = [d.t convertPoint:NSMakePoint(50, [d.t rectOfRow:3].origin.y + 10) toView:nil];
            [gLog addObject:@"--pointer now over row 3, no draggingUpdated--"];
            BOOL pf = [dd performDragOperation:i1];
            probe_line("656.D", @"validated over row 1, pointer moved to row 3, performDragOperation without another update", PF(@"perform=%d calls=[%@]", pf, [gLog componentsJoinedByString:@" "]));
            [pb releaseGlobally];
        }
        // E. performDragOperation with no prior draggingEntered/Updated.
        {
            Rig *e = rig(YES, YES, NO_RETARGET, 0, NSDragOperationMove);
            [gLog removeAllObjects];
            NSPasteboard *pb = [NSPasteboard pasteboardWithUniqueName];
            [pb declareTypes:@[NSPasteboardTypeString] owner:nil]; [pb setString:@"x" forType:NSPasteboardTypeString];
            Info *i1 = [Info new]; i1.win = e.w; i1.mask = every; i1.pb = pb;
            i1.loc = [e.t convertPoint:NSMakePoint(50, [e.t rectOfRow:1].origin.y + 10) toView:nil];
            __block BOOL pf = NO;
            NSString *ex = probe_try(^{ pf = [(id<NSDraggingDestination>)e.t performDragOperation:i1]; });
            probe_line("656.E", @"performDragOperation with no draggingEntered/Updated before it", PF(@"perform=%d [%@] calls=[%@]", pf, ex, [gLog componentsJoinedByString:@" "]));
            [pb releaseGlobally];
        }
    }
    return 0;
}
