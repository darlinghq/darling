// CFLAGS: -framework Foundation -framework AppKit -fobjc-arc -Isrc/Finder src/Finder/FinderFileOperation.m src/Finder/FinderItemActions.m
// ARGS: "$(mktemp -d)/w"
// Copyright (C) 2026 Lubos Dolezel
//
// Finder's one-shot actions (src/Finder/FinderItemActions): new folders ("untitled folder", "untitled folder 2"), the
// rules a new name must follow (not empty, no ":" or "/", no leading ".", not another item's, also one differing only
// in normalization; a case-only change is allowed), the extension-change question, renaming (case-only included), and
// Trash through NSWorkspace (an item on a read-only disk image fails and stays). See tests/finder.md.
// Usage: finder_item_actions <new directory> [<disk image>]; with the image (tests/fixtures/unmount-test.dmg) it
// attaches and detaches it with hdiutil. Prints PASS/FAIL/SKIP lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderItemActions.h"
#include <stdio.h>
#include <sys/stat.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSFileManager *fm;
static NSURL *root;

static NSURL *file(NSString *relative)
{
    NSURL *url = [root URLByAppendingPathComponent: relative];
    [fm createDirectoryAtURL: [url URLByDeletingLastPathComponent] withIntermediateDirectories: YES attributes: nil
                       error: NULL];
    [[NSData dataWithBytes: "x" length: 1] writeToURL: url atomically: NO];
    return url;
}

static BOOL exists(NSURL *url)
{
    struct stat st;
    return lstat([url fileSystemRepresentation], &st) == 0;
}

static BOOL isDirectory(NSURL *url)
{
    struct stat st;
    return lstat([url fileSystemRepresentation], &st) == 0 && S_ISDIR(st.st_mode);
}

static NSURL *attach(NSString *image)
{
    NSTask *task = [[NSTask alloc] init];
    task.launchPath = @"/usr/bin/hdiutil";
    task.arguments = @[ @"attach", @"-noautoopen", @"-plist", image ];
    NSPipe *pipe = [NSPipe pipe];
    task.standardOutput = pipe;
    [task launch];
    NSData *data = [[pipe fileHandleForReading] readDataToEndOfFile];
    [task waitUntilExit];
    NSRange start = [data rangeOfData: [@"<?xml" dataUsingEncoding: NSUTF8StringEncoding] options: 0
                                range: NSMakeRange(0, [data length])];
    if (task.terminationStatus != 0 || start.location == NSNotFound)
        return nil;
    NSDictionary *plist = [NSPropertyListSerialization
        propertyListWithData: [data subdataWithRange: NSMakeRange(start.location, [data length] - start.location)]
                     options: 0 format: NULL error: NULL];
    for (NSDictionary *entity in plist[@"system-entities"])
        if (entity[@"mount-point"])
            return [NSURL fileURLWithPath: entity[@"mount-point"] isDirectory: YES];
    return nil;
}

static void detach(NSURL *volume)
{
    NSTask *task = [NSTask launchedTaskWithLaunchPath: @"/usr/bin/hdiutil" arguments: @[ @"detach", [volume path] ]];
    [task waitUntilExit];
}

// Trashes items and waits for the completion
static NSDictionary *trash(NSArray<NSURL *> *items, NSError **error)
{
    __block BOOL done = NO;
    __block NSDictionary *result = nil;
    __block NSError *failure = nil;
    [FinderItemActions trashItems: items completion: ^(NSDictionary<NSURL *, NSURL *> *trashed, NSError *e) {
        expect([NSThread isMainThread], @"trash: the completion is on the main thread");
        result = trashed;
        failure = e;
        done = YES;
    }];
    NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: 60];
    while (!done && [limit timeIntervalSinceNow] > 0)
        [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.05]];
    expect(done, @"trash: completes");
    *error = failure;
    return result;
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
        [fm createDirectoryAtURL: root withIntermediateDirectories: YES attributes: nil error: NULL];

        // New folders
        NSError *error = nil;
        NSURL *first = [FinderItemActions createFolderIn: root error: &error];
        expect([[first lastPathComponent] isEqual: @"untitled folder"] && isDirectory(first),
               [NSString stringWithFormat: @"new folder: untitled folder (%@, %@)", first, error]);
        NSURL *second = [FinderItemActions createFolderIn: root error: &error];
        expect([[second lastPathComponent] isEqual: @"untitled folder 2"] && isDirectory(second),
               [NSString stringWithFormat: @"new folder: then untitled folder 2 (%@)", second]);
        expect([FinderItemActions createFolderIn: [root URLByAppendingPathComponent: @"missing"] error: &error] == nil
                   && error != nil,
               @"new folder in a missing folder: an error");

        // Names
        NSURL *a = file(@"names/a.txt");
        file(@"names/c.txt");
        file(@"names/ü.txt");
        expect([FinderItemActions problemWithName: @"b.txt" forItem: a] == nil, @"name: b.txt is fine");
        expect([FinderItemActions problemWithName: @"a.txt" forItem: a] == nil, @"name: the item's own name is fine");
        expect([FinderItemActions problemWithName: @"A.txt" forItem: a] == nil, @"name: a case-only change is fine");
        NSError *problem = [FinderItemActions problemWithName: @"" forItem: a];
        expect([problem.localizedDescription isEqual: @"Please enter a name."],
               [NSString stringWithFormat: @"name: an empty one (%@)", problem.localizedDescription]);
        expect([FinderItemActions problemWithName: @"   " forItem: a] != nil, @"name: only spaces is refused");
        problem = [FinderItemActions problemWithName: @"a:b" forItem: a];
        expect([problem.localizedDescription isEqual: @"The name “a:b” can't be used."],
               [NSString stringWithFormat: @"name: a colon (%@)", problem.localizedDescription]);
        expect([problem.localizedRecoverySuggestion isEqual: @"Try using a name with fewer characters, or with no punctuation marks."],
               @"name: a colon: the suggestion");
        expect([FinderItemActions problemWithName: @"a/b" forItem: a] != nil, @"name: a slash is refused");
        problem = [FinderItemActions problemWithName: @".x" forItem: a];
        expect([problem.localizedDescription isEqual: @"Names that begin with a dot “.” are reserved for the system."],
               [NSString stringWithFormat: @"name: a leading dot (%@)", problem.localizedDescription]);
        problem = [FinderItemActions problemWithName: @"c.txt" forItem: a];
        expect([problem.localizedDescription isEqual: @"The name “c.txt” is already taken. Please choose a different name."],
               [NSString stringWithFormat: @"name: another item's (%@)", problem.localizedDescription]);
        expect([FinderItemActions problemWithName: @"ü.txt" forItem: a] != nil,
               @"name: another item's, differing only in normalization, is refused");

        // The extension question
        NSString *old = nil, *new = nil;
        expect([FinderItemActions name: @"a.md" changesExtensionOf: a from: &old to: &new] && [old isEqual: @"txt"]
                   && [new isEqual: @"md"], @"extension: a.txt to a.md changes it (txt, md)");
        expect(![FinderItemActions name: @"b.txt" changesExtensionOf: a from: &old to: &new], @"extension: a.txt to b.txt doesn't");
        expect(![FinderItemActions name: @"a.txt.md" changesExtensionOf: first from: &old to: &new], @"extension: a folder never asks");

        // Renaming
        NSURL *renamed = [FinderItemActions renameItem: a to: @"b.txt" error: &error];
        expect([[renamed lastPathComponent] isEqual: @"b.txt"] && exists(renamed) && !exists(a),
               [NSString stringWithFormat: @"rename: b.txt (%@, %@)", renamed, error]);
        NSURL *upper = [FinderItemActions renameItem: renamed to: @"B.txt" error: &error];
        NSArray *listing = [fm contentsOfDirectoryAtPath: [[renamed URLByDeletingLastPathComponent] path] error: NULL];
        expect([[upper lastPathComponent] isEqual: @"B.txt"] && [listing containsObject: @"B.txt"] && ![listing containsObject: @"b.txt"],
               [NSString stringWithFormat: @"rename: a case-only change is on disk (%@)", listing]);
        expect([FinderItemActions renameItem: upper to: @"c.txt" error: &error] == nil && error != nil,
               @"rename: onto another item's name fails");
        expect(exists(upper), @"rename: the item keeps its name when it fails");

        // Trash
        NSURL *doomed = file(@"trash/doomed.txt");
        NSDictionary *trashed = trash(@[ doomed ], &error);
        NSURL *inTrash = trashed[doomed];
        expect(error == nil && inTrash != nil && [[inTrash path] rangeOfString: @".Trash"].location != NSNotFound,
               [NSString stringWithFormat: @"trash: the item goes to a Trash (%@, %@)", inTrash, error]);
        expect(!exists(doomed) && exists(inTrash), @"trash: it is no longer where it was");
        if (argc < 3)
            printf("SKIP: trash on a read-only image: no disk image given\n");
        else {
            NSURL *volume = attach([NSString stringWithUTF8String: argv[2]]);
            expect(volume != nil, @"hdiutil attach");
            NSURL *item = [[fm contentsOfDirectoryAtURL: volume includingPropertiesForKeys: nil
                                                options: NSDirectoryEnumerationSkipsHiddenFiles error: NULL] firstObject];
            if (item) {
                trashed = trash(@[ item ], &error);
                expect(error != nil && trashed[item] == nil, [NSString stringWithFormat: @"trash on a read-only image: an error (%@)", error]);
                expect(exists(item), @"trash on a read-only image: the item stays");
            }
            if (volume)
                detach(volume);
        }

        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
