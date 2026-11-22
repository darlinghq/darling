// Item (b): which ranges the standard text find (NSTextView / NSTextFinder) selects, one after another, for a fixed
// text, for each matching type (contains, starts with, full word, ends with), case sensitivity and a set of needles
// that differ only by case, diacritics, width forms and word boundaries. Public API only: the find pasteboard
// (NSPasteboardNameFind, NSPasteboardTypeFindPanelSearchOptions, NSTextFinderCaseInsensitiveKey,
// NSTextFinderMatchingTypeKey), -performFindPanelAction: and -performTextFinderAction:.
// NOTE: the patch behind this item was not available to the author of this probe; the axes below are the ones the
// public documentation names. Add axes once the question in the patch is known.
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSString *const kCorpus = @"Foo foo FOO fooBar barfoo foo.bar naïve naive naïve résumé resume ＦＯＯ foo-bar foo_bar foo123 foo";

static void setFind(NSString *needle, BOOL caseInsensitive, NSInteger matching) {
    NSPasteboard *pb = [NSPasteboard pasteboardWithName:NSPasteboardNameFind];
    [pb clearContents];
    [pb setString:needle forType:NSPasteboardTypeString];
    [pb setPropertyList:@{NSTextFinderCaseInsensitiveKey: @(caseInsensitive), NSTextFinderMatchingTypeKey: @(matching)}
                forType:NSPasteboardTypeFindPanelSearchOptions];
}

// Press "find next" up to 16 times from the start of the text and list the selected ranges until the search
// wraps to the first one or stops finding anything.
static NSString *walk(NSTextView *tv, int bar) {
    [tv setSelectedRange:NSMakeRange(0, 0)];
    NSMutableArray *found = [NSMutableArray array];
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"" action:NULL keyEquivalent:@""];
    NSRange first = NSMakeRange(NSNotFound, 0);
    for (int i = 0; i < 16; i++) {
        NSRange before = [tv selectedRange];
        NSString *r = probe_try(^{
            if (bar) { [item setTag:NSTextFinderActionNextMatch]; [tv performTextFinderAction:item]; }
            else { [item setTag:NSFindPanelActionNext]; [tv performFindPanelAction:item]; }
        });
        if (![r isEqualToString:@"no exception"]) { [found addObject:r]; break; }
        NSRange now = [tv selectedRange];
        if (NSEqualRanges(now, before)) { [found addObject:i == 0 ? @"(selection did not move: no match)" : @"(selection did not move)"]; break; }
        if (first.location != NSNotFound && NSEqualRanges(now, first)) { [found addObject:@"(wrapped to the first)"]; break; }
        if (first.location == NSNotFound) first = now;
        [found addObject:NSStringFromRange(now)];
    }
    return [found componentsJoinedByString:@" "];
}

int main(void) {
    @autoreleasepool {
        probe_env("item-b");
        probe_app();
        probe_line("item-b.constants", @"NSTextFinderMatchingTypeContains/StartsWith/FullWord/EndsWith", PF(@"%d %d %d %d", (int)NSTextFinderMatchingTypeContains, (int)NSTextFinderMatchingTypeStartsWith, (int)NSTextFinderMatchingTypeFullWord, (int)NSTextFinderMatchingTypeEndsWith));
        probe_line("item-b.constants", @"NSTextFinderCaseInsensitiveKey, NSTextFinderMatchingTypeKey", PF(@"\"%@\" \"%@\"", NSTextFinderCaseInsensitiveKey, NSTextFinderMatchingTypeKey));
        probe_line("item-b.constants", @"NSPasteboardTypeFindPanelSearchOptions", PF(@"\"%@\"", NSPasteboardTypeFindPanelSearchOptions));
        // Index the corpus so the ranges below can be read.
        NSMutableArray *words = [NSMutableArray array];
        [kCorpus enumerateSubstringsInRange:NSMakeRange(0, [kCorpus length]) options:NSStringEnumerationByWords | NSStringEnumerationSubstringNotRequired usingBlock:^(NSString *w, NSRange r, NSRange er, BOOL *s) { [words addObject:NSStringFromRange(r)]; }];
        probe_line("item-b.corpus", PF(@"text of %lu UTF-16 units; word ranges by NSStringEnumerationByWords", (unsigned long)[kCorpus length]), [words componentsJoinedByString:@" "]);
        probe_line("item-b.corpus", @"the text", kCorpus);

        NSArray *needles = @[@"foo", @"FOO", @"o", @"naive", @"naïve", @"resume", @"foo.bar", @"foo bar", @"ＦＯＯ", @"foo-bar", @"bar", @""];
        NSArray *modes = @[@"Contains", @"StartsWith", @"FullWord", @"EndsWith"];
        for (int bar = 0; bar <= 1; bar++) {
            NSScrollView *sv = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 400, 200)];
            NSTextView *tv = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, 400, 200)];
            [tv setString:kCorpus];
            [tv setUsesFindBar:bar];
            [sv setDocumentView:tv];
            NSWindow *w = probe_window(NSMakeRect(0, 0, 400, 200), NO);
            [[w contentView] addSubview:sv];
            [w makeFirstResponder:tv];
            for (NSInteger m = 0; m < 4; m++)
                for (int ci = 1; ci >= 0; ci--)
                    for (NSString *n in needles) {
                        setFind(n, ci, m);
                        probe_line("item-b.find", PF(@"%s ; matching %@ ; case-insensitive %d ; needle \"%@\"", bar ? "find bar (performTextFinderAction: NextMatch)" : "classic (performFindPanelAction: Next)", modes[m], ci, n),
                                   walk(tv, bar));
                    }
        }
    }
    return 0;
}
