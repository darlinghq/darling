// CFLAGS: -framework Foundation -framework AppKit -framework ImageIO -framework CoreServices -fobjc-arc -Isrc/Finder src/Finder/FinderDirectory.m src/Finder/FinderDSStore.m src/Finder/FinderFolderStyle.m src/Finder/FinderFreeformLayout.m src/Finder/FinderDrop.m src/Finder/FinderIconViewController.m src/Finder/FinderListViewController.m src/Finder/FinderSidebarController.m src/Finder/FinderWindowController.m src/Finder/FinderInfoPanelController.m src/Finder/FinderFileOperation.m src/Finder/FinderItemActions.m src/Finder/FinderOperationQueue.m
// ARGS: "$(mktemp -d)/w"
// Copyright (C) 2026 Lubos Dolezel
//
// A Finder window (src/Finder/FinderWindowController), in process: its title, the items of its icon view and their
// labels, selecting items, switching views, the history, a folder of 2000 items, and a folder that is removed. See tests/finder.md. Needs a display. Usage: finder_window <new directory>.
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

static NSString *controllerPath(FinderWindowController *controller)
{
    return [controller.folderURL path];
}

// YES if no table, outline or collection view under view has a data source, delegate or target left
static BOOL noDanglingViews(NSView *view)
{
    if ([view isKindOfClass: [NSTableView class]]) {
        NSTableView *table = (NSTableView *) view;
        if (table.dataSource != nil || table.delegate != nil || table.target != nil)
            return NO;
    }
    if ([view isKindOfClass: [NSCollectionView class]]) {
        NSCollectionView *collection = (NSCollectionView *) view;
        if (collection.dataSource != nil || collection.delegate != nil)
            return NO;
    }
    for (NSView *subview in [view subviews])
        if (!noDanglingViews(subview))
            return NO;
    return YES;
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
        setvbuf(stdout, NULL, _IONBF, 0); // the results so far survive a crash
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

        expect(!controller.canGoBack && !controller.canGoForward, @"a new window has no history");

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

        // History: back to the folder shown before, forward again; up to the enclosing folder, with the folder
        // that was shown selected in it
        expect(controller.canGoBack && !controller.canGoForward, @"after showFolder: it can go back");
        [controller goBack: nil];
        spin();
        expect([[controller.folderURL path] isEqual: folder] && controller.canGoForward,
               [NSString stringWithFormat: @"goBack: shows the folder before (%@)", [controller.folderURL path]]);
        [controller goForward: nil];
        spin();
        expect([[controller.folderURL path] isEqual: other] && !controller.canGoForward,
               [NSString stringWithFormat: @"goForward: shows it again (%@)", [controller.folderURL path]]);
        [controller goToEnclosingFolder: nil];
        spin();
        expect([[controller.folderURL path] isEqual: dir], [NSString stringWithFormat: @"goToEnclosingFolder: shows the enclosing folder (%@)",
                                                                                     [controller.folderURL path]]);
        expect([names(controller.selectedItems) isEqual: (@[ @"Other" ])],
               [NSString stringWithFormat: @"with the folder it came from selected (%@)", names(controller.selectedItems)]);
        [controller goBack: nil];
        spin();
        expect([[controller.folderURL path] isEqual: other], @"goBack: from there returns");

        // Opening the selection: a folder is shown in the window
        [controller goToEnclosingFolder: nil];
        spin();
        __block NSURL *opened = nil;
        controller.openItem = ^(NSURL *item) {
            opened = item;
        };
        [controller openSelection: nil];
        expect([[[opened path] stringByResolvingSymlinksInPath] isEqual: [other stringByResolvingSymlinksInPath]], [NSString stringWithFormat: @"openSelection: opens the selected item (%@)", [opened path]]);

        // A large folder: 2000 items, and scrolling to the last one makes it visible
        NSString *large = [dir stringByAppendingPathComponent: @"Large"];
        [fm createDirectoryAtPath: large withIntermediateDirectories: YES attributes: nil error: NULL];
        for (int i = 1; i <= 2000; i++)
            [fm createFileAtPath: [large stringByAppendingPathComponent: [NSString stringWithFormat: @"item %04d.txt", i]]
                        contents: [NSData data] attributes: nil];
        controller.viewStyle = FinderViewStyleIcons;
        [controller showFolder: [NSURL fileURLWithPath: large]];
        spin();
        [window.contentView layoutSubtreeIfNeeded];
        expect([icons numberOfItemsInSection: 0] == 2000,
               [NSString stringWithFormat: @"a large folder shows its 2000 items (%ld)", (long) [icons numberOfItemsInSection: 0]]);
        NSIndexPath *last = [NSIndexPath indexPathForItem: 1999 inSection: 0];
        [icons scrollToItemsAtIndexPaths: [NSSet setWithObject: last]
                        scrollPosition: NSCollectionViewScrollPositionNearestHorizontalEdge];
        spin();
        NSRect lastFrame = [icons frameForItemAtIndex: 1999];
        expect(!NSIsEmptyRect(lastFrame) && NSIntersectsRect(lastFrame, [icons visibleRect]),
               [NSString stringWithFormat: @"scrolling to the last item makes it visible (%@ in %@)", NSStringFromRect(lastFrame),
                                          NSStringFromRect([icons visibleRect])]);

        [window close];

        // A window whose folder is removed closes
        NSString *doomed = [dir stringByAppendingPathComponent: @"Doomed"];
        [fm createDirectoryAtPath: doomed withIntermediateDirectories: YES attributes: nil error: NULL];
        [fm createFileAtPath: [doomed stringByAppendingPathComponent: @"x.txt"] contents: [NSData data] attributes: nil];
        FinderWindowController *gone = [[FinderWindowController alloc] initWithFolderURL: [NSURL fileURLWithPath: doomed]];
        [gone showWindow: nil];
        spin();
        [fm removeItemAtPath: doomed error: NULL];
        NSDate *end = [NSDate dateWithTimeIntervalSinceNow: 10];
        while ([gone.window isVisible] && [end timeIntervalSinceNow] > 0)
            spin();
        expect(![gone.window isVisible], @"a window whose folder is removed closes");

        // Going to a folder that can't be read leaves the window where it was, with an alert; a history entry whose
        // folder has gone is dropped
        NSString *stay = [dir stringByAppendingPathComponent: @"Stay"];
        NSString *later = [dir stringByAppendingPathComponent: @"Later"];
        [fm createDirectoryAtPath: stay withIntermediateDirectories: YES attributes: nil error: NULL];
        [fm createDirectoryAtPath: later withIntermediateDirectories: YES attributes: nil error: NULL];
        FinderWindowController *nav = [[FinderWindowController alloc] initWithFolderURL: [NSURL fileURLWithPath: stay]];
        [nav showWindow: nil];
        spin();
        [nav showFolder: [NSURL fileURLWithPath: [dir stringByAppendingPathComponent: @"Missing"]]];
        spin();
        expect([[controllerPath(nav) lastPathComponent] isEqual: @"Stay"] && nav.window.attachedSheet != nil && !nav.canGoBack,
               [NSString stringWithFormat: @"showFolder: of a folder that can't be read stays, with an alert (%@, sheet %d)",
                                          controllerPath(nav), nav.window.attachedSheet != nil]);
        if (nav.window.attachedSheet != nil)
            [nav.window endSheet: nav.window.attachedSheet];
        spin();
        [nav showFolder: [NSURL fileURLWithPath: later]];
        spin();
        [nav goBack: nil];
        spin();
        [fm removeItemAtPath: later error: NULL];
        [nav goForward: nil];
        spin();
        expect([[controllerPath(nav) lastPathComponent] isEqual: @"Stay"] && nav.window.attachedSheet != nil && !nav.canGoForward,
               [NSString stringWithFormat: @"goForward: to a folder that has gone stays, with an alert, and drops it (%@)",
                                          controllerPath(nav)]);
        if (nav.window.attachedSheet != nil)
            [nav.window endSheet: nav.window.attachedSheet];
        [nav close];
        spin();

        // When a window's controller goes, nothing in the window still points at it (or at the view controllers)
        NSWindow *kept = nil;
        NSToolbar *keptToolbar = nil;
        @autoreleasepool {
            FinderWindowController *brief = [[FinderWindowController alloc] initWithFolderURL: folderURL];
            [brief showWindow: nil];
            spin();
            kept = brief.window;
            keptToolbar = kept.toolbar;
            [brief close];
        }
        spin();
        expect(kept.delegate == nil, @"the window's delegate is cleared when its controller goes");
        expect(keptToolbar.delegate == nil, @"the toolbar's delegate is cleared");
        BOOL toolbarTargets = YES;
        for (NSToolbarItem *item in keptToolbar.items)
            if ([item.view isKindOfClass: [NSControl class]] && [(NSControl *) item.view target] != nil)
                toolbarTargets = NO;
        expect(toolbarTargets, @"the toolbar controls' targets are cleared");
        expect(noDanglingViews(kept.contentView),
               @"the table, outline and collection views' data sources, delegates and targets are cleared");
        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
