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

// Finder's Get Info panel: an item's name, kind, size, where it is, and when it was created and modified. Read-only.
// A folder's size is counted off the main thread; the panel says "Calculating…" until then.
@interface FinderInfoPanelController : NSWindowController

// Shows the item's panel; one per item, so asking again brings the open one forward. It is released when closed.
+ (FinderInfoPanelController *)showInfoForItem:(NSURL *)item;

@property (readonly) NSURL *item;

// The row's value as shown; rows are name, kind, size, where, created and modified
- (NSString *)valueForRow:(NSString *)row;

// A folder's size is still being counted
@property (readonly, getter=isCounting) BOOL counting;

@end
