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

#import <Cocoa/Cocoa.h>

// One preference pane found on disk (a .prefPane bundle).
@interface SPPane : NSObject

// Reads the bundle at the path; nil when it is not a bundle with an identifier.
- (instancetype)initWithPath:(NSString *)path;

@property (readonly, copy) NSString *path;
@property (readonly, copy) NSString *identifier;
// The label under the icon: NSPrefPaneIconLabel, else the bundle name.
@property (readonly, copy) NSString *label;
// The title of the window while the pane is shown: the bundle's (localized) CFBundleName, as on macOS where
// the grid shows the shorter NSPrefPaneIconLabel (which may span lines); the label when there is no name.
@property (readonly, copy) NSString *title;
@property (readonly, strong) NSBundle *bundle;
// The icon from NSPrefPaneIconFile (or CFBundleIconFile); nil when the bundle
// has none or the file cannot be read.
@property (readonly, strong) NSImage *icon;
// Extra words the search matches (NSPrefPaneSearchParameters).
@property (readonly, copy) NSString *searchKeywords;
// The identifier of the group the pane is listed in, see SPPaneCatalog.
@property (copy) NSString *groupIdentifier;

- (BOOL)matchesSearch:(NSString *)text;

@end
