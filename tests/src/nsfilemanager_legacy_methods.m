// Copyright (C) 2026 Lubos Dolezel
//
// The deprecated and the link/replace/trash methods NSFileManager declares: hard links,
// content comparison, file system attributes, symbolic link creation, item replacement,
// trashing, and the iCloud queries on a system without iCloud. Expected values follow the
// documented contracts and black-box observation on macOS 10.14.
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

static NSFileManager *fm;
static NSString *root;
static NSString *P(NSString *s) { return [root stringByAppendingPathComponent:s]; }
static void W(NSString *n, NSString *c) { [fm createFileAtPath:P(n) contents:[c dataUsingEncoding:NSUTF8StringEncoding] attributes:nil]; }
static void D(NSString *n) { [fm createDirectoryAtPath:P(n) withIntermediateDirectories:YES attributes:nil error:nil]; }
static NSString *R(NSString *n) { return [NSString stringWithContentsOfFile:P(n) encoding:NSUTF8StringEncoding error:nil]; }
static BOOL Eq(NSString *a, NSString *b) { return [fm contentsEqualAtPath:P(a) andPath:P(b)]; }
static ino_t Ino(NSString *n) { struct stat st; return stat([P(n) fileSystemRepresentation], &st) == 0 ? st.st_ino : 0; }

int main(void)
{
    @autoreleasepool {
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        fm = [NSFileManager defaultManager];
        root = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"fm-legacy-%d", getpid()]];
        [fm removeItemAtPath:root error:nil];
        D(@".");

        // contentsEqualAtPath:andPath:
        W(@"a", @"hello"); W(@"b", @"hello"); W(@"c", @"hellp"); W(@"d", @"hell");
        D(@"d1"); D(@"d1/s"); W(@"d1/x", @"1"); W(@"d1/s/y", @"2");
        D(@"d2"); D(@"d2/s"); W(@"d2/x", @"1"); W(@"d2/s/y", @"2");
        D(@"d3"); D(@"d3/s"); W(@"d3/x", @"1"); W(@"d3/s/y", @"3");
        D(@"d4"); W(@"d4/x", @"1");
        D(@"e1"); D(@"e2");
        [fm createSymbolicLinkAtPath:P(@"l1") withDestinationPath:P(@"a") error:nil];
        [fm createSymbolicLinkAtPath:P(@"l2") withDestinationPath:P(@"b") error:nil];
        [fm createSymbolicLinkAtPath:P(@"l3") withDestinationPath:P(@"a") error:nil];
        CHECK(Eq(@"a", @"a"));
        CHECK(Eq(@"a", @"b"));
        CHECK(!Eq(@"a", @"c"));
        CHECK(!Eq(@"a", @"d"));
        CHECK(Eq(@"d1", @"d2"));
        CHECK(!Eq(@"d1", @"d3"));
        CHECK(!Eq(@"d1", @"d4"));
        CHECK(Eq(@"e1", @"e2"));
        CHECK(!Eq(@"a", @"d1"));
        CHECK(!Eq(@"a", @"missing"));
        CHECK(!Eq(@"missing", @"missing"));
        CHECK(!Eq(@"l1", @"a"));
        CHECK(Eq(@"l1", @"l3"));
        CHECK(!Eq(@"l1", @"l2"));
        CHECK(![fm contentsEqualAtPath:@"" andPath:@""]);

        // fileAttributesAtPath:traverseLink: describes the destination for YES and the link itself for NO
        NSDictionary *followed = [fm fileAttributesAtPath:P(@"l1") traverseLink:YES];
        NSDictionary *notFollowed = [fm fileAttributesAtPath:P(@"l1") traverseLink:NO];
        CHECK([[followed fileType] isEqual:NSFileTypeRegular]);
        CHECK([followed fileSize] == 5);
        CHECK([[notFollowed fileType] isEqual:NSFileTypeSymbolicLink]);
        CHECK([[fm fileAttributesAtPath:P(@"a") traverseLink:YES] fileSize] == 5);
        CHECK([fm fileAttributesAtPath:P(@"missing") traverseLink:YES] == nil);
        CHECK([fm fileAttributesAtPath:P(@"missing") traverseLink:NO] == nil);

        // createSymbolicLinkAtPath:pathContent: and the URL variant
        CHECK([fm createSymbolicLinkAtPath:P(@"s1") pathContent:@"target"]);
        CHECK([[fm pathContentOfSymbolicLinkAtPath:P(@"s1")] isEqualToString:@"target"]);
        CHECK(![fm createSymbolicLinkAtPath:P(@"s1") pathContent:@"other"]);
        CHECK(![fm createSymbolicLinkAtPath:P(@"a") pathContent:@"t"]);
        CHECK(![fm createSymbolicLinkAtPath:P(@"nodir/s") pathContent:@"t"]);
        NSError *e = nil;
        CHECK([fm createSymbolicLinkAtURL:[NSURL fileURLWithPath:P(@"su")] withDestinationURL:[NSURL fileURLWithPath:P(@"a")] error:&e] && e == nil);
        CHECK([[fm pathContentOfSymbolicLinkAtPath:P(@"su")] isEqualToString:P(@"a")]);

        // linkItemAtPath:toPath:error:
        e = nil;
        CHECK([fm linkItemAtPath:P(@"a") toPath:P(@"a-link") error:&e] && e == nil);
        CHECK(Ino(@"a") == Ino(@"a-link") && Ino(@"a") != 0);
        e = nil;
        CHECK(![fm linkItemAtPath:P(@"a") toPath:P(@"b") error:&e]);
        CHECK([e.domain isEqualToString:NSCocoaErrorDomain] && e.code == NSFileWriteFileExistsError);
        e = nil;
        CHECK(![fm linkItemAtPath:P(@"missing") toPath:P(@"m2") error:&e]);
        CHECK([e.domain isEqualToString:NSCocoaErrorDomain] && e.code == NSFileReadNoSuchFileError);
        e = nil;
        CHECK(![fm linkItemAtPath:P(@"a") toPath:P(@"nodir2/x") error:&e]);
        CHECK([e.domain isEqualToString:NSCocoaErrorDomain] && e.code == NSFileNoSuchFileError);
        e = nil;
        CHECK([fm linkItemAtPath:P(@"d1") toPath:P(@"d1-link") error:&e] && e == nil);
        BOOL isDir = NO;
        CHECK([fm fileExistsAtPath:P(@"d1-link") isDirectory:&isDir] && isDir);
        CHECK(Ino(@"d1/x") == Ino(@"d1-link/x") && Ino(@"d1/s/y") == Ino(@"d1-link/s/y"));
        CHECK(Ino(@"d1") != Ino(@"d1-link"));
        CHECK([fm linkItemAtURL:[NSURL fileURLWithPath:P(@"a")] toURL:[NSURL fileURLWithPath:P(@"a-link2")] error:&e]);
        CHECK(Ino(@"a") == Ino(@"a-link2"));

        // fileSystemAttributesAtPath:
        NSDictionary *fsa = [fm fileSystemAttributesAtPath:root];
        CHECK(fsa != nil);
        CHECK([fsa[NSFileSystemSize] unsignedLongLongValue] > 0);
        CHECK(fsa[NSFileSystemFreeSize] != nil && fsa[NSFileSystemNodes] != nil && fsa[NSFileSystemFreeNodes] != nil && fsa[NSFileSystemNumber] != nil);
        CHECK([fm fileSystemAttributesAtPath:P(@"missing")] == nil);

        // replaceItemAtURL:...
        NSURL *res = nil;
        W(@"orig", @"old"); W(@"new", @"new");
        e = nil;
        CHECK([fm replaceItemAtURL:[NSURL fileURLWithPath:P(@"orig")] withItemAtURL:[NSURL fileURLWithPath:P(@"new")] backupItemName:@"bk" options:0 resultingItemURL:&res error:&e]);
        CHECK([R(@"orig") isEqualToString:@"new"] && ![fm fileExistsAtPath:P(@"new")] && ![fm fileExistsAtPath:P(@"bk")]);
        CHECK([res.path isEqualToString:P(@"orig")]);
        W(@"o2", @"old2"); W(@"n2", @"new2");
        CHECK([fm replaceItemAtURL:[NSURL fileURLWithPath:P(@"o2")] withItemAtURL:[NSURL fileURLWithPath:P(@"n2")] backupItemName:nil options:0 resultingItemURL:&res error:&e]);
        CHECK([R(@"o2") isEqualToString:@"new2"] && ![fm fileExistsAtPath:P(@"n2")]);
        // a missing original is not an error: the new item is moved into place (macOS 10.14)
        e = nil; res = nil;
        CHECK([fm replaceItemAtURL:[NSURL fileURLWithPath:P(@"zz")] withItemAtURL:[NSURL fileURLWithPath:P(@"o2")] backupItemName:nil options:0 resultingItemURL:&res error:&e]);
        CHECK([R(@"zz") isEqualToString:@"new2"] && ![fm fileExistsAtPath:P(@"o2")] && [res.path isEqualToString:P(@"zz")]);
        // a missing new item is an error (macOS 10.14: NSFileWriteUnknownError)
        e = nil;
        CHECK(![fm replaceItemAtURL:[NSURL fileURLWithPath:P(@"zz")] withItemAtURL:[NSURL fileURLWithPath:P(@"o2")] backupItemName:nil options:0 resultingItemURL:&res error:&e]);
        CHECK([e.domain isEqualToString:NSCocoaErrorDomain] && e.code == NSFileWriteUnknownError);
        CHECK([R(@"zz") isEqualToString:@"new2"]);
        W(@"o3", @"old3"); W(@"n3", @"new3");
        CHECK([fm replaceItemAtURL:[NSURL fileURLWithPath:P(@"o3")] withItemAtURL:[NSURL fileURLWithPath:P(@"n3")] backupItemName:@"bk3" options:NSFileManagerItemReplacementWithoutDeletingBackupItem resultingItemURL:&res error:&e]);
        CHECK([R(@"o3") isEqualToString:@"new3"] && [R(@"bk3") isEqualToString:@"old3"]);
        D(@"od"); W(@"od/f", @"1"); D(@"nd"); W(@"nd/g", @"2");
        CHECK([fm replaceItemAtURL:[NSURL fileURLWithPath:P(@"od")] withItemAtURL:[NSURL fileURLWithPath:P(@"nd")] backupItemName:nil options:0 resultingItemURL:&res error:&e]);
        CHECK([fm fileExistsAtPath:P(@"od/g")] && ![fm fileExistsAtPath:P(@"od/f")]);

        // trashItemAtURL:resultingItemURL:error:
        NSString *trashName = [NSString stringWithFormat:@"fm-legacy-trash-%d", getpid()];
        W(trashName, @"t");
        NSURL *trashed = nil;
        e = nil;
        CHECK([fm trashItemAtURL:[NSURL fileURLWithPath:P(trashName)] resultingItemURL:&trashed error:&e] && e == nil);
        CHECK(trashed != nil && [trashed.path containsString:@"/.Trash/"] && [fm fileExistsAtPath:trashed.path]);
        CHECK(![fm fileExistsAtPath:P(trashName)]);
        if (trashed) [fm removeItemAtURL:trashed error:nil];
        e = nil;
        CHECK(![fm trashItemAtURL:[NSURL fileURLWithPath:P(@"missing")] resultingItemURL:&trashed error:&e]);
        CHECK([e.domain isEqualToString:NSCocoaErrorDomain] && e.code == NSFileNoSuchFileError);

        // iCloud queries without iCloud
        NSURL *u = [NSURL fileURLWithPath:P(@"a")];
        CHECK([fm ubiquityIdentityToken] == nil);
        CHECK([fm URLForUbiquityContainerIdentifier:nil] == nil);
        CHECK(![fm isUbiquitousItemAtURL:u]);
        e = nil;
        CHECK(![fm startDownloadingUbiquitousItemAtURL:u error:&e] && e != nil);
        e = nil;
        CHECK(![fm evictUbiquitousItemAtURL:u error:&e] && e != nil);

        [fm removeItemAtPath:root error:nil];
        printf(failures ? "FAIL (%d)\n" : "PASS\n", failures);
        return failures ? 1 : 0;
    }
}
