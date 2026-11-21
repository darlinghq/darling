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

// A Finder window's list view: an outline view with the columns Name (icon and name), Date Modified, Size and Kind
// (identifiers name, modified, size, kind). Folders expand in place; clicking a column header sorts by it.
@interface FinderListViewController : NSViewController <NSOutlineViewDataSource, NSOutlineViewDelegate>

@property (readonly) NSOutlineView *outlineView;

// The folder's items, the top-level rows. Setting them clears the selection and collapses everything.
@property (nonatomic, copy) NSArray<NSURL *> *items;

// Items not shown (in the folder or in an expanded folder) are ignored.
- (void)selectItems:(NSArray<NSURL *> *)items;

// In the order they are shown.
@property (readonly) NSArray<NSURL *> *selectedItems;

// Called with the item when a row is double-clicked.
@property (copy) void (^openItem)(NSURL *item);

@end
