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

typedef NS_ENUM(NSInteger, FinderViewStyle) {
    FinderViewStyleIcons,
    FinderViewStyleList,
};

// A Finder window: one folder's items, titled with the folder's display name.
@interface FinderWindowController : NSWindowController <NSWindowDelegate, NSToolbarDelegate>

- (instancetype)initWithFolderURL:(NSURL *)folder;

@property (readonly) NSURL *folderURL;

// Shows another folder in this window, with nothing selected.
- (void)showFolder:(NSURL *)folder;

// Selects these items of the folder; items not in it are ignored.
- (void)selectItems:(NSArray<NSURL *> *)items;

// In the order they are shown.
@property (readonly) NSArray<NSURL *> *selectedItems;

// How the items are shown; a new window shows icons. Switching keeps the selection, and the window keeps its view
// while it shows other folders.
@property (nonatomic) FinderViewStyle viewStyle;

// View > as Icons (Command-1) and as List (Command-2), and the toolbar's view control
- (IBAction)showAsIcons:(id)sender;
- (IBAction)showAsList:(id)sender;

// Called with an item the user opened (double-clicked).
@property (copy) void (^openItem)(NSURL *item);

@end
