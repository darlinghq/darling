// CFLAGS: -framework Foundation -framework AppKit -framework ImageIO -fobjc-arc -Isrc/Finder src/Finder/FinderAppDelegate.m src/Finder/FinderDirectory.m src/Finder/FinderDSStore.m src/Finder/FinderFolderStyle.m src/Finder/FinderFreeformLayout.m src/Finder/FinderDrop.m src/Finder/FinderIconViewController.m src/Finder/FinderListViewController.m src/Finder/FinderSidebarController.m src/Finder/FinderWindowController.m src/Finder/FinderInfoPanelController.m src/Finder/FinderFileOperation.m src/Finder/FinderItemActions.m src/Finder/FinderOperationQueue.m
// ARGS: "$(mktemp -d)/r"
// Copyright (C) 2026 Lubos Dolezel
//
// Revealing items in Finder (src/Finder/FinderAppDelegate), in process: openURLs: with files in two folders, a file
// that doesn't exist and a folder gives one window per folder, each with its items selected, and a window for the
// folder itself with nothing selected; revealing an item that appeared later selects it in the open window. See tests/finder.md. Needs a display.
// Usage: finder_reveal <new directory>. Prints PASS/FAIL lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderAppDelegate.h"
#import "FinderWindowController.h"
#include <stdio.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSString *names(NSArray<NSURL *> *urls)
{
    NSMutableArray *names = [NSMutableArray array];
    for (NSURL *url in urls)
        [names addObject: [url lastPathComponent]];
    return [names componentsJoinedByString: @" "];
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        if (argc != 2) {
            fprintf(stderr, "usage: %s <new directory>\n", argv[0]);
            return 2;
        }
        [NSApplication sharedApplication];
        NSFileManager *fm = [NSFileManager defaultManager];
        NSString *dir = [NSString stringWithUTF8String: argv[1]];
        for (NSString *folder in @[ @"a", @"b", @"c" ])
            [fm createDirectoryAtPath: [dir stringByAppendingPathComponent: folder] withIntermediateDirectories: YES
                           attributes: nil error: NULL];
        for (NSString *file in @[ @"a/x.txt", @"a/z.txt", @"a/other.txt", @"b/y.txt", @"c/inside.txt" ])
            [fm createFileAtPath: [dir stringByAppendingPathComponent: file] contents: [NSData data] attributes: nil];
        NSURL *(^url)(NSString *) = ^NSURL *(NSString *path) {
            return [NSURL fileURLWithPath: [dir stringByAppendingPathComponent: path]];
        };

        FinderAppDelegate *finder = [[FinderAppDelegate alloc] init];
        [finder openURLs: @[ url(@"a/x.txt"), url(@"b/y.txt"), url(@"a/z.txt"), url(@"a/missing.txt"), url(@"c") ]];
        [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.5]];

        NSMutableDictionary<NSString *, FinderWindowController *> *windows = [NSMutableDictionary dictionary];
        for (FinderWindowController *controller in finder.windowControllers)
            windows[[controller.folderURL lastPathComponent]] = controller;
        NSArray *shown = [[windows allKeys] sortedArrayUsingSelector: @selector(compare:)];
        expect([shown isEqual: (@[ @"a", @"b", @"c" ])],
               [NSString stringWithFormat: @"windows for a, b and c (%@)", [shown componentsJoinedByString: @" "]]);
        expect([names(windows[@"a"].selectedItems) isEqual: @"x.txt z.txt"],
               [NSString stringWithFormat: @"a's items are selected (%@)", names(windows[@"a"].selectedItems)]);
        expect([names(windows[@"b"].selectedItems) isEqual: @"y.txt"],
               [NSString stringWithFormat: @"b's item is selected (%@)", names(windows[@"b"].selectedItems)]);
        expect(windows[@"c"] != nil && [windows[@"c"].selectedItems count] == 0,
               [NSString stringWithFormat: @"the folder c is shown, with nothing selected (%@)", names(windows[@"c"].selectedItems)]);

        // Revealing an item that appeared after its folder's window opened: the window lists and selects it
        [fm createFileAtPath: [dir stringByAppendingPathComponent: @"a/new.txt"] contents: [NSData data] attributes: nil];
        [finder openURLs: @[ url(@"a/new.txt") ]];
        [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.5]];
        expect([finder.windowControllers count] == 3, @"no new window for a folder already shown");
        expect([names(windows[@"a"].selectedItems) isEqual: @"new.txt"],
               [NSString stringWithFormat: @"a new item revealed in an open window is selected (%@)", names(windows[@"a"].selectedItems)]);

        for (FinderWindowController *controller in finder.windowControllers)
            [controller close];
        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
