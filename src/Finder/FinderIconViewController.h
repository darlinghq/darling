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

@class FinderFolderStyle;

// A Finder window's icon view: the folder's items in a collection view with a flow layout, each a 64-point icon with
// its name below in up to two lines; or, for a folder with a style, as the style says.
@interface FinderIconViewController : NSViewController <NSCollectionViewDataSource, NSCollectionViewDelegate>

@property (readonly) NSCollectionView *collectionView;

// The items shown, in order. Setting them clears the selection.
@property (nonatomic, copy) NSArray<NSURL *> *items;

// Items not among the shown ones are ignored.
- (void)selectItems:(NSArray<NSURL *> *)items;

// In the order they are shown.
@property (readonly) NSArray<NSURL *> *selectedItems;

// Shows the items as the folder's style says: at their positions (FinderFreeformLayout), with its icon size, text size,
// label position and background. nil gives back the flow layout and the standard look.
- (void)applyStyle:(FinderFolderStyle *)style;

// Shows other items, in that style, at once
- (void)setItems:(NSArray<NSURL *> *)items style:(FinderFolderStyle *)style;

// Called with the item when one is double-clicked.
@property (copy) void (^openItem)(NSURL *item);

@end
