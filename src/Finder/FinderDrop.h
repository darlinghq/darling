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

// What a drop of files on a Finder window or sidebar does; shared by the icon view, the list view and the sidebar.

// The folder a drop on item goes into: a folder itself, or the folder a symlink resolves to. nil for an item that
// takes no drops (a file, a package, a symlink to either, a dangling symlink).
NSURL *FinderDropFolderForItem(NSURL *item);

// The file URLs on the pasteboard, or nil when there are none
NSArray<NSURL *> *FinderDraggedFileURLs(NSPasteboard *pasteboard);

// What dropping the items into folder does, given the dragging source's mask: Copy alone (Option) copies, Generic
// alone (Command) moves; otherwise items on the folder's volume move and others are copied. A move is Move, or
// Generic when the source offers only that. None for a read-only folder, a folder that is one of the items or inside
// one, a move onto the items' own folder, and anything the source doesn't offer.
NSDragOperation FinderDropOperation(NSArray<NSURL *> *items, NSURL *folder, NSDragOperation sourceMask);

// The transfer a drop operation asks for: Copy for NSDragOperationCopy, Move otherwise
FinderTransferKind FinderTransferKindForDropOperation(NSDragOperation operation);
