/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "FinderWindowController.h"
#import "FinderDirectory.h"
#import "FinderIconViewController.h"
#import "FinderListViewController.h"
#import "FinderSidebarController.h"
#import <CoreServices/CoreServices.h>

static NSToolbarItemIdentifier const FinderViewStyleItem = @"FinderViewStyle";
static NSToolbarItemIdentifier const FinderHistoryItem = @"FinderHistory";

static void FinderFolderEvents(ConstFSEventStreamRef stream, void *info, size_t count, void *paths,
                               const FSEventStreamEventFlags flags[], const FSEventStreamEventId ids[]);

@implementation FinderWindowController {
    FSEventStreamRef _watch;
    FinderDirectory *_directory;
    FinderIconViewController *_icons;
    FinderListViewController *_list;
    FinderSidebarController *_sidebar;
    NSSegmentedControl *_viewControl;
    NSSegmentedControl *_historyControl;
    NSMutableArray<NSURL *> *_back;
    NSMutableArray<NSURL *> *_forward;
}

- (instancetype)initWithFolderURL:(NSURL *)folder
{
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 800, 500)
                                                   styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                                                       | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    window.releasedWhenClosed = NO;
    window.minSize = NSMakeSize(300, 200);
    self = [super initWithWindow:window];
    if (self != nil) {
        _back = [NSMutableArray array];
        _forward = [NSMutableArray array];
        window.delegate = self;
        __weak FinderWindowController *weakSelf = self;
        void (^open)(NSURL *) = ^(NSURL *item) {
            FinderWindowController *controller = weakSelf;
            if (controller.openItem != nil)
                controller.openItem(item);
        };

        _icons = [[FinderIconViewController alloc] init];
        _icons.openItem = open;
        _list = [[FinderListViewController alloc] init];
        _list.openItem = open;
        _sidebar = [[FinderSidebarController alloc] init];
        _sidebar.showFolder = open;

        // The sidebar on the left, the folder's items on the right
        NSRect bounds = [window.contentView bounds];
        NSSplitView *split = [[NSSplitView alloc] initWithFrame:bounds];
        split.vertical = YES;
        split.dividerStyle = NSSplitViewDividerStyleThin;
        split.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        _sidebar.view.frame = NSMakeRect(0, 0, 200, NSHeight(bounds));
        NSView *items = [[NSView alloc] initWithFrame:NSMakeRect(201, 0, NSWidth(bounds) - 201, NSHeight(bounds))];
        for (NSView *content in @[ _icons.view, _list.view ]) {
            content.frame = [items bounds];
            content.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
            [items addSubview:content];
        }
        _list.view.hidden = YES;
        [split addSubview:_sidebar.view];
        [split addSubview:items];
        [split adjustSubviews];
        [split setPosition:200 ofDividerAtIndex:0];
        [window.contentView addSubview:split];

        NSToolbar *toolbar = [[NSToolbar alloc] initWithIdentifier:@"FinderWindow"];
        toolbar.delegate = self;
        toolbar.displayMode = NSToolbarDisplayModeIconOnly;
        toolbar.allowsUserCustomization = NO;
        window.toolbar = toolbar;

        [self displayFolder:folder];
    }
    return self;
}

- (void)showFolder:(NSURL *)folder
{
    NSURL *current = _folderURL;
    [self displayFolder:folder];
    if (current != nil && ![[current path] isEqual:[_folderURL path]]) {
        [_back addObject:current];
        [_forward removeAllObjects];
    }
    [self historyChanged];
}

- (void)displayFolder:(NSURL *)folder
{
    _folderURL = [NSURL fileURLWithPath:[[folder URLByStandardizingPath] path] isDirectory:YES];
    _directory = [[FinderDirectory alloc] initWithURL:_folderURL];
    [_directory reload:NULL];
    self.window.title = [FinderDirectory displayNameForURL:_folderURL];
    self.window.representedURL = _folderURL;
    _icons.items = _directory.items;
    _list.items = _directory.items;
    [self watchFolder];
}

#pragma mark Watching the folder

// An FSEvents stream on the folder (WatchRoot: its own removal or move is reported too); on any event, a window whose
// folder no longer exists closes
- (void)watchFolder
{
    [self stopWatching];
    FSEventStreamContext context = { 0, (__bridge void *)self, NULL, NULL, NULL };
    _watch = FSEventStreamCreate(NULL, FinderFolderEvents, &context, (__bridge CFArrayRef) @[ [_folderURL path] ],
                                 kFSEventStreamEventIdSinceNow, 0.3, kFSEventStreamCreateFlagWatchRoot);
    if (_watch == NULL)
        return;
    FSEventStreamScheduleWithRunLoop(_watch, CFRunLoopGetMain(), kCFRunLoopCommonModes);
    FSEventStreamStart(_watch);
}

- (void)stopWatching
{
    if (_watch == NULL)
        return;
    FSEventStreamStop(_watch);
    FSEventStreamInvalidate(_watch);
    FSEventStreamRelease(_watch);
    _watch = NULL;
}

- (void)folderChanged
{
    BOOL directory = NO;
    if (![[NSFileManager defaultManager] fileExistsAtPath:[_folderURL path] isDirectory:&directory] || !directory)
        [self close];
}

- (void)windowWillClose:(NSNotification *)notification
{
    [self stopWatching];
}

- (void)dealloc
{
    [self stopWatching];
}

- (void)selectItems:(NSArray<NSURL *> *)items
{
    if (_viewStyle == FinderViewStyleList)
        [_list selectItems:items];
    else
        [_icons selectItems:items];
}

- (NSArray<NSURL *> *)selectedItems
{
    return _viewStyle == FinderViewStyleList ? _list.selectedItems : _icons.selectedItems;
}

#pragma mark History

- (BOOL)canGoBack
{
    return [_back count] > 0;
}

- (BOOL)canGoForward
{
    return [_forward count] > 0;
}

- (void)historyChanged
{
    [_historyControl setEnabled:self.canGoBack forSegment:0];
    [_historyControl setEnabled:self.canGoForward forSegment:1];
}

- (IBAction)goBack:(id)sender
{
    if (!self.canGoBack)
        return;
    NSURL *folder = [_back lastObject];
    [_back removeLastObject];
    [_forward addObject:_folderURL];
    [self displayFolder:folder];
    [self historyChanged];
}

- (IBAction)goForward:(id)sender
{
    if (!self.canGoForward)
        return;
    NSURL *folder = [_forward lastObject];
    [_forward removeLastObject];
    [_back addObject:_folderURL];
    [self displayFolder:folder];
    [self historyChanged];
}

- (IBAction)goToEnclosingFolder:(id)sender
{
    if ([[_folderURL path] isEqual:@"/"])
        return;
    NSURL *from = _folderURL;
    [self showFolder:[from URLByDeletingLastPathComponent]];
    [self selectItems:@[ from ]];
}

- (void)historyControlChanged:(NSSegmentedControl *)sender
{
    if (sender.selectedSegment == 0)
        [self goBack:sender];
    else
        [self goForward:sender];
}

- (IBAction)openSelection:(id)sender
{
    for (NSURL *item in self.selectedItems) {
        if (_openItem != nil)
            _openItem(item);
    }
}

#pragma mark Views

- (void)setViewStyle:(FinderViewStyle)style
{
    if (style == _viewStyle)
        return;
    NSArray<NSURL *> *selected = self.selectedItems;
    _viewStyle = style;
    _icons.view.hidden = style != FinderViewStyleIcons;
    _list.view.hidden = style != FinderViewStyleList;
    [self selectItems:selected];
    [self.window makeFirstResponder:style == FinderViewStyleList ? (NSView *) _list.outlineView
                                                                 : (NSView *) _icons.collectionView];
    _viewControl.selectedSegment = style;
}

- (IBAction)showAsIcons:(id)sender
{
    self.viewStyle = FinderViewStyleIcons;
}

- (IBAction)showAsList:(id)sender
{
    self.viewStyle = FinderViewStyleList;
}

- (void)viewControlChanged:(NSSegmentedControl *)sender
{
    self.viewStyle = sender.selectedSegment == 1 ? FinderViewStyleList : FinderViewStyleIcons;
}

#pragma mark Eject

// The volume this window's folder is on, if it can be ejected
- (NSURL *)ejectableVolume
{
    NSURL *volume = nil;
    NSNumber *ejectable = nil, *removable = nil;
    if (![_folderURL getResourceValue:&volume forKey:NSURLVolumeURLKey error:NULL] || volume == nil)
        return nil;
    [volume getResourceValue:&ejectable forKey:NSURLVolumeIsEjectableKey error:NULL];
    [volume getResourceValue:&removable forKey:NSURLVolumeIsRemovableKey error:NULL];
    return [ejectable boolValue] || [removable boolValue] ? volume : nil;
}

- (IBAction)eject:(id)sender
{
    NSURL *volume = [self ejectableVolume];
    NSError *error = nil;
    if (volume == nil || [[NSWorkspace sharedWorkspace] unmountAndEjectDeviceAtURL:volume error:&error])
        return;
    NSString *name = nil;
    [volume getResourceValue:&name forKey:NSURLVolumeLocalizedNameKey error:NULL];
    NSAlert *alert = [[NSAlert alloc] init];
    alert.alertStyle = NSAlertStyleWarning;
    alert.messageText = [NSString stringWithFormat:@"The disk “%@” wasn’t ejected.", name ?: [volume lastPathComponent]];
    alert.informativeText = [error localizedDescription] ?: @"";
    [alert addButtonWithTitle:@"OK"];
    [alert beginSheetModalForWindow:self.window completionHandler:nil];
}

- (BOOL)validateMenuItem:(NSMenuItem *)item
{
    if (item.action == @selector(eject:))
        return [self ejectableVolume] != nil;
    if (item.action == @selector(goBack:))
        return self.canGoBack;
    if (item.action == @selector(goForward:))
        return self.canGoForward;
    if (item.action == @selector(goToEnclosingFolder:))
        return ![[_folderURL path] isEqual:@"/"];
    if (item.action == @selector(openSelection:))
        return [self.selectedItems count] > 0;
    if (item.action == @selector(showAsIcons:))
        item.state = _viewStyle == FinderViewStyleIcons ? NSControlStateValueOn : NSControlStateValueOff;
    else if (item.action == @selector(showAsList:))
        item.state = _viewStyle == FinderViewStyleList ? NSControlStateValueOn : NSControlStateValueOff;
    return YES;
}

#pragma mark NSToolbarDelegate

- (NSArray<NSToolbarItemIdentifier> *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    return @[ FinderHistoryItem, NSToolbarFlexibleSpaceItemIdentifier, FinderViewStyleItem ];
}

- (NSArray<NSToolbarItemIdentifier> *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    return [self toolbarDefaultItemIdentifiers:toolbar];
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar
        itemForItemIdentifier:(NSToolbarItemIdentifier)identifier
    willBeInsertedIntoToolbar:(BOOL)flag
{
    if ([identifier isEqual:FinderHistoryItem]) {
        _historyControl = [[NSSegmentedControl alloc] initWithFrame:NSMakeRect(0, 0, 60, 24)];
        _historyControl.segmentCount = 2;
        [_historyControl setLabel:@"‹" forSegment:0];
        [_historyControl setLabel:@"›" forSegment:1];
        [_historyControl setWidth:28 forSegment:0];
        [_historyControl setWidth:28 forSegment:1];
        [[_historyControl cell] setTrackingMode:NSSegmentSwitchTrackingMomentary];
        _historyControl.target = self;
        _historyControl.action = @selector(historyControlChanged:);
        [self historyChanged];
        NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:identifier];
        item.label = @"Back/Forward";
        item.paletteLabel = @"Back/Forward";
        item.toolTip = @"See the folders you viewed previously";
        item.view = _historyControl;
        return item;
    }
    if (![identifier isEqual:FinderViewStyleItem])
        return nil;
    _viewControl = [[NSSegmentedControl alloc] initWithFrame:NSMakeRect(0, 0, 110, 24)];
    _viewControl.segmentCount = 2;
    [_viewControl setLabel:@"Icons" forSegment:0];
    [_viewControl setLabel:@"List" forSegment:1];
    [_viewControl setWidth:52 forSegment:0];
    [_viewControl setWidth:52 forSegment:1];
    [[_viewControl cell] setTrackingMode:NSSegmentSwitchTrackingSelectOne];
    _viewControl.target = self;
    _viewControl.action = @selector(viewControlChanged:);
    _viewControl.selectedSegment = _viewStyle;
    NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:identifier];
    item.label = @"View";
    item.paletteLabel = @"View";
    item.toolTip = @"Show items as icons or in a list";
    item.view = _viewControl;
    return item;
}

@end

static void FinderFolderEvents(ConstFSEventStreamRef stream, void *info, size_t count, void *paths,
                               const FSEventStreamEventFlags flags[], const FSEventStreamEventId ids[])
{
    [(__bridge FinderWindowController *)info folderChanged];
}
