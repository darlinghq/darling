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

#import "FinderItemActions.h"
#import "FinderFileOperation.h"
#import <AppKit/NSWorkspace.h>
#include <errno.h>
#include <sys/stat.h>

static NSError *FinderNameError(NSInteger code, NSString *description, NSString *suggestion)
{
    NSMutableDictionary *info = [NSMutableDictionary dictionaryWithObject: description forKey: NSLocalizedDescriptionKey];
    if (suggestion)
        info[NSLocalizedRecoverySuggestionErrorKey] = suggestion;
    return [NSError errorWithDomain: NSCocoaErrorDomain code: code userInfo: info];
}

static BOOL FinderIsDirectory(NSURL *url)
{
    struct stat st;
    return lstat([url fileSystemRepresentation], &st) == 0 && S_ISDIR(st.st_mode);
}

@implementation FinderItemActions

+ (NSURL *)createFolderIn:(NSURL *)folder error:(NSError **)error
{
    NSURL *url = [folder URLByAppendingPathComponent: @"untitled folder" isDirectory: YES];
    for (int attempt = 0; attempt < 100; attempt++) {
        struct stat st;
        if (lstat([url fileSystemRepresentation], &st) == 0)
            url = [FinderFileOperation keepBothURLForItem: url inFolder: folder];
        NSError *failure = nil;
        if ([[NSFileManager defaultManager] createDirectoryAtURL: url withIntermediateDirectories: NO attributes: nil
                                                           error: &failure])
            return url;
        // Taken since it was checked: try the next name; anything else is the answer
        NSError *underlying = failure.userInfo[NSUnderlyingErrorKey];
        if (!(failure.code == NSFileWriteFileExistsError || ([underlying.domain isEqual: NSPOSIXErrorDomain] && underlying.code == EEXIST))) {
            if (error)
                *error = failure;
            return nil;
        }
    }
    if (error)
        *error = [NSError errorWithDomain: NSCocoaErrorDomain code: NSFileWriteFileExistsError userInfo: nil];
    return nil;
}

+ (NSError *)problemWithName:(NSString *)name forItem:(NSURL *)item
{
    if ([[name stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceAndNewlineCharacterSet]] length] == 0)
        return FinderNameError(NSFileWriteInvalidFileNameError, @"Please enter a name.", nil);
    if ([name hasPrefix: @"."])
        return FinderNameError(NSFileWriteInvalidFileNameError,
                               @"Names that begin with a dot “.” are reserved for the system.", nil);
    if ([name rangeOfCharacterFromSet: [NSCharacterSet characterSetWithCharactersInString: @":/"]].location != NSNotFound)
        return FinderNameError(NSFileWriteInvalidFileNameError,
                               [NSString stringWithFormat: @"The name “%@” can't be used.", name],
                               @"Try using a name with fewer characters, or with no punctuation marks.");

    // Taken: the file system finds another item under the name (it decides about case and normalization)
    NSURL *target = [[item URLByDeletingLastPathComponent] URLByAppendingPathComponent: name];
    struct stat existing, own;
    if (lstat([target fileSystemRepresentation], &existing) == 0
        && !(lstat([item fileSystemRepresentation], &own) == 0 && own.st_dev == existing.st_dev && own.st_ino == existing.st_ino))
        return FinderNameError(NSFileWriteFileExistsError,
                               [NSString stringWithFormat: @"The name “%@” is already taken. Please choose a different name.", name],
                               nil);
    return nil;
}

+ (BOOL)name:(NSString *)name changesExtensionOf:(NSURL *)item from:(NSString **)oldExtension to:(NSString **)newExtension
{
    if (FinderIsDirectory(item))
        return NO;
    NSString *from = [[item lastPathComponent] pathExtension];
    NSString *to = [name pathExtension];
    if ([from length] == 0 || [to length] == 0 || [from isEqual: to])
        return NO;
    if (oldExtension)
        *oldExtension = from;
    if (newExtension)
        *newExtension = to;
    return YES;
}

+ (NSURL *)renameItem:(NSURL *)item to:(NSString *)name error:(NSError **)error
{
    NSError *problem = [self problemWithName: name forItem: item];
    if (problem) {
        if (error)
            *error = problem;
        return nil;
    }
    NSFileManager *fm = [NSFileManager defaultManager];
    NSURL *folder = [item URLByDeletingLastPathComponent];
    NSURL *target = [folder URLByAppendingPathComponent: name];
    if ([[item lastPathComponent] isEqual: name])
        return item;

    // A name the file system takes as the item's own (a case-only change on a case-insensitive volume) goes through
    // another name, since renaming an item to itself changes nothing
    struct stat st;
    if (lstat([target fileSystemRepresentation], &st) == 0) {
        NSURL *staged = [folder URLByAppendingPathComponent: FinderStagedName(name)];
        if (![fm moveItemAtURL: item toURL: staged error: error])
            return nil;
        if (![fm moveItemAtURL: staged toURL: target error: error]) {
            [fm moveItemAtURL: staged toURL: item error: NULL];
            return nil;
        }
        return target;
    }
    return [fm moveItemAtURL: item toURL: target error: error] ? target : nil;
}

+ (void)trashItems:(NSArray<NSURL *> *)items
        completion:(void (^)(NSDictionary<NSURL *, NSURL *> *trashed, NSError *error))completion
{
    [[NSWorkspace sharedWorkspace] recycleURLs: items completionHandler: ^(NSDictionary<NSURL *, NSURL *> *trashed, NSError *error) {
        if ([NSThread isMainThread])
            completion(trashed, error);
        else
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(trashed, error);
            });
    }];
}

@end
