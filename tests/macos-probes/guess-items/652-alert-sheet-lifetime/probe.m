// Probe 652: how long does an NSAlert shown as a sheet live? Built WITHOUT ARC on purpose (see build.txt), so the
// probe can drop the only reference it owns and watch when a subclass of NSAlert is deallocated relative to the
// button action, the end of the sheet and the completion handler. Nothing reads an object after its dealloc.
//   Scenario A: sheet via beginSheetModalForWindow:completionHandler:, creator releases its reference right after
//               the call, the probe presses the alert's button with -performClick: (the button action ends the sheet).
//   Scenario B: same, but the sheet is ended with -[NSWindow endSheet:returnCode:].
//   Scenario C: control, the creator keeps its reference until the end.
//   Scenario D: the older beginSheetModalForWindow:modalDelegate:didEndSelector:contextInfo: with button press.
//   Scenario E: same as A, but the completion handler drains an inner autorelease pool before it returns.
// Output: the ordered event lines (handler-start, handler-end, alert-dealloc, pool markers) per scenario.
// Window server: yes (a host window at alpha 0 ordered front, the application is not activated).
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSMutableArray *events;
static void ev(NSString *s) { [events addObject:s]; }
static NSString *joined(void) { return [events count] ? [events componentsJoinedByString:@" ; "] : @"(none)"; }

@interface TAlert : NSAlert
@end
@implementation TAlert
- (void)dealloc { ev(@"alert-dealloc"); [super dealloc]; }
@end

@interface Obj : NSObject
@end
@implementation Obj
- (void)alertDidEnd:(NSAlert *)a returnCode:(NSInteger)r contextInfo:(void *)c { ev(PF(@"didEndSelector(returnCode=%ld)", (long)r)); }
@end

static void scenario(NSString *name, char kind, NSWindow *host, BOOL keepRef) {
    events = [NSMutableArray array];
    @autoreleasepool {
        TAlert *a = [[TAlert alloc] init];
        [a setMessageText:@"probe"];
        [a addButtonWithTitle:@"OK"];
        NSButton *btn = [[[a buttons] objectAtIndex:0] retain];
        NSWindow *aw = [[a window] retain];
        Obj *o = [[Obj alloc] init];
        if (kind == 'D') {
            [a beginSheetModalForWindow:host modalDelegate:o didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:NULL];
        } else {
            [a beginSheetModalForWindow:host completionHandler:^(NSModalResponse r) {
                ev(PF(@"handler-start(returnCode=%ld ; host.attachedSheet=%@)", (long)r, [host attachedSheet] ? @"present" : @"nil"));
                if (kind == 'E') { @autoreleasepool { } ev(@"handler: inner pool drained"); }
                ev(@"handler-end");
            }];
        }
        ev(@"sheet-begun");
        probe_spin(0.3);
        ev(PF(@"sheet-visible=%d", (int)([host attachedSheet] != nil)));
        if (!keepRef) { [a release]; ev(@"creator-released"); }
        if (kind == 'B') [host endSheet:aw returnCode:NSModalResponseStop];
        else [btn performClick:nil];
        ev(@"after-end-call");
        probe_spin(0.5);
        ev(@"spin-done");
        [btn release];
        [aw release];
        [o release];
        if (keepRef) { ev(@"creator-releasing-now"); [a release]; }
    }
    ev(@"outer-pool-drained");
    probe_line("652.alert", name, PF(@"events=[%@]", joined()));
}

int main(void) {
    @autoreleasepool {
        probe_env("652");
        probe_app();
        NSWindow *host = probe_window(NSMakeRect(300, 300, 300, 150), NO);
        [host setStyleMask:NSWindowStyleMaskTitled];
        [host setAlphaValue:0];
        [host orderFrontRegardless];
        probe_spin(0.3);
        probe_line("652.env", @"app active / host visible", PF(@"%d / %d", (int)[NSApp isActive], (int)[host isVisible]));
        scenario(@"A: completion handler, creator releases, button press ends the sheet", 'A', host, NO);
        scenario(@"B: completion handler, creator releases, endSheet:returnCode:", 'B', host, NO);
        scenario(@"C: control, creator keeps its reference, button press", 'C', host, YES);
        scenario(@"D: modalDelegate/didEndSelector, creator releases, button press", 'D', host, NO);
        scenario(@"E: handler drains an inner pool, creator releases, button press", 'E', host, NO);
    }
    return 0;
}
