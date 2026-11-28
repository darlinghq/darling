// Probe 641: NSPopUpButton content / contentObjects / contentValues bindings, and what a content change does to
// the selection (bound or not). Uses a plain KVC model object as the binding target. No window is created.
// Needs a connection to the window server only because NSPopUpButton lives in AppKit; nothing is shown.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSMutableArray *gLog; // writes the model receives

@interface PModel : NSObject
@property(nonatomic, strong) NSArray *items, *values, *objs;
@property(nonatomic, strong) id sel, val;
@property(nonatomic, strong) NSNumber *idx, *tag;
@end
@implementation PModel
- (void)setSel:(id)v { [gLog addObject:PF(@"model.sel<-%@", v)]; _sel = v; }
- (void)setVal:(id)v { [gLog addObject:PF(@"model.val<-%@", v)]; _val = v; }
- (void)setIdx:(NSNumber *)v { [gLog addObject:PF(@"model.idx<-%@", v)]; _idx = v; }
- (void)setTag:(NSNumber *)v { [gLog addObject:PF(@"model.tag<-%@", v)]; _tag = v; }
@end

static NSString *D(id o) { return o == nil ? @"nil" : PF(@"%@", o); }

static NSString *mstate(PModel *m) {
    return PF(@"model sel=%@ val=%@ idx=%@ tag=%@ ; writes=[%@]", D(m.sel), D(m.val), D(m.idx), D(m.tag),
              [gLog componentsJoinedByString:@","]);
}

static void show(const char *id, NSString *input, NSPopUpButton *p, PModel *m, NSString *exc) {
    NSMutableArray *t = [NSMutableArray array], *r = [NSMutableArray array], *tags = [NSMutableArray array];
    for (NSMenuItem *mi in [p itemArray]) {
        [t addObject:[mi title] ?: @"nil"];
        [r addObject:D([mi representedObject])];
        [tags addObject:PF(@"%ld", (long)[mi tag])];
    }
    probe_line(id, input, PF(@"[%@] n=%ld titles=[%@] reps=[%@] tags=[%@] selIndex=%ld selTitle=%@ ; %@", exc ?: @"",
                             (long)[p numberOfItems], [t componentsJoinedByString:@"|"], [r componentsJoinedByString:@"|"],
                             [tags componentsJoinedByString:@"|"], (long)[p indexOfSelectedItem],
                             D([p titleOfSelectedItem]), mstate(m)));
    [gLog removeAllObjects];
}

static NSPopUpButton *mk(void) { return [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 100, 26) pullsDown:NO]; }
static void bindTo(NSPopUpButton *p, NSString *b, PModel *m, NSString *kp) {
    NSString *r = probe_try(^{ [p bind:b toObject:m withKeyPath:kp options:nil]; });
    if (![r isEqualToString:@"no exception"]) probe_line("641.bind", PF(@"bind %@ -> %@", b, kp), r);
}
static void spin(void) { probe_spin(0.05); }

int main(void) {
    @autoreleasepool {
        probe_env("641");
        probe_app();
        gLog = [NSMutableArray array];
        PModel *m; NSPopUpButton *p; NSString *x;

        // --- Items from content, contentObjects, contentValues
        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"];
        bindTo(p, NSContentBinding, m, @"items"); spin();
        show("641.1", @"content=[a,b,c] only", p, m, nil);

        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"]; m.values = @[@"A", @"B", @"C"];
        bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSContentValuesBinding, m, @"values"); spin();
        show("641.2", @"content=[a,b,c] + contentValues=[A,B,C]", p, m, nil);

        m = [PModel new]; p = mk();
        m.objs = @[@10, @20, @30]; m.values = @[@"A", @"B", @"C"];
        bindTo(p, NSContentObjectsBinding, m, @"objs"); bindTo(p, NSContentValuesBinding, m, @"values"); spin();
        show("641.3", @"contentObjects=[10,20,30] + contentValues=[A,B,C] (content not bound)", p, m, nil);

        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"]; m.objs = @[@10, @20, @30]; m.values = @[@"A", @"B", @"C"];
        bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSContentObjectsBinding, m, @"objs");
        bindTo(p, NSContentValuesBinding, m, @"values"); spin();
        show("641.4", @"content=[a,b,c] + contentObjects=[10,20,30] + contentValues=[A,B,C]", p, m, nil);

        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"]; m.objs = @[@10, @20, @30];
        bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSContentObjectsBinding, m, @"objs"); spin();
        show("641.5", @"content=[a,b,c] + contentObjects=[10,20,30], no contentValues", p, m, nil);

        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"]; m.values = @[@"A", @"B"];
        bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSContentValuesBinding, m, @"values"); spin();
        show("641.6", @"content has 3, contentValues has 2 (length mismatch)", p, m, nil);

        // --- Equal descriptions
        m = [PModel new]; p = mk();
        m.items = @[@"x", @"x", @"y"];
        bindTo(p, NSContentBinding, m, @"items"); spin();
        show("641.7", @"content=[x,x,y] (equal strings)", p, m, nil);

        m = [PModel new]; p = mk();
        m.items = @[@1, @"1", @1];
        bindTo(p, NSContentBinding, m, @"items"); spin();
        show("641.8", @"content=[1(number),\"1\"(string),1(number)] (equal descriptions, different objects)", p, m, nil);

        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"]; m.values = @[@"T", @"T", @"U"];
        bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSContentValuesBinding, m, @"values"); spin();
        show("641.9", @"content=[a,b,c] + contentValues=[T,T,U] (equal titles)", p, m, nil);

        // --- Selection survives a content change; selectedObject bound
        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"]; m.values = @[@"A", @"B", @"C"]; m.sel = @"b";
        bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSContentValuesBinding, m, @"values");
        bindTo(p, NSSelectedObjectBinding, m, @"sel"); spin();
        show("641.10", @"selectedObject bound, model.sel=b, content=[a,b,c]: initial", p, m, nil);
        m.sel = @"c"; spin();
        show("641.11", @"model.sel set to c", p, m, nil);
        [gLog removeAllObjects];
        m.items = @[@"b", @"c", @"d"]; m.values = @[@"B", @"C", @"D"]; spin();
        show("641.12", @"content replaced by [b,c,d] (selected c moves from index 2 to 1)", p, m, nil);
        m.items = @[@"x", @"y", @"z"]; m.values = @[@"X", @"Y", @"Z"]; spin();
        show("641.13", @"content replaced by [x,y,z] (selected object no longer present)", p, m, nil);
        m.sel = @"zzz"; spin();
        show("641.14", @"model.sel set to zzz (not in content)", p, m, nil);
        m.sel = nil; spin();
        show("641.15", @"model.sel set to nil", p, m, nil);
        m.sel = @"y"; spin();
        show("641.16", @"model.sel set to y (back in content)", p, m, nil);
        [p selectItemAtIndex:2]; // does not go through the binding; then the user-style action
        [p sendAction:[p action] to:[p target]];
        show("641.17", @"selectItemAtIndex:2 then sendAction (a user pick of z)", p, m, nil);

        // selectedObject with contentObjects (what does selectedObject compare to?)
        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"]; m.objs = @[@10, @20, @30]; m.values = @[@"A", @"B", @"C"]; m.sel = @20;
        bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSContentObjectsBinding, m, @"objs");
        bindTo(p, NSContentValuesBinding, m, @"values"); bindTo(p, NSSelectedObjectBinding, m, @"sel"); spin();
        show("641.18", @"all three content bindings; selectedObject=20 (a contentObject)", p, m, nil);
        m.sel = @"b"; spin();
        show("641.19", @"same; selectedObject set to \"b\" (a content item)", p, m, nil);

        // selectedValue
        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"]; m.values = @[@"A", @"B", @"C"]; m.val = @"B";
        bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSContentValuesBinding, m, @"values");
        bindTo(p, NSSelectedValueBinding, m, @"val"); spin();
        show("641.20", @"selectedValue bound, model.val=B", p, m, nil);
        m.items = @[@"c", @"b"]; m.values = @[@"C", @"B"]; spin();
        show("641.21", @"content replaced by [c,b] values [C,B]", p, m, nil);

        // selectedIndex
        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"]; m.idx = @2;
        bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSSelectedIndexBinding, m, @"idx"); spin();
        show("641.22", @"selectedIndex bound, model.idx=2, content=[a,b,c]", p, m, nil);
        m.items = @[@"a", @"b"]; spin();
        show("641.23", @"content replaced by [a,b] (index 2 out of range)", p, m, nil);
        m.idx = @7; spin();
        show("641.24", @"model.idx set to 7", p, m, nil);

        // selectedTag
        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"]; m.tag = @1;
        bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSSelectedTagBinding, m, @"tag"); spin();
        show("641.25", @"selectedTag bound, model.tag=1, content=[a,b,c] (tags are whatever the popup assigned)", p, m, nil);

        // Several selection bindings; both bind orders
        NSArray *orders = @[@[NSSelectedObjectBinding, NSSelectedValueBinding, NSSelectedTagBinding, NSSelectedIndexBinding],
                            @[NSSelectedIndexBinding, NSSelectedTagBinding, NSSelectedValueBinding, NSSelectedObjectBinding]];
        NSDictionary *kp = @{NSSelectedObjectBinding: @"sel", NSSelectedValueBinding: @"val", NSSelectedTagBinding: @"tag", NSSelectedIndexBinding: @"idx"};
        int k = 0;
        for (NSArray *order in orders) {
            m = [PModel new]; p = mk();
            m.items = @[@"a", @"b", @"c"]; m.values = @[@"A", @"B", @"C"];
            m.sel = @"c"; m.val = @"B"; m.idx = @0; m.tag = @1;
            bindTo(p, NSContentBinding, m, @"items"); bindTo(p, NSContentValuesBinding, m, @"values");
            for (NSString *b in order) bindTo(p, b, m, kp[b]);
            spin();
            show(k ? "641.27" : "641.26", PF(@"four selection bindings with disagreeing model values (sel=c val=B idx=0 tag=1), bound in order %@", [order componentsJoinedByString:@","]), p, m, nil);
            m.items = @[@"a", @"b", @"c", @"d"]; m.values = @[@"A", @"B", @"C", @"D"]; spin();
            show(k ? "641.29" : "641.28", @"then content replaced by [a,b,c,d]", p, m, nil);
            k++;
        }

        // No selection binding: what a content change does to the selection
        m = [PModel new]; p = mk();
        m.items = @[@"a", @"b", @"c"];
        bindTo(p, NSContentBinding, m, @"items"); spin();
        [p selectItemAtIndex:2];
        show("641.30", @"no selection binding; selectItemAtIndex:2", p, m, nil);
        m.items = @[@"a", @"b", @"c", @"d"]; spin();
        show("641.31", @"content replaced by 4 items", p, m, nil);
        m.items = @[@"a"]; spin();
        show("641.32", @"content replaced by 1 item", p, m, nil);
        m.items = @[]; spin();
        show("641.33", @"content replaced by an empty array", p, m, nil);
        m.items = @[@"q", @"r"]; spin();
        show("641.34", @"content replaced by 2 items after being empty", p, m, nil);

        // Pull-down variant: the first item is the title
        m = [PModel new];
        p = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 100, 26) pullsDown:YES];
        m.items = @[@"t", @"a", @"b"];
        bindTo(p, NSContentBinding, m, @"items"); spin();
        show("641.35", @"pull-down popup, content=[t,a,b]", p, m, nil);

        // Content with nil/NSNull/dictionary elements
        m = [PModel new]; p = mk();
        m.items = @[@"a", [NSNull null], @{@"k": @"v"}];
        x = probe_try(^{ bindTo(p, NSContentBinding, m, @"items"); spin(); });
        show("641.36", @"content=[a,NSNull,{k=v}]", p, m, x);
    }
    return 0;
}
