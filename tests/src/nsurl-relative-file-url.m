// Copyright (C) 2026 Lubos Dolezel
// +fileURLWithPath:relativeToURL: and the isDirectory:relativeToURL: / init variants.
#import <Foundation/Foundation.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

int main(void)
{
    @autoreleasepool {
        char tmpl[] = "/tmp/nsurl-rel.XXXXXX";
        if (!mkdtemp(tmpl)) return 2;
        NSString *root = [[NSString stringWithUTF8String:tmpl] stringByResolvingSymlinksInPath];
        NSString *dir = [root stringByAppendingPathComponent:@"d"];
        mkdir([dir UTF8String], 0755);
        close(open([[dir stringByAppendingPathComponent:@"f"] UTF8String], O_CREAT | O_WRONLY, 0644));
        mkdir([[dir stringByAppendingPathComponent:@"sub"] UTF8String], 0755);

        NSURL *base = [NSURL fileURLWithPath:dir isDirectory:YES];
        CHECK([[base absoluteString] hasSuffix:@"/d/"]);

        // Relative path against a directory base.
        NSURL *f = [NSURL fileURLWithPath:@"f" relativeToURL:base];
        CHECK(f != nil);
        CHECK([[f path] isEqual:[dir stringByAppendingPathComponent:@"f"]]);
        CHECK([[f relativePath] isEqual:@"f"]);
        CHECK([[f baseURL] isEqual:base]);
        CHECK([f isFileURL]);
        CHECK([[f relativeString] isEqual:@"f"]);

        // An existing directory is detected against the base, not the working directory.
        chdir("/");
        NSURL *sub = [NSURL fileURLWithPath:@"sub" relativeToURL:base];
        CHECK([[sub absoluteString] hasSuffix:@"/d/sub/"]);
        CHECK([[sub path] isEqual:[dir stringByAppendingPathComponent:@"sub"]]);
        NSURL *nosuch = [NSURL fileURLWithPath:@"nosuch" relativeToURL:base];
        CHECK(![[nosuch absoluteString] hasSuffix:@"/"]);

        // Explicit isDirectory decides the trailing slash.
        NSURL *asDir = [NSURL fileURLWithPath:@"nosuch" isDirectory:YES relativeToURL:base];
        CHECK([[asDir absoluteString] hasSuffix:@"/d/nosuch/"]);
        CHECK([[asDir relativeString] isEqual:@"nosuch/"]);
        NSURL *asFile = [NSURL fileURLWithPath:@"sub" isDirectory:NO relativeToURL:base];
        CHECK(![[asFile absoluteString] hasSuffix:@"/"]);
        CHECK([[asFile baseURL] isEqual:base]);

        // An absolute path ignores the base.
        NSString *abs = [dir stringByAppendingPathComponent:@"f"];
        NSURL *a = [NSURL fileURLWithPath:abs relativeToURL:base];
        CHECK([[a path] isEqual:abs]);
        CHECK([a baseURL] == nil);
        NSURL *a2 = [NSURL fileURLWithPath:abs isDirectory:NO relativeToURL:[NSURL fileURLWithPath:@"/usr" isDirectory:YES]];
        CHECK([[a2 path] isEqual:abs] && [a2 baseURL] == nil);

        // Path with spaces, and a ".." component.
        NSURL *sp = [NSURL fileURLWithPath:@"a b/c d" isDirectory:NO relativeToURL:base];
        CHECK([[sp path] isEqual:[dir stringByAppendingPathComponent:@"a b/c d"]]);
        CHECK([[sp absoluteString] hasSuffix:@"/d/a%20b/c%20d"]);
        NSURL *up = [NSURL fileURLWithPath:@"../d/f" relativeToURL:base];
        CHECK([[[up URLByStandardizingPath] path] isEqual:abs]);

        // A base without a trailing slash is a file: the relative path replaces its last component.
        NSURL *fileBase = [NSURL fileURLWithPath:dir isDirectory:NO];
        NSURL *sib = [NSURL fileURLWithPath:@"f" isDirectory:NO relativeToURL:fileBase];
        CHECK([[sib path] isEqual:[root stringByAppendingPathComponent:@"f"]]);

        // A nil base behaves like +fileURLWithPath:.
        chdir([dir UTF8String]);
        NSURL *n1 = [NSURL fileURLWithPath:@"sub" relativeToURL:nil];
        NSURL *n2 = [NSURL fileURLWithPath:@"sub"];
        CHECK([[n1 absoluteString] isEqual:[n2 absoluteString]]);
        NSURL *n3 = [NSURL fileURLWithPath:@"sub" isDirectory:YES relativeToURL:nil];
        NSURL *n4 = [NSURL fileURLWithPath:@"sub" isDirectory:YES];
        CHECK([[n3 absoluteString] isEqual:[n4 absoluteString]]);
        chdir("/");

        // Init variants.
        NSURL *i1 = [[NSURL alloc] initFileURLWithPath:@"sub" relativeToURL:base];
        CHECK([[i1 absoluteString] hasSuffix:@"/d/sub/"] && [[i1 baseURL] isEqual:base]);
        NSURL *i2 = [[NSURL alloc] initFileURLWithPath:@"f" isDirectory:YES relativeToURL:base];
        CHECK([[i2 absoluteString] hasSuffix:@"/d/f/"]);

        // nil path raises NSInvalidArgumentException.
        BOOL raised = NO;
        @try { (void)[NSURL fileURLWithPath:nil relativeToURL:base]; }
        @catch (NSException *e) { raised = [[e name] isEqual:NSInvalidArgumentException]; }
        CHECK(raised);
        raised = NO;
        @try { (void)[NSURL fileURLWithPath:nil isDirectory:NO relativeToURL:base]; }
        @catch (NSException *e) { raised = [[e name] isEqual:NSInvalidArgumentException]; }
        CHECK(raised);

        // POSIX cleanup: the test does not depend on NSFileManager removal.
        unlink([[dir stringByAppendingPathComponent:@"f"] UTF8String]);
        rmdir([[dir stringByAppendingPathComponent:@"sub"] UTF8String]);
        rmdir([dir UTF8String]);
        rmdir([root UTF8String]);
    }
    if (failures) return 1;
    printf("PASS\n");
    return 0;
}
