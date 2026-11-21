// Copyright (C) 2026 Lubos Dolezel
// LSCopyDefaultApplicationURLForURL for directories, against what macOS 26.7.3 does (see tests/volumes.md): folders
// and volumes go to the application declaring their type, which Finder does with the OSTypes 'fold' and 'disk';
// a directory with a declared package type goes to that type's handler. A directory's own type decides: on macOS
// 26.7.3 an application claiming public.item or public.directory is no candidate for a folder, one claiming
// public.folder none for a volume, and Finder stays the default for folders when another application claims
// public.folder with the same rank (tests/volumes.md).
// Usage: launchservices_folder_handler <dir> <volume>, where <dir> holds Finder.app (a stand-in with Finder's two
// document types and bundle ID) and <volume> is a mounted volume. Where Finder is installed, it is the expected
// handler, the stand-in competing with it. Prints PASS/FAIL lines; exits 1 on a failure.
// Link with -framework Foundation -framework CoreServices.
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>

static int failures;

static void expect(NSString *path, NSString *want);

// An application bundle in dir whose one document type claims the content type, role Editor, rank Default.
static NSString *makeClaimer(NSString *dir, NSString *name, NSString *type, NSDictionary *exported)
{
    NSString *app = [dir stringByAppendingPathComponent:[name stringByAppendingString:@".app"]];
    [[NSFileManager defaultManager] createDirectoryAtPath:[app stringByAppendingPathComponent:@"Contents/MacOS"]
        withIntermediateDirectories:YES attributes:nil error:NULL];
    NSMutableDictionary *plist = [@{
        @"CFBundleExecutable" : name,
        @"CFBundleIdentifier" : [@"org.darlinghq.test.folderhandler." stringByAppendingString:name],
        @"CFBundlePackageType" : @"APPL",
        @"CFBundleDocumentTypes" : @[ @{ @"CFBundleTypeName" : @"Thing", @"CFBundleTypeRole" : @"Editor",
            @"LSHandlerRank" : @"Default", @"LSItemContentTypes" : @[ type ] } ],
    } mutableCopy];
    if (exported != nil)
        plist[@"UTExportedTypeDeclarations"] = @[ exported ];
    [plist writeToFile:[app stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:YES];
    if (LSRegisterURL((CFURLRef)[NSURL fileURLWithPath:app], true) != noErr)
    {
        printf("FAIL: %s cannot be registered\n", [name UTF8String]);
        failures++;
    }
    return app;
}

static void expectNot(NSString *path, NSString *unwanted)
{
    CFURLRef app = LSCopyDefaultApplicationURLForURL((CFURLRef)[NSURL fileURLWithPath:path], kLSRolesAll, NULL);
    NSString *got = app != NULL ? [[(NSURL *)app path] stringByStandardizingPath] : @"(none)";
    BOOL ok = ![got isEqualToString:unwanted];
    printf("%s: %s -> %s%s%s\n", ok ? "PASS" : "FAIL", [path UTF8String], [got UTF8String],
        ok ? "" : ", not expected: ", ok ? "" : [unwanted UTF8String]);
    if (!ok)
        failures++;
    if (app != NULL)
        CFRelease(app);
}

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
        NSString *standIn = [dir stringByAppendingPathComponent:@"Finder.app"];
        if (LSRegisterURL((CFURLRef)[NSURL fileURLWithPath:standIn], true) != noErr)
        {
            printf("FAIL: the stand-in Finder cannot be registered\n");
            return 1;
        }
        NSFileManager *fm = [NSFileManager defaultManager];
        // Where Finder is installed it was registered first, so it stays the default (with the same rank, the
        // application registered first is) and the stand-in is the competing claimer
        NSString *installed = @"/System/Library/CoreServices/Finder.app";
        NSString *finder = [fm fileExistsAtPath:installed] ? installed : standIn;
        for (NSString *name in @[ @"plain", @"odd.foo", @"x.rtfd" ])
            [fm createDirectoryAtPath:[dir stringByAppendingPathComponent:name] withIntermediateDirectories:YES
                attributes:nil error:NULL];

        expect([dir stringByAppendingPathComponent:@"plain"], finder);
        expect([dir stringByAppendingPathComponent:@"odd.foo"], finder);
        expect(@"/", finder);
        expect([NSString stringWithUTF8String:argv[2]], finder);
        expect([dir stringByAppendingPathComponent:@"x.rtfd"], @"/Applications/TextEdit.app");

        // Claims of the types a directory's type conforms to do not count; with the same rank, the application
        // registered first (here Finder) is the default.
        NSString *item = makeClaimer(dir, @"ItemClaimer", @"public.item", nil);
        NSString *directory = makeClaimer(dir, @"DirectoryClaimer", @"public.directory", nil);
        makeClaimer(dir, @"FolderClaimer", @"public.folder", @{
            @"UTTypeIdentifier" : @"org.darlinghq.test.folderhandler.package",
            @"UTTypeConformsTo" : @[ @"com.apple.package" ],
            @"UTTypeTagSpecification" : @{ @"public.filename-extension" : @[ @"fhpkg" ] },
        });
        [fm createDirectoryAtPath:[dir stringByAppendingPathComponent:@"y.fhpkg"] withIntermediateDirectories:YES
            attributes:nil error:NULL];
        expect([dir stringByAppendingPathComponent:@"plain"], finder);
        expect([NSString stringWithUTF8String:argv[2]], finder);
        expect(@"/", finder);
        // A package type nobody claims: not the application claiming public.item or public.directory.
        expectNot([dir stringByAppendingPathComponent:@"y.fhpkg"], item);
        expectNot([dir stringByAppendingPathComponent:@"y.fhpkg"], directory);
    }
    return failures != 0;
}
