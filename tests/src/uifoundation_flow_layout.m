// CFLAGS: -framework Foundation -framework AppKit
// Copyright (C) 2026 Lubos Dolezel
//
// NSCollectionViewFlowLayout (UIFoundation, re-exported by AppKit): where it
// places items, the content size, the elements in a rect, when a bounds change
// invalidates it, and the drop targets and inter-item gaps it reports. The
// collection view is the document view of a scroll view in a window, as
// applications use it. The documentation gives the spacings only as minimums;
// every frame here was observed on macOS 26.7.3 and 10.14.4 (both x86_64,
// tests/collection-view.md), and the program passes unchanged on both.
//
// Prints PASS or FAIL lines, then ALL PASSED or FAILED n.
#import <AppKit/AppKit.h>
#include <stdarg.h>
#include <stdio.h>

static int failures;
static void expect(BOOL ok, const char *format, ...) __attribute__((format(printf, 2, 3)));
static void expect(BOOL ok, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    printf("%s ", ok ? "PASS" : "FAIL");
    vprintf(format, ap);
    printf("\n");
    va_end(ap);
    if (!ok)
        failures++;
}

// Item counts per section and, optionally, per-item sizes (cycled) from the flow layout delegate
@interface Source : NSObject <NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout> {
@public
    NSArray *counts;
    NSArray *sizes;
}
@end

@implementation Source
- (void) dealloc {
    [counts release];
    [sizes release];
    [super dealloc];
}
- (NSInteger) numberOfSectionsInCollectionView: (NSCollectionView *) view {
    return [counts count];
}
- (NSInteger) collectionView: (NSCollectionView *) view numberOfItemsInSection: (NSInteger) section {
    return [[counts objectAtIndex: section] integerValue];
}
- (NSCollectionViewItem *) collectionView: (NSCollectionView *) view itemForRepresentedObjectAtIndexPath: (NSIndexPath *) path {
    return [view makeItemWithIdentifier: @"item" forIndexPath: path];
}
- (NSSize) collectionView: (NSCollectionView *) view layout: (NSCollectionViewLayout *) layout
    sizeForItemAtIndexPath: (NSIndexPath *) path
{
    return [[sizes objectAtIndex: path.item % [sizes count]] sizeValue];
}
@end

@interface Item : NSCollectionViewItem
@end
@implementation Item
- (void) loadView {
    self.view = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 1, 1)] autorelease];
}
@end

static NSMutableArray *keep; // windows and data sources stay alive until the end

static NSCollectionView *makeView(NSSize size, NSCollectionViewFlowLayout *layout, NSArray *counts, NSArray *sizes)
{
    NSWindow *window = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, size.width, size.height)
                                                   styleMask: NSWindowStyleMaskBorderless
                                                     backing: NSBackingStoreBuffered
                                                       defer: NO];
    NSScrollView *scroll = [[[NSScrollView alloc] initWithFrame: NSMakeRect(0, 0, size.width, size.height)] autorelease];
    NSCollectionView *view = [[[NSCollectionView alloc] initWithFrame: NSMakeRect(0, 0, size.width, size.height)] autorelease];
    Source *source = [[[Source alloc] init] autorelease];
    source->counts = [counts retain];
    source->sizes = [sizes retain];
    view.collectionViewLayout = layout;
    [view registerClass: [Item class] forItemWithIdentifier: @"item"];
    view.dataSource = source;
    if (sizes != nil)
        view.delegate = source;
    scroll.documentView = view;
    [window.contentView addSubview: scroll];
    [keep addObject: window];
    [keep addObject: source];
    [window release];
    [view reloadData];
    [window layoutIfNeeded];
    [window display];
    [view layoutSubtreeIfNeeded];
    [layout prepareLayout];
    return view;
}

static NSCollectionViewFlowLayout *flow(CGFloat width, CGFloat height, NSEdgeInsets inset)
{
    NSCollectionViewFlowLayout *layout = [[[NSCollectionViewFlowLayout alloc] init] autorelease];
    layout.itemSize = NSMakeSize(width, height);
    layout.sectionInset = inset;
    return layout;
}

static NSValue *size(CGFloat width, CGFloat height)
{
    return [NSValue valueWithSize: NSMakeSize(width, height)];
}

// expected: {section, item, x, y, width, height} per item, in order
static void checkFrames(const char *name, NSCollectionView *view, const double (*expected)[6], int count, NSSize content)
{
    NSCollectionViewLayout *layout = view.collectionViewLayout;
    for (int i = 0; i < count; i++) {
        NSIndexPath *path = [NSIndexPath indexPathForItem: (NSInteger) expected[i][1] inSection: (NSInteger) expected[i][0]];
        NSRect want = NSMakeRect(expected[i][2], expected[i][3], expected[i][4], expected[i][5]);
        NSRect got = [layout layoutAttributesForItemAtIndexPath: path].frame;
        expect(NSEqualRects(got, want), "%s: item %ld/%ld at %s (want %s)", name, (long) path.section, (long) path.item,
               [NSStringFromRect(got) UTF8String], [NSStringFromRect(want) UTF8String]);
    }
    expect(NSEqualSizes(layout.collectionViewContentSize, content), "%s: content size %s (want %s)", name,
           [NSStringFromSize(layout.collectionViewContentSize) UTF8String], [NSStringFromSize(content) UTF8String]);
}

static void test_row_distribution(void)
{
    NSEdgeInsets none = NSEdgeInsetsZero;
    // Full rows are justified: the first item at the left, the last at the right, equal gaps between
    {
        const double e[][6] = {{0, 0, 0, 0, 80, 30}, {0, 1, 120, 0, 80, 30}, {0, 2, 0, 40, 80, 30},
            {0, 3, 120, 40, 80, 30}, {0, 4, 0, 80, 80, 30}};
        checkFrames("200 wide", makeView(NSMakeSize(200, 100), flow(80, 30, none), @[@5], nil), e, 5, NSMakeSize(200, 110));
    }
    // Items of one size: the last row keeps the columns of the full rows
    {
        const double e[][6] = {{0, 0, 0, 0, 80, 30}, {0, 1, 110, 0, 80, 30}, {0, 2, 220, 0, 80, 30},
            {0, 3, 0, 40, 80, 30}, {0, 4, 110, 40, 80, 30}};
        checkFrames("300 wide", makeView(NSMakeSize(300, 100), flow(80, 30, none), @[@5], nil), e, 5, NSMakeSize(300, 70));
    }
    {
        const double e[][6] = {{0, 0, 0, 0, 80, 30}, {0, 1, 170, 0, 80, 30}, {0, 2, 0, 40, 80, 30},
            {0, 3, 170, 40, 80, 30}, {0, 4, 0, 80, 80, 30}};
        checkFrames("250 wide", makeView(NSMakeSize(250, 100), flow(80, 30, none), @[@5], nil), e, 5, NSMakeSize(250, 110));
    }
    // Fewer items than columns: still on the columns
    {
        const double e[][6] = {{0, 0, 0, 0, 80, 30}, {0, 1, 110, 0, 80, 30}};
        checkFrames("fewer than the columns", makeView(NSMakeSize(300, 100), flow(80, 30, none), @[@2], nil), e, 2,
                    NSMakeSize(300, 30));
    }
    // One column: centered
    {
        const double e[][6] = {{0, 0, -25, 0, 250, 30}, {0, 1, -25, 40, 250, 30}};
        checkFrames("item wider than the view", makeView(NSMakeSize(200, 100), flow(250, 30, none), @[@2], nil), e, 2,
                    NSMakeSize(200, 70));
    }
    // A single column that fits: centered too
    {
        const double e[][6] = {{0, 0, 20, 0, 80, 30}, {0, 2, 20, 80, 80, 30}, {1, 0, 20, 110, 80, 30}};
        checkFrames("one column", makeView(NSMakeSize(120, 100), flow(80, 30, none), @[@3, @1], nil), e, 3,
                    NSMakeSize(120, 140));
    }
    // Default 50x50 items
    {
        const double e[][6] = {{0, 0, 0, 0, 50, 50}, {0, 1, 75, 0, 50, 50}, {0, 2, 150, 0, 50, 50}, {0, 3, 0, 60, 50, 50}};
        NSCollectionViewFlowLayout *layout = [[[NSCollectionViewFlowLayout alloc] init] autorelease];
        checkFrames("default item size", makeView(NSMakeSize(200, 100), layout, @[@4], nil), e, 4, NSMakeSize(200, 110));
    }
    // Spacings other than the defaults
    {
        NSCollectionViewFlowLayout *layout = flow(80, 30, none);
        layout.minimumInteritemSpacing = 20;
        layout.minimumLineSpacing = 5;
        const double e[][6] = {{0, 0, 0, 0, 80, 30}, {0, 1, 110, 0, 80, 30}, {0, 2, 220, 0, 80, 30},
            {0, 3, 0, 35, 80, 30}, {0, 4, 110, 35, 80, 30}};
        checkFrames("spacing 20 and 5", makeView(NSMakeSize(300, 100), layout, @[@5], nil), e, 5, NSMakeSize(300, 65));
    }
    checkFrames("empty", makeView(NSMakeSize(200, 100), flow(80, 30, none), @[@0], nil), NULL, 0, NSMakeSize(200, 0));
}

static void test_section_insets(void)
{
    const double e[][6] = {{0, 0, 10, 5, 80, 30}, {0, 1, 110, 5, 80, 30}, {0, 2, 10, 45, 80, 30}};
    checkFrames("insets", makeView(NSMakeSize(200, 100), flow(80, 30, NSEdgeInsetsMake(5, 10, 5, 10)), @[@3], nil), e, 3,
                NSMakeSize(200, 80));
}

static void test_two_sections(void)
{
    // No line spacing between sections
    {
        const double e[][6] = {{0, 0, 0, 0, 80, 30}, {0, 1, 120, 0, 80, 30}, {0, 2, 0, 40, 80, 30},
            {1, 0, 0, 70, 80, 30}, {1, 1, 120, 70, 80, 30}};
        checkFrames("two sections", makeView(NSMakeSize(200, 100), flow(80, 30, NSEdgeInsetsZero), @[@3, @2], nil), e, 5,
                    NSMakeSize(200, 100));
    }
    {
        const double e[][6] = {{0, 0, 10, 5, 80, 30}, {0, 1, 110, 5, 80, 30}, {0, 2, 10, 45, 80, 30},
            {1, 0, 10, 85, 80, 30}, {1, 1, 110, 85, 80, 30}};
        checkFrames("two sections with insets",
                    makeView(NSMakeSize(200, 100), flow(80, 30, NSEdgeInsetsMake(5, 10, 5, 10)), @[@3, @2], nil), e, 5,
                    NSMakeSize(200, 120));
    }
    {
        const double e[][6] = {{0, 2, 220, 0, 80, 30}, {1, 0, 0, 30, 80, 30}};
        checkFrames("a second section of one item",
                    makeView(NSMakeSize(300, 100), flow(80, 30, NSEdgeInsetsZero), @[@3, @1], nil), e, 2, NSMakeSize(300, 60));
    }
}

static void test_delegate_item_sizes(void)
{
    NSEdgeInsets none = NSEdgeInsetsZero;
    // A row is as tall as its tallest item, the others centered in it; mixed sizes pack the last row to the left
    {
        const double e[][6] = {{0, 0, 0, 10, 80, 30}, {0, 1, 120, 0, 80, 50}, {0, 2, 0, 70, 80, 20}, {0, 3, 90, 60, 80, 40}};
        checkFrames("delegate heights", makeView(NSMakeSize(200, 100), flow(80, 30, none), @[@4],
                    @[size(80, 30), size(80, 50), size(80, 20), size(80, 40)]), e, 4, NSMakeSize(200, 100));
    }
    {
        const double e[][6] = {{0, 0, 0, 0, 60, 30}, {0, 1, 70, 0, 100, 30}, {0, 2, 180, 0, 40, 30}};
        checkFrames("delegate widths, one row", makeView(NSMakeSize(250, 100), flow(80, 30, none), @[@3],
                    @[size(60, 30), size(100, 30), size(40, 30)]), e, 3, NSMakeSize(250, 30));
    }
    // A row of one item that isn't the last is centered
    {
        const double e[][6] = {{0, 0, 50, 0, 150, 30}, {0, 1, 0, 40, 150, 30}, {0, 2, 190, 40, 60, 30}, {0, 3, 0, 80, 60, 30}};
        checkFrames("delegate widths 150, 150, 60, 60", makeView(NSMakeSize(250, 100), flow(80, 30, none), @[@4],
                    @[size(150, 30), size(150, 30), size(60, 30), size(60, 30)]), e, 4, NSMakeSize(250, 110));
    }
    {
        const double e[][6] = {{0, 0, 0, 0, 100, 30}, {0, 1, 190, 0, 60, 30}, {0, 2, 0, 40, 100, 30},
            {0, 3, 150, 40, 100, 30}, {0, 4, 0, 80, 60, 30}};
        checkFrames("delegate widths 100, 60, 100", makeView(NSMakeSize(250, 100), flow(80, 30, none), @[@5],
                    @[size(100, 30), size(60, 30), size(100, 30)]), e, 5, NSMakeSize(250, 110));
    }
    // The delegate giving every item one size is the same as itemSize: the last row keeps the columns
    {
        const double e[][6] = {{0, 0, 0, 0, 60, 30}, {0, 1, 95, 0, 60, 30}, {0, 2, 190, 0, 60, 30},
            {0, 3, 0, 40, 60, 30}, {0, 4, 95, 40, 60, 30}};
        checkFrames("delegate one size", makeView(NSMakeSize(250, 100), flow(80, 30, none), @[@5], @[size(60, 30)]), e, 5,
                    NSMakeSize(250, 70));
    }
}

static void test_horizontal_direction(void)
{
    NSCollectionViewFlowLayout *layout = flow(80, 30, NSEdgeInsetsZero);
    layout.scrollDirection = NSCollectionViewScrollDirectionHorizontal;
    expect(layout.scrollDirection == NSCollectionViewScrollDirectionHorizontal, "the scroll direction is kept");
    const double e[][6] = {{0, 0, 0, 0, 80, 30}, {0, 1, 0, 70, 80, 30}, {0, 2, 90, 0, 80, 30},
        {0, 3, 90, 70, 80, 30}, {0, 4, 180, 0, 80, 30}};
    checkFrames("horizontal", makeView(NSMakeSize(200, 100), layout, @[@5], nil), e, 5, NSMakeSize(260, 100));
}

static NSString *paths(NSArray *attributes)
{
    NSMutableArray *names = [NSMutableArray array];
    for (NSCollectionViewLayoutAttributes *a in attributes)
        [names addObject: [NSString stringWithFormat: @"%ld/%ld", (long) a.indexPath.section, (long) a.indexPath.item]];
    [names sortUsingSelector: @selector(compare:)];
    return [names componentsJoinedByString: @" "];
}

static void test_elements_in_rect(void)
{
    NSCollectionView *view = makeView(NSMakeSize(250, 100), flow(80, 30, NSEdgeInsetsZero), @[@5], nil);
    NSCollectionViewLayout *layout = view.collectionViewLayout;
    NSString *got = paths([layout layoutAttributesForElementsInRect: NSMakeRect(0, 35, 250, 10)]);
    expect([got isEqual: @"0/2 0/3"], "elements in the second row's band: %s", [got UTF8String]);
    got = paths([layout layoutAttributesForElementsInRect: NSMakeRect(100, 0, 50, 200)]);
    expect([got isEqual: @""], "elements in the column gap: '%s'", [got UTF8String]);
    got = paths([layout layoutAttributesForElementsInRect: NSMakeRect(0, 0, 250, 200)]);
    expect([got isEqual: @"0/0 0/1 0/2 0/3 0/4"], "elements in the whole content: %s", [got UTF8String]);
}

static void test_cross_axis_change_invalidates(void)
{
    // The view is 250 wide and 110 high (its content); a vertical flow depends only on the width and the x origin.
    // Asked first, macOS 26.7.3 answers YES for any rect, so the first question is one whose answer is YES anyway.
    NSCollectionView *view = makeView(NSMakeSize(250, 100), flow(80, 30, NSEdgeInsetsZero), @[@5], nil);
    NSCollectionViewLayout *layout = view.collectionViewLayout;
    expect([layout shouldInvalidateLayoutForBoundsChange: NSMakeRect(0, 0, 300, 110)], "a new width invalidates");
    expect(![layout shouldInvalidateLayoutForBoundsChange: NSMakeRect(0, 10, 250, 110)], "scrolling does not invalidate");
    expect(![layout shouldInvalidateLayoutForBoundsChange: NSMakeRect(0, 10, 250, 100)], "scrolling and a new height don't");
    expect(![layout shouldInvalidateLayoutForBoundsChange: NSMakeRect(0, 0, 250, 500)], "a new height does not invalidate");
    expect([layout shouldInvalidateLayoutForBoundsChange: NSMakeRect(5, 0, 250, 110)], "a new x origin invalidates");
}

static void checkTarget(NSCollectionViewLayout *layout, NSPoint point, NSCollectionElementCategory category, NSInteger item,
                        NSRect frame, BOOL checkFrame)
{
    NSCollectionViewLayoutAttributes *a = [layout layoutAttributesForDropTargetAtPoint: point];
    expect(a != nil && a.representedElementCategory == category && a.indexPath.section == 0 && a.indexPath.item == item
               && (!checkFrame || NSEqualRects(a.frame, frame)),
           "drop target at %s: category %ld, item %ld, frame %s", [NSStringFromPoint(point) UTF8String],
           (long) a.representedElementCategory, (long) a.indexPath.item, [NSStringFromRect(a.frame) UTF8String]);
}

static void test_drop_target_on_item_and_in_gap(void)
{
    NSCollectionView *view = makeView(NSMakeSize(250, 100), flow(80, 30, NSEdgeInsetsZero), @[@5], nil);
    NSCollectionViewLayout *layout = view.collectionViewLayout;
    NSCollectionElementCategory item = NSCollectionElementCategoryItem, gap = NSCollectionElementCategoryInterItemGap;
    checkTarget(layout, NSMakePoint(10, 10), item, 0, NSMakeRect(0, 0, 80, 30), YES);
    checkTarget(layout, NSMakePoint(5, 95), item, 4, NSMakeRect(0, 80, 80, 30), YES);
    // Between two items of a row: the gap before the second, from one item to the other
    checkTarget(layout, NSMakePoint(85, 10), gap, 1, NSMakeRect(80, 0, 90, 30), YES);
    checkTarget(layout, NSMakePoint(100, 15), gap, 1, NSMakeRect(80, 0, 90, 30), YES);
    // Between rows: the gap before the first item of the next row
    checkTarget(layout, NSMakePoint(10, 35), gap, 2, NSZeroRect, NO);
    checkTarget(layout, NSMakePoint(200, 75), gap, 4, NSZeroRect, NO);

    // Gap attributes: before an item that starts a row they lie left of it, one interitem spacing wide; after the last
    // item, right of it
    const double e[][4] = {{-10, 0, 10, 30}, {80, 0, 90, 30}, {-10, 40, 10, 30}, {80, 40, 90, 30}, {-10, 80, 10, 30},
        {80, 80, 10, 30}};
    for (int i = 0; i <= 5; i++) {
        NSCollectionViewLayoutAttributes *a =
            [layout layoutAttributesForInterItemGapBeforeIndexPath: [NSIndexPath indexPathForItem: i inSection: 0]];
        NSRect want = NSMakeRect(e[i][0], e[i][1], e[i][2], e[i][3]);
        expect(a.representedElementCategory == gap && NSEqualRects(a.frame, want), "gap before item %d: %s (want %s)", i,
               [NSStringFromRect(a.frame) UTF8String], [NSStringFromRect(want) UTF8String]);
    }
}

// An empty section takes drops too: the drop target in a view whose sections are all empty is the gap before the
// first section's first item, and the gap before an empty section's first item is one item, at the section's top
// inset and the leading edge (observed on macOS 26.7.3 and 10.14.4; the hosts also give that frame, with the item
// category, for any later index of the empty section, which is not pinned)
static void test_drop_target_in_empty_sections(void)
{
    NSEdgeInsets inset = NSEdgeInsetsMake(5, 7, 5, 7);
    NSCollectionView *view = makeView(NSMakeSize(300, 300), flow(80, 30, inset), @[@0], nil);
    NSCollectionViewLayout *layout = view.collectionViewLayout;
    NSCollectionElementCategory gap = NSCollectionElementCategoryInterItemGap;
    checkTarget(layout, NSMakePoint(150, 150), gap, 0, NSZeroRect, NO);
    checkTarget(layout, NSMakePoint(10, 10), gap, 0, NSZeroRect, NO);
    NSCollectionViewLayoutAttributes *a =
        [layout layoutAttributesForInterItemGapBeforeIndexPath: [NSIndexPath indexPathForItem: 0 inSection: 0]];
    expect(a != nil && NSEqualRects(a.frame, NSMakeRect(0, 5, 80, 30)), "gap before an empty section's first item: %s",
           [NSStringFromRect(a.frame) UTF8String]);

    // An empty section before one with items
    view = makeView(NSMakeSize(300, 300), flow(80, 30, inset), @[@0, @3], nil);
    layout = view.collectionViewLayout;
    a = [layout layoutAttributesForInterItemGapBeforeIndexPath: [NSIndexPath indexPathForItem: 0 inSection: 0]];
    expect(NSEqualRects(a.frame, NSMakeRect(0, 5, 80, 30)), "gap before the empty first section's item: %s",
           [NSStringFromRect(a.frame) UTF8String]);
    a = [layout layoutAttributesForInterItemGapBeforeIndexPath: [NSIndexPath indexPathForItem: 0 inSection: 1]];
    expect(NSEqualRects(a.frame, NSMakeRect(-3, 15, 10, 30)), "gap before the second section's first item: %s",
           [NSStringFromRect(a.frame) UTF8String]);
}

int main(void)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    keep = [[NSMutableArray alloc] init];
    test_row_distribution();
    test_section_insets();
    test_two_sections();
    test_delegate_item_sizes();
    test_horizontal_direction();
    test_elements_in_rect();
    test_cross_axis_change_invalidates();
    test_drop_target_on_item_and_in_gap();
    test_drop_target_in_empty_sections();
    [keep release];
    [pool release];
    if (failures)
        printf("FAILED %d\n", failures);
    else
        printf("ALL PASSED\n");
    return failures != 0;
}
