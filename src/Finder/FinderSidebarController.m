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

#import "FinderSidebarController.h"
#import "FinderDrop.h"

static const CGFloat FinderSidebarIconSize = 16;

@implementation FinderSidebarController {
    NSOutlineView *_outlineView;
    NSArray<NSURL *> *_favorites;
    NSArray<NSURL *> *_locations;
    NSMutableDictionary<NSString *, NSString *> *_volumeNames;
    NSMutableSet<NSString *> *_ejectable;
    BOOL _showingCurrentFolder;
}

- (instancetype)init
{
    return [self initWithFavorites:nil];
}

- (instancetype)initWithFavorites:(NSArray<NSURL *> *)favorites
{
    self = [super initWithNibName:nil bundle:nil];
    if (self != nil) {
        _sectionTitles = @[ @"Favorites", @"Locations" ];
        _volumeNames = [NSMutableDictionary dictionary];
        _ejectable = [NSMutableSet set];
        if (favorites != nil)
            _favorites = [favorites copy];
        else
            [self loadFavorites];
        [self loadLocations];
        NSNotificationCenter *center = [[NSWorkspace sharedWorkspace] notificationCenter];
        for (NSNotificationName name in @[ NSWorkspaceDidMountNotification, NSWorkspaceDidUnmountNotification,
                                           NSWorkspaceDidRenameVolumeNotification ])
            [center addObserver:self selector:@selector(volumesChanged:) name:name object:nil];
    }
    return self;
}

- (void)dealloc
{
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
    // The outline view and its eject buttons can outlive this controller (their window does)
    _outlineView.dataSource = nil;
    _outlineView.delegate = nil;
    [self clearTargetsIn:_outlineView];
}

- (void)clearTargetsIn:(NSView *)view
{
    if ([view isKindOfClass:[NSControl class]] && [(NSControl *)view target] == self)
        [(NSControl *)view setTarget:nil];
    for (NSView *subview in [view subviews])
        [self clearTargetsIn:subview];
}

#pragma mark Items

- (void)loadFavorites
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *home = NSHomeDirectory();
    NSMutableArray<NSURL *> *favorites = [NSMutableArray array];
    NSMutableArray<NSString *> *candidates = [NSMutableArray arrayWithObjects:@"/Applications", home, nil];
    for (NSString *name in @[ @"Desktop", @"Documents", @"Downloads" ])
        [candidates addObject:[home stringByAppendingPathComponent:name]];
    for (NSString *path in candidates) {
        BOOL directory = NO;
        if ([fm fileExistsAtPath:path isDirectory:&directory] && directory)
            [favorites addObject:[NSURL fileURLWithPath:[path stringByStandardizingPath] isDirectory:YES]];
    }
    _favorites = favorites;
}

- (void)loadLocations
{
    NSArray<NSURLResourceKey> *keys = @[ NSURLVolumeLocalizedNameKey, NSURLVolumeIsEjectableKey, NSURLVolumeIsRemovableKey ];
    NSArray<NSURL *> *volumes =
        [[NSFileManager defaultManager] mountedVolumeURLsIncludingResourceValuesForKeys:keys
                                                                                options:NSVolumeEnumerationSkipHiddenVolumes];
    [_volumeNames removeAllObjects];
    [_ejectable removeAllObjects];
    for (NSURL *volume in volumes) {
        NSDictionary<NSURLResourceKey, id> *values = [volume resourceValuesForKeys:keys error:NULL];
        NSString *path = [[volume URLByStandardizingPath] path];
        if (values[NSURLVolumeLocalizedNameKey] != nil)
            _volumeNames[path] = values[NSURLVolumeLocalizedNameKey];
        if ([values[NSURLVolumeIsEjectableKey] boolValue] || [values[NSURLVolumeIsRemovableKey] boolValue])
            [_ejectable addObject:path];
    }
    _locations = volumes ?: @[];
}

- (void)volumesChanged:(NSNotification *)notification
{
    [self loadLocations];
    [_outlineView reloadData];
    [self expandSections];
    [self showCurrentFolder];
}

- (void)setCurrentFolder:(NSURL *)folder
{
    _currentFolder = folder;
    [self showCurrentFolder];
}

// Selects the current folder's item, or nothing, without showing anything
- (void)showCurrentFolder
{
    NSString *path = [[_currentFolder URLByStandardizingPath] path];
    NSInteger found = -1;
    for (NSInteger row = 0; path != nil && row < [_outlineView numberOfRows]; row++) {
        id item = [_outlineView itemAtRow:row];
        if ([item isKindOfClass:[NSURL class]] && [[[item URLByStandardizingPath] path] isEqual:path]) {
            found = row;
            break;
        }
    }
    _showingCurrentFolder = YES;
    if (found >= 0)
        [_outlineView selectRowIndexes:[NSIndexSet indexSetWithIndex:found] byExtendingSelection:NO];
    else
        [_outlineView deselectAll:nil];
    _showingCurrentFolder = NO;
}

- (NSArray<NSURL *> *)itemsInSection:(NSInteger)section
{
    return section == 0 ? _favorites : section == 1 ? _locations : @[];
}

- (NSString *)titleForItem:(NSURL *)item
{
    NSString *path = [[item URLByStandardizingPath] path];
    if (path == nil)
        return nil;
    if ([path isEqual:[NSHomeDirectory() stringByStandardizingPath]])
        return NSUserName();
    if (_volumeNames[path] != nil && [_locations containsObject:item])
        return _volumeNames[path];
    return [[NSFileManager defaultManager] displayNameAtPath:path];
}

- (BOOL)showsEjectButtonForItem:(NSURL *)item
{
    return [_locations containsObject:item] && [_ejectable containsObject:[[item URLByStandardizingPath] path]];
}

#pragma mark View

- (void)loadView
{
    NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 200, 400)];
    scroll.hasVerticalScroller = YES;
    scroll.autohidesScrollers = YES;
    scroll.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    _outlineView = [[NSOutlineView alloc] initWithFrame:[scroll bounds]];
    NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier:@"item"];
    column.resizingMask = NSTableColumnAutoresizingMask;
    column.width = scroll.contentSize.width - 4;
    [_outlineView addTableColumn:column];
    _outlineView.outlineTableColumn = column;
    _outlineView.headerView = nil;
    _outlineView.selectionHighlightStyle = NSTableViewSelectionHighlightStyleSourceList;
    _outlineView.columnAutoresizingStyle = NSTableViewUniformColumnAutoresizingStyle;
    _outlineView.rowHeight = 22;
    _outlineView.indentationPerLevel = 8;
    _outlineView.dataSource = self;
    _outlineView.delegate = self;
    [_outlineView registerForDraggedTypes:@[ NSPasteboardTypeFileURL ]];
    [_outlineView sizeLastColumnToFit];

    scroll.documentView = _outlineView;
    self.view = scroll;
    [self expandSections];
    [self showCurrentFolder];
}

- (void)expandSections
{
    for (NSString *title in _sectionTitles)
        [_outlineView expandItem:title];
}

- (NSView *)cellForItem:(NSURL *)item width:(CGFloat)width
{
    NSTableCellView *cell = [[NSTableCellView alloc] initWithFrame:NSMakeRect(0, 0, width, 22)];
    NSImageView *image = [[NSImageView alloc] initWithFrame:NSMakeRect(2, 3, FinderSidebarIconSize, FinderSidebarIconSize)];
    NSImage *icon = [[[NSWorkspace sharedWorkspace] iconForFile:[item path]] copy];
    icon.size = NSMakeSize(FinderSidebarIconSize, FinderSidebarIconSize);
    image.image = icon;
    [cell addSubview:image];
    cell.imageView = image;

    BOOL eject = [self showsEjectButtonForItem:item];
    CGFloat textWidth = width - FinderSidebarIconSize - 8 - (eject ? 22 : 0);
    NSTextField *text = [NSTextField labelWithString:[self titleForItem:item] ?: @""];
    text.frame = NSMakeRect(FinderSidebarIconSize + 6, 2, MAX(textWidth, 20), 18);
    text.lineBreakMode = NSLineBreakByTruncatingTail;
    text.autoresizingMask = NSViewWidthSizable;
    [cell addSubview:text];
    cell.textField = text;

    if (eject) {
        NSButton *button = [[NSButton alloc] initWithFrame:NSMakeRect(width - 20, 2, 18, 18)];
        button.bezelStyle = NSBezelStyleInline;
        button.bordered = NO;
        button.title = @"⏏";
        button.toolTip = [NSString stringWithFormat:@"Eject “%@”", [self titleForItem:item]];
        button.autoresizingMask = NSViewMinXMargin;
        button.target = self;
        button.action = @selector(ejectClicked:);
        button.identifier = [item path];
        [cell addSubview:button];
    }
    return cell;
}

#pragma mark Actions

- (void)ejectClicked:(NSButton *)sender
{
    NSURL *volume = [NSURL fileURLWithPath:sender.identifier isDirectory:YES];
    NSString *name = [self titleForItem:volume];
    NSError *error = nil;
    if (![[NSWorkspace sharedWorkspace] unmountAndEjectDeviceAtURL:volume error:&error]) {
        NSAlert *alert = [[NSAlert alloc] init];
        alert.alertStyle = NSAlertStyleWarning;
        alert.messageText = [NSString stringWithFormat:@"The disk “%@” wasn’t ejected.", name];
        alert.informativeText = [error localizedDescription] ?: @"";
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
    }
}

#pragma mark NSOutlineViewDataSource

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if (item == nil)
        return [_sectionTitles count];
    if ([item isKindOfClass:[NSString class]])
        return [[self itemsInSection:[_sectionTitles indexOfObject:item]] count];
    return 0;
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
    if (item == nil)
        return _sectionTitles[index];
    return [self itemsInSection:[_sectionTitles indexOfObject:item]][index];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return [item isKindOfClass:[NSString class]];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)column byItem:(id)item
{
    return [item isKindOfClass:[NSString class]] ? item : [self titleForItem:item];
}

#pragma mark NSOutlineViewDelegate

- (BOOL)outlineView:(NSOutlineView *)outlineView isGroupItem:(id)item
{
    return [item isKindOfClass:[NSString class]];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
    return [item isKindOfClass:[NSURL class]];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldShowOutlineCellForItem:(id)item
{
    return NO;
}

- (NSView *)outlineView:(NSOutlineView *)outlineView viewForTableColumn:(NSTableColumn *)column item:(id)item
{
    CGFloat width = MAX([[outlineView tableColumns][0] width] - [outlineView levelForItem:item] * [outlineView indentationPerLevel], 100);
    if ([item isKindOfClass:[NSString class]]) {
        NSTableCellView *cell = [[NSTableCellView alloc] initWithFrame:NSMakeRect(0, 0, width, 22)];
        NSTextField *text = [NSTextField labelWithString:item];
        text.font = [NSFont boldSystemFontOfSize:[NSFont smallSystemFontSize]];
        text.textColor = [NSColor secondaryLabelColor];
        text.frame = NSMakeRect(2, 2, width - 4, 18);
        text.autoresizingMask = NSViewWidthSizable;
        [cell addSubview:text];
        cell.textField = text;
        return cell;
    }
    return [self cellForItem:item width:width];
}

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    if (_showingCurrentFolder)
        return;
    id item = [_outlineView itemAtRow:[_outlineView selectedRow]];
    if ([item isKindOfClass:[NSURL class]] && _showFolder != nil)
        _showFolder(item);
}

#pragma mark Drops

- (NSOutlineView *)outlineView
{
    (void) self.view;
    return _outlineView;
}

- (NSDragOperation)outlineView:(NSOutlineView *)outlineView
                  validateDrop:(id<NSDraggingInfo>)info
                  proposedItem:(id)item
            proposedChildIndex:(NSInteger)index
{
    NSArray<NSURL *> *items = FinderDraggedFileURLs([info draggingPasteboard]);
    if (items == nil || index != NSOutlineViewDropOnItemIndex || ![item isKindOfClass:[NSURL class]])
        return NSDragOperationNone;
    return FinderDropOperation(items, FinderDropFolderForItem(item), [info draggingSourceOperationMask]);
}

- (BOOL)outlineView:(NSOutlineView *)outlineView acceptDrop:(id<NSDraggingInfo>)info item:(id)item childIndex:(NSInteger)index
{
    NSArray<NSURL *> *items = FinderDraggedFileURLs([info draggingPasteboard]);
    if (items == nil || index != NSOutlineViewDropOnItemIndex || ![item isKindOfClass:[NSURL class]])
        return NO;
    NSURL *folder = FinderDropFolderForItem(item);
    NSDragOperation operation = FinderDropOperation(items, folder, [info draggingSourceOperationMask]);
    if (operation == NSDragOperationNone || self.dropItems == nil)
        return NO;
    self.dropItems(items, folder, FinderTransferKindForDropOperation(operation));
    return YES;
}

@end
