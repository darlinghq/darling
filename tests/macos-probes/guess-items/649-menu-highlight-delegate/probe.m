// Probe 649: when does an NSMenu's delegate hear -menu:willHighlightItem:?
// Items: A (enabled), a separator, B (disabled), C (enabled), D (enabled, has a submenu).
//   Part 1 (default, no input from anyone): the menu pops up at the pointer with each row as the positioning item,
//          so the pointer lies on that row; after 0.6 s the probe cancels tracking. Prints every delegate call and
//          notification in order, what -highlightedItem answers while it is up, and the order relative to
//          -menuDidClose:. Whether the delegate gets a highlight call at open for the row under the pointer, for a
//          separator row and for a disabled row, and whether a nil call comes at close, is what is observed.
//   Part 2 (--synth): also drives the open menu with public Quartz events posted to this process only
//          (CGEventPostToPid): arrow keys, and mouse moves down the menu window. May need the Accessibility
//          permission; if the events are not delivered the output says so. The application is not activated.
//   Part 3 (--manual-menubar): asks a person to move over the menu bar for 20 s (makes the application regular
//          and activates it, so it steals focus). Prints the delegate calls for the menu bar and its dropdowns.
// Window server: yes.
#define PROBE_APPKIT 1
#import "probe_common.h"
#import <CoreGraphics/CoreGraphics.h>
#include <unistd.h>

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
@end

static NSMenu *makeMenu(NSString *title, D *d, __strong NSMenuItem **rows) {
    NSMenu *m = [[NSMenu alloc] initWithTitle:title];
    [m setAutoenablesItems:NO];
    [m setDelegate:d];
    rows[0] = [m addItemWithTitle:@"A" action:@selector(description) keyEquivalent:@""];
    [m addItem:[NSMenuItem separatorItem]];
    rows[1] = [m itemAtIndex:1];
    rows[2] = [m addItemWithTitle:@"B-disabled" action:@selector(description) keyEquivalent:@""];
    [rows[2] setEnabled:NO];
    rows[3] = [m addItemWithTitle:@"C" action:@selector(description) keyEquivalent:@""];
    rows[4] = [m addItemWithTitle:@"D-submenu" action:@selector(description) keyEquivalent:@""];
    NSMenu *sub = [[NSMenu alloc] initWithTitle:@"sub"];
    [sub setDelegate:d];
    [sub addItemWithTitle:@"S1" action:@selector(description) keyEquivalent:@""];
    [rows[4] setSubmenu:sub];
    for (int i = 0; i < 5; i++) [rows[i] setTarget:d];
    return m;
}

static NSArray *menuWindows(void) {
    NSMutableArray *out = [NSMutableArray array];
    CFArrayRef list = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    for (NSDictionary *d in (__bridge NSArray *)list) {
        if ([d[(id)kCGWindowOwnerPID] intValue] != getpid()) continue;
        CGRect r; CGRectMakeWithDictionaryRepresentation((__bridge CFDictionaryRef)d[(id)kCGWindowBounds], &r);
        [out addObject:[NSValue valueWithRect:r]];
    }
    if (list) CFRelease(list);
    return out;
}

static void spinTracking(NSTimeInterval sec) { [[NSRunLoop currentRunLoop] runMode:NSEventTrackingRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:sec]]; }
static void postKey(CGKeyCode k) {
    CGEventRef dn = CGEventCreateKeyboardEvent(NULL, k, true), up = CGEventCreateKeyboardEvent(NULL, k, false);
    CGEventPostToPid(getpid(), dn); CGEventPostToPid(getpid(), up);
    CFRelease(dn); CFRelease(up);
}
static void postMove(CGPoint p) {
    CGEventRef e = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, p, kCGMouseButtonLeft);
    CGEventPostToPid(getpid(), e); CFRelease(e);
}

static void runCase(NSString *name, int positioning, BOOL synth, D *d) {
    events = [NSMutableArray array];
    NSMenuItem *rows[5];
    NSMenu *m = makeMenu(@"popup", d, rows);
    __block NSString *hl = @"(not read)";
    __block BOOL began = NO;
    id o1 = [[NSNotificationCenter defaultCenter] addObserverForName:NSMenuDidBeginTrackingNotification object:m queue:nil usingBlock:^(NSNotification *n) { began = YES; ev(@"NSMenuDidBeginTracking"); }];
    id o2 = [[NSNotificationCenter defaultCenter] addObserverForName:NSMenuDidEndTrackingNotification object:m queue:nil usingBlock:^(NSNotification *n) { ev(@"NSMenuDidEndTracking"); }];
    NSMutableArray *steps = [NSMutableArray array];
    NSTimeInterval t = 0.5;
    if (synth) {
        for (int k = 0; k < 7; k++) {
            NSString *lab = PF(@"key-down#%d", k + 1);
            [steps addObject:@[@(t), ^{ ev(lab); postKey(125); }]];
            [steps addObject:@[@(t + 0.08), ^{ ev(PF(@"highlightedItem=%@", itemName([m highlightedItem]))); }]];
            t += 0.2;
        }
        [steps addObject:@[@(t), ^{ ev(@"key-up-arrow"); postKey(126); }]];
        t += 0.2;
        [steps addObject:@[@(t), ^{
            NSArray *w = menuWindows();
            NSRect best = NSZeroRect;
            for (NSValue *v in w) if ([v rectValue].size.height > best.size.height && [v rectValue].size.width > 40 && [v rectValue].size.width < 600) best = [v rectValue];
            ev(PF(@"menu window rect(CG)=%@", probe_rect(best)));
            for (CGFloat y = NSMinY(best) + 3; y < NSMaxY(best); y += 6) {
                postMove(CGPointMake(NSMinX(best) + 20, y));
                spinTracking(0.05);
                NSString *h = itemName([m highlightedItem]);
                ev(PF(@"mouse-at-dy=%g highlighted=%@", y - NSMinY(best), h));
            }
            postMove(CGPointMake(NSMaxX(best) + 400, NSMaxY(best) + 400));
            spinTracking(0.1);
            ev(PF(@"mouse-far-away highlighted=%@", itemName([m highlightedItem])));
        }]];
        t += 1.0;
    } else {
        [steps addObject:@[@(t), ^{ hl = itemName([m highlightedItem]); ev(PF(@"highlightedItem@0.5s=%@", hl)); }]];
        t += 0.1;
    }
    [steps addObject:@[@(t), ^{ ev(@"cancelTracking-called"); [m cancelTracking]; }]];
    for (NSArray *s in steps) {
        dispatch_block_t b = s[1];
        NSTimer *tm = [NSTimer timerWithTimeInterval:[s[0] doubleValue] repeats:NO block:^(NSTimer *x) { b(); }];
        [[NSRunLoop currentRunLoop] addTimer:tm forMode:NSRunLoopCommonModes];
    }
    NSPoint loc = [NSEvent mouseLocation];
    BOOL r = [m popUpMenuPositioningItem:positioning >= 0 ? rows[positioning] : nil atLocation:loc inView:nil];
    ev(PF(@"popUp-returned(%d)", (int)r));
    probe_spin(0.3);
    ev(@"after-0.3s");
    probe_line("649.popup", PF(@"%@%@", name, synth ? @" (synthesised input)" : @""), PF(@"tracking began=%d ; events=[%@]", (int)began, joined()));
    [[NSNotificationCenter defaultCenter] removeObserver:o1];
    [[NSNotificationCenter defaultCenter] removeObserver:o2];
}

int main(int argc, char **argv) {
    @autoreleasepool {
        BOOL synth = NO, manual = NO;
        for (int i = 1; i < argc; i++) { if (!strcmp(argv[i], "--synth")) synth = YES; if (!strcmp(argv[i], "--manual-menubar")) manual = YES; }
        probe_env("649");
        probe_app();
        D *d = [[D alloc] init];
        NSArray *names = @[@"positioned on A (enabled)", @"positioned on the separator", @"positioned on B (disabled)", @"positioned on C (enabled)", @"positioned on D (has submenu)"];
        runCase(@"positioning item nil, pointer anywhere", -1, NO, d);
        for (int i = 0; i < 5; i++) runCase(names[i], i, NO, d);
        if (synth) { runCase(@"positioning item nil", -1, YES, d); runCase(names[2], 2, YES, d); }
        if (manual) {
            events = [NSMutableArray array];
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
            NSMenuItem *rows[5];
            NSMenu *bar = makeMenu(@"bar", d, rows);
            NSMenu *dd = makeMenu(@"dropdown", d, rows);
            [bar removeAllItems];
            NSMenuItem *t1 = [bar addItemWithTitle:@"Enabled" action:NULL keyEquivalent:@""]; [t1 setSubmenu:dd];
            NSMenuItem *t2 = [bar addItemWithTitle:@"DisabledTitle" action:NULL keyEquivalent:@""]; [t2 setSubmenu:[[NSMenu alloc] initWithTitle:@"dis"]]; [t2 setEnabled:NO];
            [NSApp setMainMenu:bar];
            [NSApp activateIgnoringOtherApps:YES];
            fprintf(stderr, "649 manual: for 20 s move the pointer over the menu bar titles, along the dropdown rows (including the separator, the disabled row, the submenu row), then away, then close the menu.\n");
            probe_spin(20);
            probe_line("649.menubar", @"manual run, delegate on the bar menu and on the dropdown", PF(@"events=[%@]", joined()));
        }
    }
    return 0;
}
