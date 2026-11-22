// Probe 650: the delegate calls and tracking notifications of a pop-up button's menu.
// A pop-up button (and a pull-down button) in a window at alpha 0 (ordered front without activating the app) has
// a menu with a delegate on the menu and on one item's submenu. The probe opens the menu with -performClick:
// and cancels tracking itself after 0.6 s (-cancelTracking on the menu), then prints, in order: delegate calls
// (menuNeedsUpdate:, menuWillOpen:, menuDidClose:, menu:willHighlightItem:), NSMenuDidBeginTrackingNotification / NSMenuDidEndTrackingNotification (and for which menu), the
// button's action, and changes of the button's selection (KVO on indexOfSelectedItem).
//   --synth : also selects an item (Down, Down, Return) with public Quartz key events posted to this process only
//             (CGEventPostToPid; may need the Accessibility permission). This shows where -menuDidClose: falls
//             relative to the selection change and the action. Without --synth nothing is selected.
// Window server: yes. The application is not activated.
#define PROBE_APPKIT 1
#import "probe_common.h"
#import <CoreGraphics/CoreGraphics.h>

static NSMutableArray *events;
static void ev(NSString *s) { [events addObject:s]; }
static NSString *joined(void) { return [events count] ? [events componentsJoinedByString:@" ; "] : @"(none)"; }
static NSString *itemName(NSMenuItem *i) { return i == nil ? @"nil" : [i isSeparatorItem] ? @"<separator>" : [i title]; }

@interface D : NSObject <NSMenuDelegate>
@end
@implementation D
- (void)menuNeedsUpdate:(NSMenu *)m { ev(PF(@"needsUpdate(%@)", [m title])); }
- (void)menuWillOpen:(NSMenu *)m { ev(PF(@"willOpen(%@)", [m title])); }
- (void)menuDidClose:(NSMenu *)m { ev(PF(@"didClose(%@)", [m title])); }
- (void)menu:(NSMenu *)m willHighlightItem:(NSMenuItem *)i { ev(PF(@"willHighlight(%@: %@)", [m title], itemName(i))); }
- (void)chosen:(id)sender { ev(PF(@"ACTION(selectedItem=%@)", itemName([(NSPopUpButton *)sender selectedItem]))); }
- (void)observeValueForKeyPath:(NSString *)k ofObject:(id)o change:(NSDictionary *)c context:(void *)x { ev(PF(@"SELECTION-CHANGED(now %@)", itemName([(NSPopUpButton *)o selectedItem]))); }
@end

static void postKey(CGKeyCode k) {
    CGEventRef dn = CGEventCreateKeyboardEvent(NULL, k, true), up = CGEventCreateKeyboardEvent(NULL, k, false);
    CGEventPostToPid(getpid(), dn); CGEventPostToPid(getpid(), up);
    CFRelease(dn); CFRelease(up);
}

static void runCase(NSString *name, BOOL pullsDown, BOOL autoenables, BOOL synth, D *d, NSWindow *w) {
    events = [NSMutableArray array];
    NSPopUpButton *pb = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(10, 10, 140, 26) pullsDown:pullsDown];
    [pb addItemsWithTitles:@[@"First", @"Second", @"Third"]];
    NSMenu *m = [pb menu];
    [m setTitle:@"pb-menu"];
    [m setAutoenablesItems:autoenables];
    NSMenuItem *subItem = [m addItemWithTitle:@"WithSub" action:NULL keyEquivalent:@""];
    NSMenu *sub = [[NSMenu alloc] initWithTitle:@"pb-sub"];
    [sub addItemWithTitle:@"SubOne" action:NULL keyEquivalent:@""];
    [subItem setSubmenu:sub];
    [m setDelegate:d];
    [sub setDelegate:d];
    [pb setTarget:d];
    [pb setAction:@selector(chosen:)];
    [[w contentView] setSubviews:@[pb]];
    [pb addObserver:d forKeyPath:@"indexOfSelectedItem" options:0 context:NULL];
    __block int began = 0, ended = 0;
    id o1 = [[NSNotificationCenter defaultCenter] addObserverForName:NSMenuDidBeginTrackingNotification object:nil queue:nil usingBlock:^(NSNotification *n) { began++; ev(PF(@"BeginTracking(%@)", [[n object] title])); }];
    id o2 = [[NSNotificationCenter defaultCenter] addObserverForName:NSMenuDidEndTrackingNotification object:nil queue:nil usingBlock:^(NSNotification *n) { ended++; ev(PF(@"EndTracking(%@)", [[n object] title])); }];
    NSMutableArray *steps = [NSMutableArray array];
    NSTimeInterval t = 0.5;
    if (synth) {
        for (int k = 0; k < 2; k++) { [steps addObject:@[@(t), ^{ ev(@"key-down-arrow"); postKey(125); }]]; t += 0.25; }
        [steps addObject:@[@(t), ^{ ev(@"key-return"); postKey(36); }]];
        t += 0.6;
    }
    [steps addObject:@[@(t), ^{ ev(@"cancelTracking-called"); [m cancelTracking]; }]];
    for (NSArray *s in steps) {
        dispatch_block_t b = s[1];
        [[NSRunLoop currentRunLoop] addTimer:[NSTimer timerWithTimeInterval:[s[0] doubleValue] repeats:NO block:^(NSTimer *x) { b(); }] forMode:NSRunLoopCommonModes];
    }
    ev(@"performClick-called");
    [pb performClick:nil];
    ev(@"performClick-returned");
    probe_spin(0.4);
    probe_line("650.popup", PF(@"%@%@", name, synth ? @" (synthesised keys)" : @""), PF(@"begin=%d end=%d ; events=[%@]", began, ended, joined()));
    [pb removeObserver:d forKeyPath:@"indexOfSelectedItem"];
    [[NSNotificationCenter defaultCenter] removeObserver:o1];
    [[NSNotificationCenter defaultCenter] removeObserver:o2];
}

int main(int argc, char **argv) {
    @autoreleasepool {
        BOOL synth = NO;
        for (int i = 1; i < argc; i++) if (!strcmp(argv[i], "--synth")) synth = YES;
        probe_env("650");
        probe_app();
        NSWindow *w = probe_window(NSMakeRect(300, 300, 200, 80), NO);
        [w setAlphaValue:0];
        [w orderFrontRegardless];
        probe_spin(0.3);
        D *d = [[D alloc] init];
        probe_line("650.env", @"app active / window visible", PF(@"%d / %d", (int)[NSApp isActive], (int)[w isVisible]));
        runCase(@"pop-up (pullsDown NO), autoenablesItems YES", NO, YES, NO, d, w);
        runCase(@"pop-up (pullsDown NO), autoenablesItems NO", NO, NO, NO, d, w);
        runCase(@"pull-down (pullsDown YES), autoenablesItems YES", YES, YES, NO, d, w);
        if (synth) {
            runCase(@"pop-up, select an item", NO, YES, YES, d, w);
            runCase(@"pull-down, select an item", YES, YES, YES, d, w);
        }
    }
    return 0;
}
