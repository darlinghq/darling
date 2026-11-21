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
#import "FinderDirectory.h"

static NSString *const FinderIconItemIdentifier = @"FinderIconItem";

static const CGFloat FinderIconItemWidth = 100;
static const CGFloat FinderIconItemHeight = 100;
static const CGFloat FinderIconSize = 64;

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
@end

@implementation FinderIconItem

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

- (void)setRepresentedObject:(id)object
{
    [super setRepresentedObject:object];
    [self view]; // the outlets are set when the view loads
    NSURL *url = object;
    NSImage *icon = nil;
    if (url != nil) {
        icon = [[NSWorkspace sharedWorkspace] iconForFile:[url path]];
        [icon setSize:NSMakeSize(FinderIconSize, FinderIconSize)];
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

// Tells the controller about double clicks, which the collection view leaves to its items.
@interface FinderIconCollectionView : NSCollectionView
@property (copy) void (^doubleClicked)(NSIndexPath *path);
@end

@implementation FinderIconCollectionView

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

    NSCollectionViewFlowLayout *layout = [[NSCollectionViewFlowLayout alloc] init];
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

- (void)setItems:(NSArray<NSURL *> *)items
{
    _items = [items copy];
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

#pragma mark NSCollectionViewDataSource

- (NSInteger)collectionView:(NSCollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return [_items count];
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView
     itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath
{
    NSCollectionViewItem *item = [collectionView makeItemWithIdentifier:FinderIconItemIdentifier forIndexPath:indexPath];
    item.representedObject = _items[indexPath.item];
    return item;
}

@end
