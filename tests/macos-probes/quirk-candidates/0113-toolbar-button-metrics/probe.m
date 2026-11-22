// Probe 0113: sizes that a window's toolbar gives to items whose view is an image-only button, an image-only
// pull-down button and a text button, and the frame of each view inside the window, plus a row of pixels through
// the toolbar. The window is ordered in at alpha 0 for one second so layout runs; nothing is visible.
//   ./probe [--front]   --front also samples a key window (accent colours need an active window)
#define PROBE_APPKIT 1
#import "probe_common.h"
#include <string.h>

@interface Delegate : NSObject <NSToolbarDelegate>
@property(strong) NSMutableDictionary *items;
@end
@implementation Delegate
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)t { return @[@"img", @"pull", @"txt", NSToolbarFlexibleSpaceItemIdentifier, @"img2"]; }
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)t { return [self toolbarDefaultItemIdentifiers:t]; }
- (NSToolbarItem *)toolbar:(NSToolbar *)t itemForItemIdentifier:(NSString *)ident willBeInsertedIntoToolbar:(BOOL)flag {
    NSToolbarItem *it = [[NSToolbarItem alloc] initWithItemIdentifier:ident];
    NSImage *add = [NSImage imageNamed:NSImageNameAddTemplate];
    NSImage *action = [NSImage imageNamed:NSImageNameActionTemplate];
    if ([ident isEqualToString:@"img"] || [ident isEqualToString:@"img2"]) {
        NSButton *b = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 30, 24)];
        [b setBezelStyle:NSBezelStyleTexturedRounded];
        [b setImage:ident.length == 3 ? add : action];
        [it setView:b];
    } else if ([ident isEqualToString:@"pull"]) {
        NSPopUpButton *p = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 40, 24) pullsDown:YES];
        [p setBezelStyle:NSBezelStyleTexturedRounded];
        [p addItemsWithTitles:@[@"", @"One", @"Two"]];
        [[p itemAtIndex:0] setImage:action];
        [it setView:p];
    } else {
        NSButton *b = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 60, 24)];
        [b setBezelStyle:NSBezelStyleTexturedRounded];
        [b setTitle:@"Text"];
        [it setView:b];
    }
    [it setLabel:ident];
    if (self.items == nil) self.items = [NSMutableDictionary dictionary];
    self.items[ident] = it;
    return it;
}
@end

static void measure(BOOL front) {
    for (int dark = 0; dark <= 1; dark++) {
        NSWindow *w = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, 480, 200)
                                                   styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                                                     backing:NSBackingStoreBuffered defer:NO];
        [w setReleasedWhenClosed:NO];
        NSAppearance *ap = probe_appearance(dark);
        if (ap) [w setAppearance:ap];
        Delegate *d = [[Delegate alloc] init];
        NSToolbar *tb = [[NSToolbar alloc] initWithIdentifier:PF(@"probe-%d", dark)];
        [tb setDelegate:d];
        [tb setDisplayMode:NSToolbarDisplayModeIconOnly];
        [w setToolbar:tb];
#if PROBE_SDK_AT_LEAST(110000)
        if ([w respondsToSelector:@selector(setToolbarStyle:)]) [w setToolbarStyle:NSWindowToolbarStyleUnified];
#endif
        [w setAlphaValue:front ? 1.0 : 0.0];
        if (front) [w makeKeyAndOrderFront:nil]; else [w orderFront:nil];
        probe_spin(1.0);
        const char *mode = front ? "key window" : "inactive window";
        const char *ap_name = dark ? "dark" : "light";
        NSRect fr = [w frame];
        NSSize layoutSize = [w respondsToSelector:@selector(contentLayoutRect)] ? [[w valueForKey:@"contentLayoutRect"] rectValue].size : NSZeroSize;
        probe_line("0113.window", PF(@"%s, %s appearance, content 480x200, 5 toolbar items", mode, ap_name),
                   PF(@"frame %@ ; contentRect-for-frame %@ ; contentLayoutRect %@ ; space taken above content layout = %g",
                      probe_size(fr.size), probe_size([w contentRectForFrameRect:fr].size), probe_size(layoutSize), fr.size.height - layoutSize.height));
        for (NSString *ident in @[@"img", @"pull", @"txt", @"img2"]) {
            NSToolbarItem *it = d.items[ident];
            NSView *v = [it view];
            NSRect inWin = v && [v window] ? [v convertRect:[v bounds] toView:nil] : NSZeroRect;
            probe_line("0113.item", PF(@"%s, %s appearance, item %@ (%@)", mode, ap_name, ident,
                                       [ident isEqualToString:@"pull"] ? @"image-only pull-down" : [ident isEqualToString:@"txt"] ? @"text button" : @"image-only button"),
                       PF(@"view frame size %@ ; view rect in window coordinates %@ ; item minSize %@ maxSize %@ ; view fittingSize %@ ; visibilityPriority %ld",
                          v ? probe_size([v frame].size) : @"nil", probe_rect(inWin), probe_size([it minSize]), probe_size([it maxSize]),
                          v ? probe_fitting(v) : @"nil", (long)[it visibilityPriority]));
        }
        NSView *frameView = [[w contentView] superview];
        NSBitmapImageRep *rep = probe_snapshot(frameView);
        if (rep) {
            NSRect b = [frameView bounds];
            double sx = [rep pixelsWide] / b.size.width, sy = [rep pixelsHigh] / b.size.height;
            // A row through the vertical middle of the toolbar, taken from the first item's view.
            NSView *first = [d.items[@"img"] view];
            double yMid = first && [first window] ? NSMidY([first convertRect:[first bounds] toView:nil]) : b.size.height - 40;
            double yFromTop = b.size.height - yMid;
            NSMutableArray *samples = [NSMutableArray array];
            for (double x = 2; x < 240; x += 3)
                [samples addObject:probe_pixel(rep, (NSInteger)(x * sx), (NSInteger)(yFromTop * sy))];
            probe_line("0113.pixels", PF(@"%s, %s appearance, window bitmap %ldx%ld, row %.1f points from the top, x = 2,5,8,... points, sRGB rrggbbaa", mode, ap_name,
                                         (long)[rep pixelsWide], (long)[rep pixelsHigh], yFromTop),
                       [samples componentsJoinedByString:@" "]);
        }
        [w orderOut:nil];
        [w close];
    }
}

int main(int argc, char **argv) {
    @autoreleasepool {
        probe_env("0113");
        probe_app();
        BOOL front = NO;
        for (int i = 1; i < argc; i++) if (!strcmp(argv[i], "--front")) front = YES;
        measure(NO);
        if (front) measure(YES);
        else probe_line("0113", @"key-window variant", @"SKIPPED (run with --front)");
    }
    return 0;
}
