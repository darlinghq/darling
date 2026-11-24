// CFLAGS: -framework Foundation -fobjc-arc
// ARGS: /Volumes/SystemRoot$PWD/tests/fixtures/styled-test.dmg
// Copyright (C) 2026 Lubos Dolezel
//
// Extended attributes on an attached disk image's HFS+ volume: styled-test.dmg (tests/fixtures/make-styled-fixture.py)
// has "Hidden Notes.txt" with the Finder's invisible flag in its com.apple.FinderInfo. listxattr names
// com.apple.FinderInfo, getxattr gives its 32 bytes (byte 8 = 0x40), so NSURLIsHiddenKey is YES; an item without
// FinderInfo ("Styled App.app/Contents/Info.plist") lists none and getxattr fails with ENOATTR. It attaches the image
// and detaches it. Passes unchanged on macOS 10.14.4 and 26.7.3. Usage: dmg_xattr <styled-test.dmg>
#import <Foundation/Foundation.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    failures += !ok;
}

static BOOL lists(const char *path, const char *name)
{
    char names[4096];
    ssize_t length = listxattr(path, names, sizeof names, XATTR_NOFOLLOW);
    for (ssize_t i = 0; i < length; i += strlen(names + i) + 1)
        if (strcmp(names + i, name) == 0)
            return YES;
    return NO;
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        setvbuf(stdout, NULL, _IONBF, 0);
        if (argc != 2) {
            fprintf(stderr, "usage: %s <styled-test.dmg>\n", argv[0]);
            return 2;
        }
        NSString *volume = @"/Volumes/Styled Test";
        NSTask *attach = [NSTask launchedTaskWithLaunchPath: @"/usr/bin/hdiutil"
                                                  arguments: @[ @"attach", @"-nobrowse", @"-readonly", @"-quiet",
                                                                [NSString stringWithUTF8String: argv[1]] ]];
        [attach waitUntilExit];
        expect([attach terminationStatus] == 0, @"the image attaches");

        const char *notes = [[volume stringByAppendingPathComponent: @"Hidden Notes.txt"] fileSystemRepresentation];
        expect(lists(notes, "com.apple.FinderInfo"), @"listxattr names com.apple.FinderInfo");
        unsigned char info[32] = { 0 };
        ssize_t length = getxattr(notes, "com.apple.FinderInfo", info, sizeof info, 0, XATTR_NOFOLLOW);
        expect(length == 32 && info[8] == 0x40,
               [NSString stringWithFormat: @"getxattr gives 32 bytes with the invisible flag (%zd, %s)", length,
                length < 0 ? strerror(errno) : "-"]);
        expect(getxattr(notes, "com.apple.FinderInfo", NULL, 0, 0, XATTR_NOFOLLOW) == 32, @"its size is 32");
        NSNumber *hidden = nil;
        [[NSURL fileURLWithPath: [volume stringByAppendingPathComponent: @"Hidden Notes.txt"]] getResourceValue: &hidden
                                                                                                    forKey: NSURLIsHiddenKey
                                                                                                     error: NULL];
        expect([hidden boolValue], @"NSURLIsHiddenKey is YES");

        const char *plist = [[volume stringByAppendingPathComponent: @"Styled App.app/Contents/Info.plist"] fileSystemRepresentation];
        expect(!lists(plist, "com.apple.FinderInfo"), @"an item without FinderInfo lists none");
        errno = 0;
        expect(getxattr(plist, "com.apple.FinderInfo", info, sizeof info, 0, XATTR_NOFOLLOW) < 0 && errno == ENOATTR,
               [NSString stringWithFormat: @"and getxattr fails with ENOATTR (%s)", strerror(errno)]);

        NSTask *detach = [NSTask launchedTaskWithLaunchPath: @"/usr/bin/hdiutil" arguments: @[ @"detach", @"-quiet", volume ]];
        [detach waitUntilExit];
        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
