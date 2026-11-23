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

#import "FinderFileOperation.h"
#include <copyfile.h>
#include <errno.h>
#include <limits.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <sys/stat.h>

// At most this often, plus a last call when the operation ends
static const CFTimeInterval FinderProgressInterval = 0.05;

NSString *FinderStagedName(NSString *name)
{
    NSMutableString *prefix = [NSMutableString string];
    __block NSUInteger bytes = 0;
    [name enumerateSubstringsInRange: NSMakeRange(0, [name length])
                             options: NSStringEnumerationByComposedCharacterSequences
                          usingBlock: ^(NSString *character, NSRange range, NSRange enclosing, BOOL *stop) {
        bytes += [character lengthOfBytesUsingEncoding: NSUTF8StringEncoding];
        if (bytes > 64)
            *stop = YES;
        else
            [prefix appendString: character];
    }];
    return [NSString stringWithFormat: @".%@.finder-%08x", prefix, arc4random_uniform(UINT32_MAX)];
}

// A volume's root (a symlink to one is not)
static BOOL FinderIsVolume(NSURL *url)
{
    NSNumber *isVolume = nil;
    struct stat st;
    if (lstat([url fileSystemRepresentation], &st) != 0 || S_ISLNK(st.st_mode))
        return NO;
    [[url URLByStandardizingPath] getResourceValue: &isVolume forKey: NSURLIsVolumeKey error: NULL];
    return [isVolume boolValue];
}

static BOOL FinderExists(NSString *path)
{
    struct stat st;
    return lstat([path fileSystemRepresentation], &st) == 0;
}

// The path with its folders' symlinks resolved, the item itself not (an item may be a symlink); precomposed, so that
// paths can be compared whatever form their names were given in.
static NSString *FinderCanonicalPath(NSURL *url, BOOL resolveItem)
{
    NSString *path = [[url path] stringByStandardizingPath];
    NSString *folder = resolveItem ? path : [path stringByDeletingLastPathComponent];
    char resolved[PATH_MAX];
    if (realpath([folder fileSystemRepresentation], resolved) != NULL) {
        folder = [[NSFileManager defaultManager] stringWithFileSystemRepresentation: resolved length: strlen(resolved)];
        path = resolveItem ? folder : [folder stringByAppendingPathComponent: [path lastPathComponent]];
    }
    return [path precomposedStringWithCanonicalMapping];
}

// path is inside, or is, folder
static BOOL FinderPathIsWithin(NSString *path, NSString *folder)
{
    return [path isEqual: folder] || [path hasPrefix: [folder hasSuffix: @"/"] ? folder : [folder stringByAppendingString: @"/"]];
}

static BOOL FinderIsDirectory(NSURL *url)
{
    struct stat st;
    return lstat([url fileSystemRepresentation], &st) == 0 && S_ISDIR(st.st_mode);
}

static NSError *FinderError(NSInteger code, NSURL *item, NSString *description, NSError *underlying)
{
    NSMutableDictionary *info = [NSMutableDictionary dictionary];
    if (item)
        info[NSFilePathErrorKey] = [item path];
    if (description)
        info[NSLocalizedDescriptionKey] = description;
    if (underlying)
        info[NSUnderlyingErrorKey] = underlying;
    return [NSError errorWithDomain: NSCocoaErrorDomain code: code userInfo: info];
}

// An errno as the Cocoa error a file operation gives for it, with the POSIX error underneath
static NSError *FinderErrorForErrno(int error, NSURL *item, BOOL writing)
{
    NSInteger code;
    switch (error) {
    case EACCES:
    case EPERM:
        code = writing ? NSFileWriteNoPermissionError : NSFileReadNoPermissionError;
        break;
    case ENOENT:
        code = writing ? NSFileWriteUnknownError : NSFileReadNoSuchFileError;
        break;
    case ENOSPC:
        code = NSFileWriteOutOfSpaceError;
        break;
    case EROFS:
        code = NSFileWriteVolumeReadOnlyError;
        break;
    case EEXIST:
        code = NSFileWriteFileExistsError;
        break;
    default:
        code = writing ? NSFileWriteUnknownError : NSFileReadUnknownError;
        break;
    }
    return FinderError(code, item, nil, [NSError errorWithDomain: NSPOSIXErrorDomain code: error userInfo: nil]);
}

static NSURL *FinderNumberedURL(NSURL *item, NSURL *folder, NSString *word)
{
    NSString *name = [item lastPathComponent];
    NSString *extension = [name pathExtension];
    NSString *base = [extension length] ? [name stringByDeletingPathExtension] : name;
    NSInteger number = 2;
    NSString *suffix = word ? [@" " stringByAppendingString: word] : @"";

    // A name already ending in the suffix and a number counts on from it ("a 2.txt" → "a 3.txt")
    NSRegularExpression *numbered = [NSRegularExpression
        regularExpressionWithPattern: [NSString stringWithFormat: @"^(.*%@) ([0-9]+)$", [NSRegularExpression escapedPatternForString: suffix]]
                             options: 0 error: NULL];
    NSTextCheckingResult *match = [numbered firstMatchInString: base options: 0 range: NSMakeRange(0, [base length])];
    if (match && [[base substringWithRange: [match rangeAtIndex: 2]] length] < 10) {
        number = MAX(2, [[base substringWithRange: [match rangeAtIndex: 2]] integerValue] + 1);
        base = [[base substringWithRange: [match rangeAtIndex: 1]] substringToIndex: [match rangeAtIndex: 1].length - [suffix length]];
    } else if (word && [base hasSuffix: suffix]) {
        base = [base substringToIndex: [base length] - [suffix length]];
    } else if (word) {
        // "<name> copy" first, then "<name> copy 2"
        NSString *candidate = [base stringByAppendingString: suffix];
        if ([extension length])
            candidate = [candidate stringByAppendingPathExtension: extension];
        NSURL *url = [folder URLByAppendingPathComponent: candidate];
        if (!FinderExists([url path]))
            return url;
    }
    for (;; number++) {
        NSString *candidate = [NSString stringWithFormat: @"%@%@ %ld", base, suffix, (long) number];
        if ([extension length])
            candidate = [candidate stringByAppendingPathExtension: extension];
        NSURL *url = [folder URLByAppendingPathComponent: candidate];
        if (!FinderExists([url path]))
            return url;
    }
}

@implementation FinderFileOperation {
    dispatch_queue_t _queue;
    atomic_bool _cancelled;
    BOOL _started;
    BOOL _finished;
    FinderTransferKind _kind;
    int64_t _done, _total, _itemBase;
    CFAbsoluteTime _lastProgress;
    NSURL *_current;
    BOOL _applyToAll;
    FinderConflictChoice _choiceForAll;
    int _copyError;
}

- (instancetype)initWithKind:(FinderTransferKind)kind items:(NSArray<NSURL *> *)items destination:(NSURL *)folder
{
    if ((self = [super init])) {
        _kind = kind;
        _items = [items copy];
        _destination = folder;
        _queue = dispatch_queue_create("org.darlinghq.Finder.file-operation", DISPATCH_QUEUE_SERIAL);
        atomic_init(&_cancelled, false);
    }
    return self;
}

- (FinderTransferKind)kind
{
    @synchronized (self) {
        return _kind;
    }
}

- (BOOL)isFinished
{
    @synchronized (self) {
        return _finished;
    }
}

- (void)start
{
    @synchronized (self) {
        if (_started)
            return;
        _started = YES;
    }
    dispatch_async(_queue, ^{
        NSMutableArray *results = [NSMutableArray array];
        NSError *error = [self run: results];
        [self reportProgress: YES];
        dispatch_async(dispatch_get_main_queue(), ^{
            @synchronized (self) {
                self->_finished = YES;
            }
            if (self.completion)
                self.completion(results, error);
        });
    });
}

- (void)cancel
{
    atomic_store(&_cancelled, true);
}

- (BOOL)isCancelled
{
    return atomic_load(&_cancelled);
}

- (NSError *)cancelledError
{
    return [NSError errorWithDomain: NSCocoaErrorDomain code: NSUserCancelledError userInfo: nil];
}

#pragma mark Class helpers

+ (NSURL *)keepBothURLForItem:(NSURL *)item inFolder:(NSURL *)folder
{
    return FinderNumberedURL(item, folder, nil);
}

+ (NSURL *)copyURLForItem:(NSURL *)item inFolder:(NSURL *)folder
{
    return FinderNumberedURL(item, folder, @"copy");
}

+ (id)volumeOf:(NSURL *)url
{
    id volume = nil;
    [[url URLByStandardizingPath] getResourceValue: &volume forKey: NSURLVolumeIdentifierKey error: NULL];
    return volume;
}

+ (FinderTransferKind)defaultKindForItems:(NSArray<NSURL *> *)items destination:(NSURL *)folder
{
    id volume = [self volumeOf: folder];
    if (volume == nil)
        return FinderTransferCopy;
    for (NSURL *item in items) {
        // The item's own volume, not its target's: a symlink moves with its folder. A volume itself is copied
        id itemVolume = [self volumeOf: [item URLByDeletingLastPathComponent]];
        if (![volume isEqual: itemVolume] || FinderIsVolume(item))
            return FinderTransferCopy;
    }
    return FinderTransferMove;
}

+ (NSError *)refusalForItems:(NSArray<NSURL *> *)items destination:(NSURL *)folder kind:(FinderTransferKind)kind
{
    NSString *destination = FinderCanonicalPath(folder, YES);
    for (NSURL *item in items) {
        if (!FinderIsDirectory(item))
            continue;
        if (FinderPathIsWithin(destination, FinderCanonicalPath(item, NO))) {
            NSString *format = kind == FinderTransferMove ? @"The folder “%@” can't be moved into itself."
                                                          : @"The folder “%@” can't be copied into itself.";
            return FinderError(NSFileWriteInvalidFileNameError, item,
                               [NSString stringWithFormat: format, [[NSFileManager defaultManager] displayNameAtPath: [item path]]], nil);
        }
    }
    return nil;
}

#pragma mark The work

- (void)reportProgress:(BOOL)last
{
    CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
    if (!last && now - _lastProgress < FinderProgressInterval)
        return;
    _lastProgress = now;
    int64_t done = _done, total = _total;
    NSURL *current = _current;
    void (^progress)(int64_t, int64_t, NSURL *) = self.progress;
    if (progress)
        dispatch_async(dispatch_get_main_queue(), ^{
            progress(done, total, current);
        });
}

// The bytes a copy of item will write: every file's size, links not followed
- (int64_t)sizeOfItem:(NSURL *)item
{
    struct stat st;
    if (lstat([item fileSystemRepresentation], &st) != 0)
        return 0;
    if (!S_ISDIR(st.st_mode))
        return S_ISREG(st.st_mode) ? st.st_size : 0;
    int64_t size = 0;
    NSDirectoryEnumerator *enumerator = [[NSFileManager defaultManager] enumeratorAtURL: item
                                                             includingPropertiesForKeys: nil
                                                                                options: 0
                                                                           errorHandler: nil];
    for (NSURL *child in enumerator) {
        if ([self isCancelled])
            break;
        if (lstat([child fileSystemRepresentation], &st) == 0 && S_ISREG(st.st_mode))
            size += st.st_size;
    }
    return size;
}

static int FinderCopyCallback(int what, int stage, copyfile_state_t state, const char *source, const char *destination, void *context)
{
    FinderFileOperation *operation = (__bridge FinderFileOperation *) context;
    return [operation copyfileWhat: what stage: stage state: state];
}

- (int)copyfileWhat:(int)what stage:(int)stage state:(copyfile_state_t)state
{
    if ([self isCancelled])
        return COPYFILE_QUIT;
    if (stage == COPYFILE_ERR) {
        _copyError = errno ? errno : EIO;
        return COPYFILE_QUIT;
    }
    if (what == COPYFILE_RECURSE_FILE && stage == COPYFILE_START) {
        _itemBase = _done;
    } else if (what == COPYFILE_COPY_DATA && stage == COPYFILE_PROGRESS) {
        off_t copied = 0;
        if (copyfile_state_get(state, COPYFILE_STATE_COPIED, &copied) == 0)
            _done = _itemBase + copied;
        [self reportProgress: NO];
    } else if (what == COPYFILE_RECURSE_FILE && stage == COPYFILE_FINISH) {
        const char *source = NULL;
        struct stat st;
        if (copyfile_state_get(state, COPYFILE_STATE_SRC_FILENAME, &source) == 0 && source
            && lstat(source, &st) == 0 && S_ISREG(st.st_mode))
            _done = _itemBase + st.st_size;
        _itemBase = _done;
        [self reportProgress: NO];
    }
    return COPYFILE_CONTINUE;
}

// A staged name in folder, not taken
- (NSURL *)stagedURLForItem:(NSURL *)item inFolder:(NSURL *)folder
{
    for (;;) {
        NSURL *url = [folder URLByAppendingPathComponent: FinderStagedName([item lastPathComponent])];
        if (!FinderExists([url path]))
            return url;
    }
}

// Copies item to staged; nil, or the error (cancelled included)
- (NSError *)copyItem:(NSURL *)item to:(NSURL *)staged
{
    int64_t start = _done;
    _itemBase = _done;
    _copyError = 0;
    copyfile_state_t state = copyfile_state_alloc();
    copyfile_state_set(state, COPYFILE_STATE_STATUS_CB, (const void *) &FinderCopyCallback);
    copyfile_state_set(state, COPYFILE_STATE_STATUS_CTX, (__bridge const void *) self);
    int result = copyfile([item fileSystemRepresentation], [staged fileSystemRepresentation], state,
                          COPYFILE_ALL | COPYFILE_RECURSIVE | COPYFILE_NOFOLLOW_SRC | COPYFILE_EXCL);
    int saved = errno;
    copyfile_state_free(state);
    if (result == 0 && ![self isCancelled]) {
        _done = start + [self sizeOfItem: item];
        return nil;
    }
    [[NSFileManager defaultManager] removeItemAtURL: staged error: NULL];
    if ([self isCancelled])
        return [self cancelledError];
    return FinderErrorForErrno(_copyError ? _copyError : (saved ? saved : EIO), item, YES);
}

- (FinderConflictChoice)choiceForConflictOf:(NSURL *)item with:(NSURL *)existing
{
    if (_applyToAll)
        return _choiceForAll;
    FinderConflictChoice (^resolve)(NSURL *, NSURL *, BOOL *) = self.resolveConflict;
    if (resolve == nil)
        return FinderConflictStop;
    __block FinderConflictChoice choice = FinderConflictStop;
    __block BOOL all = NO;
    dispatch_sync(dispatch_get_main_queue(), ^{
        choice = resolve(item, existing, &all);
    });
    if (all) {
        _applyToAll = YES;
        _choiceForAll = choice;
    }
    return choice;
}

- (NSError *)run:(NSMutableArray<NSURL *> *)results
{
    NSFileManager *fm = [NSFileManager defaultManager];
    FinderTransferKind kind = _kind;
    NSError *refusal = [FinderFileOperation refusalForItems: _items destination: _destination kind: kind];
    if (refusal)
        return refusal;

    // A move out of a read-only volume, or of a volume itself, can only copy
    if (kind == FinderTransferMove) {
        for (NSURL *item in _items) {
            NSNumber *readOnly = nil;
            [[item URLByDeletingLastPathComponent] getResourceValue: &readOnly forKey: NSURLVolumeIsReadOnlyKey error: NULL];
            if ([readOnly boolValue] || FinderIsVolume(item)) {
                kind = FinderTransferCopy;
                @synchronized (self) {
                    _kind = kind;
                }
                break;
            }
        }
    }
    id destinationVolume = [FinderFileOperation volumeOf: _destination];
    NSString *destination = FinderCanonicalPath(_destination, YES);

    // What each item needs: a rename, or a copy (with its bytes counted)
    NSMutableArray<NSNumber *> *renames = [NSMutableArray array];
    for (NSURL *item in _items) {
        BOOL rename = kind == FinderTransferMove && destinationVolume != nil
            && [destinationVolume isEqual: [FinderFileOperation volumeOf: [item URLByDeletingLastPathComponent]]];
        [renames addObject: @(rename)];
        if (!rename)
            _total += [self sizeOfItem: item];
        if ([self isCancelled])
            return [self cancelledError];
    }
    [self reportProgress: YES];

    for (NSUInteger i = 0; i < [_items count]; i++) {
        NSURL *item = _items[i];
        BOOL rename = [renames[i] boolValue];
        if ([self isCancelled])
            return [self cancelledError];
        if (!FinderExists([item path]))
            return FinderErrorForErrno(ENOENT, item, NO);
        _current = item;

        NSURL *target = [_destination URLByAppendingPathComponent: [item lastPathComponent]];
        NSURL *replaced = nil;
        if ([FinderCanonicalPath(item, NO) isEqual: [destination stringByAppendingPathComponent: [[item lastPathComponent] precomposedStringWithCanonicalMapping]]]) {
            // Already in the destination: a move does nothing, a copy gets a "copy" name
            if (kind == FinderTransferMove) {
                [results addObject: item];
                continue;
            }
            target = [FinderFileOperation copyURLForItem: item inFolder: _destination];
        } else if (FinderExists([target path])) {
            switch ([self choiceForConflictOf: item with: target]) {
            case FinderConflictStop:
                return [self cancelledError];
            case FinderConflictKeepBoth:
                target = [FinderFileOperation keepBothURLForItem: item inFolder: _destination];
                break;
            case FinderConflictReplace:
                if (FinderPathIsWithin(FinderCanonicalPath(item, NO), FinderCanonicalPath(target, NO)))
                    return FinderError(NSFileWriteInvalidFileNameError, item,
                                       [NSString stringWithFormat: @"“%@” can't be replaced, because it contains the item being %@.",
                                                 [target lastPathComponent], kind == FinderTransferMove ? @"moved" : @"copied"], nil);
                replaced = target;
                break;
            }
        }

        NSError *error = nil;
        if (rename) {
            // The replaced item goes aside first, and comes back if the move fails
            NSURL *aside = replaced ? [self stagedURLForItem: replaced inFolder: _destination] : nil;
            if (aside && ![fm moveItemAtURL: replaced toURL: aside error: &error])
                return FinderErrorForErrno(EACCES, replaced, YES);
            if (![fm moveItemAtURL: item toURL: target error: &error]) {
                if (aside)
                    [fm moveItemAtURL: aside toURL: replaced error: NULL];
                return FinderError(error.code, item, nil, error);
            }
            if (aside)
                [fm removeItemAtURL: aside error: NULL];
        } else {
            NSURL *staged = [self stagedURLForItem: item inFolder: _destination];
            if ((error = [self copyItem: item to: staged]))
                return error;
            // The replaced item goes aside, and is removed only once the copy has its name
            NSURL *aside = replaced ? [self stagedURLForItem: replaced inFolder: _destination] : nil;
            if (aside && ![fm moveItemAtURL: replaced toURL: aside error: &error]) {
                [fm removeItemAtURL: staged error: NULL];
                return FinderError(error.code, replaced, nil, error);
            }
            if (![fm moveItemAtURL: staged toURL: target error: &error]) {
                if (aside)
                    [fm moveItemAtURL: aside toURL: replaced error: NULL];
                [fm removeItemAtURL: staged error: NULL];
                return FinderError(error.code, item, nil, error);
            }
            if (aside)
                [fm removeItemAtURL: aside error: NULL];
            // A move across volumes: the source goes once its copy is in place
            if (kind == FinderTransferMove && ![fm removeItemAtURL: item error: &error]) {
                [results addObject: target];
                return FinderError(error.code, item, nil, error);
            }
        }
        [results addObject: target];
    }
    return nil;
}

@end
