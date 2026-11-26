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

// A Finder window's sidebar. Favorites: Applications, the home folder, and Desktop, Documents and Downloads when they
// exist. Locations: the mounted volumes that are browsable, kept up to date as volumes are mounted and unmounted;
// ejectable ones have an eject button.
@interface FinderSidebarController : NSViewController <NSOutlineViewDataSource, NSOutlineViewDelegate>

// The standard favourites
- (instancetype)init;
// The favourites given instead (tests)
- (instancetype)initWithFavorites:(NSArray<NSURL *> *)favorites;

@property (readonly) NSOutlineView *outlineView;

// Called with the items dropped on an item, the folder they go into and how; carrying it out is up to whoever set it.
// Drops go on items only: not on section titles, nor between items.
@property (copy) void (^dropItems)(NSArray<NSURL *> *items, NSURL *folder, FinderTransferKind kind);

// @[@"Favorites", @"Locations"]
@property (readonly) NSArray<NSString *> *sectionTitles;

- (NSArray<NSURL *> *)itemsInSection:(NSInteger)section;

// The home folder's is the user name, a volume's its name, another folder's its display name.
- (NSString *)titleForItem:(NSURL *)item;

- (BOOL)showsEjectButtonForItem:(NSURL *)item;

// The folder the window shows: its item, if it has one, is the selection (setting it shows nothing), so a click on
// any other item is a change of selection.
@property (nonatomic) NSURL *currentFolder;

// Called with the item the user clicked; set by the window controller.
@property (copy) void (^showFolder)(NSURL *folder);

@end
