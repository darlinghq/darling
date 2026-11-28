// Probe 0044: the layout NSAlert produces for a set of configurations (window size, title bar, icon, text, accessory,
// suppression checkbox, buttons and their key equivalents), plus the legacy return-code constants and the
// application icon identity. Nothing is shown on screen: -layout is public and builds the panel.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSString *kind(NSView *v) {
    if ([v isKindOfClass:[NSButton class]]) return @"NSButton";
    if ([v isKindOfClass:[NSTextField class]]) return @"NSTextField";
    if ([v isKindOfClass:[NSImageView class]]) return @"NSImageView";
    if ([v isKindOfClass:[NSControl class]]) return @"NSControl";
    return @"NSView";
}

static void dump(const char *id, NSString *input, NSAlert *alert) {
    [alert layout];
    NSWindow *w = [alert window];
    probe_line(id, PF(@"%@ ; window", input),
               PF(@"frame size %@ ; contentView size %@ ; titled=%d ; styleMask=0x%lx ; isSheet=%d ; level=%ld",
                  probe_size([w frame].size), probe_size([[w contentView] frame].size),
                  (int)(([w styleMask] & NSWindowStyleMaskTitled) != 0), (unsigned long)[w styleMask], (int)[w isSheet], (long)[w level]));
    NSImage *icon = [alert icon];
    probe_line(id, PF(@"%@ ; icon", input), icon ? PF(@"size %@", probe_size([icon size])) : @"nil");
    NSArray *buttons = [alert buttons];
    for (NSUInteger i = 0; i < [buttons count]; i++) {
        NSButton *b = buttons[i];
        probe_line(id, PF(@"%@ ; button %lu", input, (unsigned long)i),
                   PF(@"title=\"%@\" frame=%@ keyEquivalent=\"%@\" modifierMask=0x%lx tag=%ld bezel=%ld font=%@",
                      [b title], probe_rect([b frame]),
                      [[b keyEquivalent] isEqualToString:@"\r"] ? @"\\r" : [[b keyEquivalent] isEqualToString:@"\033"] ? @"\\e" : [b keyEquivalent],
                      (unsigned long)[b keyEquivalentModifierMask], (long)[b tag], (long)[b bezelStyle], probe_font([b font])));
    }
    for (NSView *v in [[w contentView] subviews]) {
        NSString *extra = @"";
        if ([v isKindOfClass:[NSTextField class]])
            extra = PF(@" string=\"%@\" font=%@ selectable=%d wraps=%d", [(NSTextField *)v stringValue], probe_font([(NSTextField *)v font]),
                       (int)[(NSTextField *)v isSelectable], (int)[[(NSTextField *)v cell] wraps]);
        if ([v isKindOfClass:[NSButton class]] && ![buttons containsObject:v])
            extra = PF(@" title=\"%@\" type-switch=%d", [(NSButton *)v title], (int)([(NSButton *)v allowsMixedState] == NO));
        probe_line(id, PF(@"%@ ; subview of contentView", input), PF(@"%@ frame=%@%@", kind(v), probe_rect([v frame]), extra));
    }
}

int main(void) {
    @autoreleasepool {
        probe_env("0044");
        probe_app();
        NSString *longText = @"This is a longer informative text that has to wrap over several lines inside the panel so the height depends on the wrapping width of the text column.";

        NSAlert *a = [[NSAlert alloc] init];
        dump("0044.A", @"default NSAlert, no text set", a);

        a = [[NSAlert alloc] init];
        [a setMessageText:@"Message"];
        [a addButtonWithTitle:@"OK"];
        dump("0044.B", @"message \"Message\", one button OK", a);

        a = [[NSAlert alloc] init];
        [a setMessageText:@"Do you want to save the changes?"];
        [a setInformativeText:longText];
        [a addButtonWithTitle:@"Save"];
        [a addButtonWithTitle:@"Cancel"];
        [a addButtonWithTitle:@"Don’t Save"];
        dump("0044.C", @"message, long informative text, buttons Save, Cancel, Don’t Save", a);

        a = [[NSAlert alloc] init];
        [a setMessageText:@"Message"];
        [a setInformativeText:@"Info"];
        [a setShowsSuppressionButton:YES];
        [a addButtonWithTitle:@"OK"];
        dump("0044.D", @"message, short info, suppression checkbox, OK", a);

        a = [[NSAlert alloc] init];
        [a setMessageText:@"Message"];
        NSView *acc = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 240, 60)];
        [a setAccessoryView:acc];
        [a addButtonWithTitle:@"OK"];
        dump("0044.E", @"message, accessory view 240x60, OK", a);

        a = [[NSAlert alloc] init];
        [a setMessageText:@"Message"];
        [a addButtonWithTitle:@"A button with a rather long title that does not fit"];
        [a addButtonWithTitle:@"Another long title for the second button"];
        [a addButtonWithTitle:@"Third"];
        dump("0044.F", @"three buttons with long titles", a);

        a = [[NSAlert alloc] init];
        [a setMessageText:@"Message"];
        [a addButtonWithTitle:@"OK"];
        [a setShowsHelp:YES];
        dump("0044.G", @"message, OK, showsHelp YES", a);

        NSAlertStyle styles[] = {NSAlertStyleWarning, NSAlertStyleInformational, NSAlertStyleCritical};
        const char *names[] = {"warning", "informational", "critical"};
        for (int i = 0; i < 3; i++) {
            a = [[NSAlert alloc] init];
            [a setAlertStyle:styles[i]];
            [a setMessageText:@"Message"];
            [a addButtonWithTitle:@"OK"];
            [a layout];
            NSImage *icon = [a icon];
            probe_line("0044.style", PF(@"alertStyle %s", names[i]),
                       PF(@"icon size %@ ; icon is the application icon image: %d ; window titled=%d", icon ? probe_size([icon size]) : @"nil",
                          (int)(icon != nil && icon == [NSApp applicationIconImage]), (int)(([[a window] styleMask] & NSWindowStyleMaskTitled) != 0)));
        }

        probe_line("0044.constants", @"NSAlertFirstButtonReturn/Second/Third", PF(@"%d %d %d", (int)NSAlertFirstButtonReturn, (int)NSAlertSecondButtonReturn, (int)NSAlertThirdButtonReturn));
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        probe_line("0044.constants", @"NSAlertDefaultReturn/Alternate/Other/Error", PF(@"%d %d %d %d", (int)NSAlertDefaultReturn, (int)NSAlertAlternateReturn, (int)NSAlertOtherReturn, (int)NSAlertErrorReturn));
#pragma clang diagnostic pop
        NSImage *byName = [NSImage imageNamed:NSImageNameApplicationIcon];
        NSImage *appIcon = [NSApp applicationIconImage];
        probe_line("0044.appicon", @"a process with no icon of its own", PF(@"imageNamed:NSImageNameApplicationIcon size %@ ; applicationIconImage size %@ ; same object: %d ; equal: %d",
                                                                         byName ? probe_size([byName size]) : @"nil", appIcon ? probe_size([appIcon size]) : @"nil",
                                                                         (int)(byName == appIcon), (int)[byName isEqual:appIcon]));
    }
    return 0;
}
