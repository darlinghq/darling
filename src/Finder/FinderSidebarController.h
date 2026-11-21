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

// A Finder window's sidebar. Favorites: Applications, the home folder, and Desktop, Documents and Downloads when they
// exist. Locations: the mounted volumes that are browsable, kept up to date as volumes are mounted and unmounted;
// ejectable ones have an eject button.
@interface FinderSidebarController : NSViewController <NSOutlineViewDataSource, NSOutlineViewDelegate>

// @[@"Favorites", @"Locations"]
@property (readonly) NSArray<NSString *> *sectionTitles;

- (NSArray<NSURL *> *)itemsInSection:(NSInteger)section;

// The home folder's is the user name, a volume's its name, another folder's its display name.
- (NSString *)titleForItem:(NSURL *)item;

- (BOOL)showsEjectButtonForItem:(NSURL *)item;

// Called with the item the user clicked; set by the window controller.
@property (copy) void (^showFolder)(NSURL *folder);

@end
