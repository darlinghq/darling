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

typedef NS_ENUM(NSInteger, FinderTransferKind) {
    FinderTransferCopy,
    FinderTransferMove,
};

typedef NS_ENUM(NSInteger, FinderConflictChoice) {
    FinderConflictReplace,
    FinderConflictKeepBoth,
    FinderConflictStop,
};

// Copies or moves items into a folder, off the main thread.
// - Every copy is made under a hidden name in the destination (FinderStagedName) and given its name only once
//   complete, so stopping or a failure never leaves part of an item. A Replace sets the item replaced aside under
//   such a name, and removes it only once the new item has its name (back in its place if that fails).
// - A move within a volume renames; a move across volumes copies, then removes the source; a move out of a read-only
//   volume, or of a volume itself, is a copy.
// - An item copied into its own folder is named "<name> copy"; one moved into its own folder stays where it is.
// - A name taken in the destination is a conflict, answered by resolveConflict.
// A hidden name in the destination that stands for name while an item is staged: ".<name>.finder-<random>", with
// name cut to its first 64 bytes (whole characters), so it fits whatever name the file system takes
NSString *FinderStagedName(NSString *name);

@interface FinderFileOperation : NSObject

- (instancetype)initWithKind:(FinderTransferKind)kind items:(NSArray<NSURL *> *)items destination:(NSURL *)folder;

// After -start, a move from a read-only volume reads Copy.
@property (readonly) FinderTransferKind kind;
@property (readonly) NSArray<NSURL *> *items;
@property (readonly) NSURL *destination;

// The three blocks are called on the main queue. resolveConflict may set *applyToAll, and is then not called again;
// without it, a conflict stops the operation.
@property (copy) FinderConflictChoice (^resolveConflict)(NSURL *source, NSURL *existing, BOOL *applyToAll);
@property (copy) void (^progress)(int64_t doneBytes, int64_t totalBytes, NSURL *current);
// results: the items at their final URLs, in item order (those done before a stop or a failure). error: nil; or
// NSCocoaErrorDomain NSUserCancelledError after Stop or -cancel; or the failure, with NSFilePathErrorKey set to the
// item.
@property (copy) void (^completion)(NSArray<NSURL *> *results, NSError *error);

// Once; the work runs on a private serial queue.
- (void)start;
// From any thread.
- (void)cancel;
@property (readonly, getter=isFinished) BOOL finished;

// Keep Both: " 2", " 3", … before the last extension, the first name not taken in folder.
+ (NSURL *)keepBothURLForItem:(NSURL *)item inFolder:(NSURL *)folder;
// A copy into the item's own folder: "<name> copy", "<name> copy 2", …, before the last extension.
+ (NSURL *)copyURLForItem:(NSURL *)item inFolder:(NSURL *)folder;
// Move when every item is on folder's volume, Copy otherwise.
+ (FinderTransferKind)defaultKindForItems:(NSArray<NSURL *> *)items destination:(NSURL *)folder;
// nil if the transfer may go ahead; otherwise why not (a folder into itself or one of its subfolders).
+ (NSError *)refusalForItems:(NSArray<NSURL *> *)items destination:(NSURL *)folder kind:(FinderTransferKind)kind;

@end
