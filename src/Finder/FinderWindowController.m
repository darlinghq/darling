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
#import "FinderFolderStyle.h"
#import "FinderIconViewController.h"
#import "FinderInfoPanelController.h"
#import "FinderItemActions.h"
#import "FinderListViewController.h"
#import "FinderOperationQueue.h"
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
    NSSplitView *_split;
    FinderFolderStyle *_style;
    NSSegmentedControl *_viewControl;
    NSSegmentedControl *_historyControl;
    NSMutableArray<NSURL *> *_back;
    NSMutableArray<NSURL *> *_forward;
    NSURL *_renamingItem;
    BOOL _reloadPending;
}

- (instancetype)initWithFolderURL:(NSURL *)folder
{
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 800, 500)
                                                   styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                                                       | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    window.releasedWhenClosed = NO;
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

        void (^rename)(NSURL *, NSString *) = ^(NSURL *item, NSString *name) {
            [weakSelf finishRenaming:item to:name];
        };
        _icons = [[FinderIconViewController alloc] init];
        _icons.openItem = open;
        _icons.renameItem = rename;
        void (^drop)(NSArray<NSURL *> *, NSURL *, FinderTransferKind) = ^(NSArray<NSURL *> *items, NSURL *folder, FinderTransferKind kind) {
            [[FinderOperationQueue sharedQueue] transferItems:items to:folder kind:kind window:weakSelf.window];
        };
        _icons.dropItems = drop;
        _list = [[FinderListViewController alloc] init];
        _list.openItem = open;
        _list.renameItem = rename;
        _list.dropItems = drop;
        _sidebar = [[FinderSidebarController alloc] init];
        _sidebar.showFolder = open;
        _sidebar.dropItems = drop;

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
        _split = split;

        NSToolbar *toolbar = [[NSToolbar alloc] initWithIdentifier:@"FinderWindow"];
        toolbar.delegate = self;
        toolbar.displayMode = NSToolbarDisplayModeIconOnly;
        toolbar.allowsUserCustomization = NO;
        window.toolbar = toolbar;
        // The content's minimum (a frame minimum set before the toolbar is attached grows with it on macOS)
        window.contentMinSize = NSMakeSize(300, 200);

        [self displayFolder:folder];
        [self applyWindowStyle];
    }
    return self;
}

// What only a new window takes from its folder's style: its size and whether it has a toolbar and a sidebar
- (void)applyWindowStyle
{
    if (_style == nil)
        return;
    NSWindow *window = self.window;
    window.toolbar.visible = _style.showsToolbar;
    if (!_style.showsSidebar)
        [self setSidebarShown:NO];

    NSSize size = _style.windowContentSize;
    if (size.width <= 0 || size.height <= 0)
        return;
    // At most the screen's visible frame, title bar and toolbar included; limited before the window gets it
    NSRect visible = [(window.screen ?: [NSScreen mainScreen]) visibleFrame];
    NSSize current = [window.contentView frame].size;
    NSSize chrome = NSMakeSize(NSWidth([window frame]) - current.width, NSHeight([window frame]) - current.height);
    BOOL limited = NO;
    if (!NSIsEmptyRect(visible)) {
        limited = size.width > NSWidth(visible) - chrome.width || size.height > NSHeight(visible) - chrome.height;
        size.width = MIN(size.width, NSWidth(visible) - chrome.width);
        size.height = MIN(size.height, NSHeight(visible) - chrome.height);
    }
    size.width = MAX(size.width, window.contentMinSize.width);
    size.height = MAX(size.height, window.contentMinSize.height);
    [window setContentSize:size];
    if (limited) {
        NSRect frame = [window frame];
        frame.origin = NSMakePoint(NSMinX(visible), NSMaxY(visible) - NSHeight(frame));
        [window setFrame:frame display:NO];
    }
}

- (BOOL)sidebarShown
{
    NSView *sidebar = _sidebar.view;
    return ![_split isSubviewCollapsed:sidebar];
}

// The sidebar is hidden (the split view collapses a hidden subview) or shown at its standard width
- (void)setSidebarShown:(BOOL)shown
{
    if (shown == [self sidebarShown])
        return;
    _sidebar.view.hidden = !shown;
    [_split adjustSubviews];
    if (shown)
        [_split setPosition:200 ofDividerAtIndex:0];
}

- (IBAction)toggleToolbar:(id)sender
{
    [self.window toggleToolbarShown:sender];
}

- (IBAction)toggleSidebar:(id)sender
{
    [self setSidebarShown:![self sidebarShown]];
}

+ (NSAlert *)alertForFolder:(NSURL *)folder error:(NSError *)error
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.alertStyle = NSAlertStyleWarning;
    alert.messageText = [NSString stringWithFormat:@"The folder “%@” can’t be opened.",
                                                   [FinderDirectory displayNameForURL:folder]];
    alert.informativeText = [error localizedDescription] ?: @"";
    [alert addButtonWithTitle:@"OK"];
    return alert;
}

- (void)showFolder:(NSURL *)folder
{
    NSURL *current = _folderURL;
    if (![self displayFolder:folder])
        return;
    if (current != nil && ![[current path] isEqual:[_folderURL path]]) {
        [_back addObject:current];
        [_forward removeAllObjects];
    }
    [self historyChanged];
}

// Shows the folder; NO, with an alert sheet, if it can't be read and the window already shows one (a new window
// shows its folder regardless: whoever made it has reported the error)
- (BOOL)displayFolder:(NSURL *)folder
{
    NSURL *url = [NSURL fileURLWithPath:[[folder URLByStandardizingPath] path] isDirectory:YES];
    FinderDirectory *directory = [[FinderDirectory alloc] initWithURL:url];
    NSError *error = nil;
    if (![directory reload:&error] && _folderURL != nil) {
        [[FinderWindowController alertForFolder:url error:error] beginSheetModalForWindow:self.window completionHandler:nil];
        return NO;
    }
    // A rename in progress ends unfinished
    [_list cancelEditing];
    [_icons cancelEditing];
    _folderURL = url;
    _icons.folderURL = url;
    _list.folderURL = url;
    _directory = directory;
    _style = [FinderFolderStyle styleForFolder:url];
    self.window.title = [FinderDirectory displayNameForURL:_folderURL];
    self.window.representedURL = _folderURL;
    [_icons setItems:_directory.items style:_style];
    _list.items = _directory.items;
    // A folder's style says how it is shown; without one, the window keeps its view
    if (_style != nil)
        self.viewStyle = _style.viewStyle;
    _sidebar.currentFolder = _folderURL;
    [self watchFolder];
    return YES;
}

// Keeps the selection of the items still there. While an item is renamed, the reload waits for the end of it.
- (void)reloadFolder
{
    if (self.renamingItem != nil) {
        _reloadPending = YES;
        return;
    }
    _reloadPending = NO;
    NSArray<NSURL *> *selected = self.selectedItems;
    if (![_directory reload:NULL])
        return;
    _icons.items = _directory.items;
    [_list reloadItems:_directory.items];
    [self selectItems:selected];
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
    else
        [self reloadFolder];
}

- (void)windowWillClose:(NSNotification *)notification
{
    [self stopWatching];
}

- (void)dealloc
{
    [self stopWatching];
    // What the window still points at must not outlive this controller (the window can outlive it)
    NSWindow *window = self.window;
    if (window.delegate == self)
        window.delegate = nil;
    window.toolbar.delegate = nil;
    _viewControl.target = nil;
    _historyControl.target = nil;
}

- (void)selectItems:(NSArray<NSURL *> *)items
{
    if (_viewStyle == FinderViewStyleList)
        [_list selectItems:items];
    else
        [_icons selectItems:items];
}

- (FinderIconViewController *)iconViewController
{
    return _icons;
}

- (FinderListViewController *)listViewController
{
    return _list;
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
    NSURL *current = _folderURL;
    [_back removeLastObject]; // a folder that can't be shown any more leaves the history
    if ([self displayFolder:folder])
        [_forward addObject:current];
    [self historyChanged];
}

- (IBAction)goForward:(id)sender
{
    if (!self.canGoForward)
        return;
    NSURL *folder = [_forward lastObject];
    NSURL *current = _folderURL;
    [_forward removeLastObject];
    if ([self displayFolder:folder])
        [_back addObject:current];
    [self historyChanged];
}

- (IBAction)goToEnclosingFolder:(id)sender
{
    if ([[_folderURL path] isEqual:@"/"])
        return;
    NSURL *from = _folderURL;
    [self showFolder:[from URLByDeletingLastPathComponent]];
    if (_folderURL != from)
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

#pragma mark File operations

// Nothing can be made, renamed or removed in it
- (BOOL)folderIsReadOnly
{
    NSNumber *readOnly = nil;
    [_folderURL getResourceValue:&readOnly forKey:NSURLVolumeIsReadOnlyKey error:NULL];
    return [readOnly boolValue] || ![[NSFileManager defaultManager] isWritableFileAtPath:[_folderURL path]];
}

// The file URLs on the general pasteboard, those that still exist
- (NSArray<NSURL *> *)pasteboardItems
{
    NSArray<NSURL *> *urls = [[NSPasteboard generalPasteboard] readObjectsForClasses:@[ [NSURL class] ]
                                                                             options:@{ NSPasteboardURLReadingFileURLsOnlyKey : @YES }];
    NSMutableArray<NSURL *> *items = [NSMutableArray array];
    for (NSURL *url in urls) {
        if ([url isFileURL] && [[NSFileManager defaultManager] fileExistsAtPath:[url path]])
            [items addObject:url];
    }
    return items;
}

- (void)showAlertWithError:(NSError *)error
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.alertStyle = NSAlertStyleWarning;
    alert.messageText = [error localizedDescription] ?: @"";
    alert.informativeText = [error localizedRecoverySuggestion] ?: @"";
    [alert addButtonWithTitle:@"OK"];
    [[FinderOperationQueue sharedQueue] runAlert:alert];
}

- (IBAction)copy:(id)sender
{
    NSArray<NSURL *> *items = self.selectedItems;
    if ([items count] == 0)
        return;
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard clearContents];
    [pasteboard writeObjects:items];
}

- (void)pasteAs:(FinderTransferKind)kind
{
    NSArray<NSURL *> *items = [self pasteboardItems];
    if ([items count] == 0 || [self folderIsReadOnly])
        return;
    [[FinderOperationQueue sharedQueue] transferItems:items to:_folderURL kind:kind window:self.window];
}

- (IBAction)paste:(id)sender
{
    [self pasteAs:FinderTransferCopy];
}

- (IBAction)moveItemHere:(id)sender
{
    [self pasteAs:FinderTransferMove];
}

- (IBAction)selectAll:(id)sender
{
    [self selectItems:_directory.items];
}

- (IBAction)newFolder:(id)sender
{
    if ([self folderIsReadOnly])
        return;
    NSError *error = nil;
    NSURL *folder = [FinderItemActions createFolderIn:_folderURL error:&error];
    if (folder == nil) {
        [self showAlertWithError:error];
        return;
    }
    [self reloadFolder];
    [self beginRenamingItem:folder];
}

- (IBAction)rename:(id)sender
{
    NSArray<NSURL *> *items = self.selectedItems;
    if ([items count] == 1)
        [self beginRenamingItem:items[0]];
}

- (void)beginRenamingItem:(NSURL *)item
{
    if ([self folderIsReadOnly])
        return;
    [self selectItems:@[ item ]];
    BOOL started = _viewStyle == FinderViewStyleList ? [_list beginEditingItem:item] : [_icons beginEditingItem:item];
    _renamingItem = started ? item : nil;
}

// The item being renamed, while its view still edits it (a table can end an edit by itself)
- (NSURL *)renamingItem
{
    if (_renamingItem != nil && !_list.editing && !_icons.editing)
        _renamingItem = nil;
    return _renamingItem;
}

// The view's edit ended with name (nil: cancelled): the item is renamed if the name may be used and, for a new
// extension, the user agrees
- (void)finishRenaming:(NSURL *)item to:(NSString *)name
{
    _renamingItem = nil;
    NSURL *renamed = nil;
    if (name != nil && ![name isEqual:[item lastPathComponent]]) {
        NSError *problem = [FinderItemActions problemWithName:name forItem:item];
        NSString *from = nil, *to = nil;
        if (problem != nil) {
            [self showAlertWithError:problem];
        } else if ([FinderItemActions name:name changesExtensionOf:item from:&from to:&to] && ![self agreeToExtension:to from:from]) {
            // kept as it was
        } else {
            NSError *error = nil;
            renamed = [FinderItemActions renameItem:item to:name error:&error];
            if (renamed == nil)
                [self showAlertWithError:error];
        }
    }
    if (renamed != nil || _reloadPending)
        [self reloadFolder];
    if (renamed != nil)
        [self selectItems:@[ renamed ]];
}

- (BOOL)agreeToExtension:(NSString *)to from:(NSString *)from
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.alertStyle = NSAlertStyleWarning;
    alert.messageText = [NSString stringWithFormat:@"Are you sure you want to change the extension from “.%@” to “.%@”?", from, to];
    alert.informativeText = [NSString stringWithFormat:@"If you make this change, your document may open in a different application."];
    [alert addButtonWithTitle:[NSString stringWithFormat:@"Keep .%@", from]];
    [alert addButtonWithTitle:[NSString stringWithFormat:@"Use .%@", to]];
    return [[FinderOperationQueue sharedQueue] runAlert:alert] == 1;
}

- (IBAction)moveToTrash:(id)sender
{
    NSArray<NSURL *> *items = self.selectedItems;
    if ([items count] == 0 || [self folderIsReadOnly])
        return;
    __weak FinderWindowController *weakSelf = self;
    [FinderItemActions trashItems:items completion:^(NSDictionary<NSURL *, NSURL *> *trashed, NSError *error) {
        FinderWindowController *controller = weakSelf;
        if (error != nil)
            [controller showAlertWithError:error];
        [controller reloadFolder];
    }];
}

- (IBAction)getInfo:(id)sender
{
    for (NSURL *item in self.selectedItems)
        [FinderInfoPanelController showInfoForItem:item];
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
    if (item.action == @selector(openSelection:) || item.action == @selector(copy:) || item.action == @selector(getInfo:))
        return [self.selectedItems count] > 0;
    if (item.action == @selector(paste:) || item.action == @selector(moveItemHere:))
        return ![self folderIsReadOnly] && [[self pasteboardItems] count] > 0;
    if (item.action == @selector(newFolder:))
        return ![self folderIsReadOnly];
    if (item.action == @selector(rename:))
        return [self.selectedItems count] == 1 && ![self folderIsReadOnly];
    if (item.action == @selector(moveToTrash:))
        return [self.selectedItems count] > 0 && ![self folderIsReadOnly];
    if (item.action == @selector(toggleToolbar:))
        item.title = self.window.toolbar.visible ? @"Hide Toolbar" : @"Show Toolbar";
    if (item.action == @selector(toggleSidebar:))
        item.title = [self sidebarShown] ? @"Hide Sidebar" : @"Show Sidebar";
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
