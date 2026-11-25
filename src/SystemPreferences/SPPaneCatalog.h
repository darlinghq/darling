/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "SPPane.h"

// The panes the application can show, grouped as in the grid.
//
// Panes are .prefPane bundles in the search directories, in this order:
// /System/Library/PreferencePanes, /Library/PreferencePanes and the user's
// ~/Library/PreferencePanes. When two bundles carry the same identifier, the
// one from the earlier directory is used.
//
// The grid groups come from a property list (NSPrefPaneGroups.plist) with the
// keys "groups" (each with "identifier" and an ordered list of pane
// identifiers "panes") and "catchAllGroupID", the group for every pane no
// other group lists, which is where third-party panes land.
@interface SPPaneCatalog : NSObject

+ (NSArray<NSString *> *)defaultSearchDirectories;

- (instancetype)initWithSearchDirectories:(NSArray<NSString *> *)directories groupsPath:(NSString *)groupsPath;

// Scans the directories again.
- (void)reload;

// Group identifiers in display order; groups without a pane are left out.
@property (readonly, copy) NSArray<NSString *> *groupIdentifiers;
- (NSArray<SPPane *> *)panesInGroup:(NSString *)groupIdentifier;
@property (readonly, copy) NSString *catchAllGroupIdentifier;

@property (readonly, copy) NSArray<SPPane *> *allPanes;
- (SPPane *)paneWithIdentifier:(NSString *)identifier;

// Adds (or returns the known pane for) a bundle that is not in a search
// directory, such as a .prefPane opened as a document; it is listed with the
// catch-all group. Returns nil for something that is not a pane bundle.
- (SPPane *)paneByAddingBundleAtPath:(NSString *)path;

@end
