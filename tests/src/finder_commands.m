// CFLAGS: -framework Foundation -framework AppKit -framework ImageIO -framework CoreServices -fobjc-arc -Isrc/Finder src/Finder/FinderDirectory.m src/Finder/FinderDSStore.m src/Finder/FinderFileOperation.m src/Finder/FinderFolderStyle.m src/Finder/FinderFreeformLayout.m src/Finder/FinderDrop.m src/Finder/FinderIconViewController.m src/Finder/FinderItemActions.m src/Finder/FinderListViewController.m src/Finder/FinderOperationQueue.m src/Finder/FinderSidebarController.m src/Finder/FinderWindowController.m src/Finder/FinderInfoPanelController.m
// ARGS: "$(mktemp -d)/w"
// Copyright (C) 2026 Lubos Dolezel
//
// A Finder window's commands (src/Finder/FinderWindowController), in process: Copy puts the selection's file URLs on
// the general pasteboard; Paste copies them into the window's folder and Move Item Here moves them; nothing usable on
// the pasteboard disables Paste; New Folder makes "untitled folder", selects it and starts renaming it; renaming in
// the icon view and in the list view (commit, refusal with an alert, the extension question, cancelling); Move to
// Trash; which commands are enabled with nothing selected and in a read-only folder. Alerts are answered through
// FinderOperationQueue's alertResponder. See tests/finder.md. Needs a display.
// Usage: finder_commands <new directory> [<disk image>]; with the image (tests/fixtures/unmount-test.dmg) it attaches
// and detaches it with hdiutil. Prints PASS/FAIL/SKIP lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderListViewController.h"
#import "FinderWindowController.h"
#import "FinderInfoPanelController.h"
#import "FinderOperationQueue.h"
#include <stdio.h>
#include <sys/stat.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSFileManager *fm;
static NSURL *root;

static NSURL *folder(NSString *relative)
{
    NSURL *url = [root URLByAppendingPathComponent: relative isDirectory: YES];
    [fm createDirectoryAtURL: url withIntermediateDirectories: YES attributes: nil error: NULL];
    return url;
}

static NSURL *file(NSString *relative)
{
    NSURL *url = [root URLByAppendingPathComponent: relative];
    [[NSData dataWithBytes: "x" length: 1] writeToURL: url atomically: NO];
    return url;
}

static BOOL exists(NSURL *url)
{
    struct stat st;
    return lstat([url fileSystemRepresentation], &st) == 0;
}

static BOOL spinUntil(BOOL (^done)(void), NSTimeInterval seconds)
{
    NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: seconds];
    while (!done() && [limit timeIntervalSinceNow] > 0)
        [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode beforeDate: [NSDate dateWithTimeIntervalSinceNow: 0.02]];
    return done();
}

static void spin(void)
{
    spinUntil(^BOOL { return NO; }, 0.3);
}

static NSArray<NSString *> *names(NSArray<NSURL *> *urls)
{
    NSMutableArray *names = [NSMutableArray array];
    for (NSURL *url in urls)
        [names addObject: [url lastPathComponent]];
    return names;
}

static NSArray<NSString *> *shown(FinderWindowController *controller)
{
    NSArray *selected = controller.selectedItems;
    [controller selectAll: nil];
    NSArray *all = names(controller.selectedItems);
    [controller selectItems: selected];
    return all;
}

static BOOL enabled(FinderWindowController *controller, SEL action)
{
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle: @"" action: action keyEquivalent: @""];
    return [controller validateMenuItem: item];
}

// The field editor of whatever is being edited in the window
static NSTextView *fieldEditor(FinderWindowController *controller)
{
    id responder = [controller.window firstResponder];
    return [responder isKindOfClass: [NSTextView class]] && [(NSTextView *) responder isFieldEditor] ? responder : nil;
}

// Types name into the label being edited, then ends the edit as Return does
static void typeAndReturn(FinderWindowController *controller, NSString *name)
{
    NSTextView *editor = fieldEditor(controller);
    expect(editor != nil, [NSString stringWithFormat: @"editing %@: a field editor is first responder", name]);
    [editor selectAll: nil];
    [editor insertText: name];
    [editor insertNewline: nil];
    spin();
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
    NSRange start = [data rangeOfData: [@"<?xml" dataUsingEncoding: NSUTF8StringEncoding] options: 0
                                range: NSMakeRange(0, [data length])];
    if (task.terminationStatus != 0 || start.location == NSNotFound)
        return nil;
    NSDictionary *plist = [NSPropertyListSerialization
        propertyListWithData: [data subdataWithRange: NSMakeRange(start.location, [data length] - start.location)]
                     options: 0 format: NULL error: NULL];
    for (NSDictionary *entity in plist[@"system-entities"])
        if (entity[@"mount-point"])
            return [NSURL fileURLWithPath: entity[@"mount-point"] isDirectory: YES];
    return nil;
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        setvbuf(stdout, NULL, _IONBF, 0);
        if (argc < 2) {
            fprintf(stderr, "usage: %s <new directory> [<disk image>]\n", argv[0]);
            return 2;
        }
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];
        fm = [NSFileManager defaultManager];
        root = [NSURL fileURLWithPath: [NSString stringWithUTF8String: argv[1]] isDirectory: YES];
        NSURL *one = folder(@"one");
        NSURL *two = folder(@"two");
        NSURL *a = file(@"one/a.txt");
        NSURL *b = file(@"one/b");

        __block NSMutableArray<NSAlert *> *alerts = [NSMutableArray array];
        __block NSInteger answer = 0;
        [FinderOperationQueue sharedQueue].alertResponder = ^NSInteger(NSAlert *alert, BOOL *suppress) {
            [alerts addObject: alert];
            return answer;
        };

        FinderWindowController *first = [[FinderWindowController alloc] initWithFolderURL: one];
        FinderWindowController *second = [[FinderWindowController alloc] initWithFolderURL: two];
        [first showWindow: nil];
        [second showWindow: nil];
        spin();

        // The pasteboard server may be out of reach (macOS 10.14.4 over ssh refuses every write); then the
        // pasteboard cases are skipped
        [[NSPasteboard generalPasteboard] clearContents];
        if (![[NSPasteboard generalPasteboard] setString: @"probe" forType: NSPasteboardTypeString]) {
            printf("SKIP: copy and paste: the general pasteboard can't be written\n");
        } else {
            // Copy
            [first selectItems: @[ a, b ]];
            [first copy: nil];
            NSArray *copied = [[NSPasteboard generalPasteboard] readObjectsForClasses: @[ [NSURL class] ]
                                                                             options: @{ NSPasteboardURLReadingFileURLsOnlyKey: @YES }];
            expect([names(copied) isEqual: (@[ @"a.txt", @"b" ])],
                   [NSString stringWithFormat: @"copy: the selection's file URLs are on the pasteboard (%@)", names(copied)]);
            expect(enabled(second, @selector(paste:)) && enabled(second, @selector(moveItemHere:)),
                   @"paste and move item here are enabled with file URLs on the pasteboard");

            // Paste copies
            [second paste: nil];
            expect(spinUntil(^BOOL { return [shown(second) containsObject: @"a.txt"] && [shown(second) containsObject: @"b"]; }, 20),
                   [NSString stringWithFormat: @"paste: the items appear in the window (%@)", shown(second)]);
            expect(exists(a) && exists(b), @"paste: the sources remain");

            // Move Item Here moves
            NSURL *c = file(@"one/c");
            // The window shows c once its folder's change reaches it
            expect(spinUntil(^BOOL { return [shown(first) containsObject: @"c"]; }, 20), @"a new file appears in its window");
            [first selectItems: @[ c ]];
            [first copy: nil];
            [second moveItemHere: nil];
            expect(spinUntil(^BOOL { return [shown(second) containsObject: @"c"] && ![shown(first) containsObject: @"c"]; }, 20),
                   [NSString stringWithFormat: @"move item here: c moves (%@ | %@)", shown(first), shown(second)]);
            expect(!exists(c), @"move item here: the source is gone");

            // Nothing usable on the pasteboard
            [[NSPasteboard generalPasteboard] clearContents];
            [[NSPasteboard generalPasteboard] writeObjects: @[ @"just text" ]];
            expect(!enabled(second, @selector(paste:)), @"paste is disabled with only text on the pasteboard");
            NSURL *gone = file(@"one/gone");
            [[NSPasteboard generalPasteboard] clearContents];
            [[NSPasteboard generalPasteboard] writeObjects: @[ gone ]];
            [fm removeItemAtURL: gone error: NULL];
            NSUInteger running = [[FinderOperationQueue sharedQueue].runningOperations count];
            [second paste: nil];
            expect([[FinderOperationQueue sharedQueue].runningOperations count] == running && [alerts count] == 0,
                   @"paste of items that no longer exist starts nothing");
        }

        // New Folder
        [first newFolder: nil];
        spin();
        expect(exists([one URLByAppendingPathComponent: @"untitled folder"]), @"new folder: untitled folder is made");
        expect([names(first.selectedItems) isEqual: @[ @"untitled folder" ]],
               [NSString stringWithFormat: @"new folder: it is selected (%@)", names(first.selectedItems)]);
        expect([[first.renamingItem lastPathComponent] isEqual: @"untitled folder"], @"new folder: renaming it starts");
        typeAndReturn(first, @"Neu");
        expect(spinUntil(^BOOL { return exists([one URLByAppendingPathComponent: @"Neu"]); }, 5) && first.renamingItem == nil,
               @"new folder: typing a name and Return names it");

        // Rename commits
        [first beginRenamingItem: a];
        expect([first.renamingItem isEqual: a], @"rename: renaming starts");
        NSTextView *editor = fieldEditor(first);
        expect(editor != nil && NSEqualRanges([editor selectedRange], NSMakeRange(0, 1)),
               [NSString stringWithFormat: @"rename: the name without its extension is selected (%@)",
                                          editor ? NSStringFromRange([editor selectedRange]) : @"no editor"]);
        typeAndReturn(first, @"d.txt");
        NSURL *d = [one URLByAppendingPathComponent: @"d.txt"];
        expect(exists(d) && !exists(a), @"rename: a.txt is now d.txt");
        expect(spinUntil(^BOOL { return [names(first.selectedItems) isEqual: @[ @"d.txt" ]]; }, 5),
               [NSString stringWithFormat: @"rename: the renamed item is selected (%@)", names(first.selectedItems)]);

        // Rename refused
        [alerts removeAllObjects];
        [first beginRenamingItem: d];
        typeAndReturn(first, @".x");
        expect([alerts count] == 1 && [[alerts.firstObject messageText] hasPrefix: @"Names that begin with a dot"],
               [NSString stringWithFormat: @"rename to .x: an alert (%@)", [alerts.firstObject messageText]]);
        expect(exists(d) && [shown(first) containsObject: @"d.txt"], @"rename to .x: the name stays");

        // The extension question: Keep
        [alerts removeAllObjects];
        answer = 0;
        [first beginRenamingItem: d];
        typeAndReturn(first, @"d.md");
        NSAlert *question = alerts.firstObject;
        NSMutableArray *titles = [NSMutableArray array];
        for (NSButton *button in question.buttons)
            [titles addObject: button.title];
        expect([titles isEqual: (@[ @"Keep .txt", @"Use .md" ])],
               [NSString stringWithFormat: @"rename to d.md: Keep .txt or Use .md (%@)", titles]);
        expect([question.messageText isEqual: @"Are you sure you want to change the extension from “.txt” to “.md”?"],
               [NSString stringWithFormat: @"rename to d.md: the question (%@)", question.messageText]);
        expect(exists(d), @"rename to d.md, Keep: the name stays");

        // Cancel
        [first beginRenamingItem: d];
        [fieldEditor(first) insertText: @"zzz"];
        [fieldEditor(first) doCommandBySelector: @selector(cancelOperation:)];
        spin();
        expect(exists(d) && first.renamingItem == nil, @"rename cancelled: the name stays");

        // Rename in the list view
        first.viewStyle = FinderViewStyleList;
        spin();
        [first beginRenamingItem: b];
        expect([first.renamingItem isEqual: b], @"list view: renaming starts");
        typeAndReturn(first, @"e");
        expect(spinUntil(^BOOL { return exists([one URLByAppendingPathComponent: @"e"]); }, 5) && !exists(b),
               @"list view: b is now e");

        // An edit that ends without a name (the table aborts it) or never starts leaves the window reloading
        NSOutlineView *outline = first.listViewController.outlineView;
        [first beginRenamingItem: d];
        expect([first.renamingItem isEqual: d], @"list view: renaming d starts");
        [outline abortEditing];
        spin();
        expect(first.renamingItem == nil, @"list view: an aborted edit is no rename");
        file(@"one/fresh1");
        expect(spinUntil(^BOOL { return [shown(first) containsObject: @"fresh1"]; }, 10),
               @"list view: after an aborted edit, the window still reloads");
        [first beginRenamingItem: [one URLByAppendingPathComponent: @"not there"]];
        expect(first.renamingItem == nil, @"renaming an item that isn't shown doesn't start");
        [first beginRenamingItem: d];
        [first showFolder: two];
        spin();
        expect(first.renamingItem == nil && exists(d), @"list view: going to another folder ends the rename, d stays");
        file(@"two/fresh2");
        expect(spinUntil(^BOOL { return [shown(first) containsObject: @"fresh2"]; }, 10),
               @"list view: the other folder's window reloads");
        [first showFolder: one];
        spin();

        // A reload keeps expanded folders expanded
        NSURL *neu = [one URLByAppendingPathComponent: @"Neu"];
        file(@"one/Neu/inner");
        spin();
        NSURL *neuRow = nil;
        for (NSInteger row = 0; row < outline.numberOfRows; row++)
            if ([[[outline itemAtRow: row] lastPathComponent] isEqual: @"Neu"])
                neuRow = [outline itemAtRow: row];
        [outline expandItem: neuRow];
        file(@"one/fresh3");
        expect(spinUntil(^BOOL { return [shown(first) containsObject: @"fresh3"]; }, 10), @"list view: fresh3 is shown");
        BOOL expanded = NO, innerShown = NO;
        for (NSInteger row = 0; row < outline.numberOfRows; row++) {
            if ([[[[outline itemAtRow: row] URLByStandardizingPath] path] isEqual: [[neu URLByStandardizingPath] path]])
                expanded = [outline isItemExpanded: [outline itemAtRow: row]];
            innerShown |= [[[outline itemAtRow: row] lastPathComponent] isEqual: @"inner"];
        }
        expect(expanded && innerShown, @"list view: a reload keeps Neu expanded");
        first.viewStyle = FinderViewStyleIcons;
        spin();

        // Move to Trash
        NSURL *e = [one URLByAppendingPathComponent: @"e"];
        [first selectItems: @[ e ]];
        [first moveToTrash: nil];
        expect(spinUntil(^BOOL { return !exists(e) && ![shown(first) containsObject: @"e"]; }, 20),
               @"move to trash: the item leaves the window");

        // What is enabled
        [first selectItems: @[]];
        expect(!enabled(first, @selector(copy:)) && !enabled(first, @selector(rename:)) && !enabled(first, @selector(moveToTrash:)),
               @"nothing selected: copy, rename and move to trash are disabled");
        expect(enabled(first, @selector(newFolder:)) && enabled(first, @selector(selectAll:)),
               @"nothing selected: new folder and select all are enabled");
        [first selectItems: @[ d ]];
        expect(enabled(first, @selector(copy:)) && enabled(first, @selector(rename:)) && enabled(first, @selector(moveToTrash:)),
               @"one selected: copy, rename and move to trash are enabled");

        // Get Info: a panel per selected item
        [first selectItems: @[]];
        expect(!enabled(first, @selector(getInfo:)), @"nothing selected: get info is disabled");
        [first selectItems: @[ d, [one URLByAppendingPathComponent: @"Neu"] ]];
        expect(enabled(first, @selector(getInfo:)), @"get info is enabled with items selected");
        [first getInfo: nil];
        NSMutableArray<NSString *> *infoTitles = [NSMutableArray array];
        for (NSWindow *window in [NSApp windows]) {
            if ([window isVisible] && [window.title hasSuffix: @" Info"]) {
                [infoTitles addObject: window.title];
                [window close];
            }
        }
        [infoTitles sortUsingSelector: @selector(compare:)];
        expect([infoTitles isEqual: (@[ @"Neu Info", @"d.txt Info" ])],
               [NSString stringWithFormat: @"get info: a panel for each selected item (%@)", infoTitles]);

        if (argc < 3)
            printf("SKIP: a read-only folder: no disk image given\n");
        else {
            NSURL *volume = attach([NSString stringWithUTF8String: argv[2]]);
            expect(volume != nil, @"hdiutil attach");
            if (volume) {
                FinderWindowController *image = [[FinderWindowController alloc] initWithFolderURL: volume];
                [image showWindow: nil];
                spin();
                [image selectAll: nil];
                [[NSPasteboard generalPasteboard] clearContents];
                [[NSPasteboard generalPasteboard] writeObjects: @[ d ]];
                expect([image.selectedItems count] > 0 && enabled(image, @selector(copy:)), @"read-only folder: copy is enabled");
                expect(!enabled(image, @selector(paste:)) && !enabled(image, @selector(newFolder:))
                           && !enabled(image, @selector(rename:)) && !enabled(image, @selector(moveToTrash:)),
                       @"read-only folder: paste, new folder, rename and move to trash are disabled");
                [image close];
                image = nil;
                spin();
                NSTask *task = [NSTask launchedTaskWithLaunchPath: @"/usr/bin/hdiutil" arguments: @[ @"detach", [volume path] ]];
                [task waitUntilExit];
            }
        }
        [first close];
        [second close];
        spin();

        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
