#import <AppKit/NSControl.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSDragging.h>
#import <AppKit/NSUserInterfaceValidation.h>
#import <AppKit/NSTextView.h>

@class NSTableHeaderView, NSTableColumn, NSMutableArray, NSDictionary, NSIndexSet, NSMutableIndexSet, NSTableRowView, NSNib;

@protocol NSTableViewDelegate, NSTableViewDataSource;

typedef struct __TvFlags {
#ifdef __BIG_ENDIAN__
    unsigned int allowsColumnReordering:1;
    unsigned int allowsColumnResizing:1;
    unsigned int _available:1;
    unsigned int allowsEmptySelection:1;
    unsigned int allowsMultipleSelection:1;
    unsigned int allowsColumnSelection:1;
    unsigned int selectionType:2;
    unsigned int allowGapRow:1;
    unsigned int compareWidthWithSuperview:2;
    unsigned int delegateWillDisplayCell:1;
    unsigned int delegateShouldEditTableColumn:1;
    unsigned int delegateShouldSelectRow:1;
    unsigned int delegateShouldSelectTableColumn:1;
    unsigned int delegateSelectionShouldChangeInTableView:1;
    unsigned int oldAutoresizesAllColumnsToFit:1;
    unsigned int dataSourceSetObjectValue:1;
    unsigned int selectionPostingDisableCount:7;
    unsigned int movedPostingDisableCount:6;
    unsigned int refusesFirstResponder:1;
#else
    unsigned int refusesFirstResponder:1;
    unsigned int movedPostingDisableCount:6;
    unsigned int selectionPostingDisableCount:7;
    unsigned int dataSourceSetObjectValue:1;
    unsigned int oldAutoresizesAllColumnsToFit:1;
    unsigned int delegateSelectionShouldChangeInTableView:1;
    unsigned int delegateShouldSelectTableColumn:1;
    unsigned int delegateShouldSelectRow:1;
    unsigned int delegateShouldEditTableColumn:1;
    unsigned int delegateWillDisplayCell:1;
    unsigned int compareWidthWithSuperview:2;
    unsigned int allowGapRow:1;
    unsigned int selectionType:2;
    unsigned int allowsColumnSelection:1;
    unsigned int allowsMultipleSelection:1;
    unsigned int allowsEmptySelection:1;
    unsigned int _available:1;
    unsigned int allowsColumnResizing:1;
    unsigned int allowsColumnReordering:1;
#endif
} _TvFlags;



enum { NSTableViewDropOn, NSTableViewDropAbove };
typedef NSUInteger NSTableViewDropOperation;


@interface NSTableView : NSControl <NSUserInterfaceValidations, NSTextViewDelegate, NSDraggingSource>
{

    NSTableHeaderView *_headerView;
    NSView *_cornerView;
    NSMutableArray *_tableColumns;
    NSCell *_editingCell;
    id _delegate;
    id _dataSource;
    NSSize _intercellSpacing;
    CGFloat _rowHeight;
    NSInteger _lastSelectedColumn;
    NSInteger _lastSelectedRow;
    NSInteger _editingRow;
    NSInteger _editingColumn;
    NSMutableIndexSet *_selectedColumns;
    NSMutableIndexSet *_selectedRows;
    id _columnDragInfoX;
    NSColor *_backgroundColor;
    NSColor *_gridColor;
    id _rowDataX;
    id _target;
    SEL _action;
    SEL _doubleAction;
    NSRect _rectOfLastColumn;
    NSInteger _lastCachedRectColumn;
    NSRect _rectOfLastRow;
    NSInteger _lastCachedRectRow;
    _TvFlags _tvFlags;
    id _reserved;
}



- (void)setDataSource:(id <NSTableViewDataSource>)aSource;
- (id <NSTableViewDataSource>)dataSource;



- (void)setDelegate:(id <NSTableViewDelegate>)delegate;
- (id <NSTableViewDelegate>)delegate;



- (void)setHeaderView:(NSTableHeaderView *)headerView;
- (NSTableHeaderView *)headerView;



- (void)setCornerView:(NSView *)cornerView;
- (NSView *)cornerView;



- (void)setAllowsColumnReordering:(BOOL)flag;
- (BOOL)allowsColumnReordering;



- (void)setAllowsColumnResizing:(BOOL)flag;
- (BOOL)allowsColumnResizing;

enum {


    NSTableViewNoColumnAutoresizing = 0,



    NSTableViewUniformColumnAutoresizingStyle,




    NSTableViewSequentialColumnAutoresizingStyle,
    NSTableViewReverseSequentialColumnAutoresizingStyle,





    NSTableViewLastColumnOnlyAutoresizingStyle,
    NSTableViewFirstColumnOnlyAutoresizingStyle
};
typedef NSUInteger NSTableViewColumnAutoresizingStyle;




- (void)setColumnAutoresizingStyle:(NSTableViewColumnAutoresizingStyle)style;
- (NSTableViewColumnAutoresizingStyle)columnAutoresizingStyle;



enum {
    NSTableViewGridNone = 0,
    NSTableViewSolidVerticalGridLineMask = 1 << 0,
    NSTableViewSolidHorizontalGridLineMask = 1 << 1,
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    NSTableViewDashedHorizontalGridLineMask = 1 << 3,
#endif
};

typedef NSUInteger NSTableViewGridLineStyle;



- (void)setGridStyleMask:(NSTableViewGridLineStyle)gridStyle;
- (NSTableViewGridLineStyle)gridStyleMask;



- (void)setIntercellSpacing:(NSSize)aSize;
- (NSSize)intercellSpacing;



- (void)setUsesAlternatingRowBackgroundColors:(BOOL)useAlternatingRowColors;
- (BOOL)usesAlternatingRowBackgroundColors;



- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;



- (void)setGridColor:(NSColor *)color;
- (NSColor *)gridColor;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
enum {

    NSTableViewRowSizeStyleDefault = -1,


    NSTableViewRowSizeStyleCustom = 0,




    NSTableViewRowSizeStyleSmall = 1,
    NSTableViewRowSizeStyleMedium = 2,
    NSTableViewRowSizeStyleLarge = 3,
};
#endif

typedef NSInteger NSTableViewRowSizeStyle;



- (void)setRowSizeStyle:(NSTableViewRowSizeStyle)rowSizeStyle;
- (NSTableViewRowSizeStyle)rowSizeStyle;



- (NSTableViewRowSizeStyle)effectiveRowSizeStyle;




- (void)setRowHeight:(CGFloat)rowHeight;
- (CGFloat)rowHeight;



- (void)noteHeightOfRowsWithIndexesChanged:(NSIndexSet *)indexSet;



- (NSArray *)tableColumns;



- (NSInteger)numberOfColumns;



- (NSInteger)numberOfRows;



- (void)addTableColumn:(NSTableColumn *)tableColumn;



- (void)removeTableColumn:(NSTableColumn *)tableColumn;



- (void)moveColumn:(NSInteger)oldIndex toColumn:(NSInteger)newIndex;



- (NSInteger)columnWithIdentifier:(NSString *)identifier;



- (NSTableColumn *)tableColumnWithIdentifier:(NSString *)identifier;



- (void)tile;
- (void)sizeToFit;
- (void)sizeLastColumnToFit;

- (void)scrollRowToVisible:(NSInteger)row;
- (void)scrollColumnToVisible:(NSInteger)column;




- (void)reloadData;





- (void)noteNumberOfRowsChanged;



- (void)reloadDataForRowIndexes:(NSIndexSet *)rowIndexes columnIndexes:(NSIndexSet *)columnIndexes;





- (NSInteger)editedColumn;
- (NSInteger)editedRow;

- (NSInteger)clickedColumn;
- (NSInteger)clickedRow;

- (void)setDoubleAction:(SEL)aSelector;
- (SEL)doubleAction;




- (void)setSortDescriptors:(NSArray *)array;
- (NSArray *)sortDescriptors;



- (void)setIndicatorImage:(NSImage *)anImage inTableColumn:(NSTableColumn *)tableColumn;
- (NSImage *)indicatorImageInTableColumn:(NSTableColumn *)tableColumn;



- (void)setHighlightedTableColumn:(NSTableColumn *)tableColumn;
- (NSTableColumn *)highlightedTableColumn;






- (void)setVerticalMotionCanBeginDrag:(BOOL)flag;
- (BOOL)verticalMotionCanBeginDrag;





- (BOOL)canDragRowsWithIndexes:(NSIndexSet *)rowIndexes atPoint:(NSPoint)mouseDownPoint;





- (NSImage *)dragImageForRowsWithIndexes:(NSIndexSet *)dragRows tableColumns:(NSArray *)tableColumns event:(NSEvent*)dragEvent offset:(NSPointPointer)dragImageOffset;



- (void)setDraggingSourceOperationMask:(NSDragOperation)mask forLocal:(BOOL)isLocal;



- (void)setDropRow:(NSInteger)row dropOperation:(NSTableViewDropOperation)dropOperation;






- (void)setAllowsMultipleSelection:(BOOL)flag;
- (BOOL)allowsMultipleSelection;



- (void)setAllowsEmptySelection:(BOOL)flag;
- (BOOL)allowsEmptySelection;



- (void)setAllowsColumnSelection:(BOOL)flag;
- (BOOL)allowsColumnSelection;

- (void)selectAll:(id)sender;

- (void)deselectAll:(id)sender;





- (void)selectColumnIndexes:(NSIndexSet *)indexes byExtendingSelection:(BOOL)extend;






- (void)selectRowIndexes:(NSIndexSet *)indexes byExtendingSelection:(BOOL)extend;

- (NSIndexSet *)selectedColumnIndexes;
- (NSIndexSet *)selectedRowIndexes;

- (void)deselectColumn:(NSInteger)column;
- (void)deselectRow:(NSInteger)row;
- (NSInteger)selectedColumn;
- (NSInteger)selectedRow;
- (BOOL)isColumnSelected:(NSInteger)column;
- (BOOL)isRowSelected:(NSInteger)row;
- (NSInteger)numberOfSelectedColumns;
- (NSInteger)numberOfSelectedRows;







- (BOOL)allowsTypeSelect;
- (void)setAllowsTypeSelect:(BOOL)value;

enum {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6



    NSTableViewSelectionHighlightStyleNone = -1,

#endif



    NSTableViewSelectionHighlightStyleRegular = 0,



    NSTableViewSelectionHighlightStyleSourceList = 1,
};
typedef NSInteger NSTableViewSelectionHighlightStyle;



- (NSTableViewSelectionHighlightStyle)selectionHighlightStyle;
- (void)setSelectionHighlightStyle:(NSTableViewSelectionHighlightStyle)selectionHighlightStyle;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6

enum {


    NSTableViewDraggingDestinationFeedbackStyleNone = -1,



    NSTableViewDraggingDestinationFeedbackStyleRegular = 0,



    NSTableViewDraggingDestinationFeedbackStyleSourceList = 1,
};

#endif

typedef NSInteger NSTableViewDraggingDestinationFeedbackStyle;



- (void)setDraggingDestinationFeedbackStyle:(NSTableViewDraggingDestinationFeedbackStyle)style;
- (NSTableViewDraggingDestinationFeedbackStyle)draggingDestinationFeedbackStyle;



- (NSRect)rectOfColumn:(NSInteger)column;



- (NSRect)rectOfRow:(NSInteger)row;



- (NSIndexSet *)columnIndexesInRect:(NSRect)rect;



- (NSRange)rowsInRect:(NSRect)rect;



- (NSInteger)columnAtPoint:(NSPoint)point;

- (NSInteger)rowAtPoint:(NSPoint)point;



- (NSRect)frameOfCellAtColumn:(NSInteger)column row:(NSInteger)row;



- (NSCell *)preparedCellAtColumn:(NSInteger)column row:(NSInteger)row;




- (BOOL)textShouldBeginEditing:(NSText *)textObject;
- (BOOL)textShouldEndEditing:(NSText *)textObject;
- (void)textDidBeginEditing:(NSNotification *)notification;
- (void)textDidEndEditing:(NSNotification *)notification;
- (void)textDidChange:(NSNotification *)notification;




- (void)setAutosaveName:(NSString *)name;
- (NSString *)autosaveName;



- (void)setAutosaveTableColumns:(BOOL)save;
- (BOOL)autosaveTableColumns;






- (BOOL)shouldFocusCell:(NSCell *)cell atColumn:(NSInteger)column row:(NSInteger)row;



- (NSInteger)focusedColumn;



- (void)setFocusedColumn:(NSInteger)focusedColumn;



- (void)performClickOnCellAtColumn:(NSInteger)column row:(NSInteger)row;
- (void)editColumn:(NSInteger)column row:(NSInteger)row withEvent:(NSEvent *)theEvent select:(BOOL)select;




- (void)drawRow:(NSInteger)row clipRect:(NSRect)clipRect;




- (void)highlightSelectionInClipRect:(NSRect)clipRect;



- (void)drawGridInClipRect:(NSRect)clipRect;



- (void)drawBackgroundInClipRect:(NSRect)clipRect;









- (id)viewAtColumn:(NSInteger)column row:(NSInteger)row makeIfNecessary:(BOOL)makeIfNecessary;
- (id)rowViewAtRow:(NSInteger)row makeIfNecessary:(BOOL)makeIfNecessary;



- (NSInteger)rowForView:(NSView *)view;
- (NSInteger)columnForView:(NSView *)view;



- (id)makeViewWithIdentifier:(NSString *)identifier owner:(id)owner;

#if NS_BLOCKS_AVAILABLE


- (void)enumerateAvailableRowViewsUsingBlock:(void (^)(NSTableRowView *rowView, NSInteger row))handler;
#endif



- (BOOL)floatsGroupRows;
- (void)setFloatsGroupRows:(BOOL)value;




#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7





enum {
    NSTableViewAnimationEffectNone = 0x0,
};



enum {


    NSTableViewAnimationEffectFade = 0x1,



    NSTableViewAnimationEffectGap = 0x2,
};



enum {
    NSTableViewAnimationSlideUp = 0x10,
    NSTableViewAnimationSlideDown = 0x20,
    NSTableViewAnimationSlideLeft = 0x30,
    NSTableViewAnimationSlideRight = 0x40,
};

#endif

typedef NSUInteger NSTableViewAnimationOptions;





- (void)beginUpdates;
- (void)endUpdates;




- (void)insertRowsAtIndexes:(NSIndexSet *)indexes withAnimation:(NSTableViewAnimationOptions)animationOptions;



- (void)removeRowsAtIndexes:(NSIndexSet *)indexes withAnimation:(NSTableViewAnimationOptions)animationOptions;



- (void)moveRowAtIndex:(NSInteger)oldIndex toIndex:(NSInteger)newIndex;



- (void)registerNib:(NSNib *)nib forIdentifier:(NSString *)identifier;



- (NSDictionary *)registeredNibsByIdentifier;

@end



@protocol NSTableViewDelegate <NSControlTextEditingDelegate>
@optional



- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;



- (NSTableRowView *)tableView:(NSTableView *)tableView rowViewForRow:(NSInteger)row;



- (void)tableView:(NSTableView *)tableView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row;



- (void)tableView:(NSTableView *)tableView didRemoveRowView:(NSTableRowView *)rowView forRow:(NSInteger)row;






- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;
- (BOOL)tableView:(NSTableView *)tableView shouldEditTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;




- (NSString *)tableView:(NSTableView *)tableView toolTipForCell:(NSCell *)cell rect:(NSRectPointer)rect tableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row mouseLocation:(NSPoint)mouseLocation;





- (BOOL)tableView:(NSTableView *)tableView shouldShowCellExpansionForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;




- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;






- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;




- (BOOL)selectionShouldChangeInTableView:(NSTableView *)tableView;



- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row;



- (NSIndexSet *)tableView:(NSTableView *)tableView selectionIndexesForProposedSelection:(NSIndexSet *)proposedSelectionIndexes;

- (BOOL)tableView:(NSTableView *)tableView shouldSelectTableColumn:(NSTableColumn *)tableColumn;

- (void)tableView:(NSTableView *)tableView mouseDownInHeaderOfTableColumn:(NSTableColumn *)tableColumn;
- (void)tableView:(NSTableView *)tableView didClickTableColumn:(NSTableColumn *)tableColumn;
- (void)tableView:(NSTableView *)tableView didDragTableColumn:(NSTableColumn *)tableColumn;




- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row;




- (NSString *)tableView:(NSTableView *)tableView typeSelectStringForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;




- (NSInteger)tableView:(NSTableView *)tableView nextTypeSelectMatchFromRow:(NSInteger)startRow toRow:(NSInteger)endRow forString:(NSString *)searchString;




- (BOOL)tableView:(NSTableView *)tableView shouldTypeSelectForEvent:(NSEvent *)event withCurrentSearchString:(NSString *)searchString;




- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row;




- (CGFloat)tableView:(NSTableView *)tableView sizeToFitWidthOfColumn:(NSInteger)column;




- (BOOL)tableView:(NSTableView *)tableView shouldReorderColumn:(NSInteger)columnIndex toColumn:(NSInteger)newColumnIndex;




- (void)tableViewSelectionDidChange:(NSNotification *)notification;
- (void)tableViewColumnDidMove:(NSNotification *)notification;
- (void)tableViewColumnDidResize:(NSNotification *)notification;
- (void)tableViewSelectionIsChanging:(NSNotification *)notification;

@end

EXTERN_C NSString *NSTableViewSelectionDidChangeNotification;
EXTERN_C NSString *NSTableViewColumnDidMoveNotification;
EXTERN_C NSString *NSTableViewColumnDidResizeNotification;
EXTERN_C NSString *NSTableViewSelectionIsChangingNotification;


EXTERN_C NSString *const NSTableViewRowViewKey;



@protocol NSTableViewDataSource <NSObject>
@optional




- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView;



- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;






- (void)tableView:(NSTableView *)tableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row;




- (void)tableView:(NSTableView *)tableView sortDescriptorsDidChange:(NSArray *)oldDescriptors;



- (id <NSPasteboardWriting>)tableView:(NSTableView *)tableView pasteboardWriterForRow:(NSInteger)row;



- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes;



- (void)tableView:(NSTableView *)tableView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation;



- (void)tableView:(NSTableView *)tableView updateDraggingItemsForDrag:(id <NSDraggingInfo>)draggingInfo;



- (BOOL)tableView:(NSTableView *)tableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;



- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)dropOperation;



- (BOOL)tableView:(NSTableView *)tableView acceptDrop:(id <NSDraggingInfo>)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)dropOperation;



- (NSArray *)tableView:(NSTableView *)tableView namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination forDraggedRowsWithIndexes:(NSIndexSet *)indexSet;

@end

@interface NSObject(NSTableViewDataSourceDeprecated)







- (BOOL)tableView:(NSTableView *)tableView writeRows:(NSArray *)rows toPasteboard:(NSPasteboard *)pboard;

@end




@interface NSTableView(NSDeprecated)



- (void)setDrawsGrid:(BOOL)flag;



- (BOOL)drawsGrid;



- (void)selectColumn:(NSInteger)column byExtendingSelection:(BOOL)extend;



- (void)selectRow:(NSInteger)row byExtendingSelection:(BOOL)extend;



- (NSEnumerator *)selectedColumnEnumerator;



- (NSEnumerator *)selectedRowEnumerator;



- (NSImage *)dragImageForRows:(NSArray*)dragRows event:(NSEvent *)dragEvent dragImageOffset:(NSPointPointer)dragImageOffset;



- (void)setAutoresizesAllColumnsToFit:(BOOL)flag;
- (BOOL)autoresizesAllColumnsToFit;



- (NSRange)columnsInRect:(NSRect)rect;

@end
