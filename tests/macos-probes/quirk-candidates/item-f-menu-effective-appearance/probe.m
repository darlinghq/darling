// Item (f): the appearance NSMenu reports through effectiveAppearance for a menu hierarchy when appearances are
// set on none, some, or all of the menus and on the application, including a submenu with no supermenu.
// Uses key-value coding for the property so the probe also builds against an SDK that predates it.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSString *an(id appearance) { return appearance ? [appearance name] : @"nil"; }
static NSString *eff(NSMenu *m) {
    if (![m respondsToSelector:NSSelectorFromString(@"effectiveAppearance")]) return @"(NSMenu does not respond to effectiveAppearance)";
    return an([m valueForKey:@"effectiveAppearance"]);
}
static NSString *own(NSMenu *m) {
    if (![m respondsToSelector:NSSelectorFromString(@"appearance")]) return @"(no appearance property)";
    return an([m valueForKey:@"appearance"]);
}
static void set(NSMenu *m, NSAppearance *a) {
    if ([m respondsToSelector:NSSelectorFromString(@"setAppearance:")]) [m setValue:a forKey:@"appearance"];
}

static void report(NSString *what, NSMenu *main, NSMenu *sub, NSMenu *subsub, NSMenu *loose) {
    probe_line("item-f", what,
               PF(@"main: appearance=%@ effective=%@ ; sub: appearance=%@ effective=%@ ; subsub: appearance=%@ effective=%@ ; detached: appearance=%@ effective=%@ ; NSApp.effectiveAppearance=%@",
                  own(main), eff(main), own(sub), eff(sub), own(subsub), eff(subsub), own(loose), eff(loose), an([NSApp effectiveAppearance])));
}

int main(void) {
    @autoreleasepool {
        probe_env("item-f");
        probe_app();
        NSAppearance *aqua = probe_appearance(NO), *dark = probe_appearance(YES);
        if (dark == nil) { probe_line("item-f", @"dark appearance", @"not available on this OS"); return 0; }
        NSMenu *main = [[NSMenu alloc] initWithTitle:@"main"], *sub = [[NSMenu alloc] initWithTitle:@"sub"],
               *subsub = [[NSMenu alloc] initWithTitle:@"subsub"], *loose = [[NSMenu alloc] initWithTitle:@"detached"];
        NSMenuItem *mi = [main addItemWithTitle:@"sub" action:NULL keyEquivalent:@""];
        [mi setSubmenu:sub];
        NSMenuItem *si = [sub addItemWithTitle:@"subsub" action:NULL keyEquivalent:@""];
        [si setSubmenu:subsub];
        probe_line("item-f.structure", @"sub.supermenu == main ; subsub.supermenu == sub ; detached.supermenu", PF(@"%d %d %@", (int)([sub supermenu] == main), (int)([subsub supermenu] == sub), [loose supermenu] ? @"non-nil" : @"nil"));

        report(@"1. nothing set, application appearance not set", main, sub, subsub, loose);
        set(main, dark);
        report(@"2. main.appearance = dark", main, sub, subsub, loose);
        set(sub, aqua);
        report(@"3. main.appearance = dark, sub.appearance = aqua", main, sub, subsub, loose);
        set(main, nil);
        report(@"4. main.appearance = nil again, sub.appearance = aqua", main, sub, subsub, loose);
        set(sub, nil);
        set(subsub, dark);
        report(@"5. only subsub.appearance = dark", main, sub, subsub, loose);
        set(subsub, nil);
        [NSApp setAppearance:dark];
        report(@"6. NSApp.appearance = dark, no menu has one", main, sub, subsub, loose);
        set(main, aqua);
        report(@"7. NSApp.appearance = dark, main.appearance = aqua", main, sub, subsub, loose);
        [NSApp setMainMenu:main];
        report(@"8. same, and main is the application's main menu", main, sub, subsub, loose);
        set(main, nil);
        [NSApp setAppearance:nil];
        report(@"9. everything cleared, main is the main menu", main, sub, subsub, loose);
        [mi setSubmenu:nil];
        set(main, dark);
        report(@"10. sub removed from main (sub keeps subsub), main.appearance = dark", main, sub, subsub, loose);
        probe_line("item-f.structure", @"after removal: sub.supermenu", [sub supermenu] ? @"non-nil" : @"nil");
    }
    return 0;
}
