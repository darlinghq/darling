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

@class FinderIconViewController;
@class FinderListViewController;

typedef NS_ENUM(NSInteger, FinderViewStyle) {
    FinderViewStyleIcons,
    FinderViewStyleList,
};

// A Finder window: one folder's items, titled with the folder's display name.
@interface FinderWindowController : NSWindowController <NSWindowDelegate, NSToolbarDelegate>

- (instancetype)initWithFolderURL:(NSURL *)folder;

@property (readonly) NSURL *folderURL;

// Shows another folder in this window, with nothing selected; the folder shown before goes in the history. If the
// folder can't be read, the window stays where it was and an alert sheet says why.
- (void)showFolder:(NSURL *)folder;

// Reads the folder again, for items that appeared or went since it was shown.
- (void)reloadFolder;

// The alert for a folder that can't be opened
+ (NSAlert *)alertForFolder:(NSURL *)folder error:(NSError *)error;

// The history: Go > Back (Command-[) and Forward (Command-]), and the toolbar's back and forward buttons
@property (readonly) BOOL canGoBack;
@property (readonly) BOOL canGoForward;
- (IBAction)goBack:(id)sender;
- (IBAction)goForward:(id)sender;

// Go > Enclosing Folder (Command-Up): the folder's enclosing folder, with the folder selected in it
- (IBAction)goToEnclosingFolder:(id)sender;

// File > Open (Command-O) and Go > Selected Item (Command-Down): each selected item, as if double-clicked
- (IBAction)openSelection:(id)sender;

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

// View > Show/Hide Toolbar (Option-Command-T): the window's toolbar, titled for what choosing it does
- (IBAction)toggleToolbar:(id)sender;

// View > Show/Hide Sidebar (Control-Command-S); a window whose folder's style hides the sidebar opens without it
- (IBAction)toggleSidebar:(id)sender;

// File > Eject (Command-E): ejects the volume the window's folder is on; enabled when it can be ejected
- (IBAction)eject:(id)sender;

// Edit > Copy (Command-C): the selected items' file URLs onto the general pasteboard
- (IBAction)copy:(id)sender;
// Edit > Paste (Command-V): copies the pasteboard's items (those that still exist) into the window's folder
- (IBAction)paste:(id)sender;
// Edit > Move Item Here (Option-Command-V): moves them there
- (IBAction)moveItemHere:(id)sender;
- (IBAction)selectAll:(id)sender;

// File > New Folder (Shift-Command-N): "untitled folder" in the window's folder, selected and being renamed
- (IBAction)newFolder:(id)sender;
// File > Rename, and Return: renames the one selected item in place
- (IBAction)rename:(id)sender;
// File > Move to Trash (Command-Delete)
- (IBAction)moveToTrash:(id)sender;
// File > Get Info (Command-I): a Get Info panel for each selected item
- (IBAction)getInfo:(id)sender;

// Starts renaming the item in place, in the view shown. A name the Finder doesn't allow is refused with an alert;
// changing a file's extension asks first.
- (void)beginRenamingItem:(NSURL *)item;
// The item being renamed; nil when none is
@property (readonly) NSURL *renamingItem;

// The icon view's controller, shown in the icon view style
@property (readonly) FinderIconViewController *iconViewController;
// The list view's, shown in the list view style
@property (readonly) FinderListViewController *listViewController;

// Called with an item the user opened (double-clicked, or clicked in the sidebar).
@property (copy) void (^openItem)(NSURL *item);

@end
