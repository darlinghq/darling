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
#import "FinderDrop.h"

NSURL *FinderDropFolderForItem(NSURL *item)
{
    NSURL *folder = [item URLByResolvingSymlinksInPath];
    NSDictionary<NSURLResourceKey, id> *values = [folder resourceValuesForKeys:@[ NSURLIsDirectoryKey, NSURLIsPackageKey ] error:NULL];
    if (![values[NSURLIsDirectoryKey] boolValue] || [values[NSURLIsPackageKey] boolValue])
        return nil;
    return [NSURL fileURLWithPath:[folder path] isDirectory:YES];
}

NSArray<NSURL *> *FinderDraggedFileURLs(NSPasteboard *pasteboard)
{
    NSArray<NSURL *> *urls = [pasteboard readObjectsForClasses:@[ [NSURL class] ]
                                                       options:@{ NSPasteboardURLReadingFileURLsOnlyKey : @YES }];
    return [urls count] > 0 ? urls : nil;
}

// The path with symlinks resolved, so that one folder has one spelling
static NSString *FinderResolvedPath(NSURL *url)
{
    return [[[url URLByResolvingSymlinksInPath] path] stringByStandardizingPath];
}

static BOOL FinderFolderIsReadOnly(NSURL *folder)
{
    NSNumber *readOnly = nil;
    [folder getResourceValue:&readOnly forKey:NSURLVolumeIsReadOnlyKey error:NULL];
    return [readOnly boolValue] || ![[NSFileManager defaultManager] isWritableFileAtPath:[folder path]];
}

NSDragOperation FinderDropOperation(NSArray<NSURL *> *items, NSURL *folder, NSDragOperation sourceMask)
{
    if ([items count] == 0 || folder == nil || sourceMask == NSDragOperationNone || FinderFolderIsReadOnly(folder))
        return NSDragOperationNone;

    NSString *destination = FinderResolvedPath(folder);
    BOOL anyInFolder = NO;
    for (NSURL *item in items) {
        // The item's own spelling, not its symlink's target: dragging a link moves the link
        NSString *parent = FinderResolvedPath([item URLByDeletingLastPathComponent]);
        NSString *path = [parent stringByAppendingPathComponent:[item lastPathComponent]];
        if ([destination isEqual:path] || [destination hasPrefix:[path stringByAppendingString:@"/"]])
            return NSDragOperationNone;
        if ([parent isEqual:destination])
            anyInFolder = YES;
    }

    BOOL move;
    if (sourceMask == NSDragOperationCopy)
        move = NO;
    else if (sourceMask == NSDragOperationGeneric)
        move = YES;
    else
        move = [FinderFileOperation defaultKindForItems:items destination:folder] == FinderTransferMove;

    if (move) {
        if (anyInFolder)
            return NSDragOperationNone;
        if (sourceMask & NSDragOperationMove)
            return NSDragOperationMove;
        if (sourceMask & NSDragOperationGeneric)
            return NSDragOperationGeneric;
        return NSDragOperationNone;
    }
    return (sourceMask & NSDragOperationCopy) ? NSDragOperationCopy : NSDragOperationNone;
}

FinderTransferKind FinderTransferKindForDropOperation(NSDragOperation operation)
{
    return operation == NSDragOperationCopy ? FinderTransferCopy : FinderTransferMove;
}
