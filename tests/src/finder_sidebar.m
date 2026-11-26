// CFLAGS: -framework Foundation -framework AppKit -fobjc-arc -Isrc/Finder src/Finder/FinderSidebarController.m src/Finder/FinderDrop.m src/Finder/FinderFileOperation.m
// Copyright (C) 2026 Lubos Dolezel
//
// A Finder window's sidebar (src/Finder/FinderSidebarController), in process: its Favorites (the folders that exist)
// and Locations (the mounted volumes, which follow attaching and detaching a disk image). See tests/finder.md. Needs
// a display. Usage: finder_sidebar <disk image whose volume is "Unmount Test">; it attaches and detaches the image
// with hdiutil. Prints PASS/FAIL lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderSidebarController.h"
#include <stdio.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSArray<NSString *> *paths(NSArray<NSURL *> *urls)
{
    NSMutableArray *paths = [NSMutableArray array];
    for (NSURL *url in urls)
        [paths addObject: [[url URLByStandardizingPath] path]];
    return paths;
}

static int hdiutil(NSArray<NSString *> *arguments)
{
    NSTask *task = [[NSTask alloc] init];
    task.launchPath = @"/usr/bin/hdiutil";
    task.arguments = arguments;
    task.standardOutput = [NSFileHandle fileHandleWithNullDevice];
    [task launch];
    while ([task isRunning])
        [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.1]];
    return [task terminationStatus];
}

// Runs the run loop until the condition holds, for up to the given time
static BOOL waitFor(NSTimeInterval seconds, BOOL (^condition)(void))
{
    NSDate *end = [NSDate dateWithTimeIntervalSinceNow: seconds];
    while (!condition()) {
        if ([end timeIntervalSinceNow] < 0)
            return NO;
        [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.2]];
    }
    return YES;
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        if (argc != 2) {
            fprintf(stderr, "usage: %s <disk image>\n", argv[0]);
            return 2;
        }
        [NSApplication sharedApplication];
        NSFileManager *fm = [NSFileManager defaultManager];
        NSString *image = [NSString stringWithUTF8String: argv[1]];
        NSString *volume = @"/Volumes/Unmount Test";
        NSString *home = NSHomeDirectory();
        NSString *downloads = [home stringByAppendingPathComponent: @"Downloads"];
        BOOL madeDownloads = ![fm fileExistsAtPath: downloads]
            && [fm createDirectoryAtPath: downloads withIntermediateDirectories: NO attributes: nil error: NULL];

        FinderSidebarController *sidebar = [[FinderSidebarController alloc] init];
        [sidebar view];
        expect([sidebar.sectionTitles isEqual: (@[ @"Favorites", @"Locations" ])],
               [NSString stringWithFormat: @"the sections (%@)", [sidebar.sectionTitles componentsJoinedByString: @", "]]);

        NSMutableArray *favorites = [NSMutableArray arrayWithObjects: @"/Applications", [home stringByStandardizingPath], nil];
        for (NSString *name in @[ @"Desktop", @"Documents", @"Downloads" ]) {
            NSString *path = [home stringByAppendingPathComponent: name];
            BOOL directory = NO;
            if ([fm fileExistsAtPath: path isDirectory: &directory] && directory)
                [favorites addObject: [path stringByStandardizingPath]];
        }
        NSArray *shown = paths([sidebar itemsInSection: 0]);
        expect([shown isEqual: favorites], [NSString stringWithFormat: @"Favorites are the folders that exist (%@; want %@)",
                                                                      [shown componentsJoinedByString: @", "],
                                                                      [favorites componentsJoinedByString: @", "]]);
        NSURL *homeItem = nil;
        for (NSURL *item in [sidebar itemsInSection: 0])
            if ([[[item URLByStandardizingPath] path] isEqual: [home stringByStandardizingPath]])
                homeItem = item;
        expect(homeItem != nil && [[sidebar titleForItem: homeItem] isEqual: NSUserName()],
               [NSString stringWithFormat: @"the home folder is titled with the user name (%@)", [sidebar titleForItem: homeItem]]);

        NSArray *locations = paths([sidebar itemsInSection: 1]);
        expect([locations containsObject: @"/"], [NSString stringWithFormat: @"Locations has the root volume (%@)",
                                                                           [locations componentsJoinedByString: @", "]]);
        NSURL *root = [NSURL fileURLWithPath: @"/" isDirectory: YES];
        expect(![sidebar showsEjectButtonForItem: root], @"the root volume has no eject button");

        // The window's folder is shown as the sidebar's selection, without going anywhere; a click on an item
        // that isn't selected shows it
        __block NSURL *shownFolder = nil;
        sidebar.showFolder = ^(NSURL *folder) {
            shownFolder = folder;
        };
        NSOutlineView *outline = (NSOutlineView *) [(NSScrollView *) sidebar.view documentView];
        NSURL *applications = [sidebar itemsInSection: 0][0];
        sidebar.currentFolder = [NSURL fileURLWithPath: @"/Applications"];
        expect(shownFolder == nil && [[outline itemAtRow: [outline selectedRow]] isEqual: applications],
               @"currentFolder selects its item, without showing it");
        sidebar.currentFolder = [NSURL fileURLWithPath: @"/usr"];
        expect(shownFolder == nil && [outline selectedRow] == -1, @"a folder that isn't in the sidebar selects nothing");
        [outline selectRowIndexes: [NSIndexSet indexSetWithIndex: [outline rowForItem: applications]] byExtendingSelection: NO];
        expect([[[shownFolder URLByStandardizingPath] path] isEqual: @"/Applications"],
               [NSString stringWithFormat: @"selecting an item shows it (%@)", [shownFolder path]]);
        sidebar.showFolder = nil;

        expect(hdiutil(@[ @"attach", @"-noautoopen", image ]) == 0, @"hdiutil attach");
        expect(waitFor(10, ^BOOL {
                   return [paths([sidebar itemsInSection: 1]) containsObject: volume];
               }),
               @"the attached volume appears in Locations");
        NSURL *volumeItem = nil;
        for (NSURL *item in [sidebar itemsInSection: 1])
            if ([[[item URLByStandardizingPath] path] isEqual: volume])
                volumeItem = item;
        expect(volumeItem != nil && [[sidebar titleForItem: volumeItem] isEqual: @"Unmount Test"],
               [NSString stringWithFormat: @"titled with its name (%@)", [sidebar titleForItem: volumeItem]]);
        expect(volumeItem != nil && [sidebar showsEjectButtonForItem: volumeItem], @"with an eject button");

        expect(hdiutil(@[ @"detach", volume ]) == 0, @"hdiutil detach");
        expect(waitFor(10, ^BOOL {
                   return ![paths([sidebar itemsInSection: 1]) containsObject: volume];
               }),
               @"the detached volume goes from Locations");

        if (madeDownloads)
            [fm removeItemAtPath: downloads error: NULL];
        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
