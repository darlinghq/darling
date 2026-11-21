// Copyright (C) 2026 Lubos Dolezel
// LSCopyDefaultApplicationURLForURL for directories, against what macOS 26.7.3 does (see tests/volumes.md): folders
// and volumes go to the application declaring their type, which Finder does with the OSTypes 'fold' and 'disk';
// a directory with a declared package type goes to that type's handler.
// Usage: launchservices_folder_handler <dir> <volume>, where <dir> holds Finder.app (a stand-in with Finder's two
// document types and bundle ID) and <volume> is a mounted volume. Prints PASS/FAIL lines; exits 1 on a failure.
// Link with -framework Foundation -framework CoreServices.
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>

static int failures;

static void expect(NSString *path, NSString *want)
{
    CFURLRef app = LSCopyDefaultApplicationURLForURL((CFURLRef)[NSURL fileURLWithPath:path], kLSRolesAll, NULL);
    NSString *got = app != NULL ? [[(NSURL *)app path] stringByStandardizingPath] : @"(none)";
    BOOL ok = [got isEqualToString:want];
    printf("%s: %s -> %s%s%s\n", ok ? "PASS" : "FAIL", [path UTF8String], [got UTF8String],
        ok ? "" : ", expected ", ok ? "" : [want UTF8String]);
    if (!ok)
        failures++;
    if (app != NULL)
        CFRelease(app);
}

int main(int argc, char **argv)
{
    @autoreleasepool
    {
        if (argc != 3)
        {
            fprintf(stderr, "usage: %s <dir> <volume>\n", argv[0]);
            return 2;
        }
        NSString *dir = [[NSString stringWithUTF8String:argv[1]] stringByStandardizingPath];
        NSString *finder = [dir stringByAppendingPathComponent:@"Finder.app"];
        if (LSRegisterURL((CFURLRef)[NSURL fileURLWithPath:finder], true) != noErr)
        {
            printf("FAIL: the stand-in Finder cannot be registered\n");
            return 1;
        }
        NSFileManager *fm = [NSFileManager defaultManager];
        for (NSString *name in @[ @"plain", @"odd.foo", @"x.rtfd" ])
            [fm createDirectoryAtPath:[dir stringByAppendingPathComponent:name] withIntermediateDirectories:YES
                attributes:nil error:NULL];

        expect([dir stringByAppendingPathComponent:@"plain"], finder);
        expect([dir stringByAppendingPathComponent:@"odd.foo"], finder);
        expect(@"/", finder);
        expect([NSString stringWithUTF8String:argv[2]], finder);
        expect([dir stringByAppendingPathComponent:@"x.rtfd"], @"/Applications/TextEdit.app");
    }
    return failures != 0;
}
