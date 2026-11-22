// Probe 674: NSWindow minSize, maxSize, contentMinSize, contentMaxSize.
//   Part 1: defaults of the four properties and of the increments for several window kinds.
//   Part 2: the interplay: setting one property and reading the other three; setting both (which wins, in either
//           order); min larger than max; zero, negative and huge values; changing the style mask afterwards.
//   Part 3: what clamps and when: -setFrame:display:, -setContentSize:, -setFrameOrigin: (move), -setFrameTopLeftPoint:,
//           limits that are set after the window is already outside them, a limit of 0 in one dimension only.
// Run twice by the probe itself: with windows never ordered in, and with windows ordered front at alpha 0
// (never activated, placed inside the main screen so the window server does not move them).
// Window server: yes.
#define PROBE_APPKIT 1
#import "probe_common.h"
#include <float.h>

static NSString *num(CGFloat v) { return v >= FLT_MAX * 0.99 ? (v >= CGFLOAT_MAX * 0.99 ? @"CGFLOAT_MAX" : PF(@"%g(FLT_MAX-ish)", (double)v)) : PF(@"%g", (double)v); }
static NSString *sz(NSSize s) { return PF(@"{%@, %@}", num(s.width), num(s.height)); }
static NSString *four(NSWindow *w) { return PF(@"minSize=%@ maxSize=%@ contentMinSize=%@ contentMaxSize=%@", sz([w minSize]), sz([w maxSize]), sz([w contentMinSize]), sz([w contentMaxSize])); }
static NSString *fr(NSWindow *w) { return PF(@"frame=%@ content=%@", probe_rect([w frame]), probe_size([[w contentView] frame].size)); }

static NSWindow *mk(NSWindowStyleMask style, BOOL show) {
    NSWindow *w = [[NSWindow alloc] initWithContentRect:NSMakeRect(200, 200, 400, 300) styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [w setReleasedWhenClosed:NO];
    if (show) { [w setAlphaValue:0]; [w orderFrontRegardless]; probe_spin(0.1); }
    return w;
}

static void run(NSString *pass, BOOL show) {
    const char *P = "674";
    NSWindowStyleMask T = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
    // Part 1
    NSDictionary *kinds = @{@"1 titled+resizable": @(T), @"2 titled, not resizable": @(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable), @"3 borderless": @(NSWindowStyleMaskBorderless),
                            @"4 titled+resizable+fullSizeContentView": @(T | NSWindowStyleMaskFullSizeContentView), @"5 titled+resizable+miniaturizable": @(T | NSWindowStyleMaskMiniaturizable)};
    for (NSString *k in [[kinds allKeys] sortedArrayUsingSelector:@selector(compare:)]) {
        NSWindow *w = mk((NSWindowStyleMask)[kinds[k] unsignedIntegerValue], show);
        probe_line(P, PF(@"%@ ; part 1 defaults ; %@", pass, k), PF(@"%@ ; resizeIncrements=%@ contentResizeIncrements=%@ aspectRatio=%@ contentAspectRatio=%@ ; %@ ; frame-content height diff=%g",
            four(w), sz([w resizeIncrements]), sz([w contentResizeIncrements]), sz([w aspectRatio]), sz([w contentAspectRatio]), fr(w), (double)([w frame].size.height - [[w contentView] frame].size.height)));
        [w close];
    }
    NSPanel *pn = [[NSPanel alloc] initWithContentRect:NSMakeRect(200, 200, 400, 300) styleMask:T | NSWindowStyleMaskUtilityWindow backing:NSBackingStoreBuffered defer:NO];
    [pn setReleasedWhenClosed:NO];
    probe_line(P, PF(@"%@ ; part 1 defaults ; 6 utility panel", pass), four(pn));
    [pn close];

    // Part 2
    NSWindow *w = mk(T, show);
    NSSize delta = NSMakeSize([w frame].size.width - [[w contentView] frame].size.width, [w frame].size.height - [[w contentView] frame].size.height);
    probe_line(P, PF(@"%@ ; part 2 ; frame minus content size", pass), sz(delta));
    [w setMinSize:NSMakeSize(300, 200)];
    probe_line(P, PF(@"%@ ; part 2 ; setMinSize 300x200", pass), four(w));
    [w setContentMinSize:NSMakeSize(100, 100)];
    probe_line(P, PF(@"%@ ; part 2 ; then setContentMinSize 100x100", pass), four(w));
    [w setContentMinSize:NSMakeSize(310, 210)];
    probe_line(P, PF(@"%@ ; part 2 ; then setContentMinSize 310x210", pass), four(w));
    [w setMinSize:NSMakeSize(50, 50)];
    probe_line(P, PF(@"%@ ; part 2 ; then setMinSize 50x50", pass), four(w));
    [w setMinSize:NSMakeSize(0, 0)];
    [w setContentMinSize:NSMakeSize(0, 0)];
    probe_line(P, PF(@"%@ ; part 2 ; both min back to 0x0", pass), four(w));
    [w setMaxSize:NSMakeSize(800, 600)];
    probe_line(P, PF(@"%@ ; part 2 ; setMaxSize 800x600", pass), four(w));
    [w setContentMaxSize:NSMakeSize(1000, 900)];
    probe_line(P, PF(@"%@ ; part 2 ; then setContentMaxSize 1000x900", pass), four(w));
    [w setMaxSize:NSMakeSize(700, 500)];
    probe_line(P, PF(@"%@ ; part 2 ; then setMaxSize 700x500", pass), four(w));
    [w setContentMaxSize:NSMakeSize(0, 0)];
    probe_line(P, PF(@"%@ ; part 2 ; then setContentMaxSize 0x0", pass), four(w));
    [w setMaxSize:NSMakeSize(0, 0)];
    probe_line(P, PF(@"%@ ; part 2 ; setMaxSize 0x0", pass), four(w));
    [w setMaxSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)];
    [w setContentMaxSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)];
    probe_line(P, PF(@"%@ ; part 2 ; both max set to CGFLOAT_MAX", pass), four(w));
    [w setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];
    probe_line(P, PF(@"%@ ; part 2 ; setMaxSize FLT_MAX", pass), four(w));
    [w setMinSize:NSMakeSize(-5, -5)];
    probe_line(P, PF(@"%@ ; part 2 ; setMinSize -5x-5", pass), four(w));
    [w setMinSize:NSMakeSize(500, 500)]; [w setMaxSize:NSMakeSize(300, 300)];
    probe_line(P, PF(@"%@ ; part 2 ; min 500x500 > max 300x300", pass), PF(@"%@ ; %@", four(w), fr(w)));
    [w setFrame:NSMakeRect(200, 200, 400, 400) display:NO];
    probe_line(P, PF(@"%@ ; part 2 ; ... then setFrame 400x400", pass), fr(w));
    [w setMaxSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)]; [w setMinSize:NSMakeSize(0, 0)];
    [w setContentMinSize:NSMakeSize(250, 0)];
    probe_line(P, PF(@"%@ ; part 2 ; contentMinSize 250x0 (one dimension)", pass), four(w));
    [w setFrame:NSMakeRect(200, 200, 100, 100) display:NO];
    probe_line(P, PF(@"%@ ; part 2 ; ... setFrame 100x100", pass), fr(w));
    [w setContentMinSize:NSMakeSize(0, 0)];
    [w setContentMinSize:NSMakeSize(300, 200)];
    NSWindowStyleMask before = [w styleMask];
    [w setStyleMask:NSWindowStyleMaskBorderless];
    probe_line(P, PF(@"%@ ; part 2 ; contentMin 300x200 then style mask -> borderless", pass), PF(@"%@ ; %@", four(w), fr(w)));
    [w setStyleMask:before];
    probe_line(P, PF(@"%@ ; part 2 ; ... style mask restored", pass), PF(@"%@ ; %@", four(w), fr(w)));
    [w close];

    // Part 3
    for (int pass3 = 0; pass3 < 2; pass3++) {
        BOOL content = pass3 == 1;
        NSString *lab = content ? @"content limits" : @"frame limits";
        NSWindow *x = mk(T, show);
        if (content) { [x setContentMinSize:NSMakeSize(300, 200)]; [x setContentMaxSize:NSMakeSize(600, 400)]; }
        else { [x setMinSize:NSMakeSize(300, 200)]; [x setMaxSize:NSMakeSize(600, 400)]; }
        probe_line(P, PF(@"%@ ; part 3 (%@) ; limits", pass, lab), PF(@"%@ ; %@", four(x), fr(x)));
        NSRect rects[] = {NSMakeRect(200, 200, 100, 100), NSMakeRect(200, 200, 900, 900), NSMakeRect(200, 200, 300, 200), NSMakeRect(200, 200, 600, 400), NSMakeRect(200, 200, 450, 100), NSMakeRect(200, 200, 100, 350)};
        for (int i = 0; i < 6; i++) {
            [x setFrame:rects[i] display:NO];
            probe_line(P, PF(@"%@ ; part 3 (%@) ; setFrame %@", pass, lab, probe_rect(rects[i])), fr(x));
        }
        NSSize cs[] = {NSMakeSize(50, 50), NSMakeSize(2000, 2000), NSMakeSize(400, 250)};
        for (int i = 0; i < 3; i++) {
            [x setContentSize:cs[i]];
            probe_line(P, PF(@"%@ ; part 3 (%@) ; setContentSize %@", pass, lab, probe_size(cs[i])), fr(x));
        }
        [x setFrame:NSMakeRect(200, 200, 450, 350) display:NO];
        [x setFrameOrigin:NSMakePoint(10, 20)];
        probe_line(P, PF(@"%@ ; part 3 (%@) ; setFrameOrigin on an in-range window", pass, lab), fr(x));
        [x setFrameTopLeftPoint:NSMakePoint(30, 500)];
        probe_line(P, PF(@"%@ ; part 3 (%@) ; setFrameTopLeftPoint", pass, lab), fr(x));
        [x close];
        // limits set after the window is outside them
        NSWindow *y = mk(T, show);
        [y setFrame:NSMakeRect(200, 200, 100, 100) display:NO];
        if (content) [y setContentMinSize:NSMakeSize(300, 200)]; else [y setMinSize:NSMakeSize(300, 200)];
        probe_line(P, PF(@"%@ ; part 3 (%@) ; window 100x100, then min 300x200 set", pass, lab), fr(y));
        [y setFrameOrigin:NSMakePoint(40, 40)];
        probe_line(P, PF(@"%@ ; part 3 (%@) ; ... then moved with setFrameOrigin", pass, lab), fr(y));
        [y setFrame:NSMakeRect(200, 200, 1000, 1000) display:NO];
        if (content) [y setContentMaxSize:NSMakeSize(600, 400)]; else [y setMaxSize:NSMakeSize(600, 400)];
        probe_line(P, PF(@"%@ ; part 3 (%@) ; window 1000x1000, then max 600x400 set", pass, lab), fr(y));
        [y setFrameOrigin:NSMakePoint(60, 60)];
        probe_line(P, PF(@"%@ ; part 3 (%@) ; ... then moved with setFrameOrigin", pass, lab), fr(y));
        [y setFrame:NSMakeRect(200, 200, 1000, 1000) display:NO];
        probe_line(P, PF(@"%@ ; part 3 (%@) ; ... then setFrame 1000x1000 again", pass, lab), fr(y));
        [y close];
        // zoom and constrain as extra observations
        NSWindow *z = mk(T, show);
        if (content) [z setContentMaxSize:NSMakeSize(500, 300)]; else [z setMaxSize:NSMakeSize(500, 300)];
        NSRect c = [z constrainFrameRect:NSMakeRect(100, 100, 900, 900) toScreen:[NSScreen screens][0]];
        probe_line(P, PF(@"%@ ; part 3 (%@) ; max 500x300, constrainFrameRect:900x900", pass, lab), probe_rect(c));
        NSRect cf = [z frameRectForContentRect:NSMakeRect(0, 0, 100, 100)];
        probe_line(P, PF(@"%@ ; part 3 (%@) ; frameRectForContentRect 100x100", pass, lab), probe_rect(cf));
        [z close];
    }
}

int main(void) {
    @autoreleasepool {
        probe_env("674");
        probe_app();
        run(@"windows not ordered in", NO);
        run(@"windows ordered front at alpha 0", YES);
    }
    return 0;
}
