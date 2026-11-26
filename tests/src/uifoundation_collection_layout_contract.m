// CFLAGS: -framework Foundation -framework AppKit
// Copyright (C) 2026 Lubos Dolezel
//
// The NSCollectionViewLayout subclassing contract (UIFoundation, re-exported by
// AppKit): what the base class answers, which classes it reports for its
// attributes and invalidation contexts, how -invalidateLayout reaches
// -invalidateLayoutWithContext:, and how a collection view attaches its
// layout. Values the documentation does not pin were observed on macOS 26.7.3
// and 10.14.4 (tests/collection-view.md); the program passes unchanged on both.
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

// Records the contexts it is invalidated with.
@interface RecordingLayout : NSCollectionViewLayout {
@public
    NSMutableArray *contexts;
}
@end

@implementation RecordingLayout
- (void) invalidateLayoutWithContext: (NSCollectionViewLayoutInvalidationContext *) context {
    if (contexts == nil)
        contexts = [[NSMutableArray alloc] init];
    [contexts addObject: context];
    [super invalidateLayoutWithContext: context];
}
- (void) dealloc {
    [contexts release];
    [super dealloc];
}
@end

@interface CustomContext : NSCollectionViewLayoutInvalidationContext
@end
@implementation CustomContext
@end

@interface CustomContextLayout : RecordingLayout
@end
@implementation CustomContextLayout
+ (Class) invalidationContextClass {
    return [CustomContext class];
}
@end

static void test_base_defaults(void)
{
    NSCollectionViewLayout *layout = [[[NSCollectionViewLayout alloc] init] autorelease];
    NSIndexPath *path = [NSIndexPath indexPathForItem: 0 inSection: 0];
    expect(layout.collectionView == nil, "a new layout has no collection view");
    expect(NSEqualSizes(layout.collectionViewContentSize, NSZeroSize), "base content size is zero");
    NSArray *elements = [layout layoutAttributesForElementsInRect: NSMakeRect(0, 0, 100, 100)];
    expect(elements == nil || [elements count] == 0, "base elements in a rect: none (%s)",
           elements == nil ? "nil" : "empty array");
    expect([layout layoutAttributesForItemAtIndexPath: path] == nil, "base item attributes are nil");
    expect([layout layoutAttributesForSupplementaryViewOfKind: NSCollectionElementKindSectionHeader atIndexPath: path] == nil,
           "base supplementary attributes are nil");
    expect([layout layoutAttributesForDecorationViewOfKind: @"Shelf" atIndexPath: path] == nil,
           "base decoration attributes are nil");
    expect([layout layoutAttributesForDropTargetAtPoint: NSMakePoint(5, 5)] == nil, "base drop target is nil");
    expect([layout layoutAttributesForInterItemGapBeforeIndexPath: path] == nil, "base gap attributes are nil");
    expect(![layout shouldInvalidateLayoutForBoundsChange: NSMakeRect(0, 0, 10, 10)],
           "base layout is not invalidated by a bounds change");
    NSPoint proposed = NSMakePoint(3, 7);
    expect(NSEqualPoints([layout targetContentOffsetForProposedContentOffset: proposed], proposed),
           "base target content offset is the proposed one");
    [layout prepareLayout];
    expect(YES, "base prepareLayout does nothing");
}

static void test_reported_classes(void)
{
    expect([NSCollectionViewLayout layoutAttributesClass] == [NSCollectionViewLayoutAttributes class],
           "layoutAttributesClass is NSCollectionViewLayoutAttributes");
    expect([NSCollectionViewLayout invalidationContextClass] == [NSCollectionViewLayoutInvalidationContext class],
           "invalidationContextClass is NSCollectionViewLayoutInvalidationContext");
    expect([NSCollectionViewFlowLayout invalidationContextClass] == [NSCollectionViewFlowLayoutInvalidationContext class],
           "the flow layout's invalidationContextClass is NSCollectionViewFlowLayoutInvalidationContext (%s)",
           [NSStringFromClass([NSCollectionViewFlowLayout invalidationContextClass]) UTF8String]);
}

static void test_invalidate_layout_passes_plain_context(void)
{
    RecordingLayout *layout = [[[RecordingLayout alloc] init] autorelease];
    [layout invalidateLayout];
    expect([layout->contexts count] == 1, "invalidateLayout calls invalidateLayoutWithContext: once (%lu)",
           (unsigned long) [layout->contexts count]);
    NSCollectionViewLayoutInvalidationContext *context = [layout->contexts lastObject];
    expect([context class] == [NSCollectionViewLayoutInvalidationContext class], "with a context of invalidationContextClass (%s)",
           [NSStringFromClass([context class]) UTF8String]);
    // Not documented: the context is a plain one; only a collection view's reloadData marks everything
    expect(!context.invalidateEverything && !context.invalidateDataSourceCounts,
           "that is a plain context (everything %d, counts %d)", context.invalidateEverything,
           context.invalidateDataSourceCounts);
}

static void test_custom_context_class(void)
{
    CustomContextLayout *layout = [[[CustomContextLayout alloc] init] autorelease];
    [layout invalidateLayout];
    expect([[layout->contexts lastObject] isKindOfClass: [CustomContext class]],
           "invalidateLayout makes the subclass's invalidationContextClass");
}

static void test_bounds_change_context(void)
{
    CustomContextLayout *layout = [[[CustomContextLayout alloc] init] autorelease];
    NSCollectionViewLayoutInvalidationContext *context =
        [layout invalidationContextForBoundsChange: NSMakeRect(0, 0, 50, 50)];
    expect([context isKindOfClass: [CustomContext class]], "invalidationContextForBoundsChange: makes invalidationContextClass");
    expect(!context.invalidateEverything, "a bounds-change context does not invalidate everything");
}

static void test_collection_view_attaches_layout(void)
{
    NSCollectionView *view = [[[NSCollectionView alloc] initWithFrame: NSMakeRect(0, 0, 100, 100)] autorelease];
    RecordingLayout *first = [[[RecordingLayout alloc] init] autorelease];
    RecordingLayout *second = [[[RecordingLayout alloc] init] autorelease];
    view.collectionViewLayout = first;
    expect(first.collectionView == view, "a layout set on a collection view knows it");
    view.collectionViewLayout = second;
    expect(second.collectionView == view, "the new layout knows the collection view");
    expect(first.collectionView == nil, "the replaced layout no longer has one");
}

int main(void)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    test_base_defaults();
    test_reported_classes();
    test_invalidate_layout_passes_plain_context();
    test_custom_context_class();
    test_bounds_change_context();
    test_collection_view_attaches_layout();
    [pool release];
    if (failures)
        printf("FAILED %d\n", failures);
    else
        printf("ALL PASSED\n");
    return failures != 0;
}
