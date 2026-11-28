// Probe 671: NSButton showsBorderOnlyWhileMouseInside.
// A rounded-bezel push button is put in a borderless window at alpha 0 (ordered front without activating the app).
// Its rendering is compared, by bitmap equality, against two references made from the same title: a normal bordered
// button and a button with bordered = NO. Each state is reported as "== bordered", "== borderless" or "other"
// (rendering through -cacheDisplayInRect:toBitmapImageRep:, no screen capture). Also prints the cell's public state
// and the button's tracking areas (owner class, rectangle, option bits; NSTrackingActiveInActiveApp=0x40,
// ActiveAlways=0x80, ActiveInKeyWindow=0x20, ActiveWhenFirstResponder=0x10, MouseEnteredAndExited=0x01,
// AssumeInside=0x100, InVisibleRect=0x200), and where the window believes the pointer is.
// Variant "away": the window is placed so the pointer is outside the button. Variant "inside": the window is
// placed so the pointer is over the middle of the button when the property is set (the pointer is NOT moved).
// Steps per variant: before the property; after setting it; after 0.5 s; after growing the window and the button;
// after -updateTrackingAreas; after synthesised enter and exit events given to the button and to its cell (public
// +enterExitEventWithType:..., the tracking number is 0 so a real tracking area is not involved); after setting
// the property back to NO; the property with bordered = NO; other bezel styles; a cell drawn by a view that is not
// its control view.
//   --warp : also moves the real pointer onto the button with CGWarpMouseCursorPosition (and back) and reports.
// Window server: yes. The application is never activated.
#define PROBE_APPKIT 1
#import "probe_common.h"
#import <CoreGraphics/CoreGraphics.h>

static NSData *snapData(NSView *v) { NSBitmapImageRep *r = probe_snapshot(v); return r ? [NSData dataWithBytes:[r bitmapData] length:[r bytesPerRow] * [r pixelsHigh]] : nil; }

static NSData *gBordered, *gPlain;
static NSString *cls(NSData *d) { return !d ? @"n/a" : [d isEqualToData:gBordered] ? @"== bordered" : [d isEqualToData:gPlain] ? @"== borderless" : @"other"; }

static NSButton *makeButton(NSBezelStyle style, BOOL bordered, BOOL flag) {
    NSButton *b = [[NSButton alloc] initWithFrame:NSMakeRect(20, 20, 100, 32)];
    [b setTitle:@"Probe"];
    [b setButtonType:NSButtonTypeMomentaryPushIn];
    [b setBezelStyle:style];
    [b setBordered:bordered];
    [b setShowsBorderOnlyWhileMouseInside:flag];
    return b;
}
static NSString *areas(NSButton *b) {
    NSMutableArray *a = [NSMutableArray array];
    for (NSTrackingArea *t in [b trackingAreas]) [a addObject:PF(@"{owner=%@ rect=%@ options=0x%lx}", NSStringFromClass([[t owner] class]), probe_rect([t rect]), (unsigned long)[t options])];
    return PF(@"%lu [%@]", (unsigned long)[[b trackingAreas] count], [a componentsJoinedByString:@" "]);
}
static NSString *state(NSButton *b, NSWindow *w) {
    NSPoint p = [w mouseLocationOutsideOfEventStream], q = [b convertPoint:p fromView:nil];
    NSButtonCell *c = [b cell];
    return PF(@"flag=%d cell.flag=%d bordered=%d ; window pointer %@ in button=%d ; trackingAreas %@ ; render %@",
              (int)[b showsBorderOnlyWhileMouseInside], (int)[c showsBorderOnlyWhileMouseInside], (int)[b isBordered], NSStringFromPoint(p), (int)NSPointInRect(q, [b bounds]), areas(b), cls(snapData(b)));
}
static NSEvent *enterExit(NSEventType t, NSWindow *w, NSPoint p) {
    return [NSEvent enterExitEventWithType:t location:p modifierFlags:0 timestamp:0 windowNumber:[w windowNumber] context:nil eventNumber:0 trackingNumber:0 userData:NULL];
}

@interface CellHost : NSView
@property (strong) NSButtonCell *cell;
@end
@implementation CellHost
- (void)drawRect:(NSRect)r { [self.cell drawWithFrame:NSMakeRect(20, 20, 100, 32) inView:self]; }
@end

static void variant(NSString *name, BOOL inside, BOOL warp) {
    NSPoint m = [NSEvent mouseLocation];
    NSWindow *w = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 160, 80) styleMask:NSWindowStyleMaskBorderless backing:NSBackingStoreBuffered defer:NO];
    [w setReleasedWhenClosed:NO];
    [w setBackgroundColor:[NSColor windowBackgroundColor]];
    [w setAlphaValue:0];
    NSRect scr = [[NSScreen screens][0] frame];
    NSPoint origin = inside ? NSMakePoint(m.x - 70, m.y - 36) : NSMakePoint(m.x > NSMidX(scr) ? NSMinX(scr) + 50 : NSMaxX(scr) - 250, NSMinY(scr) + 100);
    [w setFrameOrigin:origin];
    NSView *cv = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 160, 80)];
    [w setContentView:cv];
    // references
    NSButton *ref = makeButton(NSBezelStyleRounded, YES, NO); [cv addSubview:ref]; gBordered = snapData(ref); [ref removeFromSuperview];
    ref = makeButton(NSBezelStyleRounded, NO, NO); [cv addSubview:ref]; gPlain = snapData(ref); [ref removeFromSuperview];
    probe_line("671.ref", PF(@"%@ reference renders differ from each other", name), PF(@"%d", (int)![gBordered isEqualToData:gPlain]));
    [w orderFrontRegardless];
    probe_spin(0.2);
    NSButton *b = makeButton(NSBezelStyleRounded, YES, NO);
    [cv addSubview:b];
    probe_spin(0.2);
    probe_line("671.step", PF(@"%@ ; 1 property NO (before)", name), state(b, w));
    [b setShowsBorderOnlyWhileMouseInside:YES];
    probe_line("671.step", PF(@"%@ ; 2 right after setting YES", name), state(b, w));
    probe_spin(0.5);
    probe_line("671.step", PF(@"%@ ; 3 after 0.5 s", name), state(b, w));
    NSRect f = [w frame]; f.size.width += 100; f.size.height += 40;
    [w setFrame:f display:YES];
    [b setFrame:NSMakeRect(20, 20, 140, 40)];
    [cv setFrame:NSMakeRect(0, 0, f.size.width, f.size.height)];
    probe_spin(0.3);
    NSButton *ref2 = makeButton(NSBezelStyleRounded, YES, NO); [ref2 setFrame:[b frame]]; [cv addSubview:ref2]; NSData *b2 = snapData(ref2); [ref2 removeFromSuperview];
    NSButton *ref3 = makeButton(NSBezelStyleRounded, NO, NO); [ref3 setFrame:[b frame]]; [cv addSubview:ref3]; NSData *p2 = snapData(ref3); [ref3 removeFromSuperview];
    gBordered = b2; gPlain = p2;
    probe_line("671.step", PF(@"%@ ; 4 after the window and the button grew (references remade at the new size)", name), state(b, w));
    [b updateTrackingAreas];
    probe_line("671.step", PF(@"%@ ; 5 after -updateTrackingAreas", name), state(b, w));
    NSPoint centre = [b convertPoint:NSMakePoint(NSMidX([b bounds]), NSMidY([b bounds])) toView:nil];
    [b mouseEntered:enterExit(NSEventTypeMouseEntered, w, centre)];
    probe_line("671.step", PF(@"%@ ; 6a after button mouseEntered: (synthesised)", name), state(b, w));
    [b mouseExited:enterExit(NSEventTypeMouseExited, w, NSMakePoint(-50, -50))];
    probe_line("671.step", PF(@"%@ ; 6b after button mouseExited: (synthesised)", name), state(b, w));
    [[b cell] mouseEntered:enterExit(NSEventTypeMouseEntered, w, centre)];
    probe_line("671.step", PF(@"%@ ; 6c after cell mouseEntered:", name), state(b, w));
    [[b cell] mouseExited:enterExit(NSEventTypeMouseExited, w, NSMakePoint(-50, -50))];
    probe_line("671.step", PF(@"%@ ; 6d after cell mouseExited:", name), state(b, w));
    if (warp) {
        NSPoint s = [w convertRectToScreen:NSMakeRect(centre.x, centre.y, 0, 0)].origin;
        CGFloat H = scr.size.height;
        CGPoint old = CGPointMake(m.x, H - m.y);
        CGWarpMouseCursorPosition(CGPointMake(s.x, H - s.y));
        probe_spin(0.6);
        probe_line("671.step", PF(@"%@ ; 7 pointer warped onto the button", name), state(b, w));
        CGWarpMouseCursorPosition(CGPointMake(s.x + 600, H - s.y));
        probe_spin(0.6);
        probe_line("671.step", PF(@"%@ ; 8 pointer warped away", name), state(b, w));
        CGWarpMouseCursorPosition(old);
    }
    [b setShowsBorderOnlyWhileMouseInside:NO];
    probe_line("671.step", PF(@"%@ ; 9 property set back to NO", name), state(b, w));
    [b setBordered:NO];
    [b setShowsBorderOnlyWhileMouseInside:YES];
    probe_line("671.step", PF(@"%@ ; 10 bordered=NO then property YES", name), state(b, w));
    [b removeFromSuperview];

    // other bezel styles with the property on
    NSDictionary *styles = @{@"RegularSquare": @(NSBezelStyleRegularSquare), @"TexturedRounded": @(NSBezelStyleTexturedRounded), @"Rounded": @(NSBezelStyleRounded), @"Inline": @(NSBezelStyleInline), @"ShadowlessSquare": @(NSBezelStyleShadowlessSquare)};
    for (NSString *k in [[styles allKeys] sortedArrayUsingSelector:@selector(compare:)]) {
        NSBezelStyle st = (NSBezelStyle)[styles[k] integerValue];
        NSButton *r1 = makeButton(st, YES, NO), *r2 = makeButton(st, NO, NO), *t = makeButton(st, YES, YES);
        for (NSButton *x in @[r1, r2, t]) [cv addSubview:x];
        NSData *d1 = snapData(r1), *d2 = snapData(r2), *dt = snapData(t);
        for (NSButton *x in @[r1, r2, t]) [x removeFromSuperview];
        probe_line("671.bezel", PF(@"%@ ; bezelStyle %@, flag YES, pointer not entered", name, k), PF(@"references differ=%d ; render %@", (int)![d1 isEqualToData:d2], [dt isEqualToData:d1] ? @"== bordered" : [dt isEqualToData:d2] ? @"== borderless" : @"other"));
    }
    // a cell that is not drawn by its own button
    for (NSNumber *flag in @[@NO, @YES]) {
        NSButtonCell *c = [[NSButtonCell alloc] initTextCell:@"Probe"];
        [c setBezelStyle:NSBezelStyleRounded]; [c setButtonType:NSButtonTypeMomentaryPushIn]; [c setBordered:YES];
        [c setShowsBorderOnlyWhileMouseInside:[flag boolValue]];
        CellHost *h = [[CellHost alloc] initWithFrame:NSMakeRect(0, 0, 160, 80)]; h.cell = c;
        [cv addSubview:h];
        NSData *d = snapData(h);
        [h removeFromSuperview];
        static NSData *noFlag; if (![flag boolValue]) noFlag = d;
        probe_line("671.foreign", PF(@"%@ ; standalone NSButtonCell drawn by another view, flag=%@", name, flag), [flag boolValue] ? PF(@"render %@ the flag=NO render", [d isEqualToData:noFlag] ? @"==" : @"!=") : @"reference");
    }
    [w orderOut:nil];
}

int main(int argc, char **argv) {
    @autoreleasepool {
        BOOL warp = NO;
        for (int i = 1; i < argc; i++) if (!strcmp(argv[i], "--warp")) warp = YES;
        probe_env("671");
        probe_app();
        probe_line("671.env", @"app active", PF(@"%d", (int)[NSApp isActive]));
        variant(@"away", NO, warp);
        variant(@"inside", YES, NO);
    }
    return 0;
}
