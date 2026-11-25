/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "FinderFreeformLayout.h"
#include <math.h>

// The flow layout's spacing in the icon view
static const CGFloat FinderGridInset = 12, FinderGridSpacing = 8, FinderContentMargin = 20;

@implementation FinderFreeformLayout {
    NSMutableArray<NSValue *> *_frames;
    NSSize _contentSize;
}

- (instancetype)initWithItemSize:(NSSize)itemSize
{
    if ((self = [super init])) {
        _itemSize = itemSize;
        _frames = [NSMutableArray array];
    }
    return self;
}

- (void)setCenters:(NSArray<NSValue *> *)centers
{
    _centers = [centers copy];
    [self invalidateLayout];
}

- (NSSize)visibleSize
{
    NSCollectionView *view = self.collectionView;
    NSScrollView *scroll = [view enclosingScrollView];
    return scroll != nil ? [scroll contentSize] : [view bounds].size;
}

- (void)prepareLayout
{
    [super prepareLayout];
    [_frames removeAllObjects];
    NSCollectionView *view = self.collectionView;
    NSInteger count = view != nil && [view numberOfSections] > 0 ? [view numberOfItemsInSection:0] : 0;
    NSSize visible = [self visibleSize];

    CGFloat width = _itemSize.width, height = _itemSize.height;
    CGFloat right = 0, bottom = 0;
    BOOL anyPlaced = NO;
    NSMutableIndexSet *unplaced = [NSMutableIndexSet indexSet];
    for (NSInteger i = 0; i < count; i++) {
        NSPoint center = (NSUInteger)i < [_centers count] ? [_centers[i] pointValue] : NSMakePoint(NAN, NAN);
        if (!isfinite(center.x) || !isfinite(center.y)) {
            [_frames addObject:[NSValue valueWithRect:NSZeroRect]];
            [unplaced addIndex:i];
            continue;
        }
        NSRect frame = NSMakeRect(MAX(0, center.x - width / 2), MAX(0, center.y - height / 2), width, height);
        [_frames addObject:[NSValue valueWithRect:frame]];
        right = MAX(right, NSMaxX(frame));
        bottom = MAX(bottom, NSMaxY(frame));
        anyPlaced = YES;
    }

    NSInteger columns = MAX(1, (NSInteger)floor((visible.width - 2 * FinderGridInset + FinderGridSpacing) / (width + FinderGridSpacing)));
    CGFloat top = anyPlaced ? bottom + FinderGridSpacing : FinderGridInset;
    __block NSInteger placed = 0;
    [unplaced enumerateIndexesUsingBlock:^(NSUInteger index, BOOL *stop) {
        NSRect frame = NSMakeRect(FinderGridInset + (placed % columns) * (width + FinderGridSpacing),
                                  top + (placed / columns) * (height + FinderGridSpacing), width, height);
        self->_frames[index] = [NSValue valueWithRect:frame];
        placed++;
    }];
    for (NSValue *value in _frames) {
        right = MAX(right, NSMaxX([value rectValue]));
        bottom = MAX(bottom, NSMaxY([value rectValue]));
    }
    _contentSize = NSMakeSize(MAX(visible.width, count > 0 ? right + FinderContentMargin : 0),
                              MAX(visible.height, count > 0 ? bottom + FinderContentMargin : 0));
}

- (NSSize)collectionViewContentSize
{
    return _contentSize;
}

- (NSCollectionViewLayoutAttributes *)attributesForItem:(NSUInteger)index
{
    NSCollectionViewLayoutAttributes *attributes = [NSCollectionViewLayoutAttributes
        layoutAttributesForItemWithIndexPath:[NSIndexPath indexPathForItem:index inSection:0]];
    attributes.frame = [_frames[index] rectValue];
    return attributes;
}

- (NSArray<__kindof NSCollectionViewLayoutAttributes *> *)layoutAttributesForElementsInRect:(NSRect)rect
{
    NSMutableArray *result = [NSMutableArray array];
    for (NSUInteger i = 0; i < [_frames count]; i++) {
        if (NSIntersectsRect([_frames[i] rectValue], rect))
            [result addObject:[self attributesForItem:i]];
    }
    return result;
}

- (NSCollectionViewLayoutAttributes *)layoutAttributesForItemAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section != 0 || indexPath.item < 0 || (NSUInteger)indexPath.item >= [_frames count])
        return nil;
    return [self attributesForItem:indexPath.item];
}

// A drop goes on the item under the point; anywhere else, after the last item. Gaps take no room, as items aren't
// placed in an order a drop could go between.
- (NSCollectionViewLayoutAttributes *)layoutAttributesForDropTargetAtPoint:(NSPoint)point
{
    for (NSUInteger i = 0; i < [_frames count]; i++) {
        if (NSPointInRect(point, [_frames[i] rectValue]))
            return [self attributesForItem:i];
    }
    return [self layoutAttributesForInterItemGapBeforeIndexPath:[NSIndexPath indexPathForItem:[_frames count] inSection:0]];
}

- (NSCollectionViewLayoutAttributes *)layoutAttributesForInterItemGapBeforeIndexPath:(NSIndexPath *)indexPath
{
    NSCollectionViewLayoutAttributes *gap = [NSCollectionViewLayoutAttributes layoutAttributesForInterItemGapBeforeIndexPath:indexPath];
    gap.frame = NSZeroRect;
    return gap;
}

// The grid of unplaced items, and the content's minimum size, follow the visible width and height
- (BOOL)shouldInvalidateLayoutForBoundsChange:(NSRect)newBounds
{
    return YES;
}

@end
