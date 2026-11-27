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

#import <AppKit/AppKit.h>
#import <QuickLookUI/QLPreviewItem.h>

@class QLPreviewPanel;

// Supplies the items shown by a Quick Look preview panel.
@protocol QLPreviewPanelDataSource <NSObject>

@required
- (NSInteger) numberOfPreviewItemsInPreviewPanel: (QLPreviewPanel *) panel;
- (id<QLPreviewItem>) previewPanel: (QLPreviewPanel *) panel
                previewItemAtIndex: (NSInteger) index;

@end

// Controls preview panel ownership, key events and the animation origin.
@protocol QLPreviewPanelDelegate <NSObject>

@optional
- (BOOL) acceptsPreviewPanelControl: (QLPreviewPanel *) panel;
- (void) beginPreviewPanelControl: (QLPreviewPanel *) panel;
- (void) endPreviewPanelControl: (QLPreviewPanel *) panel;
- (BOOL) previewPanel: (QLPreviewPanel *) panel handleEvent: (NSEvent *) event;
- (NSRect) previewPanel: (QLPreviewPanel *) panel
sourceFrameOnScreenForPreviewItem: (id<QLPreviewItem>) item;

@end

@interface QLPreviewPanel : NSPanel {
    // Declared here rather than in a class extension: the 32-bit runtime does
    // not allow instance variables in extensions.
    @private
    id<QLPreviewPanelDataSource> _dataSource;
    NSInteger _currentPreviewItemIndex;
    NSArray<id<QLPreviewItem>> *_previewItems;
}

// There is one shared panel per process. It exists as soon as it is asked for
// with +sharedPreviewPanel; +sharedPreviewPanelExists reports that without
// creating it.
+ (QLPreviewPanel *) sharedPreviewPanel;
+ (BOOL) sharedPreviewPanelExists;

@property(nonatomic, assign) id<QLPreviewPanelDataSource> dataSource;
@property(nonatomic, assign) id<QLPreviewPanelDelegate> delegate;

@property(nonatomic) NSInteger currentPreviewItemIndex;
@property(nonatomic, readonly) id<QLPreviewItem> currentPreviewItem;

- (void) reloadData;
- (void) updateController;

@end
