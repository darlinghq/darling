#import <AppKit/NSControl.h>
#import <AppKit/NSUserInterfaceValidation.h>

@class NSColor, NSMutableArray;
@protocol NSMatrixDelegate;



enum {
    NSRadioModeMatrix = 0,
    NSHighlightModeMatrix = 1,
    NSListModeMatrix = 2,
    NSTrackModeMatrix = 3
};
typedef NSUInteger NSMatrixMode;

typedef struct __MFlags {
#ifdef __BIG_ENDIAN__
    unsigned int highlightMode:1;
    unsigned int radioMode:1;
    unsigned int listMode:1;
    unsigned int allowEmptySel:1;
    unsigned int autoscroll:1;
    unsigned int selectionByRect:1;
    unsigned int drawsCellBackground:1;
    unsigned int drawsBackground:1;
    unsigned int autosizeCells:1;
    unsigned int drawingAncestor:1;
    unsigned int tabKeyTraversesCells:1;
    unsigned int tabKeyTraversesCellsExplicitlySet:1;
    unsigned int allowsIncrementalSearching:1;
    unsigned int currentlySelectingCell:1;
    unsigned int onlySetKeyCell:1;
    unsigned int changingSelectionWithKeyboard:1;
    unsigned int dontScroll:1;
    unsigned int refusesFirstResponder:1;
    unsigned int useSimpleTrackingMode:1;
    unsigned int checkForSimpleTrackingMode:1;
    unsigned int liveResizeImageCacheingEnabled:1;
    unsigned int hasCachedSubclassIsSafeForLiveResize:1;
    unsigned int subclassIsSafeForLiveResize:1;
    unsigned int tmpAllowNonVisibleCellsToBecomeFirstResponder:1;
    unsigned int needsRedrawBeforeFirstLiveResizeCache:1;
    unsigned int browserOptimizationsEnabled:1;
    unsigned int drawingContextMenuHighlight:1;
    unsigned int drawingContextMenuHighlightOnAllSelectedRows:1;
    unsigned int autorecalculatesCellSize:1;
    unsigned int cellSizeNeedsAutorecalc:1;
    unsigned int reservedMatrix:2;
#else
    unsigned int reservedMatrix:2;
    unsigned int cellSizeNeedsAutorecalc:1;
    unsigned int autorecalculatesCellSize:1;
    unsigned int drawingContextMenuHighlightOnAllSelectedRows:1;
    unsigned int drawingContextMenuHighlight:1;
    unsigned int browserOptimizationsEnabled:1;
    unsigned int needsRedrawBeforeFirstLiveResizeCache:1;
    unsigned int tmpAllowNonVisibleCellsToBecomeFirstResponder:1;
    unsigned int subclassIsSafeForLiveResize:1;
    unsigned int hasCachedSubclassIsSafeForLiveResize:1;
    unsigned int liveResizeImageCacheingEnabled:1;
    unsigned int checkForSimpleTrackingMode:1;
    unsigned int useSimpleTrackingMode:1;
    unsigned int refusesFirstResponder:1;
    unsigned int dontScroll:1;
    unsigned int changingSelectionWithKeyboard:1;
    unsigned int onlySetKeyCell:1;
    unsigned int currentlySelectingCell:1;
    unsigned int allowsIncrementalSearching:1;
    unsigned int tabKeyTraversesCellsExplicitlySet:1;
    unsigned int tabKeyTraversesCells:1;
    unsigned int drawingAncestor:1;
    unsigned int autosizeCells:1;
    unsigned int drawsBackground:1;
    unsigned int drawsCellBackground:1;
    unsigned int selectionByRect:1;
    unsigned int autoscroll:1;
    unsigned int allowEmptySel:1;
    unsigned int listMode:1;
    unsigned int radioMode:1;
    unsigned int highlightMode:1;
#endif
} _MFlags;

@interface NSMatrix : NSControl <NSUserInterfaceValidations>
{

    id _target;
    SEL _action;
    SEL _doubleAction;
    SEL _errorAction;
    id _delegate;
    id _selectedCell;
    NSInteger _selectedRow;
    NSInteger _selectedCol;
    NSInteger _numRows;
    NSInteger _numCols;
    NSSize _cellSize;
    NSSize _intercell;
    id _font;
    id _protoCell;
    id _cellClass;
    NSColor *_backgroundColor;
    id _private;
    NSMutableArray *_cells;
    _MFlags _mFlags;
}


- (id)initWithFrame:(NSRect)frameRect;
- (id)initWithFrame:(NSRect)frameRect mode:(NSMatrixMode)aMode prototype:(NSCell *)aCell numberOfRows:(NSInteger)rowsHigh numberOfColumns:(NSInteger)colsWide;
- (id)initWithFrame:(NSRect)frameRect mode:(NSMatrixMode)aMode cellClass:(Class)factoryId numberOfRows:(NSInteger)rowsHigh numberOfColumns:(NSInteger)colsWide;


- (void)setCellClass:(Class)factoryId;
- (Class)cellClass;
- (id)prototype;
- (void)setPrototype:(NSCell *)aCell;
- (NSCell *)makeCellAtRow:(NSInteger)row column:(NSInteger)col;
- (NSMatrixMode)mode;
- (void)setMode:(NSMatrixMode)aMode;
- (void)setAllowsEmptySelection:(BOOL)flag;
- (BOOL)allowsEmptySelection;
- (void)sendAction:(SEL)aSelector to:(id)anObject forAllCells:(BOOL)flag;
- (NSArray *)cells;
- (void)sortUsingSelector:(SEL)comparator;
- (void)sortUsingFunction:(NSInteger (*)(id, id, void *))compare context:(void *)context;
- (id)selectedCell;
- (NSArray *)selectedCells;
- (NSInteger)selectedRow;
- (NSInteger)selectedColumn;
- (void)setSelectionByRect:(BOOL)flag;
- (BOOL)isSelectionByRect;
- (void)setSelectionFrom:(NSInteger)startPos to:(NSInteger)endPos anchor:(NSInteger)anchorPos highlight:(BOOL)lit;
- (void)deselectSelectedCell;
- (void)deselectAllCells;
- (void)selectCellAtRow:(NSInteger)row column:(NSInteger)col;
- (void)selectAll:(id)sender;
- (BOOL)selectCellWithTag:(NSInteger)anInt;
- (NSSize)cellSize;
- (void)setCellSize:(NSSize)aSize;
- (NSSize)intercellSpacing;
- (void)setIntercellSpacing:(NSSize)aSize;
- (void)setScrollable:(BOOL)flag;
- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;
- (void)setCellBackgroundColor:(NSColor *)color;
- (NSColor *)cellBackgroundColor;
- (void)setDrawsCellBackground:(BOOL)flag;
- (BOOL)drawsCellBackground;
- (void)setDrawsBackground:(BOOL)flag;
- (BOOL)drawsBackground;
- (void)setState:(NSInteger)value atRow:(NSInteger)row column:(NSInteger)col;
- (void)getNumberOfRows:(NSInteger *)rowCount columns:(NSInteger *)colCount;
- (NSInteger)numberOfRows;
- (NSInteger)numberOfColumns;
- (id)cellAtRow:(NSInteger)row column:(NSInteger)col;
- (NSRect)cellFrameAtRow:(NSInteger)row column:(NSInteger)col;
- (BOOL)getRow:(NSInteger *)row column:(NSInteger *)col ofCell:(NSCell *)aCell;
- (BOOL)getRow:(NSInteger *)row column:(NSInteger *)col forPoint:(NSPoint)aPoint;
- (void)renewRows:(NSInteger)newRows columns:(NSInteger)newCols;
- (void)putCell:(NSCell *)newCell atRow:(NSInteger)row column:(NSInteger)col;
- (void)addRow;
- (void)addRowWithCells:(NSArray *)newCells;
- (void)insertRow:(NSInteger)row;
- (void)insertRow:(NSInteger)row withCells:(NSArray *)newCells;
- (void)removeRow:(NSInteger)row;
- (void)addColumn;
- (void)addColumnWithCells:(NSArray *)newCells;
- (void)insertColumn:(NSInteger)column;
- (void)insertColumn:(NSInteger)column withCells:(NSArray *)newCells;
- (void)removeColumn:(NSInteger)col;
- (id)cellWithTag:(NSInteger)anInt;
- (SEL)doubleAction;
- (void)setDoubleAction:(SEL)aSelector;
- (void)setAutosizesCells:(BOOL)flag;
- (BOOL)autosizesCells;
- (void)sizeToCells;
- (void)setValidateSize:(BOOL)flag;
- (void)drawCellAtRow:(NSInteger)row column:(NSInteger)col;
- (void)highlightCell:(BOOL)flag atRow:(NSInteger)row column:(NSInteger)col;

- (void)setAutoscroll:(BOOL)flag;
- (BOOL)isAutoscroll;
- (void)scrollCellToVisibleAtRow:(NSInteger)row column:(NSInteger)col;
- (NSInteger)mouseDownFlags;
- (void)mouseDown:(NSEvent *)theEvent;
- (BOOL)performKeyEquivalent:(NSEvent *)theEvent;
- (BOOL)sendAction;
- (void)sendDoubleAction;
- (id <NSMatrixDelegate>)delegate;
- (void)setDelegate:(id <NSMatrixDelegate>)anObject;
- (BOOL)textShouldBeginEditing:(NSText *)textObject;
- (BOOL)textShouldEndEditing:(NSText *)textObject;
- (void)textDidBeginEditing:(NSNotification *)notification;
- (void)textDidEndEditing:(NSNotification *)notification;
- (void)textDidChange:(NSNotification *)notification;
- (void)selectText:(id)sender;
- (id)selectTextAtRow:(NSInteger)row column:(NSInteger)col;
- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent;
- (void)resetCursorRects;
- (void)setToolTip:(NSString *)toolTipString forCell:(NSCell *)cell;
- (NSString *)toolTipForCell:(NSCell *)cell;





- (void)setAutorecalculatesCellSize:(BOOL)flag;
- (BOOL)autorecalculatesCellSize;

@end

@interface NSMatrix(NSKeyboardUI)
- (void)setTabKeyTraversesCells:(BOOL)flag;
- (BOOL)tabKeyTraversesCells;
- (void)setKeyCell:(NSCell *)keyCell;
- (id)keyCell;
@end

@protocol NSMatrixDelegate <NSControlTextEditingDelegate> @end
