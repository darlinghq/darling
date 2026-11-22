// Probe 0024: (1) what NSArrayController answers for -selectedObjects when the selection indexes name rows the
// content does not have, and (2) where key-value-binding failures surface: an exception from -bind:, from a
// change pushed model-to-view, or from a write-back view-to-model. Each step catches and prints exceptions.
#define PROBE_APPKIT 1
#import "probe_common.h"

@interface Model : NSObject
@property(strong) NSIndexSet *sel;
@property(strong) NSString *text;
@property(strong) NSNumber *flag;
@end
@implementation Model
@end

static NSString *idx(NSIndexSet *s) {
    if (s == nil) return @"nil";
    NSMutableArray *parts = [NSMutableArray array];
    [s enumerateRangesUsingBlock:^(NSRange r, BOOL *stop) { [parts addObject:PF(@"%lu+%lu", (unsigned long)r.location, (unsigned long)r.length)]; }];
    return PF(@"{%@}", [parts componentsJoinedByString:@","]);
}

static void show(const char *id, NSString *input, NSArrayController *ac, NSString *extra) {
    __block NSString *objs = nil, *si = nil;
    NSString *r1 = probe_try(^{ objs = PF(@"%@", [ac selectedObjects]); });
    NSString *r2 = probe_try(^{ si = idx([ac selectionIndexes]); });
    probe_line(id, input, PF(@"%@selectedObjects=%@ [%@] ; selectionIndexes=%@ [%@]", extra ?: @"", objs ?: @"-", r1, si ?: @"-", r2));
}

int main(void) {
    @autoreleasepool {
        probe_env("0024");
        probe_app();
        NSArray *three = @[@"a", @"b", @"c"];

        NSArrayController *ac = [[NSArrayController alloc] init];
        [ac setContent:[three mutableCopy]];
        __block BOOL accepted = NO;
        NSString *r = probe_try(^{ accepted = [ac setSelectionIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(1, 5)]]; });
        show("0024.1", @"content [a,b,c]; setSelectionIndexes:{1..5}", ac, PF(@"setSelectionIndexes returned %d [%@] ; ", accepted, r));

        ac = [[NSArrayController alloc] init];
        [ac setContent:[three mutableCopy]];
        r = probe_try(^{ [ac setSelectionIndex:7]; });
        show("0024.2", @"content [a,b,c]; setSelectionIndex:7", ac, PF(@"[%@] ", r));

        ac = [[NSArrayController alloc] init];
        [ac setContent:[@[@"a", @"b", @"c", @"d", @"e"] mutableCopy]];
        [ac setSelectionIndexes:[NSIndexSet indexSetWithIndex:4]];
        [ac setContent:[@[@"a", @"b"] mutableCopy]];
        show("0024.3", @"content of 5, row 4 selected, then the content is replaced by 2 rows", ac, nil);

        ac = [[NSArrayController alloc] init];
        [ac setPreservesSelection:NO];
        [ac setContent:[@[@"a", @"b", @"c", @"d", @"e"] mutableCopy]];
        [ac setSelectionIndexes:[NSIndexSet indexSetWithIndex:4]];
        [ac setContent:[@[@"a", @"b"] mutableCopy]];
        show("0024.3b", @"same, with preservesSelection NO", ac, nil);

        // A selection bound to a model before the content arrives.
        Model *m = [[Model alloc] init];
        m.sel = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(1, 5)];
        ac = [[NSArrayController alloc] init];
        r = probe_try(^{ [ac bind:NSSelectionIndexesBinding toObject:m withKeyPath:@"sel" options:nil]; });
        probe_line("0024.4a", @"bind selectionIndexes to model.sel={1..5} while the content is empty", PF(@"bind: %@", r));
        show("0024.4b", @"  ... content is still empty", ac, nil);
        r = probe_try(^{ [ac setContent:[three mutableCopy]]; });
        show("0024.4c", @"  ... then content [a,b,c] arrives", ac, PF(@"setContent: %@ ; ", r));
        m.sel = [NSIndexSet indexSetWithIndex:9];
        show("0024.4d", @"  ... then model.sel is set to {9}", ac, nil);
        [ac unbind:NSSelectionIndexesBinding];

        // Binding failures.
        NSTextField *tf = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 100, 22)];
        NSWindow *w = probe_window(NSMakeRect(0, 0, 200, 100), NO);
        [[w contentView] addSubview:tf];
        r = probe_try(^{ [tf bind:NSValueBinding toObject:m withKeyPath:@"missingKey" options:nil]; });
        probe_line("0024.5a", @"bind text field value to a model key path that does not exist", PF(@"bind: %@", r));
        r = probe_try(^{ [tf setStringValue:@"x"]; });
        probe_line("0024.5b", @"  ... then -setStringValue:", r);
        r = probe_try(^{ [tf unbind:NSValueBinding]; });
        probe_line("0024.5c", @"  ... then -unbind:", r);

        NSButton *cb = [[NSButton alloc] initWithFrame:NSMakeRect(0, 30, 100, 22)];
        [cb setButtonType:NSButtonTypeSwitch];
        [[w contentView] addSubview:cb];
        r = probe_try(^{ [cb bind:NSValueBinding toObject:m withKeyPath:@"missingFlag" options:nil]; });
        probe_line("0024.6a", @"bind checkbox value to a model key path that does not exist", PF(@"bind: %@", r));
        r = probe_try(^{ [cb performClick:nil]; });
        probe_line("0024.6b", @"  ... then the checkbox is clicked (write-back from view to model)", r);
        [cb unbind:NSValueBinding];

        NSButton *cb2 = [[NSButton alloc] initWithFrame:NSMakeRect(0, 60, 100, 22)];
        [cb2 setButtonType:NSButtonTypeSwitch];
        [[w contentView] addSubview:cb2];
        [cb2 bind:NSValueBinding toObject:m withKeyPath:@"flag" options:nil];
        r = probe_try(^{ m.flag = @YES; });
        probe_line("0024.7a", @"bind checkbox to model.flag (exists), model value changed to YES", PF(@"%@ ; checkbox state %ld", r, (long)[cb2 state]));
        r = probe_try(^{ [cb2 performClick:nil]; });
        probe_line("0024.7b", @"  ... then the checkbox is clicked", PF(@"%@ ; model.flag now %@", r, m.flag));
        [cb2 unbind:NSValueBinding];

        r = probe_try(^{ [tf bind:@"noSuchBindingName" toObject:m withKeyPath:@"text" options:nil]; });
        probe_line("0024.8", @"bind a binding name the view does not have", r);

        r = probe_try(^{ [cb bind:NSValueBinding toObject:m withKeyPath:@"text" options:nil]; [m setValue:@"not a number" forKey:@"text"]; });
        probe_line("0024.9", @"bind checkbox value to a string property and change it to a non-number", r);
        [cb unbind:NSValueBinding];
    }
    return 0;
}
