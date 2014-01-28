#import <AppKit/NSView.h>

@class NSTableView;
@class NSColor;
@class NSImage;
@class NSCursor;

@interface NSTableHeaderView : NSView
{

    NSTableView *_tableView;
    NSInteger _resizedColumn;
    NSInteger _draggedColumn;
    NSInteger _pressedColumn;
    NSImage *_headerDragImage;
    CGFloat _draggedDistance;
    unsigned int _isColumnResizing:1;
    unsigned int _showHandCursorFired:1;
    unsigned int _toolTipRectsDirty:1;
    unsigned int _alignTitleWithDataCell:1;
    unsigned int _nextColumnAfterDrawnOneIsSelected:1;
    unsigned int _reserved:27;
    BOOL _skipDrawingSeparator;
    id _viewDataX;
}

- (void)setTableView:(NSTableView *)tableView;
- (NSTableView *)tableView;


- (NSInteger)draggedColumn;
- (CGFloat)draggedDistance;

- (NSInteger)resizedColumn;
- (NSRect)headerRectOfColumn:(NSInteger)column;
- (NSInteger)columnAtPoint:(NSPoint)point;

@end
