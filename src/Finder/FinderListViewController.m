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

#import "FinderListViewController.h"
#import "FinderDrop.h"
#import "FinderDirectory.h"

static const CGFloat FinderListIconSize = 16;

// What a row shows of an item, read once
@interface FinderListEntry : NSObject
@property (copy) NSString *name;
@property (strong) NSDate *modified;
@property long long size; // -1 for folders
@property (copy) NSString *kind;
@property BOOL folder;
@end

@implementation FinderListEntry
@end

// The Name column's cell: the item's icon, then its name. The item is the cell's represented object.
@interface FinderNameCell : NSTextFieldCell
@end

@implementation FinderNameCell

+ (NSImage *)iconForURL:(NSURL *)url
{
    static NSCache *icons; // path -> icon
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        icons = [[NSCache alloc] init];
    });
    NSImage *icon = [icons objectForKey:[url path]];
    if (icon == nil) {
        icon = [[[NSWorkspace sharedWorkspace] iconForFile:[url path]] copy];
        [icon setSize:NSMakeSize(FinderListIconSize, FinderListIconSize)];
        if (icon != nil)
            [icons setObject:icon forKey:[url path]];
    }
    return icon;
}

- (void)drawWithFrame:(NSRect)frame inView:(NSView *)view
{
    NSURL *url = [self representedObject];
    if ([url isKindOfClass:[NSURL class]]) {
        NSImage *icon = [FinderNameCell iconForURL:url];
        NSRect iconRect = NSMakeRect(NSMinX(frame) + 2, NSMidY(frame) - FinderListIconSize / 2, FinderListIconSize,
                                     FinderListIconSize);
        [icon drawInRect:iconRect fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:1
              respectFlipped:YES hints:nil];
        frame.origin.x += FinderListIconSize + 6;
        frame.size.width -= FinderListIconSize + 6;
    }
    [super drawWithFrame:frame inView:view];
}

@end

// Return (without modifier keys) asks for Rename, as in the Finder
@interface FinderOutlineView : NSOutlineView
@end

@implementation FinderOutlineView

- (void)keyDown:(NSEvent *)event
{
    NSString *characters = [event charactersIgnoringModifiers];
    NSEventModifierFlags modifiers = [event modifierFlags] & (NSEventModifierFlagCommand | NSEventModifierFlagOption
                                                              | NSEventModifierFlagControl | NSEventModifierFlagShift);
    if (modifiers == 0 && ([characters isEqual:@"\r"] || [characters isEqual:@"\003"])) {
        [NSApp sendAction:@selector(rename:) to:nil from:self];
        return;
    }
    [super keyDown:event];
}

@end

@implementation FinderListViewController {
    NSURL *_editingItem;
    NSOutlineView *_outlineView;
    NSMutableDictionary<NSString *, NSArray<NSURL *> *> *_children;
    NSMutableDictionary<NSString *, FinderListEntry *> *_entries;
    NSDateFormatter *_dateFormatter;
}

// The outline view can outlive this controller (its window does)
- (void)dealloc
{
    _outlineView.target = nil;
    _outlineView.dataSource = nil;
    _outlineView.delegate = nil;
}

- (instancetype)init
{
    self = [super initWithNibName:nil bundle:nil];
    if (self != nil) {
        _items = @[];
        _children = [NSMutableDictionary dictionary];
        _entries = [NSMutableDictionary dictionary];
        _dateFormatter = [[NSDateFormatter alloc] init];
        _dateFormatter.dateStyle = NSDateFormatterMediumStyle;
        _dateFormatter.timeStyle = NSDateFormatterShortStyle;
    }
    return self;
}

- (NSTableColumn *)columnWithIdentifier:(NSString *)identifier title:(NSString *)title width:(CGFloat)width
{
    NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier:identifier];
    [[column headerCell] setStringValue:title];
    column.width = width;
    column.minWidth = 40;
    column.editable = NO;
    column.sortDescriptorPrototype = [NSSortDescriptor sortDescriptorWithKey:identifier ascending:YES];
    return column;
}

- (void)loadView
{
    NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 600, 400)];
    scroll.hasVerticalScroller = YES;
    scroll.hasHorizontalScroller = YES;
    scroll.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    _outlineView = [[FinderOutlineView alloc] initWithFrame:[scroll bounds]];
    NSTableColumn *name = [self columnWithIdentifier:@"name" title:@"Name" width:300];
    FinderNameCell *nameCell = [[FinderNameCell alloc] initTextCell:@""];
    nameCell.lineBreakMode = NSLineBreakByTruncatingMiddle;
    // Renaming edits it; the column is editable only while that runs, so clicks don't start an edit
    nameCell.editable = YES;
    name.dataCell = nameCell;
    [_outlineView addTableColumn:name];
    [_outlineView addTableColumn:[self columnWithIdentifier:@"modified" title:@"Date Modified" width:180]];
    NSTableColumn *size = [self columnWithIdentifier:@"size" title:@"Size" width:90];
    [[size dataCell] setAlignment:NSTextAlignmentRight];
    [_outlineView addTableColumn:size];
    [_outlineView addTableColumn:[self columnWithIdentifier:@"kind" title:@"Kind" width:160]];
    _outlineView.outlineTableColumn = name;
    _outlineView.allowsMultipleSelection = YES;
    _outlineView.usesAlternatingRowBackgroundColors = YES;
    _outlineView.sortDescriptors = @[ name.sortDescriptorPrototype ];
    _outlineView.target = self;
    _outlineView.doubleAction = @selector(openClickedRow:);
    _outlineView.dataSource = self;
    _outlineView.delegate = self;
    [_outlineView registerForDraggedTypes:@[ NSPasteboardTypeFileURL ]];
    NSDragOperation offered = NSDragOperationCopy | NSDragOperationMove | NSDragOperationGeneric;
    [_outlineView setDraggingSourceOperationMask:offered forLocal:YES];
    [_outlineView setDraggingSourceOperationMask:offered forLocal:NO];

    scroll.documentView = _outlineView;
    self.view = scroll;
}

- (NSOutlineView *)outlineView
{
    [self view];
    return _outlineView;
}

- (void)setItems:(NSArray<NSURL *> *)items
{
    [self cancelEditing];
    [_children removeAllObjects];
    [_entries removeAllObjects];
    _items = [self sorted:items];
    [self.outlineView reloadData];
    [self.outlineView deselectAll:nil];
}

- (void)reloadItems:(NSArray<NSURL *> *)items
{
    // Expanded folders, parents before their children, and where the view is scrolled to
    NSMutableArray<NSString *> *expanded = [NSMutableArray array];
    for (NSInteger row = 0; row < [self.outlineView numberOfRows]; row++) {
        id item = [self.outlineView itemAtRow:row];
        if ([self.outlineView isItemExpanded:item])
            [expanded addObject:[[item URLByStandardizingPath] path]];
    }
    NSClipView *clip = [[self.outlineView enclosingScrollView] contentView];
    NSPoint scrolled = [clip bounds].origin;

    self.items = items;
    for (NSString *path in expanded) {
        for (NSInteger row = 0; row < [self.outlineView numberOfRows]; row++) {
            id item = [self.outlineView itemAtRow:row];
            if ([[[item URLByStandardizingPath] path] isEqual:path]) {
                [self.outlineView expandItem:item];
                break;
            }
        }
    }
    if (clip != nil) {
        [clip scrollToPoint:scrolled];
        [[self.outlineView enclosingScrollView] reflectScrolledClipView:clip];
    }
}

#pragma mark Entries and sorting

- (FinderListEntry *)entryForURL:(NSURL *)url
{
    FinderListEntry *entry = _entries[[url path]];
    if (entry != nil)
        return entry;

    entry = [[FinderListEntry alloc] init];
    NSDictionary<NSURLResourceKey, id> *values = [url resourceValuesForKeys:@[
        NSURLIsDirectoryKey, NSURLContentModificationDateKey, NSURLFileSizeKey, NSURLLocalizedTypeDescriptionKey
    ] error:NULL];
    entry.name = [FinderDirectory displayNameForURL:url];
    entry.modified = values[NSURLContentModificationDateKey];
    entry.kind = values[NSURLLocalizedTypeDescriptionKey] ?: @"";
    entry.folder = [values[NSURLIsDirectoryKey] boolValue]
        && ![[NSWorkspace sharedWorkspace] isFilePackageAtPath:[url path]];
    entry.size = entry.folder ? -1 : [values[NSURLFileSizeKey] longLongValue];
    _entries[[url path]] = entry;
    return entry;
}

- (NSComparisonResult)compare:(NSURL *)a with:(NSURL *)b key:(NSString *)key
{
    FinderListEntry *ea = [self entryForURL:a], *eb = [self entryForURL:b];
    NSComparisonResult result = NSOrderedSame;
    if ([key isEqual:@"modified"])
        result = [(ea.modified ?: [NSDate distantPast]) compare:(eb.modified ?: [NSDate distantPast])];
    else if ([key isEqual:@"size"])
        result = ea.size < eb.size ? NSOrderedAscending : ea.size > eb.size ? NSOrderedDescending : NSOrderedSame;
    else if ([key isEqual:@"kind"])
        result = [ea.kind localizedStandardCompare:eb.kind];
    if (result == NSOrderedSame)
        result = [ea.name localizedStandardCompare:eb.name];
    return result;
}

- (NSArray<NSURL *> *)sorted:(NSArray<NSURL *> *)urls
{
    NSSortDescriptor *descriptor = [self.outlineView.sortDescriptors firstObject];
    NSString *key = descriptor.key ?: @"name";
    BOOL ascending = descriptor != nil ? descriptor.ascending : YES;
    return [urls sortedArrayUsingComparator:^NSComparisonResult(NSURL *a, NSURL *b) {
        NSComparisonResult result = [self compare:a with:b key:key];
        return ascending ? result : (NSComparisonResult) -result;
    }];
}

- (NSArray<NSURL *> *)childrenOfItem:(NSURL *)item
{
    if (item == nil)
        return _items;
    NSArray<NSURL *> *children = _children[[item path]];
    if (children == nil) {
        FinderDirectory *directory = [[FinderDirectory alloc] initWithURL:item];
        [directory reload:NULL];
        children = [self sorted:directory.items];
        _children[[item path]] = children;
    }
    return children;
}

#pragma mark Selection

- (void)selectItems:(NSArray<NSURL *> *)items
{
    NSMutableSet<NSString *> *wanted = [NSMutableSet set];
    for (NSURL *item in items)
        [wanted addObject:[[item URLByStandardizingPath] path]];
    NSMutableIndexSet *rows = [NSMutableIndexSet indexSet];
    for (NSInteger row = 0; row < [self.outlineView numberOfRows]; row++) {
        NSURL *item = [self.outlineView itemAtRow:row];
        if ([wanted containsObject:[[item URLByStandardizingPath] path]])
            [rows addIndex:row];
    }
    [self.outlineView selectRowIndexes:rows byExtendingSelection:NO];
    if ([rows count] > 0)
        [self.outlineView scrollRowToVisible:[rows firstIndex]];
}

- (NSArray<NSURL *> *)selectedItems
{
    NSMutableArray<NSURL *> *selected = [NSMutableArray array];
    [[self.outlineView selectedRowIndexes] enumerateIndexesUsingBlock:^(NSUInteger row, BOOL *stop) {
        NSURL *item = [self.outlineView itemAtRow:row];
        if (item != nil)
            [selected addObject:item];
    }];
    return selected;
}

- (void)openClickedRow:(id)sender
{
    NSInteger row = [self.outlineView clickedRow];
    NSURL *item = row >= 0 ? [self.outlineView itemAtRow:row] : nil;
    if (item != nil && _openItem != nil)
        _openItem(item);
}

#pragma mark NSOutlineViewDataSource

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    return [[self childrenOfItem:item] count];
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
    return [self childrenOfItem:item][index];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return [self entryForURL:item].folder;
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)column byItem:(id)item
{
    FinderListEntry *entry = [self entryForURL:item];
    NSString *identifier = [column identifier];
    if ([identifier isEqual:@"modified"])
        return entry.modified != nil ? [_dateFormatter stringFromDate:entry.modified] : @"--";
    if ([identifier isEqual:@"size"])
        return entry.folder ? @"--"
                            : [NSByteCountFormatter stringFromByteCount:entry.size
                                                             countStyle:NSByteCountFormatterCountStyleFile];
    if ([identifier isEqual:@"kind"])
        return entry.kind;
    // The item being renamed shows its file name
    if (_editingItem != nil && [item isEqual:_editingItem])
        return [item lastPathComponent];
    return entry.name;
}

- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)value forTableColumn:(NSTableColumn *)column byItem:(id)item
{
    NSURL *editing = _editingItem;
    if (editing == nil || ![item isEqual:editing] || ![[column identifier] isEqual:@"name"])
        return;
    _editingItem = nil;
    if (_renameItem != nil)
        _renameItem(editing, [value isKindOfClass:[NSString class]] ? value : [value description]);
}

#pragma mark Editing a name

- (BOOL)isEditing
{
    // The table may have ended the edit itself
    if (_editingItem != nil && [self.outlineView editedRow] < 0)
        _editingItem = nil;
    return _editingItem != nil;
}

- (void)cancelEditing
{
    NSURL *editing = _editingItem;
    if (editing == nil)
        return;
    _editingItem = nil;
    [self.outlineView abortEditing];
    if (_renameItem != nil)
        _renameItem(editing, nil);
}

- (BOOL)beginEditingItem:(NSURL *)url
{
    [self cancelEditing];
    NSInteger row = -1;
    NSString *wanted = [[url URLByStandardizingPath] path];
    for (NSInteger r = 0; r < [self.outlineView numberOfRows] && row < 0; r++)
        if ([[[[self.outlineView itemAtRow:r] URLByStandardizingPath] path] isEqual:wanted])
            row = r;
    if (row < 0)
        return NO;
    NSURL *item = [self.outlineView itemAtRow:row];
    NSInteger column = [self.outlineView columnWithIdentifier:@"name"];
    NSTableColumn *nameColumn = [self.outlineView tableColumnWithIdentifier:@"name"];
    _editingItem = item;
    [self.outlineView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
    [self.outlineView scrollRowToVisible:row];
    // Editable only for this edit, so that clicks don't start one
    nameColumn.editable = YES;
    [self.outlineView editColumn:column row:row withEvent:nil select:YES];
    nameColumn.editable = NO;

    NSString *name = [item lastPathComponent];
    NSString *extension = [name pathExtension];
    NSUInteger length = [self entryForURL:item].folder || [extension length] == 0 ? [name length]
                                                                                 : [name length] - [extension length] - 1;
    [[self.outlineView currentEditor] setSelectedRange:NSMakeRange(0, length)];
    if ([self.outlineView editedRow] < 0) {
        _editingItem = nil;
        return NO;
    }
    return YES;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)column item:(id)item
{
    return _editingItem != nil && [item isEqual:_editingItem];
}

- (void)outlineView:(NSOutlineView *)outlineView sortDescriptorsDidChange:(NSArray<NSSortDescriptor *> *)old
{
    NSArray<NSURL *> *selected = self.selectedItems;
    _items = [self sorted:_items];
    for (NSString *path in [_children allKeys])
        _children[path] = [self sorted:_children[path]];
    [outlineView reloadData];
    [self selectItems:selected];
}

#pragma mark NSOutlineViewDelegate

- (void)outlineView:(NSOutlineView *)outlineView
    willDisplayCell:(id)cell
     forTableColumn:(NSTableColumn *)column
               item:(id)item
{
    if ([[column identifier] isEqual:@"name"])
        [cell setRepresentedObject:item];
}

#pragma mark Drag and drop

- (id<NSPasteboardWriting>)outlineView:(NSOutlineView *)outlineView pasteboardWriterForItem:(id)item
{
    return [item isKindOfClass:[NSURL class]] ? item : nil;
}

// The folder a drop goes into: a folder's (or a symlink to one) for a drop on its row or between its children;
// otherwise the window's. The outline view shows the drop on that folder's row, or on the whole view.
- (NSURL *)folderForDropOnItem:(id)item index:(NSInteger)index outlineView:(NSOutlineView *)outlineView retarget:(BOOL)retarget
{
    NSURL *folder = [item isKindOfClass:[NSURL class]] ? FinderDropFolderForItem(item) : nil;
    if (folder == nil) {
        if (retarget && (item != nil || index != NSOutlineViewDropOnItemIndex))
            [outlineView setDropItem:nil dropChildIndex:NSOutlineViewDropOnItemIndex];
        return self.folderURL;
    }
    if (retarget && index != NSOutlineViewDropOnItemIndex)
        [outlineView setDropItem:item dropChildIndex:NSOutlineViewDropOnItemIndex];
    return folder;
}

- (NSDragOperation)outlineView:(NSOutlineView *)outlineView
                  validateDrop:(id<NSDraggingInfo>)info
                  proposedItem:(id)item
            proposedChildIndex:(NSInteger)index
{
    NSArray<NSURL *> *items = FinderDraggedFileURLs([info draggingPasteboard]);
    if (items == nil)
        return NSDragOperationNone;
    NSURL *folder = [self folderForDropOnItem:item index:index outlineView:outlineView retarget:YES];
    return FinderDropOperation(items, folder, [info draggingSourceOperationMask]);
}

- (BOOL)outlineView:(NSOutlineView *)outlineView acceptDrop:(id<NSDraggingInfo>)info item:(id)item childIndex:(NSInteger)index
{
    NSArray<NSURL *> *items = FinderDraggedFileURLs([info draggingPasteboard]);
    if (items == nil)
        return NO;
    NSURL *folder = [self folderForDropOnItem:item index:index outlineView:outlineView retarget:NO];
    NSDragOperation operation = FinderDropOperation(items, folder, [info draggingSourceOperationMask]);
    if (operation == NSDragOperationNone || self.dropItems == nil)
        return NO;
    self.dropItems(items, folder, FinderTransferKindForDropOperation(operation));
    return YES;
}

@end
