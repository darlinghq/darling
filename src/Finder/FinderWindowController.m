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

@implementation FinderWindowController {
    FinderDirectory *_directory;
    FinderIconViewController *_icons;
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
        _icons = [[FinderIconViewController alloc] init];
        __weak FinderWindowController *weakSelf = self;
        _icons.openItem = ^(NSURL *item) {
            FinderWindowController *controller = weakSelf;
            if (controller.openItem != nil)
                controller.openItem(item);
        };
        NSView *content = _icons.view;
        content.frame = [window.contentView bounds];
        content.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        [window.contentView addSubview:content];
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
}

- (void)selectItems:(NSArray<NSURL *> *)items
{
    [_icons selectItems:items];
}

- (NSArray<NSURL *> *)selectedItems
{
    return _icons.selectedItems;
}

@end
