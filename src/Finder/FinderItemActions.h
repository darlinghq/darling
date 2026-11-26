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

#import <Foundation/Foundation.h>

// Finder's one-shot actions on items: a new folder, renaming with the Finder's rules for names, and the Trash.
@interface FinderItemActions : NSObject

// "untitled folder", or "untitled folder 2", … when that is taken
+ (NSURL *)createFolderIn:(NSURL *)folder error:(NSError **)error;

// nil if item may be renamed to name; otherwise why not, as the alert says it (localizedDescription, and
// localizedRecoverySuggestion for punctuation). Refused: an empty or blank name, a ":" or "/", a leading ".", a name
// another item in the folder has (also one differing only in normalization; a case-only change is allowed).
+ (NSError *)problemWithName:(NSString *)name forItem:(NSURL *)item;

// YES if renaming a file to name changes its extension from one to another (adding or removing one doesn't ask),
// with both extensions, without the dot. Folders never ask.
+ (BOOL)name:(NSString *)name changesExtensionOf:(NSURL *)item from:(NSString **)oldExtension to:(NSString **)newExtension;

// The item's new URL, or nil and the error. A case-only change works on case-insensitive volumes too.
+ (NSURL *)renameItem:(NSURL *)item to:(NSString *)name error:(NSError **)error;

// Moves the items to the Trash (NSWorkspace recycleURLs:); the completion is called on the main queue with where they
// went and the error, if any.
+ (void)trashItems:(NSArray<NSURL *> *)items
        completion:(void (^)(NSDictionary<NSURL *, NSURL *> *trashed, NSError *error))completion;

@end
