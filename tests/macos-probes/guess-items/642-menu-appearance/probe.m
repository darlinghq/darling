// Probe 642: NSMenu appearance / effectiveAppearance. Extends probe item-f (which prints the inheritance chain
// main -> sub -> subsub, a detached menu, and the application's appearance). This probe adds:
//   A. copies: does -copy keep the appearance (own setting), also for submenus of the copy?
//   B. identity: is effectiveAppearance the very object that was set?
//   C. other sources: a pop-up button menu inside a window or button that has its own appearance, a menu that
//      is a view's contextual menu, a menu given vibrant and non-dark/non-aqua appearances.
//   D. a menu whose supermenu has an appearance but whose own setting was cleared after the copy.
// Uses key-value coding for the property so the probe also builds against an SDK that predates it.
// Window server: yes (one window at alpha 0, never ordered in, the application is never activated).
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSString *an(id a) { return a ? [a name] : @"nil"; }
static BOOL has(NSMenu *m) { return [m respondsToSelector:NSSelectorFromString(@"effectiveAppearance")]; }
static id effObj(NSMenu *m) { return has(m) ? [m valueForKey:@"effectiveAppearance"] : nil; }
static id ownObj(NSMenu *m) { return [m respondsToSelector:NSSelectorFromString(@"appearance")] ? [m valueForKey:@"appearance"] : nil; }
static NSString *eff(NSMenu *m) { return has(m) ? an(effObj(m)) : @"(no effectiveAppearance)"; }
static NSString *own(NSMenu *m) { return has(m) ? an(ownObj(m)) : @"(no appearance)"; }
static void set(NSMenu *m, NSAppearance *a) { if ([m respondsToSelector:NSSelectorFromString(@"setAppearance:")]) [m setValue:a forKey:@"appearance"]; }
static NSString *pair(NSMenu *m) { return PF(@"appearance=%@ effective=%@", own(m), eff(m)); }

int main(void) {
    @autoreleasepool {
        probe_env("642");
        probe_app();
        NSAppearance *aqua = probe_appearance(NO), *dark = probe_appearance(YES);
        NSAppearance *vdark = [NSAppearance appearanceNamed:@"NSAppearanceNameVibrantDark"];
        NSAppearance *vlight = [NSAppearance appearanceNamed:@"NSAppearanceNameVibrantLight"];
        probe_line("642.env", @"NSMenu responds to appearance / setAppearance: / effectiveAppearance",
                   PF(@"%d / %d / %d", (int)[NSMenu instancesRespondToSelector:NSSelectorFromString(@"appearance")],
                      (int)[NSMenu instancesRespondToSelector:NSSelectorFromString(@"setAppearance:")],
                      (int)[NSMenu instancesRespondToSelector:NSSelectorFromString(@"effectiveAppearance")]));
        if (dark == nil || !has([[NSMenu alloc] init])) { probe_line("642", @"prerequisites", @"dark appearance or NSMenu.effectiveAppearance not available on this OS"); return 0; }

        // A. copies
        NSMenu *main = [[NSMenu alloc] initWithTitle:@"main"], *sub = [[NSMenu alloc] initWithTitle:@"sub"];
        NSMenuItem *mi = [main addItemWithTitle:@"sub" action:NULL keyEquivalent:@""];
        [mi setSubmenu:sub];
        [sub addItemWithTitle:@"leaf" action:NULL keyEquivalent:@""];
        set(main, dark);
        NSMenu *c1 = [main copy];
        NSMenu *c1sub = [[c1 itemAtIndex:0] submenu];
        probe_line("642.A", @"main.appearance=dark ; copy of main", pair(c1));
        probe_line("642.A", @"... the copy's submenu (own appearance never set on sub)", c1sub ? pair(c1sub) : @"nil");
        probe_line("642.A", @"... copy.supermenu / copy's submenu.supermenu == copy", PF(@"%@ / %d", [c1 supermenu] ? @"non-nil" : @"nil", (int)([c1sub supermenu] == c1)));
        set(sub, aqua);
        NSMenu *c2 = [main copy];
        probe_line("642.A", @"main.appearance=dark, sub.appearance=aqua ; copy of main: copy", pair(c2));
        probe_line("642.A", @"... copy's submenu", pair([[c2 itemAtIndex:0] submenu]));
        NSMenu *c3 = [sub copy];
        probe_line("642.A", @"copy of sub alone (sub.appearance=aqua, supermenu=main with dark)", pair(c3));
        probe_line("642.A", @"... c3.supermenu", [c3 supermenu] ? @"non-nil" : @"nil");
        set(main, nil);
        set(sub, nil);
        NSMenu *c4 = [main copy];
        probe_line("642.A", @"nothing set ; copy of main", pair(c4));
        NSMenu *c5 = [[NSMenu alloc] initWithTitle:@"c5"];
        set(c5, vdark);
        probe_line("642.A", @"c5.appearance=vibrantDark ; copy", pair([c5 copy]));
        NSMenu *mc = [c5 mutableCopy];
        probe_line("642.A", @"c5.appearance=vibrantDark ; mutableCopy", pair(mc));
        set(c5, dark);
        probe_line("642.A", @"after c5.appearance=dark, the earlier copy (c1) still", pair(c1));

        // B. identity
        NSMenu *m = [[NSMenu alloc] initWithTitle:@"m"];
        set(m, dark);
        probe_line("642.B", @"m.appearance=dark: appearance == dark / effective == dark / effective == appearance",
                   PF(@"%d / %d / %d", (int)(ownObj(m) == dark), (int)(effObj(m) == dark), (int)(effObj(m) == ownObj(m))));
        set(m, nil);
        probe_line("642.B", @"m.appearance=nil: effective == NSApp.effectiveAppearance (object identity)", PF(@"%d", (int)(effObj(m) == [NSApp effectiveAppearance])));
        for (NSAppearance *a in @[aqua, dark, vlight ?: (id)[NSNull null], vdark ?: (id)[NSNull null]]) {
            if ((id)a == [NSNull null]) continue;
            set(m, a);
            probe_line("642.B", PF(@"m.appearance=%@", an(a)), pair(m));
        }
        set(m, nil);

        // C. other sources
        NSWindow *w = probe_window(NSMakeRect(-20000, -20000, 200, 100), NO);
        [w setAlphaValue:0];
        NSPopUpButton *pb = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(10, 10, 120, 26) pullsDown:NO];
        [pb addItemsWithTitles:@[@"one", @"two"]];
        [[w contentView] addSubview:pb];
        NSMenu *pm = [pb menu];
        probe_line("642.C", @"pop-up button menu, nothing set anywhere", PF(@"%@ ; pb.effectiveAppearance=%@ ; window.effectiveAppearance=%@", pair(pm), an([pb effectiveAppearance]), an([w effectiveAppearance])));
        [w setAppearance:dark];
        probe_line("642.C", @"window.appearance=dark (button's window), menu has none", PF(@"%@ ; pb.effectiveAppearance=%@", pair(pm), an([pb effectiveAppearance])));
        [w setAppearance:nil];
        [pb setAppearance:dark];
        probe_line("642.C", @"popUpButton.appearance=dark, window and application have none", PF(@"%@ ; pb.effectiveAppearance=%@", pair(pm), an([pb effectiveAppearance])));
        [pb setAppearance:nil];
        [NSApp setAppearance:dark];
        probe_line("642.C", @"NSApp.appearance=dark, button window none, menu none", PF(@"%@ ; pb.effectiveAppearance=%@", pair(pm), an([pb effectiveAppearance])));
        [w setAppearance:aqua];
        probe_line("642.C", @"NSApp.appearance=dark, window.appearance=aqua, menu none", PF(@"%@ ; pb.effectiveAppearance=%@", pair(pm), an([pb effectiveAppearance])));
        set(pm, dark);
        [NSApp setAppearance:nil];
        [w setAppearance:nil];
        probe_line("642.C", @"menu.appearance=dark, everything else none", pair(pm));
        set(pm, nil);
        NSMenu *ctx = [[NSMenu alloc] initWithTitle:@"ctx"];
        [ctx addItemWithTitle:@"x" action:NULL keyEquivalent:@""];
        [[w contentView] setMenu:ctx];
        [w setAppearance:dark];
        probe_line("642.C", @"contextual menu of the content view, window.appearance=dark", pair(ctx));
        [w setAppearance:nil];
        [[w contentView] setMenu:nil];
        probe_line("642.C", @"... after setMenu:nil", pair(ctx));

        // D. clearing own appearance on a copy leaves the original unchanged
        NSMenu *o = [[NSMenu alloc] initWithTitle:@"o"];
        set(o, dark);
        NSMenu *oc = [o copy];
        set(oc, nil);
        probe_line("642.D", @"o.appearance=dark ; copy cleared", PF(@"original: %@ ; copy: %@", pair(o), pair(oc)));
        [NSApp setAppearance:nil];
    }
    return 0;
}
