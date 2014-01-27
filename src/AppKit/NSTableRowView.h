#import <Foundation/NSObject.h>

#import <AppKit/NSView.h>
#import <AppKit/NSCell.h>
#import <AppKit/NSTableView.h>




@interface NSTableRowView : NSView {
@private
    NSView **_columnViews;
    NSInteger _columnCount;

    NSTableViewSelectionHighlightStyle _selectionHighlightStyle;
    NSTableViewDraggingDestinationFeedbackStyle _draggingDestinationFeedbackStyle;

    CGFloat _dropOperationIndentation;

    NSColor *_backgroundColor;
    CGFloat _selectionAlpha;

    unsigned int _targetForDropOperation:1;
    unsigned int _groupRowStyle:2;
    unsigned int _priorRowSelected:1;
    unsigned int _nextRowSelected:1;
    unsigned int _selected:1;
    unsigned int _emphasized:1;
    unsigned int _hasAssociatedViews:1;
    unsigned int _backgroundStylesDirty:1;
    unsigned int _isFloating:1;
    unsigned int _hasCustomGridColor:1;
    unsigned int _usingCachedImageOnly:1;
    unsigned int _gridStyleMask:4;
    unsigned int _updatingBackgroundStyle:1;
    unsigned int _locationNeedsUpdating:1;
    unsigned int _reserved2:13;
}



@property NSTableViewSelectionHighlightStyle selectionHighlightStyle;



@property(getter=isEmphasized) BOOL emphasized;



@property(getter=isGroupRowStyle) BOOL groupRowStyle;



@property(getter=isSelected) BOOL selected;



@property(getter=isFloating) BOOL floating;



@property(getter=isTargetForDropOperation) BOOL targetForDropOperation;
@property NSTableViewDraggingDestinationFeedbackStyle draggingDestinationFeedbackStyle;
@property CGFloat indentationForDropOperation;



@property(readonly) NSBackgroundStyle interiorBackgroundStyle;



@property(copy) NSColor *backgroundColor;



- (void)drawBackgroundInRect:(NSRect)dirtyRect;



- (void)drawSelectionInRect:(NSRect)dirtyRect;



- (void)drawSeparatorInRect:(NSRect)dirtyRect;



- (void)drawDraggingDestinationFeedbackInRect:(NSRect)dirtyRect;







- (id)viewAtColumn:(NSInteger)column;



@property(readonly) NSInteger numberOfColumns;




@end
