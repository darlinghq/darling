/*
 This file is part of Darling.

 Copyright (C) 2021-2026 Lubos Dolezel

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

#import <AppKit/NSCollectionViewLayoutAttributes.h>

@interface NSCollectionViewLayoutAttributes ()
- (instancetype) _initWithIndexPath: (NSIndexPath *) indexPath
                           category: (NSCollectionElementCategory) category
                               kind: (NSString *) kind;
@end

@implementation NSCollectionViewLayoutAttributes

- (instancetype) _initWithIndexPath: (NSIndexPath *) indexPath
                           category: (NSCollectionElementCategory) category
                               kind: (NSString *) kind
{
    if ((self = [super init]) != nil) {
        _alpha = 1;
        _indexPath = [indexPath copy];
        _representedElementCategory = category;
        _representedElementKind = [kind copy];
    }
    return self;
}

- (instancetype) init {
    return [self _initWithIndexPath: nil category: NSCollectionElementCategoryItem kind: nil];
}

+ (instancetype) layoutAttributesForItemWithIndexPath: (NSIndexPath *) indexPath {
    return [[[self alloc] _initWithIndexPath: indexPath category: NSCollectionElementCategoryItem kind: nil] autorelease];
}

+ (instancetype) layoutAttributesForInterItemGapBeforeIndexPath: (NSIndexPath *) indexPath {
    return [[[self alloc] _initWithIndexPath: indexPath
                                    category: NSCollectionElementCategoryInterItemGap
                                        kind: NSCollectionElementKindInterItemGapIndicator] autorelease];
}

+ (instancetype) layoutAttributesForSupplementaryViewOfKind: (NSCollectionViewSupplementaryElementKind) elementKind
                                              withIndexPath: (NSIndexPath *) indexPath
{
    return [[[self alloc] _initWithIndexPath: indexPath
                                    category: NSCollectionElementCategorySupplementaryView
                                        kind: elementKind] autorelease];
}

+ (instancetype) layoutAttributesForDecorationViewOfKind: (NSCollectionViewDecorationElementKind) decorationViewKind
                                           withIndexPath: (NSIndexPath *) indexPath
{
    return [[[self alloc] _initWithIndexPath: indexPath
                                    category: NSCollectionElementCategoryDecorationView
                                        kind: decorationViewKind] autorelease];
}

- (void) dealloc {
    [_indexPath release];
    [_representedElementKind release];
    [super dealloc];
}

- (id) copyWithZone: (NSZone *) zone {
    NSCollectionViewLayoutAttributes *copy = [[[self class] allocWithZone: zone] _initWithIndexPath: _indexPath
                                                                                          category: _representedElementCategory
                                                                                              kind: _representedElementKind];
    copy->_frame = _frame;
    copy->_alpha = _alpha;
    copy->_zIndex = _zIndex;
    copy->_hidden = _hidden;
    return copy;
}

- (BOOL) isEqual: (id) other {
    if (other == self)
        return YES;
    if (![other isKindOfClass: [NSCollectionViewLayoutAttributes class]])
        return NO;
    NSCollectionViewLayoutAttributes *o = other;
    return NSEqualRects(_frame, o->_frame) && _alpha == o->_alpha && _zIndex == o->_zIndex && _hidden == o->_hidden
        && _representedElementCategory == o->_representedElementCategory
        && (_indexPath == o->_indexPath || [_indexPath isEqual: o->_indexPath])
        && (_representedElementKind == o->_representedElementKind || [_representedElementKind isEqual: o->_representedElementKind]);
}

- (NSUInteger) hash {
    return [_indexPath hash] ^ ((NSUInteger) _representedElementCategory << 8);
}

- (NSRect) frame {
    return _frame;
}

- (void) setFrame: (NSRect) frame {
    _frame = frame;
}

- (NSSize) size {
    return _frame.size;
}

// QUIRK: setting the size keeps the frame's center, not its origin. Not documented. Evidence: black-box observation
// on macOS 26.7.3 and 10.14.4 (tier 2; tests/collection-view.md).
// Test: tests/src/uifoundation_collection_layout_attributes.m (test_size_follows_frame)
- (void) setSize: (NSSize) size {
    _frame.origin.x += (_frame.size.width - size.width) / 2;
    _frame.origin.y += (_frame.size.height - size.height) / 2;
    _frame.size = size;
}

- (CGFloat) alpha {
    return _alpha;
}

- (void) setAlpha: (CGFloat) alpha {
    _alpha = alpha;
}

- (NSInteger) zIndex {
    return _zIndex;
}

- (void) setZIndex: (NSInteger) zIndex {
    _zIndex = zIndex;
}

- (BOOL) isHidden {
    return _hidden;
}

- (void) setHidden: (BOOL) hidden {
    _hidden = hidden;
}

- (NSIndexPath *) indexPath {
    return _indexPath;
}

- (void) setIndexPath: (NSIndexPath *) indexPath {
    indexPath = [indexPath copy];
    [_indexPath release];
    _indexPath = indexPath;
}

- (NSCollectionElementCategory) representedElementCategory {
    return _representedElementCategory;
}

- (NSString *) representedElementKind {
    return _representedElementKind;
}

- (NSString *) description {
    return [NSString stringWithFormat: @"<%@: %p; index path: %@; frame: %@; category %ld>", [self class], self, _indexPath,
                                       NSStringFromRect(_frame), (long) _representedElementCategory];
}

@end
