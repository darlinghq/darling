// CFLAGS: -framework Foundation -framework AppKit -fobjc-arc -Isrc/Finder src/Finder/FinderDirectory.m src/Finder/FinderIconViewController.m src/Finder/FinderListViewController.m src/Finder/FinderSidebarController.m src/Finder/FinderWindowController.m
// ARGS: "$(mktemp -d)/w"
// Copyright (C) 2026 Lubos Dolezel
//
// A Finder window (src/Finder/FinderWindowController), in process: its title, the items of its icon view and their
// labels, and selecting items. See tests/finder.md. Needs a display. Usage: finder_window <new directory>.
// Prints PASS/FAIL lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderWindowController.h"
#import "FinderDirectory.h"
#include <stdio.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSArray<NSString *> *names(NSArray<NSURL *> *urls)
{
    NSMutableArray *names = [NSMutableArray array];
    for (NSURL *url in urls)
        [names addObject: [url lastPathComponent]];
    return names;
}

static NSCollectionView *iconView(NSView *view)
{
    if ([view isKindOfClass: [NSCollectionView class]])
        return (NSCollectionView *) view;
    for (NSView *subview in [view subviews]) {
        NSCollectionView *found = iconView(subview);
        if (found != nil)
            return found;
    }
    return nil;
}

// The list view under view, if it is shown: the outline view with a Kind column (the sidebar is one too)
static NSOutlineView *shownListView(NSView *view)
{
    if ([view isKindOfClass: [NSOutlineView class]] && [(NSOutlineView *) view columnWithIdentifier: @"kind"] >= 0
        && ![view isHiddenOrHasHiddenAncestor])
        return (NSOutlineView *) view;
    for (NSView *subview in [view subviews]) {
        NSOutlineView *found = shownListView(subview);
        if (found != nil)
            return found;
    }
    return nil;
}

static void spin(void)
{
    [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.2]];
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
        NSString *folder = [dir stringByAppendingPathComponent: @"Folder One"];
        NSString *other = [dir stringByAppendingPathComponent: @"Other"];
        [fm createDirectoryAtPath: folder withIntermediateDirectories: YES attributes: nil error: NULL];
        [fm createDirectoryAtPath: other withIntermediateDirectories: YES attributes: nil error: NULL];
        for (NSString *name in @[ @"c.txt", @"a.txt", @"b.txt" ])
            [fm createFileAtPath: [folder stringByAppendingPathComponent: name] contents: [NSData data] attributes: nil];
        [fm createFileAtPath: [other stringByAppendingPathComponent: @"elsewhere.txt"] contents: [NSData data] attributes: nil];
        NSURL *folderURL = [NSURL fileURLWithPath: folder];

        FinderWindowController *controller = [[FinderWindowController alloc] initWithFolderURL: folderURL];
        [controller showWindow: nil];
        spin();
        NSWindow *window = controller.window;
        expect(window != nil && [window isVisible], @"the window shows");
        expect([[controller.folderURL path] isEqual: folder], @"folderURL is the folder");
        expect([window.title isEqual: [fm displayNameAtPath: folder]],
               [NSString stringWithFormat: @"the title is the folder's display name (%@)", window.title]);
        // The area the toolbar and title bar leave: 800x500 (macOS 10.14), or taller where the toolbar makes the
        // window grow (macOS 26 gives 532)
        NSSize size = window.contentLayoutRect.size;
        expect(size.width == 800 && size.height >= 500 && size.height <= 540,
               [NSString stringWithFormat: @"a new window's content is 800x500 or a little taller (%@)", NSStringFromSize(size)]);

        NSCollectionView *icons = iconView(window.contentView);
        [window.contentView layoutSubtreeIfNeeded];
        expect(icons != nil && [icons numberOfItemsInSection: 0] == 3, @"the icon view has the folder's three items");
        NSMutableArray *labels = [NSMutableArray array];
        for (NSInteger i = 0; i < 3; i++) {
            NSCollectionViewItem *item = [icons itemAtIndexPath: [NSIndexPath indexPathForItem: i inSection: 0]];
            [labels addObject: item.textField.stringValue ?: @"(none)"];
        }
        expect([labels isEqual: (@[ @"a.txt", @"b.txt", @"c.txt" ])],
               [NSString stringWithFormat: @"the labels are the names, in order (%@)", labels]);
        NSCollectionViewItem *first = [icons itemAtIndexPath: [NSIndexPath indexPathForItem: 0 inSection: 0]];
        expect(first.imageView.image != nil, @"items have an icon");

        expect([controller.selectedItems count] == 0, @"nothing is selected in a new window");
        NSURL *a = [folderURL URLByAppendingPathComponent: @"a.txt"];
        NSURL *c = [folderURL URLByAppendingPathComponent: @"c.txt"];
        [controller selectItems: @[ c, a ]];
        expect([names(controller.selectedItems) isEqual: (@[ @"a.txt", @"c.txt" ])],
               [NSString stringWithFormat: @"selectItems: selects them (%@)", names(controller.selectedItems)]);
        NSSet *paths = [NSSet setWithObjects: [NSIndexPath indexPathForItem: 0 inSection: 0],
                                              [NSIndexPath indexPathForItem: 2 inSection: 0], nil];
        expect([icons.selectionIndexPaths isEqual: paths], @"the icon view's selection is the same");

        NSURL *elsewhere = [NSURL fileURLWithPath: [other stringByAppendingPathComponent: @"elsewhere.txt"]];
        [controller selectItems: @[ elsewhere, [NSURL fileURLWithPath: [folder stringByAppendingPathComponent: @"b.txt"]] ]];
        expect([names(controller.selectedItems) isEqual: (@[ @"b.txt" ])],
               [NSString stringWithFormat: @"items of another folder are ignored (%@)", names(controller.selectedItems)]);

        // Switching views keeps the selection; the list view shows the same items
        expect(controller.viewStyle == FinderViewStyleIcons, @"a new window uses the icon view");
        [controller selectItems: @[ a, c ]];
        controller.viewStyle = FinderViewStyleList;
        spin();
        NSOutlineView *list = shownListView(window.contentView);
        expect(list != nil && [list numberOfRows] == 3, @"the list view shows the folder's three items");
        expect([names(controller.selectedItems) isEqual: (@[ @"a.txt", @"c.txt" ])],
               [NSString stringWithFormat: @"switching to the list keeps the selection (%@)", names(controller.selectedItems)]);
        expect([[list selectedRowIndexes] count] == 2, @"the list view shows it");
        expect(![list isHiddenOrHasHiddenAncestor] && [icons isHiddenOrHasHiddenAncestor],
               @"the list view is shown instead of the icon view");
        controller.viewStyle = FinderViewStyleIcons;
        spin();
        expect([names(controller.selectedItems) isEqual: (@[ @"a.txt", @"c.txt" ])] && [icons.selectionIndexPaths count] == 2,
               [NSString stringWithFormat: @"switching back keeps it (%@)", names(controller.selectedItems)]);
        controller.viewStyle = FinderViewStyleList;

        // Showing another folder keeps the view
        [controller showFolder: [NSURL fileURLWithPath: other]];
        spin();
        expect(controller.viewStyle == FinderViewStyleList && [list numberOfRows] == 1,
               @"showFolder: keeps the list view and shows the other folder in it");
        controller.viewStyle = FinderViewStyleIcons;
        spin();
        expect([window.title isEqual: [fm displayNameAtPath: other]], @"showFolder: changes the title");
        expect([icons numberOfItemsInSection: 0] == 1 && [controller.selectedItems count] == 0,
               @"showFolder: shows the other folder's items, with nothing selected");

        [window close];
        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
