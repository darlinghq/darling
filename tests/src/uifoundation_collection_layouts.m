// CFLAGS: -framework Foundation -framework AppKit
// Copyright (C) 2026 Lubos Dolezel
//
// The collection view layout classes. On macOS 10.14 NSCollectionViewLayout,
// NSCollectionViewFlowLayout, NSCollectionViewLayoutAttributes and
// NSCollectionViewLayoutInvalidationContext are defined by UIFoundation (as
// are the NSCollectionElementKind and flow layout alignment constants and the
// NSIndexPath item/section category), while AppKit defines
// NSCollectionViewGridLayout, a subclass of NSCollectionViewLayout. This
// checks that placement through the runtime, that the flow layout keeps its
// documented defaults and its instance variables, that NSCollectionView lays
// items out as a flow layout describes (frames for four configurations), and
// that the NSIndexPath category works.
//
// Prints PASS or FAIL lines, then ALL PASSED or FAILED n.
#import <AppKit/AppKit.h>
#include <dlfcn.h>
#include <objc/runtime.h>
#include <stdio.h>
#include <string.h>

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

static BOOL endsWith(const char *s, const char *suffix)
{
    size_t a = s ? strlen(s) : 0, b = strlen(suffix);
    return s != NULL && a >= b && strcmp(s + a - b, suffix) == 0;
}

@interface FlowSource : NSObject {
@public
    NSArray *counts;
}
@end
@implementation FlowSource
- (NSInteger)numberOfSectionsInCollectionView:(NSCollectionView *)view
{
    return [counts count];
}
- (NSInteger)collectionView:(NSCollectionView *)view numberOfItemsInSection:(NSInteger)section
{
    return [[counts objectAtIndex: section] integerValue];
}
- (NSCollectionViewItem *)collectionView:(NSCollectionView *)view
    itemForRepresentedObjectAtIndexPath:(NSIndexPath *)path
{
    NSCollectionViewItem *item = [view makeItemWithIdentifier: @"flow" forIndexPath: path];
    item.view = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 1, 1)] autorelease];
    return item;
}
@end

// Lays out a collection view of the given width with a flow layout and checks
// the frame of the items listed in `expected` ({section, item, x, y}) and the
// resulting view height.
static void checkFlow(const char *name, CGFloat width, NSSize itemSize, NSEdgeInsets inset, BOOL useDefaultSize,
                      NSArray *counts, const double (*expected)[4], size_t expectedCount, double height)
{
    NSCollectionView *view = [[[NSCollectionView alloc] initWithFrame: NSMakeRect(0, 0, width, 100)] autorelease];
    NSCollectionViewFlowLayout *layout = [[[NSCollectionViewFlowLayout alloc] init] autorelease];
    if (!useDefaultSize)
        layout.itemSize = itemSize;
    layout.sectionInset = inset;
    view.collectionViewLayout = layout;
    [view registerClass: [NSCollectionViewItem class] forItemWithIdentifier: @"flow"];
    FlowSource *source = [[[FlowSource alloc] init] autorelease];
    source->counts = counts;
    view.dataSource = source;
    [view reloadData];
    NSSize size = useDefaultSize ? NSMakeSize(50, 50) : itemSize;
    for (size_t i = 0; i < expectedCount; i++) {
        NSIndexPath *path = [NSIndexPath indexPathForItem: (NSInteger) expected[i][1]
                                                inSection: (NSInteger) expected[i][0]];
        NSRect frame = [[[view itemAtIndexPath: path] view] frame];
        NSRect want = NSMakeRect(expected[i][2], expected[i][3], size.width, size.height);
        expect(NSEqualRects(frame, want), "%s: item %ld/%ld frame %g,%g %gx%g (want %g,%g)", name,
              (long) expected[i][0], (long) expected[i][1], frame.origin.x, frame.origin.y, frame.size.width,
              frame.size.height, want.origin.x, want.origin.y);
    }
    expect(fabs(NSHeight([view frame]) - height) < 0.001, "%s: view height %g (want %g)", name,
          NSHeight([view frame]), height);
}

int main(void)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // Placement.
    const char *uifClasses[] = {"NSCollectionViewLayout", "NSCollectionViewFlowLayout",
        "NSCollectionViewLayoutAttributes", "NSCollectionViewLayoutInvalidationContext"};
    for (size_t i = 0; i < sizeof(uifClasses) / sizeof(uifClasses[0]); i++) {
        Class c = objc_getClass(uifClasses[i]);
        expect(c != Nil && endsWith(class_getImageName(c), "/UIFoundation"), "%s defined by UIFoundation (%s)",
              uifClasses[i], c ? class_getImageName(c) : "missing");
    }
    Class grid = objc_getClass("NSCollectionViewGridLayout");
    expect(grid != Nil && endsWith(class_getImageName(grid), "/AppKit") &&
                  class_getSuperclass(grid) == objc_getClass("NSCollectionViewLayout"),
          "NSCollectionViewGridLayout is AppKit's subclass of UIFoundation's NSCollectionViewLayout");
    Class flow = objc_getClass("NSCollectionViewFlowLayout");
    expect(class_getSuperclass(flow) == objc_getClass("NSCollectionViewLayout"),
          "NSCollectionViewFlowLayout inherits NSCollectionViewLayout");
    Method m = class_getClassMethod([NSIndexPath class], @selector(indexPathForItem:inSection:));
    expect(m != NULL, "NSIndexPath +indexPathForItem:inSection: exists");

    // Instance variable layout of the public header is unchanged.
    expect(class_getInstanceVariable(flow, "_itemSize") != NULL &&
                  class_getInstanceVariable(flow, "_sectionInset") != NULL &&
                  class_getInstanceVariable(flow, "_minimumLineSpacing") != NULL &&
                  class_getInstanceVariable(flow, "_minimumInteritemSpacing") != NULL,
          "flow layout keeps its instance variables");

    // Defaults.
    NSCollectionViewFlowLayout *layout = [[[NSCollectionViewFlowLayout alloc] init] autorelease];
    expect(NSEqualSizes(layout.itemSize, NSMakeSize(50, 50)) && layout.minimumLineSpacing == 10 &&
                  layout.minimumInteritemSpacing == 10,
          "flow layout defaults: 50x50 items, 10 point spacing");
    NSIndexPath *path = [NSIndexPath indexPathForItem: 3 inSection: 2];
    expect(path.item == 3 && path.section == 2 && [path length] == 2, "index path item/section");

    NSEdgeInsets none = {0, 0, 0, 0};
    // 200 wide, 80x30 items, 10 point gaps: two per row, rows 40 apart.
    {
        const double e[][4] = {{0, 0, 0, 0}, {0, 1, 90, 0}, {0, 2, 0, 40}, {0, 4, 0, 80}};
        checkFlow("two columns", 200, NSMakeSize(80, 30), none, NO, [NSArray arrayWithObject: @5], e, 4, 110);
    }
    // Section insets (top 5, left 10, bottom 5, right 10): 180 usable, still two columns.
    {
        NSEdgeInsets inset = {5, 10, 5, 10};
        const double e[][4] = {{0, 0, 10, 5}, {0, 1, 100, 5}, {0, 2, 10, 45}};
        checkFlow("section insets", 200, NSMakeSize(80, 30), inset, NO, [NSArray arrayWithObject: @3], e, 3, 80);
    }
    // One column at 120 wide, two sections.
    {
        const double e[][4] = {{0, 0, 0, 0}, {0, 1, 0, 40}, {0, 2, 0, 80}, {1, 0, 0, 110}};
        checkFlow("two sections", 120, NSMakeSize(80, 30), none, NO, [NSArray arrayWithObjects: @3, @1, nil], e, 4,
                  140);
    }
    // Default 50x50 items in 200 points: three columns, 60 apart.
    {
        const double e[][4] = {{0, 0, 0, 0}, {0, 2, 120, 0}, {0, 3, 0, 60}};
        checkFlow("default item size", 200, NSZeroSize, none, YES, [NSArray arrayWithObject: @4], e, 3, 110);
    }

    [pool release];
    if (failures)
        printf("FAILED %d\n", failures);
    else
        printf("ALL PASSED\n");
    return failures != 0;
}
