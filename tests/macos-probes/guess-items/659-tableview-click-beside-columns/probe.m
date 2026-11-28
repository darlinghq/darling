// Probe 659: a click (and a double click) beside the columns of an NSTableView, below the rows, between columns,
// and in a table with no columns or only hidden columns: clickedRow, clickedColumn, the selection, which of
// action and doubleAction is sent, and whether an edit starts. Mouse events are made with the public NSEvent
// factory and delivered either by -[NSWindow sendEvent:] or by calling -[NSTableView mouseDown:] directly (the
// two routes are printed separately because a window that is not key may swallow a first click).
// Window server needed. The window is at alpha 0, ordered front but never made key, the application is never
// activated. A watchdog exits after 90 seconds if a mouse-down tracking loop never finishes.
#define PROBE_APPKIT 1
#import "probe_common.h"
#include <unistd.h>
#import <dispatch/dispatch.h>

static NSMutableArray *gLog;
@interface Target : NSObject
@property(weak) NSTableView *t;
@end
@implementation Target
- (void)act:(id)s { [gLog addObject:PF(@"action(clickedRow=%ld,clickedColumn=%ld,selectedRow=%ld)", (long)[self.t clickedRow], (long)[self.t clickedColumn], (long)[self.t selectedRow])]; }
- (void)dbl:(id)s { [gLog addObject:PF(@"doubleAction(clickedRow=%ld,clickedColumn=%ld,selectedRow=%ld)", (long)[self.t clickedRow], (long)[self.t clickedColumn], (long)[self.t selectedRow])]; }
@end
@interface DS : NSObject <NSTableViewDataSource>
@end
@implementation DS
- (NSInteger)numberOfRowsInTableView:(NSTableView *)t { return 4; }
- (id)tableView:(NSTableView *)t objectValueForTableColumn:(NSTableColumn *)c row:(NSInteger)r { return PF(@"%@r%ld", [c identifier], (long)r); }
- (void)tableView:(NSTableView *)t setObjectValue:(id)o forTableColumn:(NSTableColumn *)c row:(NSInteger)r { [gLog addObject:PF(@"setObjectValue(%@,row %ld)", o, (long)r)]; }
@end

@interface Rig : NSObject
@property NSTableView *t; @property NSWindow *w; @property Target *tg; @property DS *ds;
@end
@implementation Rig
@end

// cfg: 0 default (action+double, editable) 1 action only 2 not editable (action+double) 3 no columns 4 all columns hidden
static Rig *rig(int cfg) {
    Rig *r = [Rig new];
    r.w = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 400, 300) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
    [r.w setReleasedWhenClosed:NO]; [r.w setAlphaValue:0];
    NSScrollView *sv = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 400, 300)];
    NSTableView *t = [[NSTableView alloc] initWithFrame:NSMakeRect(0, 0, 400, 300)];
    [t setHeaderView:nil]; [t setRowHeight:20];
    r.ds = [DS new]; [t setDataSource:r.ds];
    if (cfg != 3) for (int i = 0; i < 3; i++) {
        NSTableColumn *c = [[NSTableColumn alloc] initWithIdentifier:PF(@"c%d", i)];
        [c setWidth:80]; [c setEditable:cfg != 2]; [c setHidden:cfg == 4];
        [t addTableColumn:c];
    }
    [t setColumnAutoresizingStyle:NSTableViewNoColumnAutoresizing];
    r.tg = [Target new]; r.tg.t = t;
    [t setTarget:r.tg]; [t setAction:@selector(act:)];
    if (cfg != 1) [t setDoubleAction:@selector(dbl:)];
    [sv setDocumentView:t]; [r.w setContentView:sv]; [r.w orderFront:nil];
    [t reloadData];
    r.t = t;
    return r;
}

static NSEvent *ev(NSEventType type, NSPoint wp, NSWindow *w, NSInteger n) {
    return [NSEvent mouseEventWithType:type location:wp modifierFlags:0 timestamp:[[NSProcessInfo processInfo] systemUptime]
                           windowNumber:[w windowNumber] context:nil eventNumber:n clickCount:n pressure:type == NSEventTypeLeftMouseUp ? 0 : 1];
}

static NSString *click(Rig *r, NSPoint tp, int count, BOOL viaWindow) {
    [gLog removeAllObjects];
    NSPoint wp = [r.t convertPoint:tp toView:nil];
    NSString *ex = probe_try(^{
        for (int n = 1; n <= count; n++) {
            [NSApp postEvent:ev(NSEventTypeLeftMouseUp, wp, r.w, n) atStart:NO];
            NSEvent *down = ev(NSEventTypeLeftMouseDown, wp, r.w, n);
            if (viaWindow) [r.w sendEvent:down]; else [r.t mouseDown:down];
            // drop any event the tracking loop left unread
            NSEvent *left;
            while ((left = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES]) != nil)
                [gLog addObject:PF(@"leftover-event %ld", (long)[left type])];
        }
    });
    NSMutableArray *sel = [NSMutableArray array];
    [[r.t selectedRowIndexes] enumerateIndexesUsingBlock:^(NSUInteger i, BOOL *s) { [sel addObject:PF(@"%lu", (unsigned long)i)]; }];
    return PF(@"[%@] calls=[%@] ; after: selectedRows={%@} clickedRow=%ld clickedColumn=%ld editedRow=%ld editedColumn=%ld key=%d firstResponder=%@",
              ex, [gLog componentsJoinedByString:@" "], [sel componentsJoinedByString:@","], (long)[r.t clickedRow], (long)[r.t clickedColumn],
              (long)[r.t editedRow], (long)[r.t editedColumn], (int)[r.w isKeyWindow], NSStringFromClass([[r.w firstResponder] class]));
}

int main(void) {
    @autoreleasepool {
        probe_env("659");
        probe_app();
        gLog = [NSMutableArray array];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 90 * NSEC_PER_SEC), dispatch_get_global_queue(0, 0), ^{
            probe_line("659.WATCHDOG", @"a mouse-down tracking loop did not finish in 90 s", @"exiting");
            _exit(2);
        });
        const char *cfgn[] = {"action+doubleAction, editable columns", "action only", "action+doubleAction, columns not editable", "no columns", "three columns, all hidden"};
        for (int cfg = 0; cfg < 5; cfg++) {
            Rig *g = rig(cfg);
            NSRect r1 = [g.t rectOfRow:1], r3 = [g.t rectOfRow:3];
            if (cfg == 0) {
                NSMutableArray *cols = [NSMutableArray array];
                for (NSInteger i = 0; i < [g.t numberOfColumns]; i++) [cols addObject:probe_rect([g.t rectOfColumn:i])];
                probe_line("659.0", @"geometry", PF(@"rows=%ld row1=%@ row3=%@ columns=[%@] table.frame=%@ clip=%@", (long)[g.t numberOfRows], probe_rect(r1), probe_rect(r3),
                    [cols componentsJoinedByString:@" "], probe_rect([g.t frame]), probe_rect([[[g.t enclosingScrollView] contentView] frame])));
            }
            struct { const char *n; double x, y; } spots[] = {
                {"inside column 0, row 1", 40, 0}, {"inside column 2, row 1", 200, 0},
                {"beside the columns (x=300), row 1", 300, 0}, {"beside the columns (x=395), row 1", 395, 0},
                {"beside the columns (x=300), below the last row", 300, 1000},
                {"inside column 0, below the last row", 40, 1000},
                {"between column 0 and 1 (x=81.5), row 1", 81.5, 0}, {"beside the columns (x=300), in the gap between row 1 and row 2", 300, 2000},
            };
            for (int route = 0; route < 2; route++)
                for (int s = 0; s < 8; s++)
                    for (int count = 1; count <= 2; count++) {
                        // a fresh rig per click so that selection and edit state do not leak between rows of the table of results
                        Rig *c = rig(cfg);
                        double y = spots[s].y == 0 ? NSMidY(r1) : spots[s].y == 1000 ? NSMaxY(r3) + 30 : NSMaxY(r1) + [c.t intercellSpacing].height / 2;
                        probe_line("659.1", PF(@"%s; %s; click count %d (%s); fresh table", cfgn[cfg], spots[s].n, count, route ? "direct mouseDown:" : "window sendEvent:"),
                                   click(c, NSMakePoint(spots[s].x, y), count, route == 0));
                    }
            // with a row already selected
            for (int route = 0; route < 2; route++) {
                Rig *c = rig(cfg);
                [c.t selectRowIndexes:[NSIndexSet indexSetWithIndex:3] byExtendingSelection:NO];
                probe_line("659.2", PF(@"%s; row 3 selected, then a single click beside the columns (x=300) on row 1 (%s)", cfgn[cfg], route ? "direct mouseDown:" : "window sendEvent:"),
                           click(c, NSMakePoint(300, NSMidY([c.t rectOfRow:1])), 1, route == 0));
            }
        }
    }
    return 0;
}
