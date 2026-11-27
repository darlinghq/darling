/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel
 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <QuickLookUI/QLPreviewPanel.h>

@interface QLPreviewPanel () {
    id<QLPreviewPanelDataSource> _dataSource;
    id<QLPreviewPanelDelegate> _delegate;
    NSInteger _currentPreviewItemIndex;
    NSArray<id<QLPreviewItem>> *_previewItems;
}
@end

static QLPreviewPanel *sharedPreviewPanel = nil;

@implementation QLPreviewPanel

+ (QLPreviewPanel *) sharedPreviewPanel {
    if (sharedPreviewPanel == nil) {
        sharedPreviewPanel = [[self alloc]
                initWithContentRect: NSMakeRect(0, 0, 640, 480)
                          styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                            backing: NSBackingStoreBuffered
                              defer: YES];
    }
    return sharedPreviewPanel;
}

+ (BOOL) sharedPreviewPanelExists {
    return sharedPreviewPanel != nil;
}

- (id<QLPreviewPanelDataSource>) dataSource {
    return _dataSource;
}

- (void) setDataSource: (id<QLPreviewPanelDataSource>) dataSource {
    _dataSource = dataSource;
    [self reloadData];
}

- (id<QLPreviewPanelDelegate>) delegate {
    return _delegate;
}

- (void) setDelegate: (id<QLPreviewPanelDelegate>) delegate {
    _delegate = delegate;
}

- (NSInteger) currentPreviewItemIndex {
    return _currentPreviewItemIndex;
}

- (void) setCurrentPreviewItemIndex: (NSInteger) index {
    NSArray *items = [self previewItems];
    if (index < 0 || index >= (NSInteger)[items count])
        return;
    _currentPreviewItemIndex = index;
}

- (id<QLPreviewItem>) currentPreviewItem {
    NSArray *items = [self previewItems];
    if (_currentPreviewItemIndex < 0 || _currentPreviewItemIndex >= (NSInteger)[items count])
        return nil;
    return [items objectAtIndex: _currentPreviewItemIndex];
}

- (NSArray<id<QLPreviewItem>> *) previewItems {
    return _previewItems ?: @[];
}

- (void) reloadData {
    NSInteger count = [_dataSource numberOfPreviewItemsInPreviewPanel: self];
    if (count < 0)
        count = 0;

    NSMutableArray *items = [NSMutableArray arrayWithCapacity: count];
    for (NSInteger i = 0; i < count; i++) {
        id<QLPreviewItem> item = [_dataSource previewPanel: self previewItemAtIndex: i];
        [items addObject: (item ?: (id)[NSNull null])];
    }
    _previewItems = [items copy];

    if (_currentPreviewItemIndex >= count)
        _currentPreviewItemIndex = count ? count - 1 : 0;
}

// Darling has no separate preview controller to keep in sync; the panel owns
// its item list directly.
- (void) updateController {
}

@end
