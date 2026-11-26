// CFLAGS: -framework Foundation -framework AppKit
// Copyright (C) 2026 Lubos Dolezel
//
// NSCollectionViewLayoutAttributes and NSCollectionViewLayoutInvalidationContext
// (UIFoundation, re-exported by AppKit): the class factories, the default and
// stored values, copying and equality of attributes, and what a fresh
// invalidation context reports. Values the documentation does not pin were
// observed on macOS 26.7.3 and 10.14.4 (tests/collection-view.md); the program
// passes unchanged on both.
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

static void test_item_attributes_defaults(void)
{
    NSIndexPath *path = [NSIndexPath indexPathForItem: 2 inSection: 1];
    NSCollectionViewLayoutAttributes *a = [NSCollectionViewLayoutAttributes layoutAttributesForItemWithIndexPath: path];
    expect(a != nil && [a isKindOfClass: [NSCollectionViewLayoutAttributes class]], "item attributes are made");
    expect([a.indexPath isEqual: path], "item attributes keep the index path");
    expect(a.alpha == 1.0, "item attributes: alpha defaults to 1 (%g)", (double) a.alpha);
    expect(a.zIndex == 0, "item attributes: zIndex defaults to 0 (%ld)", (long) a.zIndex);
    expect(!a.hidden, "item attributes: not hidden");
    expect(NSEqualRects(a.frame, NSZeroRect), "item attributes: frame defaults to zero");
    expect(a.representedElementCategory == NSCollectionElementCategoryItem, "item attributes: category Item (%ld)",
           (long) a.representedElementCategory);
    expect(a.representedElementKind == nil, "item attributes: no element kind");
}

static void test_size_follows_frame(void)
{
    NSCollectionViewLayoutAttributes *a =
        [NSCollectionViewLayoutAttributes layoutAttributesForItemWithIndexPath: [NSIndexPath indexPathForItem: 0 inSection: 0]];
    a.frame = NSMakeRect(10, 20, 30, 40);
    expect(NSEqualSizes(a.size, NSMakeSize(30, 40)), "setting frame sets size");
    // Not documented: the size changes around the frame's center (observed on macOS 26.7.3 and 10.14.4)
    a.size = NSMakeSize(50, 60);
    expect(NSEqualRects(a.frame, NSMakeRect(0, 10, 50, 60)), "setting size keeps the center (%s)",
           [NSStringFromRect(a.frame) UTF8String]);
}

static void test_gap_attributes_category(void)
{
    NSIndexPath *path = [NSIndexPath indexPathForItem: 3 inSection: 0];
    NSCollectionViewLayoutAttributes *a =
        [NSCollectionViewLayoutAttributes layoutAttributesForInterItemGapBeforeIndexPath: path];
    expect(a.representedElementCategory == NSCollectionElementCategoryInterItemGap, "gap attributes: category InterItemGap (%ld)",
           (long) a.representedElementCategory);
    expect([a.representedElementKind isEqual: NSCollectionElementKindInterItemGapIndicator],
           "gap attributes: kind is the gap indicator (%s)", [[a.representedElementKind description] UTF8String]);
    expect([a.indexPath isEqual: path], "gap attributes keep the index path");
}

static void test_supplementary_and_decoration_kinds(void)
{
    NSIndexPath *path = [NSIndexPath indexPathForItem: 0 inSection: 2];
    NSCollectionViewLayoutAttributes *s =
        [NSCollectionViewLayoutAttributes layoutAttributesForSupplementaryViewOfKind: NSCollectionElementKindSectionHeader
                                                                       withIndexPath: path];
    expect(s.representedElementCategory == NSCollectionElementCategorySupplementaryView &&
               [s.representedElementKind isEqual: NSCollectionElementKindSectionHeader],
           "supplementary attributes: category and kind");
    NSCollectionViewLayoutAttributes *d =
        [NSCollectionViewLayoutAttributes layoutAttributesForDecorationViewOfKind: @"Shelf" withIndexPath: path];
    expect(d.representedElementCategory == NSCollectionElementCategoryDecorationView &&
               [d.representedElementKind isEqual: @"Shelf"],
           "decoration attributes: category and kind");
}

static NSCollectionViewLayoutAttributes *sample(void)
{
    NSCollectionViewLayoutAttributes *a =
        [NSCollectionViewLayoutAttributes layoutAttributesForItemWithIndexPath: [NSIndexPath indexPathForItem: 1 inSection: 0]];
    a.frame = NSMakeRect(1, 2, 3, 4);
    a.alpha = 0.5;
    a.zIndex = 7;
    a.hidden = YES;
    return a;
}

static void test_copy_is_equal_and_independent(void)
{
    NSCollectionViewLayoutAttributes *a = sample();
    NSCollectionViewLayoutAttributes *c = [[a copy] autorelease];
    expect(c != a && [c isEqual: a], "a copy is a different object and equal");
    expect(NSEqualRects(c.frame, a.frame) && c.alpha == 0.5 && c.zIndex == 7 && c.hidden && [c.indexPath isEqual: a.indexPath],
           "a copy keeps every property");
    c.alpha = 1;
    expect(a.alpha == 0.5, "changing a copy leaves the original");
}

static void test_equality_by_properties(void)
{
    NSCollectionViewLayoutAttributes *a = sample(), *b = sample();
    expect([a isEqual: b] && [a hash] == [b hash], "attributes with the same values are equal and hash equally");
    b.alpha = 0.25;
    expect(![a isEqual: b], "attributes with a different alpha differ");
    b = sample();
    b.frame = NSMakeRect(1, 2, 3, 5);
    expect(![a isEqual: b], "attributes with a different frame differ");
}

static void test_context_defaults(void)
{
    NSCollectionViewLayoutInvalidationContext *c = [[[NSCollectionViewLayoutInvalidationContext alloc] init] autorelease];
    expect(!c.invalidateEverything, "a fresh context does not invalidate everything");
    expect(!c.invalidateDataSourceCounts, "a fresh context does not invalidate the data source counts");
    expect(c.invalidatedItemIndexPaths == nil, "a fresh context has no invalidated items");
    expect(NSEqualPoints(c.contentOffsetAdjustment, NSZeroPoint) && NSEqualSizes(c.contentSizeAdjustment, NSZeroSize),
           "a fresh context has no adjustments");
    NSCollectionViewFlowLayoutInvalidationContext *f =
        [[[NSCollectionViewFlowLayoutInvalidationContext alloc] init] autorelease];
    expect(f.invalidateFlowLayoutDelegateMetrics, "a flow context invalidates the delegate metrics by default");
    expect(f.invalidateFlowLayoutAttributes, "a flow context invalidates the flow attributes by default");
}

static void test_context_collects_index_paths(void)
{
    NSCollectionViewLayoutInvalidationContext *c = [[[NSCollectionViewLayoutInvalidationContext alloc] init] autorelease];
    NSIndexPath *p1 = [NSIndexPath indexPathForItem: 0 inSection: 0], *p2 = [NSIndexPath indexPathForItem: 4 inSection: 1];
    [c invalidateItemsAtIndexPaths: [NSSet setWithObject: p1]];
    [c invalidateItemsAtIndexPaths: [NSSet setWithObject: p2]];
    expect([c.invalidatedItemIndexPaths isEqual: [NSSet setWithObjects: p1, p2, nil]], "the context collects invalidated items (%s)",
           [[c.invalidatedItemIndexPaths description] UTF8String]);
    c.contentOffsetAdjustment = NSMakePoint(3, 4);
    c.contentSizeAdjustment = NSMakeSize(5, 6);
    expect(NSEqualPoints(c.contentOffsetAdjustment, NSMakePoint(3, 4)) && NSEqualSizes(c.contentSizeAdjustment, NSMakeSize(5, 6)),
           "the context keeps its adjustments");
}

int main(void)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    test_item_attributes_defaults();
    test_size_follows_frame();
    test_gap_attributes_category();
    test_supplementary_and_decoration_kinds();
    test_copy_is_equal_and_independent();
    test_equality_by_properties();
    test_context_defaults();
    test_context_collects_index_paths();
    [pool release];
    if (failures)
        printf("FAILED %d\n", failures);
    else
        printf("ALL PASSED\n");
    return failures != 0;
}
