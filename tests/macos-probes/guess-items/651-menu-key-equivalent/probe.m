// Probe 651: -[NSMenu performKeyEquivalent:] and the menu delegate.
// Menu tree:  main -> "File" (submenu S1: Open=cmd-o, Save=cmd-s, "More" -> S1b: Deep=cmd-d), "Edit" (submenu S2: Copy=cmd-c),
// and a top-level item "Top"=cmd-t in main. Every menu has a delegate that logs menuNeedsUpdate: and
// menuHasKeyEquivalent:forEvent:target:action: (the latter only for the delegate named in the scenario), and every
// item's target logs validateMenuItem:. A synthesised command-key event (NSEvent keyEventWithType:..., public) is
// given to -performKeyEquivalent: and, for some scenarios, to -[NSApplication sendEvent:] with the menu as the
// main menu. Prints the delegate calls in order, the validations, the result, and whether the item's action ran.
// Scenarios vary autoenablesItems of main, S1, S2, the key searched (found in the first submenu, the second, the
// top level, nested, not found), a delegate that changes an item's key equivalent inside menuNeedsUpdate:, and a
// delegate implementing menuHasKeyEquivalent:. An explicit -update call is printed as a control.
// Window server: needs a connection (no window is created); the application is never activated.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSMutableArray *events;
static void ev(NSString *s) { [events addObject:s]; }
static NSString *joined(void) { return [events count] ? [events componentsJoinedByString:@" ; "] : @"(none)"; }

@interface Rec : NSObject
- (void)act:(id)sender;
@end
@implementation Rec
- (void)act:(id)sender { ev(PF(@"ACTION(%@)", [sender title])); }
- (BOOL)validateMenuItem:(NSMenuItem *)i { ev(PF(@"validate(%@)", [i title])); return YES; }
@end

@interface Del : NSObject <NSMenuDelegate>
@property (copy) NSString *name;
@property BOOL changeKeyOnUpdate;   // first item's key equivalent becomes "x" inside menuNeedsUpdate:
@property BOOL answerKeyEquivalent; // implements-and-answers menuHasKeyEquivalent: with YES
@property (strong) Rec *rec;
@end
@implementation Del
- (void)menuNeedsUpdate:(NSMenu *)m {
    ev(PF(@"needsUpdate(%@)", self.name));
    if (self.changeKeyOnUpdate && [m numberOfItems] > 0) [[m itemAtIndex:0] setKeyEquivalent:@"x"];
}
- (BOOL)menuHasKeyEquivalent:(NSMenu *)m forEvent:(NSEvent *)e target:(id *)t action:(SEL *)a {
    ev(PF(@"hasKeyEquivalent(%@)", self.name));
    if (!self.answerKeyEquivalent) return NO;
    *t = self.rec; *a = @selector(act:);
    return YES;
}
@end

static NSEvent *keyEvent(NSString *ch, unsigned short code) {
    return [NSEvent keyEventWithType:NSEventTypeKeyDown location:NSZeroPoint modifierFlags:NSEventModifierFlagCommand timestamp:0 windowNumber:0 context:nil
                          characters:ch charactersIgnoringModifiers:ch isARepeat:NO keyCode:code];
}

static void scenario(NSString *name, BOOL aMain, BOOL a1, BOOL a2, NSString *key, BOOL viaSendEvent, BOOL changeKey, BOOL answerKey, BOOL controlUpdate) {
    Rec *rec = [[Rec alloc] init];
    NSMenu *main = [[NSMenu alloc] initWithTitle:@"main"], *s1 = [[NSMenu alloc] initWithTitle:@"S1"], *s2 = [[NSMenu alloc] initWithTitle:@"S2"], *s1b = [[NSMenu alloc] initWithTitle:@"S1b"];
    NSArray *names = @[@"main", @"S1", @"S2", @"S1b"];
    NSArray *menus = @[main, s1, s2, s1b];
    NSMutableArray *dels = [NSMutableArray array];
    for (int i = 0; i < 4; i++) {
        Del *d = [[Del alloc] init]; d.name = names[i]; d.rec = rec;
        d.changeKeyOnUpdate = changeKey && i == 1;
        d.answerKeyEquivalent = answerKey && i == 1;
        [menus[i] setDelegate:d];
        [dels addObject:d];
    }
    [main setAutoenablesItems:aMain]; [s1 setAutoenablesItems:a1]; [s2 setAutoenablesItems:a2]; [s1b setAutoenablesItems:a1];
    NSMenuItem *file = [main addItemWithTitle:@"File" action:NULL keyEquivalent:@""]; [file setSubmenu:s1];
    NSMenuItem *edit = [main addItemWithTitle:@"Edit" action:NULL keyEquivalent:@""]; [edit setSubmenu:s2];
    NSMenuItem *top = [main addItemWithTitle:@"Top" action:@selector(act:) keyEquivalent:@"t"];
    [s1 addItemWithTitle:@"Open" action:@selector(act:) keyEquivalent:@"o"];
    [s1 addItemWithTitle:@"Save" action:@selector(act:) keyEquivalent:@"s"];
    NSMenuItem *more = [s1 addItemWithTitle:@"More" action:NULL keyEquivalent:@""]; [more setSubmenu:s1b];
    [s1b addItemWithTitle:@"Deep" action:@selector(act:) keyEquivalent:@"d"];
    [s2 addItemWithTitle:@"Copy" action:@selector(act:) keyEquivalent:@"c"];
    for (NSMenu *m in menus) for (NSMenuItem *i in [m itemArray]) if ([i action]) [i setTarget:rec];
    (void)top;
    events = [NSMutableArray array];
    if (controlUpdate) { [main update]; probe_line("651.control", PF(@"%@ ; explicit [main update]", name), PF(@"events=[%@]", joined())); events = [NSMutableArray array]; }
    unsigned short code = [key isEqualToString:@"o"] ? 31 : [key isEqualToString:@"c"] ? 8 : [key isEqualToString:@"t"] ? 17 : [key isEqualToString:@"d"] ? 2 : [key isEqualToString:@"x"] ? 7 : 6;
    NSEvent *e = keyEvent(key, code);
    BOOL r = NO;
    if (viaSendEvent) { [NSApp setMainMenu:main]; [NSApp sendEvent:e]; ev(@"sendEvent-returned"); }
    else r = [main performKeyEquivalent:e];
    probe_line("651.search", PF(@"%@ ; main.autoenables=%d S1=%d S2=%d ; key cmd-%@ ; via %@", name, aMain, a1, a2, key, viaSendEvent ? @"NSApp sendEvent:" : @"main performKeyEquivalent:"),
               PF(@"%@events=[%@]", viaSendEvent ? @"" : PF(@"result=%d ; ", (int)r), joined()));
    if (viaSendEvent) [NSApp setMainMenu:nil];
}

int main(void) {
    @autoreleasepool {
        probe_env("651");
        probe_app();
        // autoenablesItems YES everywhere (the default)
        scenario(@"1 found in S1 (first submenu)", YES, YES, YES, @"o", NO, NO, NO, YES);
        scenario(@"2 found in S2 (second submenu)", YES, YES, YES, @"c", NO, NO, NO, NO);
        scenario(@"3 top-level item", YES, YES, YES, @"t", NO, NO, NO, NO);
        scenario(@"4 nested in S1b", YES, YES, YES, @"d", NO, NO, NO, NO);
        scenario(@"5 not found", YES, YES, YES, @"z", NO, NO, NO, NO);
        // main autoenablesItems NO
        scenario(@"6 main NO, found in S1", NO, YES, YES, @"o", NO, NO, NO, YES);
        scenario(@"7 main NO, found in S2", NO, YES, YES, @"c", NO, NO, NO, NO);
        scenario(@"8 main NO, not found", NO, YES, YES, @"z", NO, NO, NO, NO);
        // submenu with its own NO under a YES parent
        scenario(@"9 main YES, S1 NO, found in S1", YES, NO, YES, @"o", NO, NO, NO, YES);
        scenario(@"10 main YES, S1 NO, found in S2", YES, NO, YES, @"c", NO, NO, NO, NO);
        scenario(@"11 main YES, S2 NO, S1 YES, found in S1", YES, YES, NO, @"o", NO, NO, NO, YES);
        scenario(@"12 everything NO, found in S1", NO, NO, NO, @"o", NO, NO, NO, YES);
        scenario(@"13 everything NO, not found", NO, NO, NO, @"z", NO, NO, NO, NO);
        // a delegate that changes the key equivalent when asked to update
        scenario(@"14 S1 delegate turns Open into cmd-x on update; search cmd-x", YES, YES, YES, @"x", NO, YES, NO, NO);
        scenario(@"15 same; search cmd-o (the old key)", YES, YES, YES, @"o", NO, YES, NO, NO);
        scenario(@"16 same, everything NO; search cmd-x", NO, NO, NO, @"x", NO, YES, NO, NO);
        // delegate implementing menuHasKeyEquivalent:
        scenario(@"17 S1 delegate answers menuHasKeyEquivalent YES; search cmd-o", YES, YES, YES, @"o", NO, NO, YES, NO);
        scenario(@"18 same; search cmd-c (in S2)", YES, YES, YES, @"c", NO, NO, YES, NO);
        scenario(@"19 same; search cmd-z (nowhere)", YES, YES, YES, @"z", NO, NO, YES, NO);
        // the application's path
        scenario(@"20 found in S2", YES, YES, YES, @"c", YES, NO, NO, NO);
        scenario(@"21 not found", YES, YES, YES, @"z", YES, NO, NO, NO);
        scenario(@"22 main NO, found in S1", NO, YES, YES, @"o", YES, NO, NO, NO);
    }
    return 0;
}
