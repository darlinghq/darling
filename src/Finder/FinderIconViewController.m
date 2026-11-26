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

#import "FinderIconViewController.h"
#import "FinderDrop.h"
#import "FinderDirectory.h"
#import "FinderFolderStyle.h"
#import "FinderFreeformLayout.h"
#import <ImageIO/ImageIO.h>
#include <math.h>

static NSString *const FinderIconItemIdentifier = @"FinderIconItem";

static const CGFloat FinderIconItemWidth = 100;
static const CGFloat FinderIconItemHeight = 100;
static const CGFloat FinderIconSize = 64;

// How an item looks: its size, its icon's, its label's font size and where the label goes
typedef struct {
    NSSize itemSize;
    CGFloat iconSize;
    CGFloat textSize;
    BOOL labelOnBottom;
} FinderIconLook;

static FinderIconLook FinderStandardLook(void)
{
    return (FinderIconLook){ NSMakeSize(FinderIconItemWidth, FinderIconItemHeight), FinderIconSize,
                             [NSFont smallSystemFontSize], YES };
}

// Room for the icon and two lines of its name, below it or on its right
static FinderIconLook FinderStyledLook(FinderFolderStyle *style)
{
    CGFloat icon = style.iconSize, text = style.textSize;
    CGFloat lines = 2 * ceil(text * 1.25) + 4;
    NSSize size = style.labelOnBottom ? NSMakeSize(MAX(FinderIconItemWidth, icon + 36), icon + lines + 12)
                                      : NSMakeSize(icon + 12 + 10 * text, MAX(icon + 8, lines + 8));
    return (FinderIconLook){ size, icon, text, style.labelOnBottom };
}

// The background behind the items, scrolling with them: a colour, and a picture at its natural size (its pixels as
// points) at the top left
@interface FinderIconBackgroundView : NSView
@property (nonatomic) NSColor *color;
@property (nonatomic) NSImage *image;
@end

@implementation FinderIconBackgroundView

- (BOOL)isFlipped
{
    return YES;
}

- (NSView *)hitTest:(NSPoint)point
{
    return nil;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [(_color ?: [NSColor controlBackgroundColor]) setFill];
    NSRectFill(dirtyRect);
    if (_image != nil) {
        NSRect rect = { NSZeroPoint, [_image size] };
        [_image drawInRect:rect fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:1
            respectFlipped:YES hints:nil];
    }
}

@end

// Pictures larger than this either way aren't used: a small file can declare a size that takes all memory to decode
static const NSInteger FinderBackgroundMaximumPixels = 8192;

// The picture's size in pixels, from its header (nothing is decoded), or NO if it isn't a picture
static BOOL FinderPictureSize(NSURL *url, NSInteger *width, NSInteger *height)
{
    CGImageSourceRef source = CGImageSourceCreateWithURL((__bridge CFURLRef) url, NULL);
    if (source == NULL)
        return NO;
    NSDictionary *properties = CFBridgingRelease(CGImageSourceCopyPropertiesAtIndex(source, 0, NULL));
    CFRelease(source);
    id w = properties[(__bridge NSString *) kCGImagePropertyPixelWidth], h = properties[(__bridge NSString *) kCGImagePropertyPixelHeight];
    if (![w isKindOfClass:[NSNumber class]] || ![h isKindOfClass:[NSNumber class]])
        return NO;
    *width = [w integerValue];
    *height = [h integerValue];
    return YES;
}

// The picture at its natural size, or nil if it can't be loaded or is too large
static NSImage *FinderBackgroundImage(NSURL *url)
{
    NSInteger width = 0, height = 0;
    if (url == nil || !FinderPictureSize(url, &width, &height) || width <= 0 || height <= 0
        || width > FinderBackgroundMaximumPixels || height > FinderBackgroundMaximumPixels)
        return nil;
    NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
    NSImageRep *rep = [[image representations] firstObject];
    if (rep == nil || [rep pixelsWide] <= 0 || [rep pixelsHigh] <= 0)
        return nil;
    [image setSize:NSMakeSize([rep pixelsWide], [rep pixelsHigh])];
    return image;
}

// An item's view: the icon above, the name below. Clicks go through it to the collection view, which selects.
@interface FinderIconItemView : NSView
@property (nonatomic) BOOL selected;
@end

@implementation FinderIconItemView

- (NSView *)hitTest:(NSPoint)point
{
    return nil;
}

- (void)setSelected:(BOOL)selected
{
    _selected = selected;
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect
{
    if (!_selected)
        return;
    [[NSColor selectedControlColor] setFill];
    [[NSBezierPath bezierPathWithRoundedRect:NSInsetRect([self bounds], 2, 2) xRadius:6 yRadius:6] fill];
}

@end

@interface FinderIconItem : NSCollectionViewItem
@property (nonatomic) FinderIconLook look;
@end

@implementation FinderIconItem {
    BOOL _hasLook;
}

- (void)loadView
{
    FinderIconItemView *view = [[FinderIconItemView alloc]
        initWithFrame:NSMakeRect(0, 0, FinderIconItemWidth, FinderIconItemHeight)];

    NSImageView *image = [[NSImageView alloc] initWithFrame:NSMakeRect((FinderIconItemWidth - FinderIconSize) / 2,
        FinderIconItemHeight - FinderIconSize - 4, FinderIconSize, FinderIconSize)];
    image.imageScaling = NSImageScaleProportionallyUpOrDown;
    [view addSubview:image];

    NSTextField *label = [[NSTextField alloc] initWithFrame:NSMakeRect(2, 2, FinderIconItemWidth - 4,
        FinderIconItemHeight - FinderIconSize - 8)];
    label.editable = NO;
    label.selectable = NO;
    label.bordered = NO;
    label.drawsBackground = NO;
    label.alignment = NSTextAlignmentCenter;
    label.font = [NSFont systemFontOfSize:[NSFont smallSystemFontSize]];
    // Two lines fit in the label's height; the last one shown is truncated
    [[label cell] setWraps:YES];
    [[label cell] setLineBreakMode:NSLineBreakByWordWrapping];
    [[label cell] setTruncatesLastVisibleLine:YES];
    [view addSubview:label];

    self.view = view;
    self.imageView = image;
    self.textField = label;
}

// Lays out the icon and the label for the look
- (void)setLook:(FinderIconLook)look
{
    [self view];
    if (_hasLook && NSEqualSizes(look.itemSize, _look.itemSize) && look.iconSize == _look.iconSize
        && look.textSize == _look.textSize && look.labelOnBottom == _look.labelOnBottom)
        return;
    _look = look;
    _hasLook = YES;
    NSSize size = look.itemSize;
    CGFloat icon = look.iconSize;
    [self.view setFrameSize:size];
    if (look.labelOnBottom) {
        self.imageView.frame = NSMakeRect((size.width - icon) / 2, size.height - icon - 4, icon, icon);
        self.textField.frame = NSMakeRect(2, 2, size.width - 4, size.height - icon - 8);
        self.textField.alignment = NSTextAlignmentCenter;
    } else {
        self.imageView.frame = NSMakeRect(4, (size.height - icon) / 2, icon, icon);
        self.textField.frame = NSMakeRect(icon + 8, 4, size.width - icon - 10, size.height - 8);
        self.textField.alignment = NSTextAlignmentLeft;
    }
    self.textField.font = [NSFont systemFontOfSize:look.textSize];
}

- (void)setRepresentedObject:(id)object
{
    [super setRepresentedObject:object];
    [self view]; // the outlets are set when the view loads
    NSURL *url = object;
    NSImage *icon = nil;
    if (url != nil) {
        icon = [[NSWorkspace sharedWorkspace] iconForFile:[url path]];
        CGFloat side = _hasLook ? _look.iconSize : FinderIconSize;
        [icon setSize:NSMakeSize(side, side)];
    }
    self.imageView.image = icon;
    self.textField.stringValue = url != nil ? [FinderDirectory displayNameForURL:url] : @"";
}

- (void)setSelected:(BOOL)selected
{
    [super setSelected:selected];
    ((FinderIconItemView *) self.view).selected = selected;
}

@end

// Tells the controller about double clicks, which the collection view leaves to its items; Return (without modifier
// keys) asks for Rename, as in the Finder.
@interface FinderIconCollectionView : NSCollectionView
@property (copy) void (^doubleClicked)(NSIndexPath *path);
@end

@implementation FinderIconCollectionView

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

- (void)mouseDown:(NSEvent *)event
{
    [super mouseDown:event];
    if ([event clickCount] != 2 || _doubleClicked == nil)
        return;
    NSIndexPath *path = [self indexPathForItemAtPoint:[self convertPoint:[event locationInWindow] fromView:nil]];
    if (path != nil)
        _doubleClicked(path);
}

@end

@implementation FinderIconViewController {
    FinderIconCollectionView *_collectionView;
    NSCollectionViewFlowLayout *_flowLayout;
    FinderFolderStyle *_style;
    FinderIconLook _look;
    NSURL *_editingItem;
    NSTextField *_editingField;
}

// The collection view can outlive this controller (its window does)
- (void)dealloc
{
    _collectionView.dataSource = nil;
    _collectionView.delegate = nil;
}

- (void)loadView
{
    NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 600, 400)];
    scroll.hasVerticalScroller = YES;
    scroll.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    _look = FinderStandardLook();
    NSCollectionViewFlowLayout *layout = [[NSCollectionViewFlowLayout alloc] init];
    _flowLayout = layout;
    layout.itemSize = NSMakeSize(FinderIconItemWidth, FinderIconItemHeight);
    layout.minimumInteritemSpacing = 8;
    layout.minimumLineSpacing = 8;
    layout.sectionInset = NSEdgeInsetsMake(12, 12, 12, 12);

    _collectionView = [[FinderIconCollectionView alloc] initWithFrame:[scroll bounds]];
    _collectionView.collectionViewLayout = layout;
    _collectionView.selectable = YES;
    _collectionView.allowsMultipleSelection = YES;
    _collectionView.backgroundColors = @[ [NSColor controlBackgroundColor] ];
    [_collectionView registerClass:[FinderIconItem class] forItemWithIdentifier:FinderIconItemIdentifier];
    _collectionView.dataSource = self;
    _collectionView.delegate = self;
    [_collectionView registerForDraggedTypes:@[ NSPasteboardTypeFileURL ]];
    NSDragOperation offered = NSDragOperationCopy | NSDragOperationMove | NSDragOperationGeneric;
    [_collectionView setDraggingSourceOperationMask:offered forLocal:YES];
    [_collectionView setDraggingSourceOperationMask:offered forLocal:NO];

    __weak FinderIconViewController *weakSelf = self;
    _collectionView.doubleClicked = ^(NSIndexPath *path) {
        FinderIconViewController *controller = weakSelf;
        if (controller.openItem != nil && (NSUInteger) path.item < [controller.items count])
            controller.openItem(controller.items[path.item]);
    };

    scroll.documentView = _collectionView;
    self.view = scroll;
}

- (NSCollectionView *)collectionView
{
    [self view];
    return _collectionView;
}

- (void)applyStyle:(FinderFolderStyle *)style
{
    [self setItems:_items style:style];
}

- (void)setItems:(NSArray<NSURL *> *)items style:(FinderFolderStyle *)style
{
    NSCollectionView *view = self.collectionView;
    _items = [items copy];
    _style = style;
    if (style == nil) {
        _look = FinderStandardLook();
        if (view.collectionViewLayout != _flowLayout)
            view.collectionViewLayout = _flowLayout;
        view.backgroundView = nil;
        view.backgroundColors = @[ [NSColor controlBackgroundColor] ];
    } else {
        _look = FinderStyledLook(style);
        view.collectionViewLayout = [[FinderFreeformLayout alloc] initWithItemSize:_look.itemSize];
        [self placeItems];
        NSImage *image = FinderBackgroundImage(style.backgroundImageURL);
        if (image != nil) {
            FinderIconBackgroundView *background = [[FinderIconBackgroundView alloc] initWithFrame:[view bounds]];
            background.color = style.backgroundColor;
            background.image = image;
            view.backgroundView = background;
            view.backgroundViewScrollsWithContent = YES;
        } else {
            view.backgroundView = nil;
        }
        view.backgroundColors = @[ style.backgroundColor ?: [NSColor controlBackgroundColor] ];
    }
    [view reloadData];
    view.selectionIndexPaths = [NSSet set];
}

// The freeform layout's centres for the items shown
- (void)placeItems
{
    FinderFreeformLayout *layout = (FinderFreeformLayout *) self.collectionView.collectionViewLayout;
    if (_style == nil || ![layout isKindOfClass:[FinderFreeformLayout class]])
        return;
    NSMutableArray<NSValue *> *centers = [NSMutableArray arrayWithCapacity:[_items count]];
    for (NSURL *item in _items) {
        NSPoint center = NSMakePoint(NAN, NAN);
        if (![_style getCenter:&center forItemNamed:[item lastPathComponent]])
            center = NSMakePoint(NAN, NAN);
        [centers addObject:[NSValue valueWithPoint:center]];
    }
    layout.centers = centers;
}

- (void)setItems:(NSArray<NSURL *> *)items
{
    _items = [items copy];
    [self placeItems];
    [self.collectionView reloadData];
    self.collectionView.selectionIndexPaths = [NSSet set];
}

- (void)selectItems:(NSArray<NSURL *> *)items
{
    NSMutableSet<NSIndexPath *> *paths = [NSMutableSet set];
    NSArray<NSURL *> *standardized = [self standardized:items];
    NSArray<NSURL *> *shown = [self standardized:_items];
    for (NSURL *item in standardized) {
        NSUInteger index = [shown indexOfObject:item];
        if (index != NSNotFound)
            [paths addObject:[NSIndexPath indexPathForItem:index inSection:0]];
    }
    self.collectionView.selectionIndexPaths = paths;
    if ([paths count] > 0)
        [self.collectionView scrollToItemsAtIndexPaths:paths scrollPosition:NSCollectionViewScrollPositionNearestHorizontalEdge];
}

- (NSArray<NSURL *> *)selectedItems
{
    NSMutableArray<NSIndexPath *> *paths = [[self.collectionView.selectionIndexPaths allObjects] mutableCopy];
    [paths sortUsingSelector:@selector(compare:)];
    NSMutableArray<NSURL *> *selected = [NSMutableArray arrayWithCapacity:[paths count]];
    for (NSIndexPath *path in paths) {
        if ((NSUInteger) path.item < [_items count])
            [selected addObject:_items[path.item]];
    }
    return selected;
}

// Paths compared as the file system names them: standardized, without a trailing slash
- (NSArray<NSURL *> *)standardized:(NSArray<NSURL *> *)urls
{
    NSMutableArray<NSURL *> *result = [NSMutableArray arrayWithCapacity:[urls count]];
    for (NSURL *url in urls)
        [result addObject:[NSURL fileURLWithPath:[[url URLByStandardizingPath] path]]];
    return result;
}

#pragma mark Editing a label

- (BOOL)isEditing
{
    return _editingItem != nil;
}

- (void)cancelEditing
{
    [self finishEditingWithName:nil];
}

- (BOOL)beginEditingItem:(NSURL *)url
{
    [self finishEditingWithName:nil];
    NSUInteger index = [[self standardized:_items] indexOfObject:[[self standardized:@[ url ]] firstObject]];
    if (index == NSNotFound)
        return NO;
    NSIndexPath *path = [NSIndexPath indexPathForItem:index inSection:0];
    [self.collectionView scrollToItemsAtIndexPaths:[NSSet setWithObject:path]
                                    scrollPosition:NSCollectionViewScrollPositionNearestHorizontalEdge];
    // Items are made by the next layout pass; one that was just reloaded may not be there yet
    [self.collectionView layoutSubtreeIfNeeded];
    NSTextField *field = [self.collectionView itemAtIndexPath:path].textField;
    if (field == nil || field.window == nil)
        return NO;
    _editingItem = _items[index];
    _editingField = field;
    NSString *name = [_editingItem lastPathComponent];
    field.stringValue = name;
    field.editable = YES;
    field.selectable = YES;
    field.drawsBackground = YES;
    field.backgroundColor = [NSColor textBackgroundColor];
    field.delegate = self;
    [field.window makeFirstResponder:field];

    // The name without its extension; all of a folder's
    BOOL directory = NO;
    [[NSFileManager defaultManager] fileExistsAtPath:[_editingItem path] isDirectory:&directory];
    NSString *extension = [name pathExtension];
    NSUInteger length = directory || [extension length] == 0 ? [name length] : [name length] - [extension length] - 1;
    [[field currentEditor] setSelectedRange:NSMakeRange(0, length)];
    return YES;
}

// Ends the edit: the label shows the item's name again, and renameItem hears what was typed (nil: cancelled)
- (void)finishEditingWithName:(NSString *)name
{
    NSURL *item = _editingItem;
    NSTextField *field = _editingField;
    if (item == nil)
        return;
    _editingItem = nil;
    _editingField = nil;
    field.delegate = nil;
    field.editable = NO;
    field.selectable = NO;
    field.drawsBackground = NO;
    [field abortEditing];
    field.stringValue = [FinderDirectory displayNameForURL:item];
    // The collection view gets the keys back, unless the edit ended because something else took them
    id responder = [field.window firstResponder];
    if (responder == nil || responder == field.window || responder == field
        || ([responder isKindOfClass:[NSText class]] && [(NSText *) responder delegate] == (id) field))
        [field.window makeFirstResponder:self.collectionView];
    if (_renameItem != nil)
        _renameItem(item, name);
}

- (void)controlTextDidEndEditing:(NSNotification *)notification
{
    if (notification.object != _editingField)
        return;
    [self finishEditingWithName:_editingField.stringValue];
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)selector
{
    if (control != _editingField || selector != @selector(cancelOperation:))
        return NO;
    [self finishEditingWithName:nil];
    return YES;
}

#pragma mark NSCollectionViewDataSource

- (NSInteger)collectionView:(NSCollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return [_items count];
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView
     itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath
{
    FinderIconItem *item = (FinderIconItem *) [collectionView makeItemWithIdentifier:FinderIconItemIdentifier forIndexPath:indexPath];
    item.look = _look;
    item.representedObject = _items[indexPath.item];
    return item;
}

#pragma mark Drag and drop

- (BOOL)collectionView:(NSCollectionView *)collectionView
    canDragItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
                   withEvent:(NSEvent *)event
{
    return YES;
}

- (id<NSPasteboardWriting>)collectionView:(NSCollectionView *)collectionView pasteboardWriterForItemAtIndexPath:(NSIndexPath *)indexPath
{
    return (NSUInteger) indexPath.item < [_items count] ? _items[indexPath.item] : nil;
}

// The folder a proposed drop goes into: a folder item's for a drop on it; otherwise the window's, as a drop before
// the index (a drop with no index path is never performed)
- (NSURL *)folderForDropAtIndexPath:(NSIndexPath *)path operation:(NSCollectionViewDropOperation *)operation
{
    if (*operation == NSCollectionViewDropOn && path != nil && (NSUInteger) path.item < [_items count]) {
        NSURL *folder = FinderDropFolderForItem(_items[path.item]);
        if (folder != nil)
            return folder;
    }
    *operation = NSCollectionViewDropBefore;
    return self.folderURL;
}

- (NSDragOperation)collectionView:(NSCollectionView *)collectionView
                     validateDrop:(id<NSDraggingInfo>)draggingInfo
                proposedIndexPath:(NSIndexPath **)proposedDropIndexPath
                    dropOperation:(NSCollectionViewDropOperation *)proposedDropOperation
{
    NSArray<NSURL *> *items = FinderDraggedFileURLs([draggingInfo draggingPasteboard]);
    if (items == nil)
        return NSDragOperationNone;
    NSURL *folder = [self folderForDropAtIndexPath:*proposedDropIndexPath operation:proposedDropOperation];
    return FinderDropOperation(items, folder, [draggingInfo draggingSourceOperationMask]);
}

- (BOOL)collectionView:(NSCollectionView *)collectionView
            acceptDrop:(id<NSDraggingInfo>)draggingInfo
             indexPath:(NSIndexPath *)indexPath
         dropOperation:(NSCollectionViewDropOperation)dropOperation
{
    NSArray<NSURL *> *items = FinderDraggedFileURLs([draggingInfo draggingPasteboard]);
    if (items == nil)
        return NO;
    NSURL *folder = [self folderForDropAtIndexPath:indexPath operation:&dropOperation];
    NSDragOperation operation = FinderDropOperation(items, folder, [draggingInfo draggingSourceOperationMask]);
    if (operation == NSDragOperationNone || self.dropItems == nil)
        return NO;
    self.dropItems(items, folder, FinderTransferKindForDropOperation(operation));
    return YES;
}

@end
