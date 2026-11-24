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

// Edits the item's name in its row, the file name in it with the name before the extension selected; NO if the item
// isn't shown. Ending the edit calls renameItem with the name typed; renaming is up to whoever set it. An edit the
// table ends without a name (aborted) calls nothing: editing is then NO.
- (BOOL)beginEditingItem:(NSURL *)item;
@property (readonly, getter=isEditing) BOOL editing;
// Ends an edit without renaming: renameItem is called with nil. Setting the items does it too.
- (void)cancelEditing;
// The folder's items again, after a change: expanded folders still there stay expanded, the scroll position stays.
- (void)reloadItems:(NSArray<NSURL *> *)items;
@property (copy) void (^renameItem)(NSURL *item, NSString *name);

// The folder shown: a drop on a file's row, or outside a folder's rows, goes there
@property (copy) NSURL *folderURL;

// Called with the items dropped, the folder they go into and how; carrying it out is up to whoever set it. Rows are
// dragged as their items' file URLs, and file URLs are taken from other applications too.
@property (copy) void (^dropItems)(NSArray<NSURL *> *items, NSURL *folder, FinderTransferKind kind);

@end
