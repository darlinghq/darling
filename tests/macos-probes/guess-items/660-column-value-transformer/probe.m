// Probe 660: an NSTableColumn value binding (to an NSArrayController's arrangedObjects) with a value
// transformer that cannot reverse, a reversible one, none, one by registered name, and with the null / marker
// placeholder options. Prints what the cell shows per row, whether the cell and column are editable, whether an
// edit can start and ends in a write-back, what the transformer was asked, and exceptions.
// Window server needed (table and cell editing); window at alpha 0, never key, no activation.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSMutableArray *gLog;

@interface Upper : NSValueTransformer
@end
@implementation Upper
+ (Class)transformedValueClass { return [NSString class]; }
+ (BOOL)allowsReverseTransformation { return NO; }
- (id)transformedValue:(id)v { [gLog addObject:PF(@"fwd(%@)", v == nil ? @"nil" : v)]; return [v isKindOfClass:[NSString class]] ? [v uppercaseString] : v; }
@end

@interface Wrap : NSValueTransformer
@end
@implementation Wrap
+ (Class)transformedValueClass { return [NSString class]; }
+ (BOOL)allowsReverseTransformation { return YES; }
- (id)transformedValue:(id)v { [gLog addObject:PF(@"fwd(%@)", v == nil ? @"nil" : v)]; return [v isKindOfClass:[NSString class]] ? PF(@"<%@>", v) : v; }
- (id)reverseTransformedValue:(id)v {
    [gLog addObject:PF(@"rev(%@)", v == nil ? @"nil" : v)];
    if ([v isKindOfClass:[NSString class]] && [v length] >= 2) return [v substringWithRange:NSMakeRange(1, [v length] - 2)];
    return v;
}
@end

// Declares reverse support but returns nil: a transformer that "cannot reverse" at call time.
@interface ClaimsReverse : NSValueTransformer
@end
@implementation ClaimsReverse
+ (Class)transformedValueClass { return [NSString class]; }
+ (BOOL)allowsReverseTransformation { return YES; }
- (id)transformedValue:(id)v { [gLog addObject:PF(@"fwd(%@)", v == nil ? @"nil" : v)]; return [v isKindOfClass:[NSString class]] ? [v uppercaseString] : v; }
- (id)reverseTransformedValue:(id)v { [gLog addObject:PF(@"rev(%@)", v == nil ? @"nil" : v)]; return nil; }
@end

static NSString *D(id o) { return o == nil ? @"nil" : [o isKindOfClass:[NSNull class]] ? @"NSNull" : PF(@"'%@'", o); }

static NSWindow *gWin;
static NSArrayController *gAC;
static NSTableView *gT;

static void build(NSDictionary *opts, BOOL forceEditable) {
    gAC = [[NSArrayController alloc] init];
    [gAC setObjectClass:[NSMutableDictionary class]];
    [gAC setContent:[@[[@{@"name": @"alpha"} mutableCopy], [@{@"name": [NSNull null]} mutableCopy], [@{} mutableCopy], [@{@"name": @"gamma"} mutableCopy]] mutableCopy]];
    gWin = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 300, 200) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
    [gWin setReleasedWhenClosed:NO]; [gWin setAlphaValue:0];
    NSScrollView *sv = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200)];
    gT = [[NSTableView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200)];
    NSTableColumn *c = [[NSTableColumn alloc] initWithIdentifier:@"name"]; [c setWidth:200];
    if (forceEditable) [c setEditable:YES];
    [gT addTableColumn:c];
    [sv setDocumentView:gT]; [gWin setContentView:sv]; [gWin orderFront:nil];
    NSString *ex = probe_try(^{ [c bind:NSValueBinding toObject:gAC withKeyPath:@"arrangedObjects.name" options:opts]; });
    if (![ex isEqualToString:@"no exception"]) probe_line("660.bind", @"bind value", ex);
    [gT reloadData];
}

static NSString *rows(void) {
    NSMutableArray *a = [NSMutableArray array];
    for (NSInteger r = 0; r < 4; r++) {
        NSCell *cell = [gT preparedCellAtColumn:0 row:r];
        [a addObject:PF(@"r%ld{obj=%@ str=%@ editable=%d}", (long)r, D([cell objectValue]), D([cell stringValue]), (int)[cell isEditable])];
    }
    return [a componentsJoinedByString:@" "];
}
static NSString *model(void) {
    NSMutableArray *a = [NSMutableArray array];
    for (NSDictionary *d in [gAC content]) [a addObject:D(d[@"name"])];
    return [a componentsJoinedByString:@","];
}
static NSString *edit(NSInteger row, NSString *text) {
    [gLog removeAllObjects];
    __block NSInteger started = -2;
    __block NSString *editorText = nil;
    NSString *ex = probe_try(^{
        [gT editColumn:0 row:row withEvent:nil select:YES];
        started = [gT editedRow];
        NSText *ed = [gT currentEditor];
        editorText = [ed string];
        if (ed) { [ed setString:text]; [gWin makeFirstResponder:gWin]; }
    });
    NSInteger after = [gT editedRow];
    if (after >= 0) [gT abortEditing];
    return PF(@"[%@] editedRow after editColumn=%ld editor-initial='%@' ; editedRow after commit=%ld ; model=[%@] ; transformer calls=[%@]",
              ex, (long)started, editorText ?: @"(no editor)", (long)after, model(), [gLog componentsJoinedByString:@" "]);
}

int main(void) {
    @autoreleasepool {
        probe_env("660");
        probe_app();
        gLog = [NSMutableArray array];
        [NSValueTransformer setValueTransformer:[Upper new] forName:@"ProbeUpper"];
        struct { const char *n; NSDictionary *o; BOOL ed; } cfg[] = {
            {"no transformer", nil, NO},
            {"non-reversible transformer (allowsReverseTransformation NO)", @{NSValueTransformerBindingOption: [Upper new]}, NO},
            {"reversible transformer", @{NSValueTransformerBindingOption: [Wrap new]}, NO},
            {"transformer that says YES to reversing but returns nil from reverse", @{NSValueTransformerBindingOption: [ClaimsReverse new]}, NO},
            {"non-reversible transformer by registered name (NSValueTransformerNameBindingOption)", @{NSValueTransformerNameBindingOption: @"ProbeUpper"}, NO},
            {"non-reversible transformer and column.editable forced YES", @{NSValueTransformerBindingOption: [Upper new]}, YES},
        };
        for (int i = 0; i < 6; i++) {
            [gLog removeAllObjects];
            build(cfg[i].o, cfg[i].ed);
            [gLog removeAllObjects];
            NSString *shown = rows();
            probe_line("660.1", PF(@"%s; rows: 0='alpha' 1=NSNull 2=missing key 3='gamma'", cfg[i].n),
                       PF(@"column.isEditable=%d ; %@ ; transformer calls while displaying=[%@]", (int)[[gT tableColumns][0] isEditable], shown, [gLog componentsJoinedByString:@" "]));
            probe_line("660.2", PF(@"%s; edit row 0 to 'new'", cfg[i].n), edit(0, @"new"));
            probe_line("660.3", PF(@"%s; edit row 3 to 'x' then check the model", cfg[i].n), edit(3, @"x"));
            probe_line("660.4", PF(@"%s; edit row 1 (value NSNull) to 'n'", cfg[i].n), edit(1, @"n"));
            probe_line("660.5", PF(@"%s; edit row 2 (missing key) to 'm'", cfg[i].n), edit(2, @"m"));
            // display again after the edits
            [gT reloadData];
            probe_line("660.6", PF(@"%s; display after the edits", cfg[i].n), PF(@"%@ ; model=[%@]", rows(), model()));
        }

        // Placeholders
        NSString *kNull = NSNullPlaceholderBindingOption, *kNA = NSNotApplicablePlaceholderBindingOption, *kNS = NSNoSelectionPlaceholderBindingOption, *kMV = NSMultipleValuesPlaceholderBindingOption;
        struct { const char *n; NSDictionary *o; } ph[] = {
            {"NullPlaceholder only", @{kNull: @"NULLPH"}},
            {"all four placeholders (NULLPH, NAPH, NOSELPH, MVPH)", @{kNull: @"NULLPH", kNA: @"NAPH", kNS: @"NOSELPH", kMV: @"MVPH"}},
            {"NullPlaceholder + non-reversible transformer", @{kNull: @"NULLPH", NSValueTransformerBindingOption: [Upper new]}},
            {"NullPlaceholder + reversible transformer", @{kNull: @"NULLPH", NSValueTransformerBindingOption: [Wrap new]}},
            {"NotApplicablePlaceholder only", @{kNA: @"NAPH"}},
        };
        for (int i = 0; i < 5; i++) {
            [gLog removeAllObjects];
            build(ph[i].o, NO);
            [gLog removeAllObjects];
            NSString *shown = rows();
            probe_line("660.7", PF(@"%s; rows 0='alpha' 1=NSNull 2=missing key 3='gamma'", ph[i].n),
                       PF(@"%@ ; placeholderString(row1 cell)=%@ ; transformer calls=[%@]", shown, D([(NSTextFieldCell *)[gT preparedCellAtColumn:0 row:1] placeholderString]), [gLog componentsJoinedByString:@" "]));
            probe_line("660.8", PF(@"%s; edit row 1 (null value) to 'n' and clear row 0 to ''", ph[i].n), PF(@"%@ ; then %@", edit(1, @"n"), edit(0, @"")));
        }
        // No rows: what the empty controller shows
        build(@{kNull: @"NULLPH", kNS: @"NOSELPH", kNA: @"NAPH"}, NO);
        [gAC setContent:[NSMutableArray array]]; [gT reloadData];
        probe_line("660.9", @"empty controller content with placeholders; numberOfRows", PF(@"rows=%ld", (long)[gT numberOfRows]));
    }
    return 0;
}
