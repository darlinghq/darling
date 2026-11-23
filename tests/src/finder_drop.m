// CFLAGS: -framework Foundation -framework AppKit -framework ImageIO -framework CoreServices -fobjc-arc -Isrc/Finder src/Finder/FinderDirectory.m src/Finder/FinderDrop.m src/Finder/FinderDSStore.m src/Finder/FinderFileOperation.m src/Finder/FinderFolderStyle.m src/Finder/FinderFreeformLayout.m src/Finder/FinderIconViewController.m src/Finder/FinderInfoPanelController.m src/Finder/FinderItemActions.m src/Finder/FinderListViewController.m src/Finder/FinderOperationQueue.m src/Finder/FinderSidebarController.m src/Finder/FinderWindowController.m
// ARGS: "$(mktemp -d)/w"
// Copyright (C) 2026 Lubos Dolezel
//
// Drag and drop in a Finder window's icon view (src/Finder/FinderDrop, FinderIconViewController), in process:
// - FinderDropFolderForItem: a folder takes drops itself, a symlink to a folder takes them for its target; files,
//   symlinks to files and dangling symlinks take none.
// - FinderDropOperation: Copy alone (Option) copies, Generic alone (Command) moves, otherwise a move within a volume
//   and a copy between volumes; nothing onto a read-only volume, into a dragged folder, or a move onto the items' own
//   folder.
// - The icon view's collection view delegate: a drop proposed on a folder item stays on it; one on another item, or
//   in a gap, goes to the window's folder (a drop before an index); dropping copies or moves through the window;
//   dragging an item puts its file URL on the pasteboard. FinderFreeformLayout proposes the item under the point, and
//   the end of the items elsewhere.
// The dragging system can't be driven in process: a stand-in NSDraggingInfo holds the pasteboard and the source's
// mask. See tests/finder.md. Needs a display.
// Usage: finder_drop <new directory> [<disk image>]; with the image (tests/fixtures/unmount-test.dmg) it attaches and
// detaches it with hdiutil. Prints PASS/FAIL/SKIP lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderDrop.h"
#import "FinderFreeformLayout.h"
#import "FinderIconViewController.h"
#import "FinderOperationQueue.h"
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
    spinUntil(^BOOL { return NO; }, 0.5);
}

static NSString *operationName(NSDragOperation operation)
{
    switch (operation) {
    case NSDragOperationNone:
        return @"none";
    case NSDragOperationCopy:
        return @"copy";
    case NSDragOperationMove:
        return @"move";
    case NSDragOperationGeneric:
        return @"generic";
    default:
        return [NSString stringWithFormat: @"%lu", (unsigned long) operation];
    }
}

// What the dragging system would hand the destination
@interface TestDrag : NSObject <NSDraggingInfo>
@property NSPasteboard *pasteboard;
@property NSDragOperation mask;
@end

@implementation TestDrag
@synthesize draggingFormation, animatesToDestination, numberOfValidItemsForDrop;
- (NSWindow *)draggingDestinationWindow { return nil; }
- (NSDragOperation)draggingSourceOperationMask { return _mask; }
- (NSPoint)draggingLocation { return NSZeroPoint; }
- (NSPoint)draggedImageLocation { return NSZeroPoint; }
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

static TestDrag *dragOf(NSArray<NSURL *> *items, NSDragOperation mask)
{
    TestDrag *drag = [[TestDrag alloc] init];
    drag.pasteboard = [NSPasteboard pasteboardWithUniqueName];
    [drag.pasteboard clearContents];
    [drag.pasteboard writeObjects: items];
    drag.mask = mask;
    return drag;
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

static NSIndexPath *pathOf(FinderIconViewController *icons, NSURL *item)
{
    NSString *wanted = [[item URLByStandardizingPath] path];
    for (NSUInteger i = 0; i < [icons.items count]; i++) {
        if ([[[icons.items[i] URLByStandardizingPath] path] isEqual: wanted])
            return [NSIndexPath indexPathForItem: i inSection: 0];
    }
    return nil;
}

// Validates a drop the collection view proposes, and returns the operation; the proposal as the delegate left it is
// put in path and on
static NSDragOperation validateDrop(FinderIconViewController *icons, TestDrag *drag, NSIndexPath **path, BOOL *on)
{
    NSCollectionViewDropOperation operation = *on ? NSCollectionViewDropOn : NSCollectionViewDropBefore;
    NSDragOperation result = [icons collectionView: icons.collectionView validateDrop: drag proposedIndexPath: path dropOperation: &operation];
    *on = operation == NSCollectionViewDropOn;
    return result;
}

static BOOL acceptDrop(FinderIconViewController *icons, TestDrag *drag, NSIndexPath *path, BOOL on)
{
    return [icons collectionView: icons.collectionView
                      acceptDrop: drag
                       indexPath: path
                   dropOperation: on ? NSCollectionViewDropOn : NSCollectionViewDropBefore];
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        setvbuf(stdout, NULL, _IONBF, 0);
        if (argc < 2) {
            fprintf(stderr, "usage: finder_drop <new directory> [<disk image>]\n");
            return 2;
        }
        [NSApplication sharedApplication];
        NSFileManager *fm = [NSFileManager defaultManager];
        NSURL *root = [NSURL fileURLWithPath: [NSString stringWithUTF8String: argv[1]] isDirectory: YES];
        NSURL *one = [root URLByAppendingPathComponent: @"one" isDirectory: YES];
        NSURL *two = [root URLByAppendingPathComponent: @"two" isDirectory: YES];
        NSURL *target = [root URLByAppendingPathComponent: @"T" isDirectory: YES];
        NSURL *folder = [one URLByAppendingPathComponent: @"F" isDirectory: YES];
        NSURL *sub = [folder URLByAppendingPathComponent: @"sub" isDirectory: YES];
        for (NSURL *url in @[ one, two, target, sub ])
            [fm createDirectoryAtURL: url withIntermediateDirectories: YES attributes: nil error: NULL];
        NSURL *a = [one URLByAppendingPathComponent: @"a"];
        NSURL *plain = [one URLByAppendingPathComponent: @"file.txt"];
        [@"a" writeToURL: a atomically: NO encoding: NSUTF8StringEncoding error: NULL];
        [@"file" writeToURL: plain atomically: NO encoding: NSUTF8StringEncoding error: NULL];
        NSURL *link = [one URLByAppendingPathComponent: @"Link"];
        NSURL *fileLink = [one URLByAppendingPathComponent: @"FileLink"];
        NSURL *dangling = [one URLByAppendingPathComponent: @"Dangling"];
        [fm createSymbolicLinkAtPath: [link path] withDestinationPath: @"../T" error: NULL];
        [fm createSymbolicLinkAtPath: [fileLink path] withDestinationPath: @"file.txt" error: NULL];
        [fm createSymbolicLinkAtPath: [dangling path] withDestinationPath: @"nowhere" error: NULL];

        // Which items take drops
        NSString *resolvedTarget = [[target URLByResolvingSymlinksInPath] path];
        expect([[[FinderDropFolderForItem(folder) URLByResolvingSymlinksInPath] path] isEqual: [[folder URLByResolvingSymlinksInPath] path]],
               @"a folder takes drops");
        expect([[[FinderDropFolderForItem(link) URLByResolvingSymlinksInPath] path] isEqual: resolvedTarget],
               [NSString stringWithFormat: @"a symlink to a folder takes them for its target (%@)", FinderDropFolderForItem(link)]);
        expect(FinderDropFolderForItem(plain) == nil, @"a file takes none");
        expect(FinderDropFolderForItem(fileLink) == nil, @"a symlink to a file takes none");
        expect(FinderDropFolderForItem(dangling) == nil, @"a dangling symlink takes none");

        // Which operation
        struct {
            NSArray<NSURL *> *items;
            NSURL *folder;
            NSDragOperation mask;
            NSDragOperation expected;
            NSString *what;
        } cases[] = {
            { @[ a ], two, NSDragOperationEvery, NSDragOperationMove, @"within a volume, a move" },
            { @[ a ], two, NSDragOperationCopy, NSDragOperationCopy, @"Copy alone (Option), a copy" },
            { @[ a ], two, NSDragOperationGeneric, NSDragOperationGeneric, @"Generic alone (Command), a move" },
            { @[ folder ], folder, NSDragOperationEvery, NSDragOperationNone, @"a folder onto itself, nothing" },
            { @[ folder ], sub, NSDragOperationCopy, NSDragOperationNone, @"a folder into its subfolder, nothing" },
            { @[ a ], one, NSDragOperationEvery, NSDragOperationNone, @"a move onto the items' own folder, nothing" },
            { @[ a ], one, NSDragOperationCopy, NSDragOperationCopy, @"a copy onto the items' own folder, a copy" },
            { @[ a ], two, NSDragOperationNone, NSDragOperationNone, @"a source that allows nothing, nothing" },
        };
        for (size_t i = 0; i < sizeof cases / sizeof *cases; i++) {
            NSDragOperation got = FinderDropOperation(cases[i].items, cases[i].folder, cases[i].mask);
            expect(got == cases[i].expected,
                   [NSString stringWithFormat: @"drop operation: %@ (%@)", cases[i].what, operationName(got)]);
        }

        // The freeform layout proposes the item under the point, and the end of the items elsewhere
        @autoreleasepool {
            NSCollectionView *view = [[NSCollectionView alloc] initWithFrame: NSMakeRect(0, 0, 400, 300)];
            FinderFreeformLayout *layout = [[FinderFreeformLayout alloc] initWithItemSize: NSMakeSize(80, 80)];
            layout.centers = @[ [NSValue valueWithPoint: NSMakePoint(100, 100)], [NSValue valueWithPoint: NSMakePoint(250, 100)] ];
            view.collectionViewLayout = layout;
            FinderIconViewController *source = [[FinderIconViewController alloc] init];
            (void) source.view;
            view.dataSource = source;
            source.items = @[ a, plain ];
            [view reloadData];
            [layout prepareLayout];
            NSCollectionViewLayoutAttributes *on = [layout layoutAttributesForDropTargetAtPoint: NSMakePoint(250, 110)];
            expect(on.representedElementCategory == NSCollectionElementCategoryItem && on.indexPath.item == 1,
                   @"freeform layout: the item under the point is the drop target");
            NSCollectionViewLayoutAttributes *off = [layout layoutAttributesForDropTargetAtPoint: NSMakePoint(380, 280)];
            expect(off.representedElementCategory == NSCollectionElementCategoryInterItemGap && off.indexPath.item == 2,
                   [NSString stringWithFormat: @"freeform layout: elsewhere, the gap after the last item (%ld)", (long) off.indexPath.item]);
            expect(NSIsEmptyRect([layout layoutAttributesForInterItemGapBeforeIndexPath: off.indexPath].frame),
                   @"freeform layout: its gaps take no room");
            view.dataSource = nil;
        }

        // The pasteboard server may be out of reach (macOS 10.14.4 over ssh refuses every write); then the
        // drop cases are skipped
        NSPasteboard *probe = [NSPasteboard pasteboardWithUniqueName];
        [probe clearContents];
        if (![probe setString: @"probe" forType: NSPasteboardTypeString]) {
            printf("SKIP: dropping: a pasteboard can't be written\n");
        } else {
            NSPasteboard *text = [NSPasteboard pasteboardWithUniqueName];
            [text clearContents];
            [text writeObjects: @[ @"just text" ]];
            expect(FinderDraggedFileURLs(text) == nil, @"a pasteboard with only text has no dragged files");
            TestDrag *aMove = dragOf(@[ a ], NSDragOperationEvery);
            expect([FinderDraggedFileURLs(aMove.pasteboard) isEqual: @[ a ]], @"the dragged file URLs");

            FinderWindowController *first = [[FinderWindowController alloc] initWithFolderURL: one];
            FinderWindowController *second = [[FinderWindowController alloc] initWithFolderURL: two];
            [first showWindow: nil];
            [second showWindow: nil];
            spin();
            FinderIconViewController *icons = first.iconViewController;
            FinderIconViewController *otherIcons = second.iconViewController;
            expect([[icons.folderURL path] isEqual: [one path]], @"the icon view's folder is the window's");

            // Validating
            NSIndexPath *path = pathOf(icons, folder);
            BOOL on = YES;
            NSDragOperation got = validateDrop(icons, aMove, &path, &on);
            expect(got == NSDragOperationMove && on && path.item == pathOf(icons, folder).item,
                   [NSString stringWithFormat: @"validate: a drop on a folder item stays on it (%@)", operationName(got)]);
            TestDrag *aCopy = dragOf(@[ a ], NSDragOperationCopy);
            path = pathOf(icons, plain);
            on = YES;
            got = validateDrop(icons, aCopy, &path, &on);
            expect(got == NSDragOperationCopy && !on && path != nil,
                   [NSString stringWithFormat: @"validate: a drop on a file goes to the window's folder (%@, %s)", operationName(got), on ? "on" : "before"]);
            path = pathOf(icons, plain);
            on = YES;
            got = validateDrop(icons, aMove, &path, &on);
            expect(got == NSDragOperationNone, @"validate: moving an item onto its own folder is refused");
            path = [NSIndexPath indexPathForItem: 0 inSection: 0];
            on = NO;
            got = validateDrop(otherIcons, aMove, &path, &on);
            expect(got == NSDragOperationMove && !on, @"validate: a drop in a gap goes to the window's folder");
            path = pathOf(icons, folder);
            on = YES;
            TestDrag *textDrag = [[TestDrag alloc] init];
            textDrag.pasteboard = text;
            textDrag.mask = NSDragOperationEvery;
            expect(validateDrop(icons, textDrag, &path, &on) == NSDragOperationNone, @"validate: text is refused");

            // Accepting
            expect(acceptDrop(icons, aCopy, pathOf(icons, folder), YES), @"accept: a copy on a folder item");
            expect(spinUntil(^BOOL { return [fm fileExistsAtPath: [[folder URLByAppendingPathComponent: @"a"] path]]; }, 20) &&
                       [fm fileExistsAtPath: [a path]],
                   @"accept: the folder gets a copy, and the original stays");
            expect(acceptDrop(otherIcons, aCopy, [NSIndexPath indexPathForItem: 0 inSection: 0], NO), @"accept: a copy in another window's gap");
            expect(spinUntil(^BOOL { return [fm fileExistsAtPath: [[two URLByAppendingPathComponent: @"a"] path]]; }, 20),
                   @"accept: the other window's folder gets a copy");
            expect(acceptDrop(icons, aCopy, pathOf(icons, link), YES), @"accept: a copy on a symlink to a folder");
            expect(spinUntil(^BOOL { return [fm fileExistsAtPath: [[target URLByAppendingPathComponent: @"a"] path]]; }, 20),
                   @"accept: the symlink's target gets a copy");
            expect(!acceptDrop(icons, aMove, pathOf(icons, plain), NO), @"accept: a refused drop is not accepted");

            // Dragging
            NSIndexPath *aPath = pathOf(icons, a);
            id<NSPasteboardWriting> writer = [icons collectionView: icons.collectionView pasteboardWriterForItemAtIndexPath: aPath];
            expect([(id) writer isKindOfClass: [NSURL class]] && [[[(NSURL *) writer URLByResolvingSymlinksInPath] path] isEqual: [[a URLByResolvingSymlinksInPath] path]],
                   [NSString stringWithFormat: @"drag: an item's file URL goes on the pasteboard (%@)", writer]);
            expect([icons collectionView: icons.collectionView canDragItemsAtIndexPaths: [NSSet setWithObject: aPath] withEvent: [NSApp currentEvent]],
                   @"drag: items can be dragged");
            expect([[icons.collectionView registeredDraggedTypes] containsObject: NSPasteboardTypeFileURL],
                   @"the icon view takes file URLs");

            // The disk image: a read-only volume
            if (argc < 3)
                printf("SKIP: a read-only volume: no disk image given\n");
            else {
                NSURL *volume = attach([NSString stringWithUTF8String: argv[2]]);
                expect(volume != nil, @"hdiutil attach");
                if (volume) {
                    NSURL *hello = [volume URLByAppendingPathComponent: @"hello.txt"];
                    got = FinderDropOperation(@[ hello ], two, NSDragOperationEvery);
                    expect(got == NSDragOperationCopy,
                           [NSString stringWithFormat: @"drop operation: from the image to another volume, a copy (%@)", operationName(got)]);
                    got = FinderDropOperation(@[ a ], volume, NSDragOperationEvery);
                    expect(got == NSDragOperationNone,
                           [NSString stringWithFormat: @"drop operation: onto the read-only image, nothing (%@)", operationName(got)]);
                    NSTask *task = [NSTask launchedTaskWithLaunchPath: @"/usr/bin/hdiutil" arguments: @[ @"detach", [volume path] ]];
                    [task waitUntilExit];
                }
            }
            [first close];
            [second close];
        }

        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
