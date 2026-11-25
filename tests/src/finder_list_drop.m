// CFLAGS: -framework Foundation -framework AppKit -framework ImageIO -framework CoreServices -fobjc-arc -Isrc/Finder src/Finder/FinderDirectory.m src/Finder/FinderDrop.m src/Finder/FinderDSStore.m src/Finder/FinderFileOperation.m src/Finder/FinderFolderStyle.m src/Finder/FinderFreeformLayout.m src/Finder/FinderIconViewController.m src/Finder/FinderInfoPanelController.m src/Finder/FinderItemActions.m src/Finder/FinderListViewController.m src/Finder/FinderOperationQueue.m src/Finder/FinderSidebarController.m src/Finder/FinderWindowController.m
// ARGS: "$(mktemp -d)/w"
// Copyright (C) 2026 Lubos Dolezel
//
// Drag and drop in a Finder window's list view and sidebar (src/Finder/FinderListViewController,
// FinderSidebarController), in process. The list view's outline view is driven through its NSDraggingDestination
// methods at points over its rows, with a stand-in NSDraggingInfo holding file URLs from another folder:
// - a drop on a folder's row goes into the folder; on a file's row, or below the rows, into the window's folder; between
//   an expanded folder's children, into that folder; text is refused;
// - a dragged row writes its item's file URL.
// The sidebar's data source is asked directly, with favourites the test gives: a drop on a favourite goes into it; on
// a section title, or between items, nothing is dropped; nor on a read-only volume (with the image).
// The dragging system can't be driven in process. See tests/finder.md. Needs a display.
// Usage: finder_list_drop <new directory> [<disk image>]; with the image (tests/fixtures/unmount-test.dmg) it attaches
// and detaches it with hdiutil. Prints PASS/FAIL/SKIP lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderDrop.h"
#import "FinderListViewController.h"
#import "FinderOperationQueue.h"
#import "FinderSidebarController.h"
#import "FinderWindowController.h"
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

static void spin(void)
{
    spinUntil(^BOOL { return NO; }, 0.7);
}

@interface TestDrag : NSObject <NSDraggingInfo>
@property NSPasteboard *pasteboard;
@property NSDragOperation mask;
@property NSPoint location;
@property NSWindow *window;
@end

@implementation TestDrag
@synthesize draggingFormation, animatesToDestination, numberOfValidItemsForDrop;
- (NSWindow *)draggingDestinationWindow { return _window; }
- (NSDragOperation)draggingSourceOperationMask { return _mask; }
- (NSPoint)draggingLocation { return _location; }
- (NSPoint)draggedImageLocation { return _location; }
- (NSImage *)draggedImage { return nil; }
- (NSPasteboard *)draggingPasteboard { return _pasteboard; }
- (id)draggingSource { return nil; }
- (NSInteger)draggingSequenceNumber { return 1; }
- (void)slideDraggedImageTo:(NSPoint)screenPoint {}
- (NSArray<NSString *> *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination { return nil; }
- (void)enumerateDraggingItemsWithOptions:(NSDraggingItemEnumerationOptions)enumOpts
                                  forView:(NSView *)view
                                  classes:(NSArray<Class> *)classArray
                            searchOptions:(NSDictionary<NSPasteboardReadingOptionKey, id> *)searchOptions
                               usingBlock:(void (^)(NSDraggingItem *draggingItem, NSInteger idx, BOOL *stop))block {}
- (NSSpringLoadingHighlight)springLoadingHighlight { return NSSpringLoadingHighlightNone; }
- (void)resetSpringLoading {}
@end

static TestDrag *dragOf(NSArray *objects, NSDragOperation mask)
{
    TestDrag *drag = [[TestDrag alloc] init];
    drag.pasteboard = [NSPasteboard pasteboardWithUniqueName];
    [drag.pasteboard clearContents];
    [drag.pasteboard writeObjects: objects];
    drag.mask = mask;
    return drag;
}

static NSString *resolved(NSURL *url)
{
    return [[url URLByResolvingSymlinksInPath] path];
}

static NSInteger rowOf(NSOutlineView *outline, NSURL *item)
{
    for (NSInteger row = 0; row < outline.numberOfRows; row++) {
        id candidate = [outline itemAtRow: row];
        if ([candidate isKindOfClass: [NSURL class]] && [resolved(candidate) isEqual: resolved(item)])
            return row;
    }
    return -1;
}

// Drops at a point of the outline view: what draggingUpdated answered, and whether the drop was performed
static NSDragOperation dropAt(NSOutlineView *outline, TestDrag *drag, NSPoint point, BOOL *performed)
{
    drag.window = outline.window;
    drag.location = [outline convertPoint: point toView: nil];
    [outline draggingEntered: drag];
    NSDragOperation operation = [outline draggingUpdated: drag];
    BOOL done = NO;
    if (operation != NSDragOperationNone)
        done = [outline prepareForDragOperation: drag] && [outline performDragOperation: drag];
    [outline concludeDragOperation: drag];
    if ([outline respondsToSelector: @selector(draggingEnded:)])
        [outline draggingEnded: drag];
    if (performed)
        *performed = done;
    return operation;
}

static NSPoint middleOfRow(NSOutlineView *outline, NSInteger row)
{
    NSRect rect = [outline rectOfRow: row];
    return NSMakePoint(NSMidX(rect), NSMidY(rect));
}

static BOOL exists(NSURL *url)
{
    return [[NSFileManager defaultManager] fileExistsAtPath: [url path]];
}

static NSURL *attach(NSString *image)
{
    NSTask *task = [[NSTask alloc] init];
    task.launchPath = @"/usr/bin/hdiutil";
    task.arguments = @[ @"attach", @"-noautoopen", @"-plist", image ];
    NSPipe *pipe = [NSPipe pipe];
    task.standardOutput = pipe;
    [task launch];
    NSData *data = [[pipe fileHandleForReading] readDataToEndOfFile];
    [task waitUntilExit];
    NSDictionary *plist = [NSPropertyListSerialization propertyListWithData: data options: 0 format: NULL error: NULL];
    for (NSDictionary *entity in plist[@"system-entities"]) {
        if (entity[@"mount-point"])
            return [NSURL fileURLWithPath: entity[@"mount-point"] isDirectory: YES];
    }
    return nil;
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        setvbuf(stdout, NULL, _IONBF, 0);
        if (argc < 2) {
            fprintf(stderr, "usage: finder_list_drop <new directory> [<disk image>]\n");
            return 2;
        }
        [NSApplication sharedApplication];
        NSFileManager *fm = [NSFileManager defaultManager];
        NSURL *root = [NSURL fileURLWithPath: [NSString stringWithUTF8String: argv[1]] isDirectory: YES];
        NSURL *one = [root URLByAppendingPathComponent: @"one" isDirectory: YES];
        NSURL *two = [root URLByAppendingPathComponent: @"two" isDirectory: YES];
        NSURL *target = [root URLByAppendingPathComponent: @"T" isDirectory: YES];
        NSURL *folder = [one URLByAppendingPathComponent: @"F" isDirectory: YES];
        for (NSURL *url in @[ one, two, target, folder ])
            [fm createDirectoryAtURL: url withIntermediateDirectories: YES attributes: nil error: NULL];
        NSURL *plain = [one URLByAppendingPathComponent: @"file.txt"];
        NSURL *inside = [folder URLByAppendingPathComponent: @"x"];
        [@"file" writeToURL: plain atomically: NO encoding: NSUTF8StringEncoding error: NULL];
        [@"x" writeToURL: inside atomically: NO encoding: NSUTF8StringEncoding error: NULL];
        NSMutableDictionary<NSString *, NSURL *> *sources = [NSMutableDictionary dictionary];
        for (NSString *name in @[ @"b", @"c", @"d", @"e", @"g" ]) {
            NSURL *url = [two URLByAppendingPathComponent: name];
            [name writeToURL: url atomically: NO encoding: NSUTF8StringEncoding error: NULL];
            sources[name] = url;
        }

        // The pasteboard server may be out of reach (macOS 10.14.4 over ssh refuses every write); then the cases
        // are skipped
        NSPasteboard *probe = [NSPasteboard pasteboardWithUniqueName];
        [probe clearContents];
        if (![probe setString: @"probe" forType: NSPasteboardTypeString]) {
            printf("SKIP: list view and sidebar drops: a pasteboard can't be written\n");
            printf("ALL PASSED\n");
            return 0;
        }

        // The list view
        FinderWindowController *window = [[FinderWindowController alloc] initWithFolderURL: one];
        window.viewStyle = FinderViewStyleList;
        [window showWindow: nil];
        spin();
        FinderListViewController *list = window.listViewController;
        NSOutlineView *outline = list.outlineView;
        expect([[list.folderURL path] isEqual: [one path]], @"the list view's folder is the window's");
        expect([[outline registeredDraggedTypes] containsObject: NSPasteboardTypeFileURL], @"the list view takes file URLs");

        NSInteger row = rowOf(outline, folder);
        id<NSPasteboardWriting> writer = [list outlineView: outline pasteboardWriterForItem: [outline itemAtRow: row]];
        expect([(id) writer isKindOfClass: [NSURL class]] && [resolved((NSURL *) writer) isEqual: resolved(folder)],
               @"a dragged row writes its item's file URL");

        BOOL performed = NO;
        NSDragOperation got = dropAt(outline, dragOf(@[ sources[@"b"] ], NSDragOperationCopy), middleOfRow(outline, rowOf(outline, folder)), &performed);
        expect(got == NSDragOperationCopy && performed, @"a drop on a folder's row is taken");
        expect(spinUntil(^BOOL { return exists([folder URLByAppendingPathComponent: @"b"]); }, 20), @"it goes into the folder");

        spin();
        got = dropAt(outline, dragOf(@[ sources[@"c"] ], NSDragOperationCopy), middleOfRow(outline, rowOf(outline, plain)), &performed);
        expect(got == NSDragOperationCopy && performed, @"a drop on a file's row is taken");
        expect(spinUntil(^BOOL { return exists([one URLByAppendingPathComponent: @"c"]); }, 20), @"it goes into the window's folder");

        spin();
        NSRect last = [outline rectOfRow: outline.numberOfRows - 1];
        got = dropAt(outline, dragOf(@[ sources[@"d"] ], NSDragOperationCopy), NSMakePoint(NSMidX(last), NSMaxY(last) + 5), &performed);
        expect(got == NSDragOperationCopy && performed, @"a drop below the rows is taken");
        expect(spinUntil(^BOOL { return exists([one URLByAppendingPathComponent: @"d"]); }, 20), @"it goes into the window's folder");

        spin();
        [outline expandItem: [outline itemAtRow: rowOf(outline, folder)]];
        NSInteger insideRow = rowOf(outline, inside);
        expect(insideRow >= 0, @"the folder expands");
        if (insideRow >= 0) {
            NSRect rect = [outline rectOfRow: insideRow];
            got = dropAt(outline, dragOf(@[ sources[@"e"] ], NSDragOperationCopy), NSMakePoint(NSMidX(rect), NSMinY(rect) + 1), &performed);
            expect(got == NSDragOperationCopy && performed, @"a drop between an expanded folder's children is taken");
            expect(spinUntil(^BOOL { return exists([folder URLByAppendingPathComponent: @"e"]); }, 20), @"it goes into that folder");
        }

        spin();
        got = dropAt(outline, dragOf(@[ @"just text" ], NSDragOperationEvery), middleOfRow(outline, rowOf(outline, folder)), NULL);
        expect(got == NSDragOperationNone, @"text is refused");
        [window close];

        // The sidebar, with the favourites given
        FinderSidebarController *sidebar = [[FinderSidebarController alloc] initWithFavorites: @[ target ]];
        (void) sidebar.view;
        __block NSURL *droppedInto = nil;
        sidebar.dropItems = ^(NSArray<NSURL *> *items, NSURL *into, FinderTransferKind kind) {
            droppedInto = into;
            [[FinderOperationQueue sharedQueue] transferItems: items to: into kind: kind window: nil];
        };
        NSOutlineView *side = sidebar.outlineView;
        expect([[sidebar itemsInSection: 0] isEqual: @[ target ]], @"sidebar: the favourites given");
        expect([[side registeredDraggedTypes] containsObject: NSPasteboardTypeFileURL], @"sidebar: it takes file URLs");
        TestDrag *gCopy = dragOf(@[ sources[@"g"] ], NSDragOperationCopy);
        got = [sidebar outlineView: side validateDrop: gCopy proposedItem: target proposedChildIndex: NSOutlineViewDropOnItemIndex];
        expect(got == NSDragOperationCopy, @"sidebar: a drop on a favourite is taken");
        expect([sidebar outlineView: side acceptDrop: gCopy item: target childIndex: NSOutlineViewDropOnItemIndex]
                   && [resolved(droppedInto) isEqual: resolved(target)],
               @"sidebar: it is dropped into the favourite");
        expect(spinUntil(^BOOL { return exists([target URLByAppendingPathComponent: @"g"]); }, 20), @"sidebar: the favourite gets it");
        got = [sidebar outlineView: side validateDrop: gCopy proposedItem: @"Favorites" proposedChildIndex: NSOutlineViewDropOnItemIndex];
        expect(got == NSDragOperationNone, @"sidebar: nothing is dropped on a section title");
        got = [sidebar outlineView: side validateDrop: gCopy proposedItem: @"Favorites" proposedChildIndex: 0];
        expect(got == NSDragOperationNone, @"sidebar: nothing is dropped between items");

        if (argc < 3)
            printf("SKIP: sidebar: a read-only volume: no disk image given\n");
        else {
            NSURL *volume = attach([NSString stringWithUTF8String: argv[2]]);
            expect(volume != nil, @"hdiutil attach");
            if (volume) {
                got = [sidebar outlineView: side validateDrop: gCopy proposedItem: volume proposedChildIndex: NSOutlineViewDropOnItemIndex];
                expect(got == NSDragOperationNone, @"sidebar: nothing is dropped on a read-only volume");
                NSTask *task = [NSTask launchedTaskWithLaunchPath: @"/usr/bin/hdiutil" arguments: @[ @"detach", [volume path] ]];
                [task waitUntilExit];
            }
        }

        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
