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

#import <AppKit/NSCollectionViewLayout.h>
#import <AppKit/NSCollectionViewLayoutAttributes.h>
#import <AppKit/NSCollectionViewLayoutInvalidationContext.h>
#import "NSCollectionViewLayoutPrivate.h"

NSCollectionViewSupplementaryElementKind const
        NSCollectionElementKindInterItemGapIndicator =
                @"NSCollectionElementKindInterItemGapIndicator";

// Exported by UIFoundation but not declared in the public headers.
APPKIT_EXPORT NSCollectionViewSupplementaryElementKind const NSCollectionElementKindInterSectionGapIndicator;
APPKIT_EXPORT NSCollectionViewSupplementaryElementKind const NSCollectionElementKindItem;
NSCollectionViewSupplementaryElementKind const NSCollectionElementKindInterSectionGapIndicator =
        @"NSCollectionElementKindInterSectionGapIndicator";
NSCollectionViewSupplementaryElementKind const NSCollectionElementKindItem =
        @"NSCollectionElementKindItem";

@implementation NSCollectionViewLayout

- (instancetype) initWithCoder: (NSCoder *) coder {
    return [self init];
}

- (void) encodeWithCoder: (NSCoder *) coder {
}

- (void) dealloc {
    [_decorationViews release];
    [super dealloc];
}

- (NSCollectionView *) collectionView {
    return _collectionView;
}

- (void) _setCollectionView: (NSCollectionView *) collectionView {
    _collectionView = collectionView;
}

// QUIRK: -invalidateLayout passes a plain context, which marks nothing as invalid; only a collection view's
// reloadData does. Not documented. Evidence: black-box observation on macOS 26.7.3 and 10.14.4 (tier 2; darling's tests/collection-view.md).
// Test: uifoundation_collection_layout_contract.m test_invalidate_layout_passes_plain_context
- (void) invalidateLayout {
    NSCollectionViewLayoutInvalidationContext *context = [[[[self class] invalidationContextClass] alloc] init];
    [self invalidateLayoutWithContext: context];
    [context release];
}

// The collection view lays its items out again on its next layout pass
- (void) invalidateLayoutWithContext: (NSCollectionViewLayoutInvalidationContext *) context {
    id view = _collectionView;
    if ([view respondsToSelector: @selector(_collectionViewLayout:didInvalidateWithContext:)])
        [view _collectionViewLayout: self didInvalidateWithContext: context];
}

- (void) registerClass: (Class) viewClass forDecorationViewOfKind: (NSCollectionViewDecorationElementKind) elementKind {
    if (_decorationViews == nil)
        _decorationViews = [[NSMutableDictionary alloc] init];
    if (viewClass != Nil)
        [_decorationViews setObject: viewClass forKey: elementKind];
    else
        [_decorationViews removeObjectForKey: elementKind];
}

- (void) registerNib: (NSNib *) nib forDecorationViewOfKind: (NSCollectionViewDecorationElementKind) elementKind {
    if (_decorationViews == nil)
        _decorationViews = [[NSMutableDictionary alloc] init];
    if (nib != nil)
        [_decorationViews setObject: nib forKey: elementKind];
    else
        [_decorationViews removeObjectForKey: elementKind];
}

- (id) _decorationViewSourceOfKind: (NSCollectionViewDecorationElementKind) elementKind {
    return [_decorationViews objectForKey: elementKind];
}

+ (Class) layoutAttributesClass {
    return [NSCollectionViewLayoutAttributes class];
}

+ (Class) invalidationContextClass {
    return [NSCollectionViewLayoutInvalidationContext class];
}

- (void) prepareLayout {
}

- (NSArray *) layoutAttributesForElementsInRect: (NSRect) rect {
    return [NSArray array];
}

- (NSCollectionViewLayoutAttributes *) layoutAttributesForItemAtIndexPath: (NSIndexPath *) indexPath {
    return nil;
}

- (NSCollectionViewLayoutAttributes *) layoutAttributesForSupplementaryViewOfKind: (NSCollectionViewSupplementaryElementKind) elementKind
                                                                      atIndexPath: (NSIndexPath *) indexPath
{
    return nil;
}

- (NSCollectionViewLayoutAttributes *) layoutAttributesForDecorationViewOfKind: (NSCollectionViewDecorationElementKind) elementKind
                                                                   atIndexPath: (NSIndexPath *) indexPath
{
    return nil;
}

- (NSCollectionViewLayoutAttributes *) layoutAttributesForDropTargetAtPoint: (NSPoint) pointInCollectionView {
    return nil;
}

- (NSCollectionViewLayoutAttributes *) layoutAttributesForInterItemGapBeforeIndexPath: (NSIndexPath *) indexPath {
    return nil;
}

- (BOOL) shouldInvalidateLayoutForBoundsChange: (NSRect) newBounds {
    return NO;
}

- (NSCollectionViewLayoutInvalidationContext *) invalidationContextForBoundsChange: (NSRect) newBounds {
    return [[[[[self class] invalidationContextClass] alloc] init] autorelease];
}

- (NSPoint) targetContentOffsetForProposedContentOffset: (NSPoint) proposedContentOffset {
    return proposedContentOffset;
}

- (NSSize) collectionViewContentSize {
    return NSZeroSize;
}

@end
