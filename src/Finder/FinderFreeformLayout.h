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

#import <AppKit/AppKit.h>

// A layout of items at given centres (in the collection view's flipped coordinates), as a folder's .DS_Store places
// them. Items without a centre go below the lowest placed one, in a grid with the flow layout's spacing, in item
// order. No item is placed above or left of the content. The content is the items' frames with a 20-point margin, and
// at least the visible area.
@interface FinderFreeformLayout : NSCollectionViewLayout
- (instancetype)initWithItemSize:(NSSize)itemSize;
@property (readonly) NSSize itemSize;
// One per item, in item order: [NSValue valueWithPoint:], NSMakePoint(NAN, NAN) for an item without a position.
// Items past the end have none either.
@property (nonatomic, copy) NSArray<NSValue *> *centers;
@end
