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

static NSToolbarItemIdentifier const FinderViewStyleItem = @"FinderViewStyle";

@implementation FinderWindowController {
    FinderDirectory *_directory;
    FinderIconViewController *_icons;
    FinderListViewController *_list;
    NSSegmentedControl *_viewControl;
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
        for (NSView *content in @[ _icons.view, _list.view ]) {
            content.frame = [window.contentView bounds];
            content.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
            [window.contentView addSubview:content];
        }
        _list.view.hidden = YES;

        NSToolbar *toolbar = [[NSToolbar alloc] initWithIdentifier:@"FinderWindow"];
        toolbar.delegate = self;
        toolbar.displayMode = NSToolbarDisplayModeIconOnly;
        toolbar.allowsUserCustomization = NO;
        window.toolbar = toolbar;

        [self showFolder:folder];
    }
    return self;
}

- (void)showFolder:(NSURL *)folder
{
    _folderURL = [NSURL fileURLWithPath:[[folder URLByStandardizingPath] path] isDirectory:YES];
    _directory = [[FinderDirectory alloc] initWithURL:_folderURL];
    [_directory reload:NULL];
    self.window.title = [FinderDirectory displayNameForURL:_folderURL];
    self.window.representedURL = _folderURL;
    _icons.items = _directory.items;
    _list.items = _directory.items;
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

- (BOOL)validateMenuItem:(NSMenuItem *)item
{
    if (item.action == @selector(showAsIcons:))
        item.state = _viewStyle == FinderViewStyleIcons ? NSControlStateValueOn : NSControlStateValueOff;
    else if (item.action == @selector(showAsList:))
        item.state = _viewStyle == FinderViewStyleList ? NSControlStateValueOn : NSControlStateValueOff;
    return YES;
}

#pragma mark NSToolbarDelegate

- (NSArray<NSToolbarItemIdentifier> *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    return @[ NSToolbarFlexibleSpaceItemIdentifier, FinderViewStyleItem ];
}

- (NSArray<NSToolbarItemIdentifier> *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    return [self toolbarDefaultItemIdentifiers:toolbar];
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar
        itemForItemIdentifier:(NSToolbarItemIdentifier)identifier
    willBeInsertedIntoToolbar:(BOOL)flag
{
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
