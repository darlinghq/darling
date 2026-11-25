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
#import "FinderWindowController.h"

NS_ASSUME_NONNULL_BEGIN

// How Finder shows a folder, from the settings in its .DS_Store: the window, the view, the icons, the background and
// the items' positions. Settings that are missing, or stored with an unexpected type, have their defaults.
@interface FinderFolderStyle : NSObject
// The folder's style, with the volume from its NSURLVolumeURLKey
+ (nullable instancetype)styleForFolder:(NSURL *)folder;
// nil when the folder has no usable .DS_Store: none, a malformed one, or one with no window or view settings and no
// item positions. A background picture is looked for inside volume only.
+ (nullable instancetype)styleForFolder:(NSURL *)folder volume:(NSURL *)volume;
@property (readonly) NSSize windowContentSize; // NSZeroSize when none is given
@property (readonly) BOOL showsToolbar, showsSidebar; // default YES
@property (readonly) FinderViewStyle viewStyle;
@property (readonly) CGFloat iconSize; // 16 to 512 points, default 64
@property (readonly) CGFloat textSize; // 10 to 16 points, default 12
@property (readonly) BOOL labelOnBottom; // default YES; NO puts it on the right
@property (readonly, nullable) NSColor *backgroundColor;
// A regular file inside the volume; nil when there is none, or the one given is elsewhere or not a file
@property (readonly, nullable) NSURL *backgroundImageURL;
// The centre of the item's icon, with the origin at the top left of the view; NO when it has no position. Names are
// compared case-sensitively, by canonical equivalence.
- (BOOL)getCenter:(NSPoint *)center forItemNamed:(NSString *)name;
@end

NS_ASSUME_NONNULL_END
