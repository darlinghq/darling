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
#import "FinderFileOperation.h"

@class FinderFolderStyle;

// A Finder window's icon view: the folder's items in a collection view with a flow layout, each a 64-point icon with
// its name below in up to two lines; or, for a folder with a style, as the style says.
@interface FinderIconViewController : NSViewController <NSCollectionViewDataSource, NSCollectionViewDelegate, NSTextFieldDelegate>

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

// Edits the item's label in place, its file name in it with the name before the extension selected. Return or ending
// the edit otherwise calls renameItem with the name typed; Escape calls it with nil. The label then shows the item's
// name again; renaming is up to whoever set renameItem. NO if the item has no label to edit.
- (BOOL)beginEditingItem:(NSURL *)item;
@property (readonly, getter=isEditing) BOOL editing;
// Ends an edit without renaming: renameItem is called with nil.
- (void)cancelEditing;
@property (copy) void (^renameItem)(NSURL *item, NSString *name);

// The folder shown: a drop on the background, or on an item that takes none, goes there
@property (copy) NSURL *folderURL;

// Called with the items dropped, the folder they go into and how; carrying it out is up to whoever set it. Items are
// dragged as their file URLs, and file URLs are taken from other applications too.
@property (copy) void (^dropItems)(NSArray<NSURL *> *items, NSURL *folder, FinderTransferKind kind);

@end
