// CFLAGS: -framework Foundation -framework AppKit -fobjc-arc -Isrc/Finder src/Finder/FinderInfoPanelController.m
// ARGS: "$(mktemp -d)/w"
// Copyright (C) 2026 Lubos Dolezel
//
// Finder's Get Info panel (src/Finder/FinderInfoPanelController), in process: for a file, the title "<name> Info" and
// the rows name, kind (the file's localized type description), size ("<formatted> (<n> bytes)"), where (the parent's
// path) and modified (medium date, short time); for a folder, "Calculating…" while it is counted, then
// "<formatted> for <n> items", every file and folder inside counting as an item; asking again for the same item gives
// the same panel; closing the panel releases it. See tests/finder.md. Needs a display.
// Usage: finder_get_info <new directory>. Prints PASS/FAIL lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderInfoPanelController.h"
#include <stdio.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    failures += !ok;
}

static BOOL spinUntil(BOOL (^done)(void), NSTimeInterval seconds)
{
    NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: seconds];
    while (!done() && [limit timeIntervalSinceNow] > 0) {
        @autoreleasepool {
            [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.05]];
        }
    }
    return done();
}

static void writeBytes(NSURL *url, NSUInteger count)
{
    NSMutableData *data = [NSMutableData dataWithLength: count];
    [data writeToURL: url atomically: NO];
}

static NSString *grouped(NSInteger n)
{
    NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
    formatter.numberStyle = NSNumberFormatterDecimalStyle;
    return [formatter stringFromNumber: @(n)];
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        setvbuf(stdout, NULL, _IONBF, 0);
        if (argc < 2) {
            fprintf(stderr, "usage: finder_get_info <new directory>\n");
            return 2;
        }
        [NSApplication sharedApplication];
        NSFileManager *fm = [NSFileManager defaultManager];
        NSURL *root = [NSURL fileURLWithPath: [NSString stringWithUTF8String: argv[1]] isDirectory: YES];
        [fm createDirectoryAtURL: root withIntermediateDirectories: YES attributes: nil error: NULL];

        // A file
        NSURL *file = [root URLByAppendingPathComponent: @"f.txt"];
        writeBytes(file, 1234);
        // AppKit's own autoreleased references to the panel go with this pool
        __weak FinderInfoPanelController *weakInfo = nil;
        @autoreleasepool {
            FinderInfoPanelController *info = [FinderInfoPanelController showInfoForItem: file];
            expect(info != nil && [info.window isVisible], @"file: a panel is shown");
            expect([info.window.title isEqual: @"f.txt Info"], [NSString stringWithFormat: @"file: the title (%@)", info.window.title]);
            expect([[info valueForRow: @"name"] isEqual: @"f.txt"], [NSString stringWithFormat: @"file: name (%@)", [info valueForRow: @"name"]]);
            NSString *kind = nil;
            [file getResourceValue: &kind forKey: NSURLLocalizedTypeDescriptionKey error: NULL];
            expect(kind != nil && [[info valueForRow: @"kind"] isEqual: kind],
                   [NSString stringWithFormat: @"file: kind (%@, expected %@)", [info valueForRow: @"kind"], kind]);
            NSString *size = [NSString stringWithFormat: @"%@ (%@ bytes)",
                                                         [NSByteCountFormatter stringFromByteCount: 1234 countStyle: NSByteCountFormatterCountStyleFile],
                                                         grouped(1234)];
            expect([[info valueForRow: @"size"] isEqual: size],
                   [NSString stringWithFormat: @"file: size (%@, expected %@)", [info valueForRow: @"size"], size]);
            expect([[info valueForRow: @"where"] isEqual: [[file URLByDeletingLastPathComponent] path]],
                   [NSString stringWithFormat: @"file: where (%@)", [info valueForRow: @"where"]]);
            NSDate *modified = nil;
            [file getResourceValue: &modified forKey: NSURLContentModificationDateKey error: NULL];
            NSDateFormatter *dates = [[NSDateFormatter alloc] init];
            dates.dateStyle = NSDateFormatterMediumStyle;
            dates.timeStyle = NSDateFormatterShortStyle;
            expect([[info valueForRow: @"modified"] isEqual: [dates stringFromDate: modified]],
                   [NSString stringWithFormat: @"file: modified (%@)", [info valueForRow: @"modified"]]);
            expect([info valueForRow: @"created"] != nil, @"file: created has a value");
            expect(!info.counting, @"file: nothing is counted");
            expect([FinderInfoPanelController showInfoForItem: file] == info, @"file: asking again gives the same panel");

            weakInfo = info;
            [info close];
        }
        expect(spinUntil(^BOOL { return weakInfo == nil; }, 5), @"file: closing the panel releases it");

        // A folder: three 1000-byte files in a subfolder
        NSURL *folder = [root URLByAppendingPathComponent: @"folder"];
        NSURL *sub = [folder URLByAppendingPathComponent: @"sub"];
        [fm createDirectoryAtURL: sub withIntermediateDirectories: YES attributes: nil error: NULL];
        for (NSString *name in @[ @"1", @"2", @"3" ])
            writeBytes([sub URLByAppendingPathComponent: name], 1000);
        FinderInfoPanelController *folderInfo = [FinderInfoPanelController showInfoForItem: folder];
        expect([folderInfo.window.title isEqual: @"folder Info"], [NSString stringWithFormat: @"folder: the title (%@)", folderInfo.window.title]);
        expect(!folderInfo.counting || [[folderInfo valueForRow: @"size"] isEqual: @"Calculating…"],
               [NSString stringWithFormat: @"folder: Calculating… while counting (%@)", [folderInfo valueForRow: @"size"]]);
        expect(spinUntil(^BOOL { return !folderInfo.counting; }, 20), @"folder: counting ends");
        NSString *folderSize = [NSString stringWithFormat: @"%@ for 4 items",
                                                           [NSByteCountFormatter stringFromByteCount: 3000 countStyle: NSByteCountFormatterCountStyleFile]];
        expect([[folderInfo valueForRow: @"size"] isEqual: folderSize],
               [NSString stringWithFormat: @"folder: size (%@, expected %@)", [folderInfo valueForRow: @"size"], folderSize]);
        expect(folderInfo != [FinderInfoPanelController showInfoForItem: file], @"another item gets its own panel");
        [folderInfo close];

        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
