// CFLAGS: -framework Foundation -framework AppKit -framework ImageIO -framework CoreServices -fobjc-arc -Isrc/Finder src/Finder/FinderDirectory.m src/Finder/FinderDSStore.m src/Finder/FinderFolderStyle.m src/Finder/FinderFreeformLayout.m src/Finder/FinderDrop.m src/Finder/FinderIconViewController.m src/Finder/FinderListViewController.m src/Finder/FinderSidebarController.m src/Finder/FinderWindowController.m src/Finder/FinderInfoPanelController.m src/Finder/FinderFileOperation.m src/Finder/FinderItemActions.m src/Finder/FinderOperationQueue.m
// ARGS: tests/fixtures/dsstore
// Copyright (C) 2026 Lubos Dolezel
//
// Finder windows styled by their folder's .DS_Store (src/Finder/FinderWindowController, FinderIconViewController,
// FinderFreeformLayout), in process: the window's content size and hidden toolbar and sidebar, items at their stored
// centres and the others below them, the icon and text sizes, the sidebar shown again, the flow layout for a folder
// without a style and the stored layout again on going back, hostile positions, and window sizes and icon sizes out
// of range. See tests/finder.md. Needs a display.
// Usage: finder_styled_window <fixture dir>. Prints PASS/FAIL lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderFolderStyle.h"
#import "FinderFreeformLayout.h"
#import "FinderIconViewController.h"
#import "FinderWindowController.h"
#include <math.h>
#include <stdio.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSString *fixtureDir;

static void spin(NSTimeInterval seconds)
{
    [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: seconds]];
}

static id findView(NSView *view, Class kind)
{
    if ([view isKindOfClass: kind])
        return view;
    for (NSView *subview in [view subviews]) {
        id found = findView(subview, kind);
        if (found != nil)
            return found;
    }
    return nil;
}

static NSCollectionView *collectionViewOf(FinderWindowController *controller)
{
    return findView([controller.window contentView], [NSCollectionView class]);
}

// The sidebar's view, the first of the split view's
static NSView *sidebarOf(FinderWindowController *controller)
{
    NSSplitView *split = findView([controller.window contentView], [NSSplitView class]);
    return [[split subviews] firstObject];
}

static BOOL sidebarShown(FinderWindowController *controller)
{
    NSView *sidebar = sidebarOf(controller);
    NSSplitView *split = (NSSplitView *) [sidebar superview];
    return sidebar != nil && ![split isSubviewCollapsed: sidebar] && NSWidth([sidebar frame]) > 0;
}

static NSSize contentSize(FinderWindowController *controller)
{
    return [[controller.window contentView] frame].size;
}

// Each item's frame in the icon view, by file name
static NSDictionary<NSString *, NSValue *> *frames(FinderWindowController *controller)
{
    NSCollectionView *view = collectionViewOf(controller);
    FinderIconViewController *icons = (FinderIconViewController *) view.dataSource;
    NSMutableDictionary *result = [NSMutableDictionary dictionary];
    for (NSUInteger i = 0; i < [icons.items count]; i++) {
        NSCollectionViewLayoutAttributes *attributes = [view.collectionViewLayout
            layoutAttributesForItemAtIndexPath: [NSIndexPath indexPathForItem: i inSection: 0]];
        if (attributes != nil)
            result[[[icons.items[i] lastPathComponent] precomposedStringWithCanonicalMapping]] = [NSValue valueWithRect: attributes.frame];
    }
    return result;
}

static BOOL centredAt(NSDictionary *frames, NSString *name, CGFloat x, CGFloat y)
{
    NSValue *value = frames[[name precomposedStringWithCanonicalMapping]];
    if (value == nil)
        return NO;
    NSRect frame = [value rectValue];
    return fabs(NSMidX(frame) - x) <= 1 && fabs(NSMidY(frame) - y) <= 1;
}

// The image view of the first item that has a view loaded
static NSCollectionViewItem *someItem(FinderWindowController *controller)
{
    NSCollectionView *view = collectionViewOf(controller);
    for (NSInteger i = 0; i < [view numberOfItemsInSection: 0]; i++) {
        NSCollectionViewItem *item = [view itemAtIndexPath: [NSIndexPath indexPathForItem: i inSection: 0]];
        if (item != nil)
            return item;
    }
    return nil;
}

static NSString *makeFolder(NSString *parent, NSString *name, NSString *fixture, NSArray<NSString *> *files)
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *folder = [parent stringByAppendingPathComponent: name];
    [fm createDirectoryAtPath: folder withIntermediateDirectories: YES attributes: nil error: NULL];
    if (fixture != nil)
        [fm copyItemAtPath: [fixtureDir stringByAppendingPathComponent: [fixture stringByAppendingString: @".DS_Store"]]
                    toPath: [folder stringByAppendingPathComponent: @".DS_Store"] error: NULL];
    for (NSString *file in files)
        [fm createFileAtPath: [folder stringByAppendingPathComponent: file] contents: [NSData data] attributes: nil];
    return folder;
}

static FinderWindowController *openWindow(NSString *folder)
{
    FinderWindowController *controller = [[FinderWindowController alloc] initWithFolderURL: [NSURL fileURLWithPath: folder]];
    [controller showWindow: nil];
    spin(0.5);
    return controller;
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        setvbuf(stdout, NULL, _IONBF, 0);
        if (argc != 2) {
            fprintf(stderr, "usage: %s <fixture dir>\n", argv[0]);
            return 2;
        }
        [NSApplication sharedApplication];
        NSFileManager *fm = [NSFileManager defaultManager];
        fixtureDir = [NSString stringWithUTF8String: argv[1]];
        NSString *dir = [NSTemporaryDirectory() stringByAppendingPathComponent: [[NSUUID UUID] UUIDString]];

        // The styled folder. Its volume is the one the temporary folder is on, so the picture isn't found there:
        // finder_folder_style.m tests that, and tests/finder.sh's styled window its pixels.
        NSString *styled = makeFolder(dir, @"Styled", @"styled", @[ @"Ünï 😀.txt", @"Unplaced.txt" ]);
        [fm createDirectoryAtPath: [styled stringByAppendingPathComponent: @"Styled App.app"] withIntermediateDirectories: NO
                       attributes: nil error: NULL];
        [fm createDirectoryAtPath: [styled stringByAppendingPathComponent: @".background"] withIntermediateDirectories: NO
                       attributes: nil error: NULL];
        [fm createSymbolicLinkAtPath: [styled stringByAppendingPathComponent: @"Applications"] withDestinationPath: @"/Applications"
                               error: NULL];
        NSString *plain = makeFolder(styled, @"Plain", nil, @[ @"inside.txt" ]);

        FinderWindowController *window = openWindow(styled);
        expect(NSEqualSizes(contentSize(window), NSMakeSize(640, 400)),
               [NSString stringWithFormat: @"content 640x400 (%@)", NSStringFromSize(contentSize(window))]);
        expect(![window.window.toolbar isVisible], @"no toolbar");
        expect(!sidebarShown(window), @"no sidebar");
        NSCollectionView *icons = collectionViewOf(window);
        expect(NSWidth([[icons enclosingScrollView] frame]) >= 639,
               [NSString stringWithFormat: @"the items take the whole width (%g)", NSWidth([[icons enclosingScrollView] frame])]);
        expect([icons.collectionViewLayout isKindOfClass: [FinderFreeformLayout class]], @"the freeform layout");
        NSDictionary *positions = frames(window);
        expect(centredAt(positions, @"Styled App.app", 160, 200) && centredAt(positions, @"Applications", 480, 200)
                   && centredAt(positions, @"Ünï 😀.txt", 320, 330),
               [NSString stringWithFormat: @"items centred at (160, 200), (480, 200) and (320, 330) (%@)",
                [[positions allKeys] componentsJoinedByString: @", "]]);
        NSRect unplaced = [positions[@"Unplaced.txt"] rectValue];
        expect(positions[@"Unplaced.txt"] != nil && NSMinY(unplaced) > 330 + 64 && NSMinX(unplaced) >= 0,
               [NSString stringWithFormat: @"an item without a position goes below the others (%@)", NSStringFromRect(unplaced)]);
        NSCollectionViewItem *item = someItem(window);
        expect(NSWidth([item.imageView frame]) == 128 && [item.textField.font pointSize] == 13,
               [NSString stringWithFormat: @"icons 128, text 13 (%g, %g)", NSWidth([item.imageView frame]), [item.textField.font pointSize]]);

        NSMenuItem *toolbarItem = [[NSMenuItem alloc] initWithTitle: @"" action: @selector(toggleToolbar:) keyEquivalent: @""];
        NSMenuItem *sidebarItem = [[NSMenuItem alloc] initWithTitle: @"" action: @selector(toggleSidebar:) keyEquivalent: @""];
        [window validateMenuItem: toolbarItem];
        [window validateMenuItem: sidebarItem];
        expect([toolbarItem.title isEqual: @"Show Toolbar"] && [sidebarItem.title isEqual: @"Show Sidebar"],
               [NSString stringWithFormat: @"the View menu offers to show them (%@, %@)", toolbarItem.title, sidebarItem.title]);
        [window toggleToolbar: toolbarItem];
        [window validateMenuItem: toolbarItem];
        expect([window.window.toolbar isVisible] && [toolbarItem.title isEqual: @"Hide Toolbar"],
               [NSString stringWithFormat: @"toggleToolbar: shows the toolbar (%@)", toolbarItem.title]);
        [window toggleToolbar: toolbarItem];

        [window toggleSidebar: nil];
        spin(0.2);
        [window validateMenuItem: sidebarItem];
        expect(sidebarShown(window) && [sidebarItem.title isEqual: @"Hide Sidebar"], @"toggleSidebar: shows the sidebar");
        [window toggleSidebar: nil];
        spin(0.2);
        expect(!sidebarShown(window), @"and hides it again");

        [window showFolder: [NSURL fileURLWithPath: plain]];
        spin(0.3);
        icons = collectionViewOf(window);
        item = someItem(window);
        expect([icons.collectionViewLayout isKindOfClass: [NSCollectionViewFlowLayout class]] && NSWidth([item.imageView frame]) == 64,
               [NSString stringWithFormat: @"a folder without a style: the flow layout, icons 64 (%@, %g)",
                [icons.collectionViewLayout class], NSWidth([item.imageView frame])]);
        expect(NSEqualSizes(contentSize(window), NSMakeSize(640, 400)), @"the window keeps its size");
        [window goBack: nil];
        spin(0.3);
        icons = collectionViewOf(window);
        expect([icons.collectionViewLayout isKindOfClass: [FinderFreeformLayout class]]
                   && centredAt(frames(window), @"Applications", 480, 200),
               @"back in the styled folder: its layout again");
        [window close];

        // Hostile positions: every item inside the content, none above or left of it
        NSString *hostile = makeFolder(dir, @"Hostile", @"hostile-positions", @[ @"a.txt", @"b.txt", @"c.txt", @"d.txt", @"e.txt" ]);
        window = openWindow(hostile);
        icons = collectionViewOf(window);
        NSSize content = [icons.collectionViewLayout collectionViewContentSize];
        positions = frames(window);
        BOOL inside = [positions count] == 5;
        for (NSString *name in positions) {
            NSRect frame = [positions[name] rectValue];
            inside = inside && NSMinX(frame) >= 0 && NSMinY(frame) >= 0 && NSMaxX(frame) <= content.width
                && NSMaxY(frame) <= content.height;
        }
        expect(inside, [NSString stringWithFormat: @"hostile positions: every item inside the content %@ (%lu items)",
                        NSStringFromSize(content), (unsigned long) [positions count]]);

        [window close];

        // Window and icon sizes out of range
        NSString *reference = makeFolder(dir, @"Reference", nil, @[]);
        window = openWindow(reference);
        NSSize standard = contentSize(window);
        [window close];
        NSRect visible = [[NSScreen mainScreen] visibleFrame];

        window = openWindow(makeFolder(dir, @"Small", @"bounds-small", @[]));
        expect(NSEqualSizes(contentSize(window), NSMakeSize(300, 200)),
               [NSString stringWithFormat: @"a tiny window: 300x200 (%@)", NSStringFromSize(contentSize(window))]);
        [window close];
        window = openWindow(makeFolder(dir, @"Huge", @"bounds-huge", @[]));
        NSSize huge = contentSize(window);
        expect(huge.width <= NSWidth(visible) && huge.height <= NSHeight(visible) && NSWidth([window.window frame]) <= NSWidth(visible)
                   && NSHeight([window.window frame]) <= NSHeight(visible),
               [NSString stringWithFormat: @"a huge window: within the screen's visible frame %@ (%@)",
                NSStringFromSize(visible.size), NSStringFromRect([window.window frame])]);
        [window close];
        window = openWindow(makeFolder(dir, @"Garbage", @"bounds-garbage", @[]));
        expect(NSEqualSizes(contentSize(window), standard),
               [NSString stringWithFormat: @"unreadable bounds: the standard size %@ (%@)", NSStringFromSize(standard),
                NSStringFromSize(contentSize(window))]);
        [window close];
        window = openWindow(makeFolder(dir, @"NaN Bounds", @"bounds-nan", @[]));
        expect(NSEqualSizes(contentSize(window), standard),
               [NSString stringWithFormat: @"bounds-nan: the standard size %@ (%@)", NSStringFromSize(standard),
                NSStringFromSize(contentSize(window))]);
        [window close];
        for (NSString *name in @[ @"bounds-inf", @"bounds-vast" ]) {
            window = openWindow(makeFolder(dir, name, name, @[]));
            NSRect frame = [window.window frame];
            expect(NSWidth(frame) <= NSWidth(visible) && NSHeight(frame) <= NSHeight(visible) && NSMinY(frame) >= NSMinY(visible),
                   [NSString stringWithFormat: @"%@: within the screen's visible frame %@ (%@)", name,
                    NSStringFromRect(visible), NSStringFromRect(frame)]);
            [window close];
        }
        window = openWindow(makeFolder(dir, @"NaN Icons", @"icon-nan", @[ @"x.txt" ]));
        expect(NSWidth([someItem(window).imageView frame]) == 64,
               [NSString stringWithFormat: @"icon size NaN: 64 (%g)", NSWidth([someItem(window).imageView frame])]);
        [window close];

        // Background pictures that can't be used give the colour; one that can is shown. The style is made with the
        // temporary folder as its volume, so the pictures resolve.
        NSString *pictures = makeFolder(dir, @"Pictures", nil, @[ @"x.txt" ]);
        [fm createDirectoryAtPath: [pictures stringByAppendingPathComponent: @".background"] withIntermediateDirectories: NO
                       attributes: nil error: NULL];
        [@"not a picture" writeToFile: [pictures stringByAppendingPathComponent: @".background/notes.png"] atomically: NO
                             encoding: NSUTF8StringEncoding error: NULL];
        NSDictionary *sizes = @{ @"wide.png": [NSValue valueWithSize: NSMakeSize(9000, 2)],
                                 @"background.png": [NSValue valueWithSize: NSMakeSize(4, 4)] };
        for (NSString *file in sizes) {
            NSSize size = [sizes[file] sizeValue];
            NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL pixelsWide: size.width
                                                                            pixelsHigh: size.height bitsPerSample: 8
                                                                       samplesPerPixel: 3 hasAlpha: NO isPlanar: NO
                                                                        colorSpaceName: NSDeviceRGBColorSpace
                                                                           bytesPerRow: 0 bitsPerPixel: 0];
            [[rep representationUsingType: NSPNGFileType properties: @{}]
                writeToFile: [pictures stringByAppendingPathComponent: [@".background" stringByAppendingPathComponent: file]]
                 atomically: NO];
        }
        window = openWindow(pictures);
        icons = collectionViewOf(window);
        FinderIconViewController *controller = (FinderIconViewController *) icons.dataSource;
        NSDictionary *cases = @{ @"picture-not-image": @"a file that isn't a picture",
                                 @"picture-too-wide": @"a picture 9000 pixels wide" };
        for (NSString *fixture in cases) {
            [fm removeItemAtPath: [pictures stringByAppendingPathComponent: @".DS_Store"] error: NULL];
            [fm copyItemAtPath: [fixtureDir stringByAppendingPathComponent: [fixture stringByAppendingString: @".DS_Store"]]
                        toPath: [pictures stringByAppendingPathComponent: @".DS_Store"] error: NULL];
            FinderFolderStyle *style = [FinderFolderStyle styleForFolder: [NSURL fileURLWithPath: pictures]
                                                                  volume: [NSURL fileURLWithPath: pictures]];
            [controller applyStyle: style];
            NSColor *colour = [[icons.backgroundColors firstObject] colorUsingColorSpaceName: NSCalibratedRGBColorSpace];
            expect(style.backgroundImageURL != nil && icons.backgroundView == nil && colour != nil
                       && fabs([colour redComponent] - 0.2) < 0.01 && fabs([colour blueComponent] - 0.6) < 0.01,
                   [NSString stringWithFormat: @"%@: no picture, the colour (%@, %@, %@)", cases[fixture],
                    style.backgroundImageURL, icons.backgroundView, colour]);
        }
        [fm removeItemAtPath: [pictures stringByAppendingPathComponent: @".DS_Store"] error: NULL];
        [fm copyItemAtPath: [fixtureDir stringByAppendingPathComponent: @"styled.DS_Store"]
                    toPath: [pictures stringByAppendingPathComponent: @".DS_Store"] error: NULL];
        [controller applyStyle: [FinderFolderStyle styleForFolder: [NSURL fileURLWithPath: pictures]
                                                           volume: [NSURL fileURLWithPath: pictures]]];
        expect(icons.backgroundView != nil, @"a 4x4 picture is shown");
        [window close];

        window = openWindow(makeFolder(dir, @"Tiny Icons", @"icon-tiny", @[ @"x.txt" ]));
        expect(NSWidth([someItem(window).imageView frame]) == 16,
               [NSString stringWithFormat: @"icon size 0: 16 (%g)", NSWidth([someItem(window).imageView frame])]);
        [window close];
        window = openWindow(makeFolder(dir, @"Huge Icons", @"icon-huge", @[ @"x.txt" ]));
        expect(NSWidth([someItem(window).imageView frame]) == 512,
               [NSString stringWithFormat: @"icon size 4096: 512 (%g)", NSWidth([someItem(window).imageView frame])]);
        [window close];

        [fm removeItemAtPath: dir error: NULL];
        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
