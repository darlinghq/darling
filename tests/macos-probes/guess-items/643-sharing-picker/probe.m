// Probe 643: NSSharingServicePicker and its delegate.
//   Part 1 (always): showRelativeToRect:ofView:preferredEdge: for several delegate set-ups. Prints the order of
//          the delegate calls, whether show returns before any callback, whether menu tracking begins, which windows
//          of this process are on screen while it is up and where they are relative to the rectangle, for each edge.
//          The menu is cancelled by the probe (cancelTracking from the begin-tracking notification) and the host
//          window has alpha 0 and is never activated. No item is chosen by a person.
//   Part 2 (always): the order of "delegateForSharingService:", the service's own handler and
//          "didChooseSharingService:" when a custom, do-nothing service (initWithTitle:image:alternateImage:handler:)
//          is chosen through the picker's standardShareMenuItem submenu, performed by the probe itself.
// Window server: yes. Nothing is shared: the only service the probe makes does nothing.
#define PROBE_APPKIT 1
#import "probe_common.h"
#import <CoreGraphics/CoreGraphics.h>
#include <unistd.h>

static NSMutableArray *events;
static void ev(NSString *s) { [events addObject:s]; }
static NSString *joined(void) { return [events count] ? [events componentsJoinedByString:@" ; "] : @"(none)"; }
static NSString *titles(NSArray *services) {
    NSMutableArray *a = [NSMutableArray array];
    for (NSSharingService *s in services) [a addObject:[s title] ?: @"(nil)"];
    return [a componentsJoinedByString:@","];
}

@interface SvcDelegate : NSObject <NSSharingServiceDelegate>
@end
@implementation SvcDelegate
- (void)sharingService:(NSSharingService *)s willShareItems:(NSArray *)items { ev(@"service-delegate willShareItems"); }
- (void)sharingService:(NSSharingService *)s didShareItems:(NSArray *)items { ev(@"service-delegate didShareItems"); }
- (void)sharingService:(NSSharingService *)s didFailToShareItems:(NSArray *)items error:(NSError *)e { ev(@"service-delegate didFailToShareItems"); }
@end

typedef NS_ENUM(int, Mode) { ModeNone, ModePassThrough, ModeEmpty, ModeCustomOnly, ModeNilReturn };

@interface PickerDelegate : NSObject <NSSharingServicePickerDelegate>
@property Mode mode;
@property (strong) NSSharingService *custom;
@property (strong) SvcDelegate *svcDelegate;
@end
@implementation PickerDelegate
- (NSArray *)sharingServicePicker:(NSSharingServicePicker *)p sharingServicesForItems:(NSArray *)items proposedSharingServices:(NSArray *)proposed {
    ev(PF(@"sharingServicesForItems(items=%lu proposed=%lu)", (unsigned long)[items count], (unsigned long)[proposed count]));
    switch (self.mode) {
        case ModeEmpty: return @[];
        case ModeNilReturn: return nil;
        case ModeCustomOnly: return @[self.custom];
        default: return proposed;
    }
}
- (id<NSSharingServiceDelegate>)sharingServicePicker:(NSSharingServicePicker *)p delegateForSharingService:(NSSharingService *)s {
    ev(PF(@"delegateForSharingService(%@)", [s title]));
    return self.svcDelegate;
}
- (void)sharingServicePicker:(NSSharingServicePicker *)p didChooseSharingService:(NSSharingService *)s {
    ev(PF(@"didChooseSharingService(%@)", s ? [s title] : @"nil"));
}
@end

static NSArray *myWindows(NSWindow *host) {
    NSMutableArray *out = [NSMutableArray array];
    CFArrayRef list = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    CGFloat H = [[NSScreen screens][0] frame].size.height;
    for (NSDictionary *d in (__bridge NSArray *)list) {
        if ([d[(id)kCGWindowOwnerPID] intValue] != getpid() || [d[(id)kCGWindowNumber] integerValue] == [host windowNumber]) continue;
        CGRect r; CGRectMakeWithDictionaryRepresentation((__bridge CFDictionaryRef)d[(id)kCGWindowBounds], &r);
        r.origin.y = H - r.origin.y - r.size.height;
        [out addObject:@{@"layer": d[(id)kCGWindowLayer], @"rect": [NSValue valueWithRect:r]}];
    }
    if (list) CFRelease(list);
    return out;
}

static NSString *edgeName(NSRectEdge e) { return e == NSMinXEdge ? @"minX" : e == NSMinYEdge ? @"minY" : e == NSMaxXEdge ? @"maxX" : @"maxY"; }

static void runCase(NSString *name, NSArray *items, Mode mode, BOOL withDelegate, NSRectEdge edge, NSWindow *host, NSView *view, PickerDelegate *pd, NSSharingService *custom) {
    events = [NSMutableArray array];
    pd.mode = mode; pd.custom = custom;
    NSSharingServicePicker *picker = [[NSSharingServicePicker alloc] initWithItems:items];
    if (withDelegate) [picker setDelegate:pd];
    __block BOOL began = NO, ended = NO;
    __block NSString *menuDesc = @"";
    id o1 = [[NSNotificationCenter defaultCenter] addObserverForName:NSMenuDidBeginTrackingNotification object:nil queue:nil usingBlock:^(NSNotification *n) {
        began = YES; ev(@"NSMenuDidBeginTrackingNotification");
        NSMenu *m = [n object];
        NSMutableArray *t = [NSMutableArray array];
        for (NSMenuItem *i in [m itemArray]) [t addObject:[i isSeparatorItem] ? @"-" : [i title]];
        menuDesc = PF(@"menu items=%lu [%@]", (unsigned long)[t count], [t componentsJoinedByString:@"|"]);
        [m cancelTracking];
    }];
    id o2 = [[NSNotificationCenter defaultCenter] addObserverForName:NSMenuDidEndTrackingNotification object:nil queue:nil usingBlock:^(NSNotification *n) { ended = YES; ev(@"NSMenuDidEndTrackingNotification"); }];
    NSRect rect = NSMakeRect(20, 10, 60, 30);
    ev(@"show-called");
    [picker showRelativeToRect:rect ofView:view preferredEdge:edge];
    ev(@"show-returned");
    NSArray *wins = nil;
    for (int i = 0; i < 8; i++) {
        probe_spin(0.1);
        NSArray *w = myWindows(host);
        if ([w count] && !wins) wins = w;   // first moment any window of ours is on screen
    }
    NSRect target = [host convertRectToScreen:[view convertRect:rect toView:nil]];
    NSMutableArray *wd = [NSMutableArray array];
    for (NSDictionary *d in wins) {
        NSRect r = [d[@"rect"] rectValue];
        [wd addObject:PF(@"layer=%@ rect=%@ ; (minX-rect.midX, midX-rect.midX, minY-rect.midY, maxY-rect.midY)=(%g, %g, %g, %g)", d[@"layer"], probe_rect(r),
            NSMinX(r) - NSMidX(target), NSMidX(r) - NSMidX(target), NSMinY(r) - NSMidY(target), NSMaxY(r) - NSMidY(target))];
    }
    probe_line("643.show", PF(@"%@ ; edge=%@ ; rect on screen=%@", name, edgeName(edge), probe_rect(target)),
               PF(@"events=[%@] ; tracking began=%d ended=%d ; %@ ; own windows on screen=%lu %@", joined(), (int)began, (int)ended, menuDesc, (unsigned long)[wins count], [wd componentsJoinedByString:@" || "]));
    [[NSNotificationCenter defaultCenter] removeObserver:o1];
    [[NSNotificationCenter defaultCenter] removeObserver:o2];
}

int main(void) {
    @autoreleasepool {
        probe_env("643");
        probe_app();
        NSWindow *host = probe_window(NSMakeRect(300, 300, 200, 100), NO);
        [host setAlphaValue:0];
        NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 200, 100)];
        [host setContentView:view];
        [host orderFrontRegardless];   // visible at alpha 0, no activation
        probe_spin(0.3);
        PickerDelegate *pd = [[PickerDelegate alloc] init];
        pd.svcDelegate = [[SvcDelegate alloc] init];
        NSSharingService *custom = [[NSSharingService alloc] initWithTitle:@"ProbeService" image:[NSImage imageNamed:NSImageNameInfo] alternateImage:nil handler:^{ ev(@"custom-service-handler"); }];
        probe_line("643.env", @"app active / host window visible / host key", PF(@"%d / %d / %d", (int)[NSApp isActive], (int)[host isVisible], (int)[host isKeyWindow]));

        // Part 1
        NSArray *text = @[@"probe text"];
        runCase(@"no delegate, one string item", text, ModeNone, NO, NSMinYEdge, host, view, pd, custom);
        runCase(@"delegate passes proposed services through", text, ModePassThrough, YES, NSMinYEdge, host, view, pd, custom);
        runCase(@"delegate returns an empty array", text, ModeEmpty, YES, NSMinYEdge, host, view, pd, custom);
        runCase(@"delegate returns nil", text, ModeNilReturn, YES, NSMinYEdge, host, view, pd, custom);
        runCase(@"delegate returns only the custom service", text, ModeCustomOnly, YES, NSMinYEdge, host, view, pd, custom);
        runCase(@"empty items array, no delegate", @[], ModeNone, NO, NSMinYEdge, host, view, pd, custom);
        runCase(@"empty items array, delegate adds custom", @[], ModeCustomOnly, YES, NSMinYEdge, host, view, pd, custom);
        runCase(@"unshareable item (a bare NSObject), no delegate", @[[[NSObject alloc] init]], ModeNone, NO, NSMinYEdge, host, view, pd, custom);
        for (NSNumber *e in @[@(NSMinXEdge), @(NSMaxXEdge), @(NSMinYEdge), @(NSMaxYEdge)])
            runCase(@"edges, delegate returns only the custom service", text, ModeCustomOnly, YES, (NSRectEdge)[e intValue], host, view, pd, custom);

        // Part 2: choose the custom service through standardShareMenuItem, performed by the probe
        events = [NSMutableArray array];
        pd.mode = ModeCustomOnly; pd.custom = custom;
        NSSharingServicePicker *picker = [[NSSharingServicePicker alloc] initWithItems:text];
        [picker setDelegate:pd];
        NSMenuItem *std = [picker respondsToSelector:NSSelectorFromString(@"standardShareMenuItem")] ? [picker valueForKey:@"standardShareMenuItem"] : nil;
        probe_line("643.choose", @"picker.standardShareMenuItem", std ? PF(@"title=%@ submenu items=%lu", [std title], (unsigned long)[[[std submenu] itemArray] count]) : @"not available");
        probe_line("643.choose", @"events so far (after creating and reading the item)", joined());
        NSMenu *sm = [std submenu];
        [sm update];
        NSMutableArray *t = [NSMutableArray array];
        for (NSMenuItem *i in [sm itemArray]) [t addObject:[i isSeparatorItem] ? @"-" : [i title]];
        probe_line("643.choose", @"submenu after -update", PF(@"[%@] ; events=[%@]", [t componentsJoinedByString:@"|"], joined()));
        probe_line("643.choose", @"custom.delegate before choosing", [custom delegate] ? NSStringFromClass([[custom delegate] class]) : @"nil");
        NSInteger idx = [sm indexOfItemWithTitle:@"ProbeService"];
        if (idx < 0) {
            probe_line("643.choose", @"custom service in the submenu", @"not found; choosing skipped");
        } else {
            [events removeAllObjects];
            [sm performActionForItemAtIndex:idx];
            probe_spin(0.5);
            probe_line("643.choose", @"perform the custom item (submenu performActionForItemAtIndex:)", PF(@"events=[%@] ; custom.delegate after=%@", joined(), [custom delegate] ? NSStringFromClass([[custom delegate] class]) : @"nil"));
        }
        // Control: the service alone, without the picker
        events = [NSMutableArray array];
        [custom setDelegate:pd.svcDelegate];
        probe_line("643.control", @"custom performWithItems: directly, service delegate set", PF(@"canPerform=%d", (int)[custom canPerformWithItems:text]));
        [custom performWithItems:text];
        probe_spin(0.3);
        probe_line("643.control", @"events after performWithItems:", joined());
    }
    return 0;
}
