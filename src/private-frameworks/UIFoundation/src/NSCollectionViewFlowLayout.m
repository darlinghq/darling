/*
 This file is part of Darling.

 Copyright (C) 2019-2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <AppKit/NSCollectionViewFlowLayout.h>
#import <UIFoundation/NSCollectionViewFlowLayoutInvalidationContext.h>
#import <AppKit/NSCollectionViewLayoutAttributes.h>
#import <Foundation/NSIndexPath.h>
#import <Foundation/NSValue.h>
#include <math.h>
#include <stdlib.h>

NSString *const NSCollectionElementKindSectionHeader =
        @"UICollectionElementKindSectionHeader";
NSString *const NSCollectionElementKindSectionFooter =
        @"UICollectionElementKindSectionFooter";

// Not declared in a public header.
APPKIT_EXPORT NSString *const NSFlowLayoutCommonRowHorizontalAlignmentKey;
APPKIT_EXPORT NSString *const NSFlowLayoutLastRowHorizontalAlignmentKey;
APPKIT_EXPORT NSString *const NSFlowLayoutRowVerticalAlignmentKey;
NSString *const NSFlowLayoutCommonRowHorizontalAlignmentKey =
        @"UIFlowLayoutCommonRowHorizontalAlignmentKey";
NSString *const NSFlowLayoutLastRowHorizontalAlignmentKey =
        @"UIFlowLayoutLastRowHorizontalAlignmentKey";
NSString *const NSFlowLayoutRowVerticalAlignmentKey =
        @"UIFlowLayoutRowVerticalAlignmentKey";

// A flow fills lines one after another. For a vertical flow the lines are rows: "across" is x, the direction a row
// fills, and "along" is y, the direction the rows follow each other. A horizontal flow swaps the two.
typedef struct {
    CGFloat across, along, acrossLength, alongLength;
} FlowRect;

static NSRect rectFromFlow(FlowRect r, BOOL horizontal)
{
    if (horizontal)
        return NSMakeRect(r.along, r.across, r.alongLength, r.acrossLength);
    return NSMakeRect(r.across, r.along, r.acrossLength, r.alongLength);
}

static FlowRect flowFromRect(NSRect r, BOOL horizontal)
{
    FlowRect f;
    f.across = horizontal ? NSMinY(r) : NSMinX(r);
    f.along = horizontal ? NSMinX(r) : NSMinY(r);
    f.acrossLength = horizontal ? NSHeight(r) : NSWidth(r);
    f.alongLength = horizontal ? NSWidth(r) : NSHeight(r);
    return f;
}

// Where the items of one line go across it. `lengths` are the items' lengths across, `start` and `available` the
// part of the line between the section insets. The documentation only says the spacings are minimums.
//
// QUIRK: the space a line doesn't need is spread between its items. When every item of the section has the same size,
// the items sit on columns: as many as fit with the minimum spacing, the first at the start, the last at the end and
// equal gaps between them, and every line (the last one too) uses those columns; a single column is centered. When
// sizes differ, each line except the last puts its first item at the start, its last item at the end and equal gaps
// between them, a line of one item is centered, and the last line is packed at the start with the minimum spacing.
// Not documented. Evidence: black-box observation on macOS 26.7.3 and 10.14.4 (tier 2; tests/collection-view.md).
// Test: test_row_distribution and test_delegate_item_sizes in tests/src/uifoundation_flow_layout.m.
static void placeLine(const CGFloat *lengths, NSInteger count, BOOL uniform, NSInteger columns, BOOL lastLine,
                      CGFloat start, CGFloat available, CGFloat spacing, CGFloat *positions)
{
    if (uniform) {
        CGFloat length = lengths[0];
        if (columns <= 1) {
            positions[0] = start + (available - length) / 2;
            return;
        }
        CGFloat gap = (available - columns * length) / (columns - 1);
        for (NSInteger i = 0; i < count; i++)
            positions[i] = start + i * (length + gap);
        return;
    }

    CGFloat total = 0;
    for (NSInteger i = 0; i < count; i++)
        total += lengths[i];
    CGFloat gap = spacing;
    if (lastLine)
        ; // packed at the start
    else if (count == 1)
        start += (available - total) / 2;
    else
        gap = (available - total) / (count - 1);
    CGFloat position = start;
    for (NSInteger i = 0; i < count; i++) {
        positions[i] = position;
        position += lengths[i] + gap;
    }
}

@interface NSCollectionViewFlowLayout ()
- (NSCollectionViewLayoutAttributes *) _gapInEmptySection: (NSInteger) section;
@end

@implementation NSCollectionViewFlowLayout

+ (Class) invalidationContextClass {
    return [NSCollectionViewFlowLayoutInvalidationContext class];
}

- (instancetype) init {
    if ((self = [super init]) != nil) {
        _itemSize = NSMakeSize(50, 50);
        _minimumLineSpacing = 10;
        _minimumInteritemSpacing = 10;
    }
    return self;
}

// The archived properties are not restored yet; a flow layout from a nib has the defaults of -init
- (instancetype) initWithCoder: (NSCoder *) coder {
    return [self init];
}

- (void) dealloc {
    [_sectionItems release];
    [_sectionLines release];
    [super dealloc];
}

#define FLOW_PROPERTY(type, getter, setter, ivar)                                                                      \
    -(type) getter {                                                                                                   \
        return ivar;                                                                                                   \
    }                                                                                                                  \
    -(void) setter: (type) value {                                                                                     \
        ivar = value;                                                                                                  \
        [self invalidateLayout];                                                                                       \
    }

FLOW_PROPERTY(NSSize, itemSize, setItemSize, _itemSize)
FLOW_PROPERTY(NSSize, estimatedItemSize, setEstimatedItemSize, _estimatedItemSize)
FLOW_PROPERTY(NSEdgeInsets, sectionInset, setSectionInset, _sectionInset)
FLOW_PROPERTY(CGFloat, minimumLineSpacing, setMinimumLineSpacing, _minimumLineSpacing)
FLOW_PROPERTY(CGFloat, minimumInteritemSpacing, setMinimumInteritemSpacing, _minimumInteritemSpacing)
FLOW_PROPERTY(NSCollectionViewScrollDirection, scrollDirection, setScrollDirection, _scrollDirection)
FLOW_PROPERTY(NSSize, headerReferenceSize, setHeaderReferenceSize, _headerReferenceSize)
FLOW_PROPERTY(NSSize, footerReferenceSize, setFooterReferenceSize, _footerReferenceSize)

- (BOOL) _isHorizontal {
    return _scrollDirection == NSCollectionViewScrollDirectionHorizontal;
}

- (void) _discardLayout {
    [_sectionItems release];
    _sectionItems = nil;
    [_sectionLines release];
    _sectionLines = nil;
    [_sectionStarts release];
    _sectionStarts = nil;
    _contentSize = NSZeroSize;
}

- (void) _prepareIfNeeded {
    if (_sectionItems == nil)
        [self prepareLayout];
}

- (NSSize) _sizeForItemAtIndexPath: (NSIndexPath *) path delegate: (id) delegate {
    if ([delegate respondsToSelector: @selector(collectionView:layout:sizeForItemAtIndexPath:)])
        return [delegate collectionView: self.collectionView layout: self sizeForItemAtIndexPath: path];
    return _itemSize;
}

- (NSEdgeInsets) _insetForSection: (NSInteger) section delegate: (id) delegate {
    if ([delegate respondsToSelector: @selector(collectionView:layout:insetForSectionAtIndex:)])
        return [delegate collectionView: self.collectionView layout: self insetForSectionAtIndex: section];
    return _sectionInset;
}

- (CGFloat) _lineSpacingForSection: (NSInteger) section delegate: (id) delegate {
    if ([delegate respondsToSelector: @selector(collectionView:layout:minimumLineSpacingForSectionAtIndex:)])
        return [delegate collectionView: self.collectionView layout: self minimumLineSpacingForSectionAtIndex: section];
    return _minimumLineSpacing;
}

- (CGFloat) _interitemSpacingForSection: (NSInteger) section delegate: (id) delegate {
    if ([delegate respondsToSelector: @selector(collectionView:layout:minimumInteritemSpacingForSectionAtIndex:)])
        return [delegate collectionView: self.collectionView layout: self minimumInteritemSpacingForSectionAtIndex: section];
    return _minimumInteritemSpacing;
}

- (void) prepareLayout {
    [super prepareLayout];
    [self _discardLayout];
    _sectionItems = [[NSMutableArray alloc] init];
    _sectionLines = [[NSMutableArray alloc] init];
    _sectionStarts = [[NSMutableArray alloc] init];

    NSCollectionView *view = self.collectionView;
    BOOL horizontal = [self _isHorizontal];
    NSRect bounds = view != nil ? [view bounds] : NSZeroRect;
    CGFloat extent = horizontal ? NSHeight(bounds) : NSWidth(bounds);
    id delegate = [view delegate];
    Class attributesClass = [[self class] layoutAttributesClass];
    NSInteger sections = view != nil ? [view numberOfSections] : 0;
    CGFloat along = 0;

    for (NSInteger section = 0; section < sections; section++) {
        NSEdgeInsets inset = [self _insetForSection: section delegate: delegate];
        CGFloat lineSpacing = [self _lineSpacingForSection: section delegate: delegate];
        CGFloat spacing = [self _interitemSpacingForSection: section delegate: delegate];
        CGFloat start = horizontal ? inset.top : inset.left;
        CGFloat available = extent - start - (horizontal ? inset.bottom : inset.right);
        NSInteger count = [view numberOfItemsInSection: section];
        NSMutableArray *items = [NSMutableArray arrayWithCapacity: count];
        NSMutableArray *lines = [NSMutableArray arrayWithCapacity: count];
        [_sectionItems addObject: items];
        [_sectionLines addObject: lines];
        along += horizontal ? inset.left : inset.top;
        [_sectionStarts addObject: [NSNumber numberWithDouble: along]];

        CGFloat *acrossLengths = malloc(sizeof(CGFloat) * (count + 1) * 3);
        CGFloat *alongLengths = acrossLengths + count + 1;
        CGFloat *positions = alongLengths + count + 1;
        BOOL uniform = YES;
        NSSize first = NSZeroSize;
        for (NSInteger i = 0; i < count; i++) {
            NSSize size = [self _sizeForItemAtIndexPath: [NSIndexPath indexPathForItem: i inSection: section]
                                               delegate: delegate];
            if (i == 0)
                first = size;
            else if (!NSEqualSizes(size, first))
                uniform = NO;
            acrossLengths[i] = horizontal ? size.height : size.width;
            alongLengths[i] = horizontal ? size.width : size.height;
        }
        NSInteger columns = 1;
        if (uniform && count > 0 && acrossLengths[0] + spacing > 0)
            columns = MAX(1, (NSInteger) floor((available + spacing) / (acrossLengths[0] + spacing)));

        for (NSInteger lineStart = 0; lineStart < count;) {
            NSInteger lineEnd = lineStart + 1; // one past the line's last item
            if (uniform)
                lineEnd = MIN(lineStart + columns, count);
            else {
                CGFloat used = acrossLengths[lineStart];
                while (lineEnd < count && used + spacing + acrossLengths[lineEnd] <= available) {
                    used += spacing + acrossLengths[lineEnd];
                    lineEnd++;
                }
            }
            CGFloat thickness = 0;
            for (NSInteger i = lineStart; i < lineEnd; i++)
                thickness = MAX(thickness, alongLengths[i]);
            BOOL lastLine = lineEnd == count;
            placeLine(acrossLengths + lineStart, lineEnd - lineStart, uniform, columns, lastLine, start, available,
                      spacing, positions);

            FlowRect line = {start, along, available, thickness};
            NSValue *lineValue = [NSValue valueWithRect: rectFromFlow(line, horizontal)];
            for (NSInteger i = lineStart; i < lineEnd; i++) {
                // A line is as thick as its thickest item; the others are centered in it
                FlowRect frame = {positions[i - lineStart], along + (thickness - alongLengths[i]) / 2, acrossLengths[i],
                                  alongLengths[i]};
                NSCollectionViewLayoutAttributes *attributes = [attributesClass
                    layoutAttributesForItemWithIndexPath: [NSIndexPath indexPathForItem: i inSection: section]];
                attributes.frame = rectFromFlow(frame, horizontal);
                [items addObject: attributes];
                [lines addObject: lineValue];
            }
            along += thickness;
            if (!lastLine)
                along += lineSpacing;
            lineStart = lineEnd;
        }
        free(acrossLengths);
        along += horizontal ? inset.right : inset.bottom;
    }
    _contentSize = horizontal ? NSMakeSize(along, extent) : NSMakeSize(extent, along);
}

- (void) invalidateLayoutWithContext: (NSCollectionViewLayoutInvalidationContext *) context {
    [self _discardLayout];
    [super invalidateLayoutWithContext: context];
}

- (NSSize) collectionViewContentSize {
    [self _prepareIfNeeded];
    return _contentSize;
}

- (NSArray *) layoutAttributesForElementsInRect: (NSRect) rect {
    [self _prepareIfNeeded];
    NSMutableArray *result = [NSMutableArray array];
    for (NSArray *items in _sectionItems)
        for (NSCollectionViewLayoutAttributes *attributes in items)
            if (NSIntersectsRect(attributes.frame, rect))
                [result addObject: [[attributes copy] autorelease]];
    return result;
}

- (NSCollectionViewLayoutAttributes *) _attributesAtIndexPath: (NSIndexPath *) path {
    [self _prepareIfNeeded];
    if (path == nil || path.section < 0 || path.section >= (NSInteger) [_sectionItems count])
        return nil;
    NSArray *items = [_sectionItems objectAtIndex: path.section];
    if (path.item < 0 || path.item >= (NSInteger) [items count])
        return nil;
    return [items objectAtIndex: path.item];
}

- (NSCollectionViewLayoutAttributes *) layoutAttributesForItemAtIndexPath: (NSIndexPath *) indexPath {
    return [[[self _attributesAtIndexPath: indexPath] copy] autorelease];
}

// Before an item that starts a line, the gap lies before it and is one interitem spacing long; before any other
// item, it spans from the previous item to this one; after the section's last item, it lies after that item.
- (NSCollectionViewLayoutAttributes *) layoutAttributesForInterItemGapBeforeIndexPath: (NSIndexPath *) indexPath {
    [self _prepareIfNeeded];
    if (indexPath == nil || indexPath.section < 0 || indexPath.section >= (NSInteger) [_sectionItems count])
        return nil;
    NSArray *items = [_sectionItems objectAtIndex: indexPath.section];
    NSArray *lines = [_sectionLines objectAtIndex: indexPath.section];
    NSInteger count = [items count], index = indexPath.item;
    if (index < 0 || index > count)
        return nil;

    BOOL horizontal = [self _isHorizontal];
    if (count == 0)
        return [self _gapInEmptySection: indexPath.section];
    CGFloat spacing = [self _interitemSpacingForSection: indexPath.section delegate: [self.collectionView delegate]];
    FlowRect gap;
    if (index == count) {
        FlowRect previous = flowFromRect([[items objectAtIndex: index - 1] frame], horizontal);
        FlowRect line = flowFromRect([[lines objectAtIndex: index - 1] rectValue], horizontal);
        gap = (FlowRect){previous.across + previous.acrossLength, line.along, spacing, line.alongLength};
    } else {
        FlowRect item = flowFromRect([[items objectAtIndex: index] frame], horizontal);
        NSValue *lineValue = [lines objectAtIndex: index];
        FlowRect line = flowFromRect([lineValue rectValue], horizontal);
        if (index == 0 || ![[lines objectAtIndex: index - 1] isEqual: lineValue])
            gap = (FlowRect){item.across - spacing, line.along, spacing, line.alongLength};
        else {
            FlowRect previous = flowFromRect([[items objectAtIndex: index - 1] frame], horizontal);
            CGFloat from = previous.across + previous.acrossLength;
            gap = (FlowRect){from, line.along, item.across - from, line.alongLength};
        }
    }
    NSCollectionViewLayoutAttributes *attributes = [[[self class] layoutAttributesClass]
        layoutAttributesForInterItemGapBeforeIndexPath: indexPath];
    attributes.frame = rectFromFlow(gap, horizontal);
    return attributes;
}

// QUIRK: in a section with no items, the gap before its first item is one item size at the leading edge (across
// position 0, not the section inset) and at the section's start along the flow. Not documented. Evidence: black-box
// observation on macOS 26.7.3 and 10.14.4 (tier 2; darling's tests/collection-view.md).
// Test: uifoundation_flow_layout.m test_drop_target_in_empty_sections
- (NSCollectionViewLayoutAttributes *) _gapInEmptySection: (NSInteger) section {
    BOOL horizontal = [self _isHorizontal];
    NSIndexPath *path = [NSIndexPath indexPathForItem: 0 inSection: section];
    NSSize size = [self _sizeForItemAtIndexPath: path delegate: [self.collectionView delegate]];
    FlowRect gap = {0, [[_sectionStarts objectAtIndex: section] doubleValue], horizontal ? size.height : size.width,
                    horizontal ? size.width : size.height};
    NSCollectionViewLayoutAttributes *attributes = [[[self class] layoutAttributesClass]
        layoutAttributesForInterItemGapBeforeIndexPath: path];
    attributes.frame = rectFromFlow(gap, horizontal);
    return attributes;
}

// An item's attributes when the point is on it. Otherwise the gap where an item dropped there would go: within a
// line, before the first item whose middle lies past the point; between lines, before the first item of the next.
- (NSCollectionViewLayoutAttributes *) layoutAttributesForDropTargetAtPoint: (NSPoint) point {
    [self _prepareIfNeeded];
    BOOL horizontal = [self _isHorizontal];
    CGFloat across = horizontal ? point.y : point.x, along = horizontal ? point.x : point.y;
    NSInteger lastSection = -1;

    for (NSInteger section = 0; section < (NSInteger) [_sectionItems count]; section++) {
        NSArray *items = [_sectionItems objectAtIndex: section];
        NSArray *lines = [_sectionLines objectAtIndex: section];
        NSInteger count = [items count];
        if (count > 0)
            lastSection = section;
        for (NSInteger i = 0; i < count; i++) {
            NSCollectionViewLayoutAttributes *attributes = [items objectAtIndex: i];
            if (NSPointInRect(point, attributes.frame))
                return [[attributes copy] autorelease];
        }
        for (NSInteger i = 0; i < count;) {
            NSValue *lineValue = [lines objectAtIndex: i];
            FlowRect line = flowFromRect([lineValue rectValue], horizontal);
            NSInteger end = i;
            while (end < count && [[lines objectAtIndex: end] isEqual: lineValue])
                end++;
            NSInteger before = -1;
            if (along < line.along)
                before = i;
            else if (along < line.along + line.alongLength) {
                before = end;
                for (NSInteger k = i; k < end; k++) {
                    FlowRect item = flowFromRect([[items objectAtIndex: k] frame], horizontal);
                    if (item.across + item.acrossLength / 2 > across) {
                        before = k;
                        break;
                    }
                }
            }
            if (before >= 0)
                return [self layoutAttributesForInterItemGapBeforeIndexPath: [NSIndexPath indexPathForItem: before
                                                                                                  inSection: section]];
            i = end;
        }
    }
    // With no items at all, a drop goes before the first item of the first section
    if (lastSection < 0)
        return [_sectionItems count] > 0
                   ? [self layoutAttributesForInterItemGapBeforeIndexPath: [NSIndexPath indexPathForItem: 0 inSection: 0]]
                   : nil;
    NSInteger count = [[_sectionItems objectAtIndex: lastSection] count];
    return [self layoutAttributesForInterItemGapBeforeIndexPath: [NSIndexPath indexPathForItem: count
                                                                                      inSection: lastSection]];
}

// QUIRK: a vertical flow is invalidated by a new width or horizontal origin, a horizontal one by a new height or
// vertical origin; scrolling along the flow and growing along it change nothing. Not documented. Evidence: black-box observation on macOS 26.7.3 and 10.14.4 (tier 2; darling's tests/collection-view.md).
// Test: uifoundation_flow_layout.m test_cross_axis_change_invalidates
- (BOOL) shouldInvalidateLayoutForBoundsChange: (NSRect) newBounds {
    NSRect bounds = [self.collectionView bounds];
    if ([self _isHorizontal])
        return NSHeight(newBounds) != NSHeight(bounds) || NSMinY(newBounds) != NSMinY(bounds);
    return NSWidth(newBounds) != NSWidth(bounds) || NSMinX(newBounds) != NSMinX(bounds);
}

@end
