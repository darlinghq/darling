// CFLAGS: -framework Foundation -fobjc-arc -Isrc/Finder src/Finder/FinderFileOperation.m
// ARGS: "$(mktemp -d)/w"
// Copyright (C) 2026 Lubos Dolezel
//
// Finder's copy and move engine (src/Finder/FinderFileOperation), without a display: copies of files and trees
// (symlinks copied as links), moves, the conflict choices (Replace, Keep Both, Stop, Apply to All), names that differ
// only in normalization, copies into the item's own folder ("copy" names), Keep Both names, transfers refused up front
// (a folder into itself, a Replace of the source's own ancestor), failures and cancelling (nothing left at the final
// name, no staged item), and the default kind across volumes. See tests/finder.md.
// Usage: finder_file_operation <new directory> [<disk image>]; with the image (tests/fixtures/unmount-test.dmg) it
// attaches it with hdiutil for the cases across volumes and detaches it afterwards. Prints PASS/FAIL/SKIP lines;
// exits 1 on a failure.
#import <Foundation/Foundation.h>
#import "FinderFileOperation.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static void skip(NSString *what)
{
    printf("SKIP: %s\n", [what UTF8String]);
}

static NSFileManager *fm;
static NSURL *root;

static NSURL *path(NSString *relative)
{
    return [root URLByAppendingPathComponent: relative];
}

static NSURL *folder(NSString *relative)
{
    NSURL *url = path(relative);
    [fm createDirectoryAtURL: url withIntermediateDirectories: YES attributes: nil error: NULL];
    return url;
}

static NSURL *file(NSString *relative, NSString *contents)
{
    NSURL *url = path(relative);
    [fm createDirectoryAtURL: [url URLByDeletingLastPathComponent] withIntermediateDirectories: YES attributes: nil
                       error: NULL];
    [[contents dataUsingEncoding: NSUTF8StringEncoding] writeToURL: url atomically: NO];
    return url;
}

static NSString *contents(NSURL *url)
{
    NSData *data = [NSData dataWithContentsOfURL: url];
    return data ? [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding] : nil;
}

static BOOL exists(NSURL *url)
{
    struct stat st;
    return lstat([url fileSystemRepresentation], &st) == 0;
}

// Names in folder that look like a staged copy
static NSArray<NSString *> *staged(NSURL *folder)
{
    NSMutableArray *names = [NSMutableArray array];
    for (NSString *name in [fm contentsOfDirectoryAtPath: [folder path] error: NULL])
        if ([name hasPrefix: @"."] && [name rangeOfString: @".finder-"].location != NSNotFound)
            [names addObject: name];
    return names;
}

@interface Run : NSObject
@property NSArray<NSURL *> *results;
@property NSError *error;
@property BOOL done;
@property int conflicts;
@property int64_t lastDone, lastTotal;
@property int progressCalls;
@end
@implementation Run
@end

// Runs op until its completion; conflicts are answered with choice (applyToAll as given).
static Run *run(FinderFileOperation *op, FinderConflictChoice choice, BOOL applyToAll)
{
    Run *r = [Run new];
    op.resolveConflict = ^FinderConflictChoice(NSURL *source, NSURL *existing, BOOL *all) {
        expect([NSThread isMainThread], @"resolveConflict is called on the main thread");
        r.conflicts++;
        *all = applyToAll;
        return choice;
    };
    op.progress = ^(int64_t done, int64_t total, NSURL *current) {
        r.progressCalls++;
        r.lastDone = done;
        r.lastTotal = total;
    };
    op.completion = ^(NSArray<NSURL *> *results, NSError *error) {
        expect([NSThread isMainThread], @"completion is called on the main thread");
        r.results = results;
        r.error = error;
        r.done = YES;
    };
    [op start];
    NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: 120];
    while (!r.done && [limit timeIntervalSinceNow] > 0)
        [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.05]];
    expect(r.done, @"the operation completes");
    expect(op.finished, @"the operation is finished");
    return r;
}

static FinderFileOperation *op(FinderTransferKind kind, NSArray<NSURL *> *items, NSURL *destination)
{
    return [[FinderFileOperation alloc] initWithKind: kind items: items destination: destination];
}

static BOOL isCancelled(NSError *error)
{
    return [error.domain isEqual: NSCocoaErrorDomain] && error.code == NSUserCancelledError;
}

static BOOL sameURLs(NSArray<NSURL *> *a, NSArray<NSURL *> *b)
{
    if ([a count] != [b count])
        return NO;
    for (NSUInteger i = 0; i < [a count]; i++)
        if (![[a[i] URLByStandardizingPath].path isEqual: [b[i] URLByStandardizingPath].path])
            return NO;
    return YES;
}

static int hdiutil(NSArray<NSString *> *arguments, NSData **output)
{
    NSTask *task = [[NSTask alloc] init];
    task.launchPath = @"/usr/bin/hdiutil";
    task.arguments = arguments;
    NSPipe *pipe = [NSPipe pipe];
    task.standardOutput = pipe;
    [task launch];
    NSData *data = [[pipe fileHandleForReading] readDataToEndOfFile];
    [task waitUntilExit];
    if (output)
        *output = data;
    return task.terminationStatus;
}

// The mount point hdiutil attach -plist reports
static NSURL *attach(NSString *image)
{
    NSData *data;
    if (hdiutil(@[ @"attach", @"-noautoopen", @"-plist", image ], &data) != 0)
        return nil;
    // Anything before the plist (a licence text) is skipped
    NSRange start = [data rangeOfData: [@"<?xml" dataUsingEncoding: NSUTF8StringEncoding] options: 0
                                range: NSMakeRange(0, [data length])];
    if (start.location == NSNotFound)
        return nil;
    NSDictionary *plist = [NSPropertyListSerialization
        propertyListWithData: [data subdataWithRange: NSMakeRange(start.location, [data length] - start.location)]
                     options: 0 format: NULL error: NULL];
    for (NSDictionary *entity in plist[@"system-entities"])
        if (entity[@"mount-point"])
            return [NSURL fileURLWithPath: entity[@"mount-point"] isDirectory: YES];
    return nil;
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        setvbuf(stdout, NULL, _IONBF, 0);
        if (argc < 2) {
            fprintf(stderr, "usage: %s <new directory> [<disk image>]\n", argv[0]);
            return 2;
        }
        fm = [NSFileManager defaultManager];
        root = [NSURL fileURLWithPath: [NSString stringWithUTF8String: argv[1]] isDirectory: YES];
        if (![fm createDirectoryAtURL: root withIntermediateDirectories: YES attributes: nil error: NULL]) {
            fprintf(stderr, "can't create %s\n", argv[1]);
            return 2;
        }

        // copy one file
        NSURL *a = file(@"src/a.txt", @"hello");
        NSURL *dst = folder(@"dst");
        Run *r = run(op(FinderTransferCopy, @[ a ], dst), FinderConflictStop, NO);
        NSURL *copied = [dst URLByAppendingPathComponent: @"a.txt"];
        expect(r.error == nil, [NSString stringWithFormat: @"copy one file: no error (%@)", r.error]);
        expect([contents(copied) isEqual: @"hello"], @"copy one file: the copy has the same bytes");
        expect(exists(a), @"copy one file: the source remains");
        expect(sameURLs(r.results, @[ copied ]), [NSString stringWithFormat: @"copy one file: results (%@)", r.results]);
        expect(r.lastDone == 5 && r.lastTotal == 5,
               [NSString stringWithFormat: @"copy one file: the last progress is 5 of 5 (%lld of %lld)", r.lastDone, r.lastTotal]);
        expect(r.conflicts == 0, @"copy one file: no conflict");

        // copy a tree, with links copied as links
        NSURL *tree = folder(@"src/tree");
        file(@"src/tree/sub/inner.txt", @"inner");
        file(@"src/x", @"x");
        [fm createSymbolicLinkAtPath: [path(@"src/tree/link") path] withDestinationPath: @"../x" error: NULL];
        [fm createSymbolicLinkAtPath: [path(@"src/tree/dangling") path] withDestinationPath: @"nowhere/at/all" error: NULL];
        r = run(op(FinderTransferCopy, @[ tree ], dst), FinderConflictStop, NO);
        expect(r.error == nil, [NSString stringWithFormat: @"copy a tree: no error (%@)", r.error]);
        expect([contents(path(@"dst/tree/sub/inner.txt")) isEqual: @"inner"], @"copy a tree: the subfolder's file");
        expect([[fm destinationOfSymbolicLinkAtPath: [path(@"dst/tree/link") path] error: NULL] isEqual: @"../x"],
               @"copy a tree: a symlink is copied as a link with its target");
        expect([[fm destinationOfSymbolicLinkAtPath: [path(@"dst/tree/dangling") path] error: NULL] isEqual: @"nowhere/at/all"],
               @"copy a tree: a dangling symlink is copied");
        expect([staged(dst) count] == 0, @"copy a tree: no staged item is left");

        // move within a volume
        NSURL *m = file(@"src/m.txt", @"move me");
        struct stat before, after;
        lstat([m fileSystemRepresentation], &before);
        r = run(op(FinderTransferMove, @[ m ], dst), FinderConflictStop, NO);
        NSURL *moved = [dst URLByAppendingPathComponent: @"m.txt"];
        BOOL same = lstat([moved fileSystemRepresentation], &after) == 0 && before.st_dev == after.st_dev
            && before.st_ino == after.st_ino;
        expect(r.error == nil && sameURLs(r.results, @[ moved ]), @"move within a volume: done");
        expect(!exists(m), @"move within a volume: the source is gone");
        expect(same, @"move within a volume: the same file (not a copy)");

        // move into its own folder is a no-op
        r = run(op(FinderTransferMove, @[ moved ], dst), FinderConflictStop, NO);
        expect(r.error == nil && sameURLs(r.results, @[ moved ]) && r.conflicts == 0 && exists(moved),
               @"move into its own folder: nothing happens, successfully");

        // conflict: Replace
        file(@"dst/a.txt", @"old");
        file(@"src/a.txt", @"new!");
        r = run(op(FinderTransferCopy, @[ a ], dst), FinderConflictReplace, NO);
        expect(r.conflicts == 1, @"conflict replace: asked once");
        expect(r.error == nil && [contents(copied) isEqual: @"new!"], @"conflict replace: the item is replaced");
        expect([staged(dst) count] == 0, @"conflict replace: no staged item is left");

        // conflict: Keep Both
        r = run(op(FinderTransferCopy, @[ a ], dst), FinderConflictKeepBoth, NO);
        NSURL *a2 = [dst URLByAppendingPathComponent: @"a 2.txt"];
        expect(r.error == nil && sameURLs(r.results, @[ a2 ]), [NSString stringWithFormat: @"conflict keep both: a 2.txt (%@)", r.results]);
        expect(exists(copied) && [contents(a2) isEqual: @"new!"], @"conflict keep both: both exist");

        // conflict: Stop
        file(@"src/a.txt", @"newer");
        r = run(op(FinderTransferCopy, @[ a ], dst), FinderConflictStop, NO);
        expect(isCancelled(r.error), [NSString stringWithFormat: @"conflict stop: cancelled (%@)", r.error]);
        expect([contents(copied) isEqual: @"new!"], @"conflict stop: the existing item is unchanged");
        expect([staged(dst) count] == 0, @"conflict stop: no staged item is left");

        // Apply to All
        NSURL *all = folder(@"all");
        NSMutableArray *three = [NSMutableArray array];
        for (NSString *name in @[ @"p.txt", @"q.txt", @"r.txt" ]) {
            [three addObject: file([@"src/" stringByAppendingString: name], name)];
            file([@"all/" stringByAppendingString: name], @"there");
        }
        r = run(op(FinderTransferCopy, three, all), FinderConflictKeepBoth, YES);
        expect(r.conflicts == 1, [NSString stringWithFormat: @"apply to all: asked once (%d)", r.conflicts]);
        expect(r.error == nil && exists([all URLByAppendingPathComponent: @"p 2.txt"])
                   && exists([all URLByAppendingPathComponent: @"q 2.txt"])
                   && exists([all URLByAppendingPathComponent: @"r 2.txt"]),
               @"apply to all: all three are kept both");

        // Names that differ only in normalization conflict
        NSURL *norm = folder(@"norm");
        file(@"norm/ü.txt", @"precomposed");
        NSURL *decomposed = file(@"src/ü.txt", @"decomposed");
        r = run(op(FinderTransferCopy, @[ decomposed ], norm), FinderConflictStop, NO);
        expect(r.conflicts == 1, @"normalization conflict: a decomposed name conflicts with the precomposed one");

        // copy into its own folder: "copy" names, no question
        NSURL *own = file(@"own/a.txt", @"own");
        r = run(op(FinderTransferCopy, @[ own ], folder(@"own")), FinderConflictStop, NO);
        expect(r.conflicts == 0 && sameURLs(r.results, @[ path(@"own/a copy.txt") ]),
               [NSString stringWithFormat: @"copy into own folder: a copy.txt (%@)", r.results]);
        r = run(op(FinderTransferCopy, @[ own ], folder(@"own")), FinderConflictStop, NO);
        expect(r.conflicts == 0 && sameURLs(r.results, @[ path(@"own/a copy 2.txt") ]),
               [NSString stringWithFormat: @"copy into own folder again: a copy 2.txt (%@)", r.results]);

        // Keep Both names
        NSURL *names = folder(@"names");
        NSURL *k = [names URLByAppendingPathComponent: @"a.txt"];
        expect([[[FinderFileOperation keepBothURLForItem: k inFolder: names] lastPathComponent] isEqual: @"a 2.txt"],
               @"keep both name: a 2.txt");
        file(@"names/a 2.txt", @"");
        expect([[[FinderFileOperation keepBothURLForItem: k inFolder: names] lastPathComponent] isEqual: @"a 3.txt"],
               @"keep both name: a 3.txt when a 2.txt exists");
        expect([[[FinderFileOperation keepBothURLForItem: [names URLByAppendingPathComponent: @"x.tar.gz"] inFolder: names]
                   lastPathComponent] isEqual: @"x.tar 2.gz"], @"keep both name: x.tar 2.gz");
        NSURL *f = folder(@"names/f");
        expect([[[FinderFileOperation keepBothURLForItem: f inFolder: names] lastPathComponent] isEqual: @"f 2"],
               @"keep both name: f 2 for a folder");
        expect([[[FinderFileOperation keepBothURLForItem: [names URLByAppendingPathComponent: @"\U0001F600.txt"] inFolder: names]
                   lastPathComponent] isEqual: @"\U0001F600 2.txt"], @"keep both name: an emoji name");
        expect([[[FinderFileOperation copyURLForItem: k inFolder: names] lastPathComponent] isEqual: @"a copy.txt"],
               @"copy name: a copy.txt");

        // a folder into itself
        NSURL *self_ = folder(@"self");
        NSURL *sub = folder(@"self/sub");
        expect([FinderFileOperation refusalForItems: @[ self_ ] destination: sub kind: FinderTransferCopy] != nil,
               @"folder into itself: refused");
        expect([FinderFileOperation refusalForItems: @[ self_ ] destination: self_ kind: FinderTransferMove] != nil,
               @"folder onto itself: refused");
        expect([FinderFileOperation refusalForItems: @[ a ] destination: dst kind: FinderTransferCopy] == nil,
               @"an ordinary copy: not refused");
        r = run(op(FinderTransferCopy, @[ self_ ], sub), FinderConflictStop, NO);
        expect(r.error != nil && !isCancelled(r.error), [NSString stringWithFormat: @"folder into itself: an error (%@)", r.error]);
        expect([[fm contentsOfDirectoryAtPath: [sub path] error: NULL] count] == 0, @"folder into itself: nothing is written");

        // Replace of the source's own ancestor
        NSURL *p = folder(@"p");
        NSURL *q = folder(@"p/q");
        NSURL *qq = file(@"p/q/q", @"inner q");
        r = run(op(FinderTransferCopy, @[ qq ], p), FinderConflictReplace, NO);
        expect(r.error != nil && !isCancelled(r.error), [NSString stringWithFormat: @"replace own ancestor: an error (%@)", r.error]);
        expect(exists(q) && exists(qq), @"replace own ancestor: the existing item and the source remain");

        // a folder holding an unreadable file
        if (geteuid() == 0)
            skip(@"unreadable file in a folder: root reads it anyway");
        else {
            NSURL *t = folder(@"src/t");
            file(@"src/t/ok", @"ok");
            NSURL *no = file(@"src/t/no", @"no");
            chmod([no fileSystemRepresentation], 0);
            NSURL *dstT = folder(@"dstT");
            r = run(op(FinderTransferCopy, @[ t ], dstT), FinderConflictStop, NO);
            expect(r.error != nil && !isCancelled(r.error) && r.error.userInfo[NSFilePathErrorKey] != nil,
                   [NSString stringWithFormat: @"unreadable file: an error naming the item (%@)", r.error]);
            expect(!exists([dstT URLByAppendingPathComponent: @"t"]), @"unreadable file: nothing at the final name");
            expect([staged(dstT) count] == 0, @"unreadable file: no staged item is left");
            chmod([no fileSystemRepresentation], 0644);
        }

        // Replace of an item that can't be wholly removed: the new item takes its place, nothing is lost (the old one
        // is set aside before the new one goes in)
        if (geteuid() == 0)
            skip(@"replace of an undeletable item: root deletes anything");
        else {
            NSURL *newP = folder(@"src/repl/P");
            file(@"src/repl/P/new", @"new");
            NSURL *dstRepl = folder(@"dstRepl");
            NSURL *oldLocked = folder(@"dstRepl/P/locked");
            file(@"dstRepl/P/locked/keep", @"keep");
            file(@"dstRepl/P/old", @"old");
            chmod([oldLocked fileSystemRepresentation], 0555);
            r = run(op(FinderTransferCopy, @[ newP ], dstRepl), FinderConflictReplace, NO);
            NSURL *target = [dstRepl URLByAppendingPathComponent: @"P"];
            expect(r.error == nil && [contents([target URLByAppendingPathComponent: @"new"]) isEqual: @"new"],
                   [NSString stringWithFormat: @"replace of an undeletable item: the new item is in place (%@)", r.error]);
            NSArray<NSString *> *aside = staged(dstRepl);
            BOOL kept = NO;
            for (NSString *name in aside)
                kept |= [contents([[dstRepl URLByAppendingPathComponent: name] URLByAppendingPathComponent: @"locked/keep"]) isEqual: @"keep"];
            expect(kept, @"replace of an undeletable item: what couldn't be deleted is still there, set aside");
            for (NSString *name in aside)
                chmod([[[dstRepl URLByAppendingPathComponent: name] URLByAppendingPathComponent: @"locked"] fileSystemRepresentation], 0755);
        }

        // a name of 255 bytes (the longest a file system takes) copies, and replaces
        NSString *longName = [[@"" stringByPaddingToLength: 251 withString: @"n" startingAtIndex: 0] stringByAppendingString: @".txt"];
        NSURL *longFile = file([@"src/" stringByAppendingString: longName], @"long");
        NSURL *dstLong = folder(@"dstLong");
        r = run(op(FinderTransferCopy, @[ longFile ], dstLong), FinderConflictStop, NO);
        expect(r.error == nil && [contents([dstLong URLByAppendingPathComponent: longName]) isEqual: @"long"],
               [NSString stringWithFormat: @"a 255-byte name copies (%@)", r.error]);
        r = run(op(FinderTransferCopy, @[ longFile ], dstLong), FinderConflictReplace, NO);
        expect(r.error == nil && r.conflicts == 1 && [contents([dstLong URLByAppendingPathComponent: longName]) isEqual: @"long"],
               [NSString stringWithFormat: @"a 255-byte name replaces (%@)", r.error]);

        // cancel during a large file
        NSURL *big = path(@"src/big");
        [[NSMutableData dataWithLength: 64 << 20] writeToURL: big atomically: NO];
        NSURL *dstBig = folder(@"dstBig");
        FinderFileOperation *cancelled = op(FinderTransferCopy, @[ big ], dstBig);
        __weak FinderFileOperation *weakCancelled = cancelled;
        __block BOOL sent = NO;
        Run *rc = [Run new];
        cancelled.progress = ^(int64_t done, int64_t total, NSURL *current) {
            if (!sent) {
                sent = YES;
                [weakCancelled cancel];
            }
        };
        cancelled.completion = ^(NSArray<NSURL *> *results, NSError *error) {
            rc.error = error;
            rc.done = YES;
        };
        [cancelled start];
        NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: 120];
        while (!rc.done && [limit timeIntervalSinceNow] > 0)
            [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.05]];
        expect(rc.done && isCancelled(rc.error), [NSString stringWithFormat: @"cancel mid-file: cancelled (%@)", rc.error]);
        expect(!exists([dstBig URLByAppendingPathComponent: @"big"]), @"cancel mid-file: nothing at the final name");
        expect([staged(dstBig) count] == 0, @"cancel mid-file: no staged item is left");

        // default kind
        expect([FinderFileOperation defaultKindForItems: @[ a ] destination: dst] == FinderTransferMove,
               @"default kind: Move within a volume");
        if (argc < 3)
            skip(@"default kind across volumes: no disk image given");
        else {
            NSURL *volume = attach([NSString stringWithUTF8String: argv[2]]);
            expect(volume != nil, @"hdiutil attach");
            if (volume) {
                expect([FinderFileOperation defaultKindForItems: @[ a ] destination: volume] == FinderTransferCopy,
                       @"default kind: Copy to another volume");
                // A volume itself is copied, never moved: not even to the volume holding its mount point
                expect([FinderFileOperation defaultKindForItems: @[ volume ] destination: dst] == FinderTransferCopy,
                       @"default kind: a volume is copied");
                NSURL *inImage = nil;
                for (NSURL *item in [fm contentsOfDirectoryAtURL: volume includingPropertiesForKeys: nil
                                                         options: NSDirectoryEnumerationSkipsHiddenFiles error: NULL])
                    if (!inImage)
                        inImage = item;
                expect(inImage != nil, @"the image has an item");
                if (inImage) {
                    expect([FinderFileOperation defaultKindForItems: @[ inImage ] destination: dst] == FinderTransferCopy,
                           @"default kind: Copy from another volume");
                    NSURL *outOf = folder(@"outOfImage");
                    FinderFileOperation *move = op(FinderTransferMove, @[ inImage ], outOf);
                    r = run(move, FinderConflictStop, NO);
                    expect(move.kind == FinderTransferCopy, @"a move out of a read-only image becomes a copy");
                    expect(r.error == nil && exists([outOf URLByAppendingPathComponent: [inImage lastPathComponent]]),
                           [NSString stringWithFormat: @"a move out of a read-only image: copied (%@)", r.error]);
                    expect(exists(inImage), @"a move out of a read-only image: the source remains");
                }
                expect(hdiutil(@[ @"detach", [volume path] ], NULL) == 0, @"hdiutil detach");
            }
        }

        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
