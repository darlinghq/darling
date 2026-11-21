// CFLAGS: -framework Foundation -framework AppKit -fobjc-arc -Isrc/Finder src/Finder/FinderDirectory.m
// ARGS: "$(mktemp -d)/d"
// Copyright (C) 2026 Lubos Dolezel
//
// Finder's folder model (src/Finder/FinderDirectory): a folder's visible items, sorted by localized name, and the
// errors for folders that can't be read. See tests/finder.md. Usage: finder_directory <new directory>.
// Prints PASS/FAIL lines; exits 1 on a failure. The order is the one macOS gives (observed on 10.14.4 and 26.7.3 with
// this program and FinderDirectory.m).
#import <Foundation/Foundation.h>
#import "FinderDirectory.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSString *names(NSArray<NSURL *> *items)
{
    NSMutableArray *names = [NSMutableArray array];
    for (NSURL *url in items)
        [names addObject: [[url lastPathComponent] stringByReplacingOccurrencesOfString: @"\n" withString: @"\\n"]];
    return [names componentsJoinedByString: @" "];
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        if (argc != 2) {
            fprintf(stderr, "usage: %s <new directory>\n", argv[0]);
            return 2;
        }
        NSFileManager *fm = [NSFileManager defaultManager];
        NSString *dir = [NSString stringWithUTF8String: argv[1]];
        [fm createDirectoryAtPath: dir withIntermediateDirectories: YES attributes: nil error: NULL];
        for (NSString *name in @[ @"b.txt", @"A.txt", @"a10.txt", @"a9.txt", @".hidden", @"Ünï",
                                  @"日本.txt", @"x\ny", @"flagged" ])
            [fm createFileAtPath: [dir stringByAppendingPathComponent: name] contents: [NSData data] attributes: nil];

        NSURL *flagged = [NSURL fileURLWithPath: [dir stringByAppendingPathComponent: @"flagged"]];
        NSError *error = nil;
        NSNumber *hidden = nil;
        // Read back through a new URL: the one it was set on caches the value
        BOOL canHide = [flagged setResourceValue: @YES forKey: NSURLIsHiddenKey error: &error]
            && [[NSURL fileURLWithPath: [flagged path]] getResourceValue: &hidden forKey: NSURLIsHiddenKey error: NULL]
            && [hidden boolValue];
        if (!canHide) {
            printf("SKIP: NSURLIsHiddenKey can't be set here (%s)\n", [[error description] UTF8String]);
            [fm removeItemAtURL: flagged error: NULL];
        }

        FinderDirectory *folder = [[FinderDirectory alloc] initWithURL: [NSURL fileURLWithPath: dir]];
        expect([[folder.URL path] isEqual: dir], [NSString stringWithFormat: @"URL is the folder (%@)", folder.URL]);
        expect([folder.items count] == 0, @"no items before -reload:");
        error = nil;
        expect([folder reload: &error], [NSString stringWithFormat: @"reload: succeeds (%@)", error]);
        NSString *want = @"A.txt a9.txt a10.txt b.txt Ünï x\\ny 日本.txt";
        // Compared canonically: macOS gives the names decomposed in URLs
        expect([[names(folder.items) decomposedStringWithCanonicalMapping]
                  isEqual: [want decomposedStringWithCanonicalMapping]],
              [NSString stringWithFormat: @"visible items in order: \"%@\", want \"%@\"", names(folder.items), want]);
        NSURL *parent = [[folder.items[0] URLByDeletingLastPathComponent] URLByResolvingSymlinksInPath];
        expect([[parent path] isEqual: [[[NSURL fileURLWithPath: dir] URLByResolvingSymlinksInPath] path]]
                  && [[folder.items[0] lastPathComponent] isEqual: @"A.txt"],
              @"items are URLs in the folder");

        expect([[FinderDirectory displayNameForURL: [NSURL fileURLWithPath: dir]] isEqual: [fm displayNameAtPath: dir]],
              @"displayNameForURL: is NSFileManager's display name");

        // The model re-reads the folder
        [fm createFileAtPath: [dir stringByAppendingPathComponent: @"c.txt"] contents: [NSData data] attributes: nil];
        [folder reload: NULL];
        expect([names(folder.items) containsString: @"b.txt c.txt"], @"reload: sees a new file");

        // A folder that does not exist
        FinderDirectory *missing = [[FinderDirectory alloc] initWithURL:
            [NSURL fileURLWithPath: [dir stringByAppendingPathComponent: @"missing"]]];
        error = nil;
        BOOL ok = [missing reload: &error];
        NSError *underlying = error.userInfo[NSUnderlyingErrorKey];
        BOOL enoent = ([error.domain isEqual: NSCocoaErrorDomain] && error.code == NSFileReadNoSuchFileError)
            || ([underlying.domain isEqual: NSPOSIXErrorDomain] && underlying.code == ENOENT)
            || ([error.domain isEqual: NSPOSIXErrorDomain] && error.code == ENOENT);
        expect(!ok && enoent, [NSString stringWithFormat: @"reload: of a missing folder fails with no-such-file (%@)", error]);
        expect([missing.items count] == 0, @"a missing folder has no items");

        // A folder that can't be read
        if (geteuid() == 0) {
            printf("SKIP: running as root, every folder is readable\n");
        } else {
            NSString *locked = [dir stringByAppendingPathComponent: @"locked"];
            [fm createDirectoryAtPath: locked withIntermediateDirectories: NO attributes: nil error: NULL];
            [fm createFileAtPath: [locked stringByAppendingPathComponent: @"inner"] contents: [NSData data] attributes: nil];
            chmod([locked fileSystemRepresentation], 0);
            FinderDirectory *unreadable = [[FinderDirectory alloc] initWithURL: [NSURL fileURLWithPath: locked]];
            error = nil;
            expect(![unreadable reload: &error] && error != nil,
                  [NSString stringWithFormat: @"reload: of an unreadable folder fails (%@)", error]);
            chmod([locked fileSystemRepresentation], 0755);
        }
        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
