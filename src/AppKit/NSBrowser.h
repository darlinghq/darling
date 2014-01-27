#import <AppKit/NSControl.h>
#import <AppKit/NSDragging.h>
#import <AppKit/NSViewController.h>

#define NSAppKitVersionNumberWithContinuousScrollingBrowser 680.0
#define NSAppKitVersionNumberWithColumnResizingBrowser 685.0

@class NSMatrix, NSScroller, NSMutableArray, NSIndexSet;
@protocol NSBrowserDelegate;

typedef struct __Brflags {
#ifdef __BIG_ENDIAN__
    unsigned int allowsMultipleSelection:1;
    unsigned int allowsBranchSelection:1;
    unsigned int reuseColumns:1;
    unsigned int isTitled:1;
    unsigned int titleFromPrevious:1;
    unsigned int separateColumns:1;
    unsigned int delegateImplementsWillDisplayCell:1;
    unsigned int delegateSetsTitles:1;
    unsigned int delegateSelectsCellsByString:1;
    unsigned int delegateDoesNotCreateRowsInMatrix:1;
    unsigned int delegateValidatesColumns:1;
    unsigned int acceptArrowKeys:1;
    unsigned int dontDrawTitles:1;
    unsigned int sendActionOnArrowKeys:1;
    unsigned int prohibitEmptySel:1;
    unsigned int hasHorizontalScroller:1;
    unsigned int time:1;
    unsigned int allowsIncrementalSearching:1;
    unsigned int delegateSelectsCellsByRow:1;
    unsigned int disableCompositing:6;
    unsigned int refusesFirstResponder:1;
    unsigned int acceptsFirstMouse:1;
    unsigned int actionNeedsToBeSent:1;
    unsigned int usesSmallSizeTitleFont:1;
    unsigned int usesSmallScrollers:1;
    unsigned int prefersAllColumnUserResizing:1;
    unsigned int firstVisibleCalculationDisabled:1;
#else
    unsigned int firstVisibleCalculationDisabled:1;
    unsigned int prefersAllColumnUserResizing:1;
    unsigned int usesSmallScrollers:1;
    unsigned int usesSmallSizeTitleFont:1;
    unsigned int actionNeedsToBeSent:1;
    unsigned int acceptsFirstMouse:1;
    unsigned int refusesFirstResponder:1;
    unsigned int disableCompositing:6;
    unsigned int delegateSelectsCellsByRow:1;
    unsigned int allowsIncrementalSearching:1;
    unsigned int time:1;
    unsigned int hasHorizontalScroller:1;
    unsigned int prohibitEmptySel:1;
    unsigned int sendActionOnArrowKeys:1;
    unsigned int dontDrawTitles:1;
    unsigned int acceptArrowKeys:1;
    unsigned int delegateValidatesColumns:1;
    unsigned int delegateDoesNotCreateRowsInMatrix:1;
    unsigned int delegateSelectsCellsByString:1;
    unsigned int delegateSetsTitles:1;
    unsigned int delegateImplementsWillDisplayCell:1;
    unsigned int separateColumns:1;
    unsigned int titleFromPrevious:1;
    unsigned int isTitled:1;
    unsigned int reuseColumns:1;
    unsigned int allowsBranchSelection:1;
    unsigned int allowsMultipleSelection:1;
#endif
} _Brflags;


enum {


    NSBrowserNoColumnResizing = 0,



    NSBrowserAutoColumnResizing = 1,



    NSBrowserUserColumnResizing = 2,
};


typedef NSUInteger NSBrowserColumnResizingType;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5



enum {
    NSBrowserDropOn,
    NSBrowserDropAbove,
};

#endif

typedef NSUInteger NSBrowserDropOperation;

@interface NSBrowser : NSControl
{

    id _target;
    SEL _action;
    id _delegate;
    SEL _doubleAction;
    Class _matrixClass;
    id _cellPrototype;
    NSSize _columnSize;
    short _numberOfVisibleColumns;
    short _minColumnWidth;
    short _firstVisibleColumn;
    short _maxVisibleColumns;
    NSMutableArray *_titles;
    NSString *_pathSeparator;
    NSMutableArray *_columns;
    id _brAuxiliaryStorage;
    NSString *_firstColumnTitle;
    NSScroller *_scroller;
    _Brflags _brflags;
}

+ (Class)cellClass;

- (void)loadColumnZero;
- (BOOL)isLoaded;

- (void)setDoubleAction:(SEL)aSelector;
- (SEL)doubleAction;
- (void)setMatrixClass:(Class)factoryId;
- (Class)matrixClass;
- (void)setCellClass:(Class)factoryId;
- (void)setCellPrototype:(NSCell *)aCell;
- (id)cellPrototype;
- (void)setDelegate:(id <NSBrowserDelegate>)anObject;
- (id <NSBrowserDelegate>)delegate;
- (void)setReusesColumns:(BOOL)flag;
- (BOOL)reusesColumns;

- (void)setHasHorizontalScroller:(BOOL)flag;
- (BOOL)hasHorizontalScroller;
- (void)setAutohidesScroller:(BOOL)flag;
- (BOOL)autohidesScroller;
- (void)setSeparatesColumns:(BOOL)flag;
- (BOOL)separatesColumns;
- (void)setTitled:(BOOL)flag;
- (BOOL)isTitled;
- (void)setMinColumnWidth:(CGFloat)columnWidth;
- (CGFloat)minColumnWidth;

- (void)setMaxVisibleColumns:(NSInteger)columnCount;
- (NSInteger)maxVisibleColumns;

- (void)setAllowsMultipleSelection:(BOOL)flag;
- (BOOL)allowsMultipleSelection;
- (void)setAllowsBranchSelection:(BOOL)flag;
- (BOOL)allowsBranchSelection;
- (void)setAllowsEmptySelection:(BOOL)flag;
- (BOOL)allowsEmptySelection;
- (void)setTakesTitleFromPreviousColumn:(BOOL)flag;
- (BOOL)takesTitleFromPreviousColumn;

- (void)setSendsActionOnArrowKeys:(BOOL)flag;
- (BOOL)sendsActionOnArrowKeys;



- (id)itemAtIndexPath:(NSIndexPath *)indexPath;



- (id)itemAtRow:(NSInteger)row inColumn:(NSInteger)column;



- (NSIndexPath *)indexPathForColumn:(NSInteger)column;



- (BOOL)isLeafItem:(id)item;



- (void)reloadDataForRowIndexes:(NSIndexSet *)rowIndexes inColumn:(NSInteger)column;



- (id)parentForItemsInColumn:(NSInteger)column;



- (void)scrollRowToVisible:(NSInteger)row inColumn:(NSInteger)column;

- (void)setTitle:(NSString *)aString ofColumn:(NSInteger)column;
- (NSString *)titleOfColumn:(NSInteger)column;
- (void)setPathSeparator:(NSString *)newString;
- (NSString *)pathSeparator;
- (BOOL)setPath:(NSString *)path;
- (NSString *)path;
- (NSString *)pathToColumn:(NSInteger)column;



- (NSInteger)clickedColumn;
- (NSInteger)clickedRow;

- (NSInteger)selectedColumn;



- (id)selectedCell;
- (id)selectedCellInColumn:(NSInteger)column;



- (NSArray *)selectedCells;

- (void)selectRow:(NSInteger)row inColumn:(NSInteger)column;

- (NSInteger)selectedRowInColumn:(NSInteger)column;



- (NSIndexPath *)selectionIndexPath;



- (void)setSelectionIndexPath:(NSIndexPath *)path;



- (NSArray *)selectionIndexPaths;



- (void)setSelectionIndexPaths:(NSArray *)paths;



- (void)selectRowIndexes:(NSIndexSet *)indexes inColumn:(NSInteger)column;



- (NSIndexSet *)selectedRowIndexesInColumn:(NSInteger)column;

- (void)reloadColumn:(NSInteger)column;
- (void)validateVisibleColumns;
- (void)scrollColumnsRightBy:(NSInteger)shiftAmount;
- (void)scrollColumnsLeftBy:(NSInteger)shiftAmount;
- (void)scrollColumnToVisible:(NSInteger)column;



- (NSInteger)lastColumn;



- (void)setLastColumn:(NSInteger)column;



- (void)addColumn;

- (NSInteger)numberOfVisibleColumns;
- (NSInteger)firstVisibleColumn;
- (NSInteger)lastVisibleColumn;



- (NSInteger)columnOfMatrix:(NSMatrix *)matrix;



- (NSMatrix *)matrixInColumn:(NSInteger)column;

- (id)loadedCellAtRow:(NSInteger)row column:(NSInteger)col;
- (void)selectAll:(id)sender;
- (void)tile;
- (void)doClick:(id)sender;
- (void)doDoubleClick:(id)sender;
- (BOOL)sendAction;

- (NSRect)titleFrameOfColumn:(NSInteger)column;
- (void)drawTitleOfColumn:(NSInteger)column inRect:(NSRect)aRect;
- (CGFloat)titleHeight;
- (NSRect)frameOfColumn:(NSInteger)column;
- (NSRect)frameOfInsideOfColumn:(NSInteger)column;



- (NSRect)frameOfRow:(NSInteger)row inColumn:(NSInteger)column;



- (BOOL)getRow:(NSInteger *)row column:(NSInteger *)column forPoint:(NSPoint)point;



- (CGFloat)columnWidthForColumnContentWidth:(CGFloat)columnContentWidth;
- (CGFloat)columnContentWidthForColumnWidth:(CGFloat)columnWidth;



- (void)setColumnResizingType:(NSBrowserColumnResizingType)columnResizingType;
- (NSBrowserColumnResizingType)columnResizingType;



- (void)setPrefersAllColumnUserResizing:(BOOL)prefersAllColumnResizing;
- (BOOL)prefersAllColumnUserResizing;



- (void)setWidth:(CGFloat)columnWidth ofColumn:(NSInteger)columnIndex;
- (CGFloat)widthOfColumn:(NSInteger)column;



- (void)setRowHeight:(CGFloat)height;
- (CGFloat)rowHeight;



- (void)noteHeightOfRowsWithIndexesChanged:(NSIndexSet *)indexSet inColumn:(NSInteger)columnIndex;




- (void)setDefaultColumnWidth:(CGFloat)columnWidth;
- (CGFloat)defaultColumnWidth;



- (void)setColumnsAutosaveName:(NSString *)name;
- (NSString *)columnsAutosaveName;



+ (void)removeSavedColumnsWithAutosaveName:(NSString *)name;







- (BOOL)canDragRowsWithIndexes:(NSIndexSet *)rowIndexes inColumn:(NSInteger)column withEvent:(NSEvent *)event;



- (NSImage *)draggingImageForRowsWithIndexes:(NSIndexSet *)rowIndexes inColumn:(NSInteger)column withEvent:(NSEvent *)event offset:(NSPointPointer)dragImageOffset;



- (void)setDraggingSourceOperationMask:(NSDragOperation)mask forLocal:(BOOL)isLocal;





- (BOOL)allowsTypeSelect;
- (void)setAllowsTypeSelect:(BOOL)value;



- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;



- (void)editItemAtIndexPath:(NSIndexPath *)indexPath withEvent:(NSEvent *)theEvent select:(BOOL)select;

@end






EXTERN_C NSString *NSBrowserColumnConfigurationDidChangeNotification;




@protocol NSBrowserDelegate <NSObject>
@optional







- (NSInteger)browser:(NSBrowser *)sender numberOfRowsInColumn:(NSInteger)column;



- (void)browser:(NSBrowser *)sender createRowsForColumn:(NSInteger)column inMatrix:(NSMatrix *)matrix;







- (NSInteger)browser:(NSBrowser *)browser numberOfChildrenOfItem:(id)item;



- (id)browser:(NSBrowser *)browser child:(NSInteger)index ofItem:(id)item;



- (BOOL)browser:(NSBrowser *)browser isLeafItem:(id)item;



- (id)browser:(NSBrowser *)browser objectValueForItem:(id)item;




- (CGFloat)browser:(NSBrowser *)browser heightOfRow:(NSInteger)row inColumn:(NSInteger)columnIndex;






- (id)rootItemForBrowser:(NSBrowser *)browser;




- (void)browser:(NSBrowser *)browser setObjectValue:(id)object forItem:(id)item;




- (BOOL)browser:(NSBrowser *)browser shouldEditItem:(id)item;






- (void)browser:(NSBrowser *)sender willDisplayCell:(id)cell atRow:(NSInteger)row column:(NSInteger)column;

- (NSString *)browser:(NSBrowser *)sender titleOfColumn:(NSInteger)column;



- (BOOL)browser:(NSBrowser *)sender selectCellWithString:(NSString *)title inColumn:(NSInteger)column;



- (BOOL)browser:(NSBrowser *)sender selectRow:(NSInteger)row inColumn:(NSInteger)column;



- (BOOL)browser:(NSBrowser *)sender isColumnValid:(NSInteger)column;

- (void)browserWillScroll:(NSBrowser *)sender;
- (void)browserDidScroll:(NSBrowser *)sender;
- (CGFloat)browser:(NSBrowser *)browser shouldSizeColumn:(NSInteger)columnIndex forUserResize:(BOOL)forUserResize toWidth:(CGFloat)suggestedWidth;




- (CGFloat)browser:(NSBrowser *)browser sizeToFitWidthOfColumn:(NSInteger)columnIndex;




- (void)browserColumnConfigurationDidChange:(NSNotification *)notification;




- (BOOL)browser:(NSBrowser *)browser shouldShowCellExpansionForRow:(NSInteger)row column:(NSInteger)column;









- (BOOL)browser:(NSBrowser *)browser writeRowsWithIndexes:(NSIndexSet *)rowIndexes inColumn:(NSInteger)column toPasteboard:(NSPasteboard *)pasteboard;



- (NSArray *)browser:(NSBrowser *)browser namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination forDraggedRowsWithIndexes:(NSIndexSet *)rowIndexes inColumn:(NSInteger)column;



- (BOOL)browser:(NSBrowser *)browser canDragRowsWithIndexes:(NSIndexSet *)rowIndexes inColumn:(NSInteger)column withEvent:(NSEvent *)event;



- (NSImage *)browser:(NSBrowser *)browser draggingImageForRowsWithIndexes:(NSIndexSet *)rowIndexes inColumn:(NSInteger)column withEvent:(NSEvent *)event offset:(NSPointPointer)dragImageOffset;









- (NSDragOperation)browser:(NSBrowser *)browser validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger *)row column:(NSInteger *)column dropOperation:(NSBrowserDropOperation *)dropOperation;



- (BOOL)browser:(NSBrowser *)browser acceptDrop:(id <NSDraggingInfo>)info atRow:(NSInteger)row column:(NSInteger)column dropOperation:(NSBrowserDropOperation)dropOperation;








- (NSString *)browser:(NSBrowser *)browser typeSelectStringForRow:(NSInteger)row inColumn:(NSInteger)column;




- (BOOL)browser:(NSBrowser *)browser shouldTypeSelectForEvent:(NSEvent *)event withCurrentSearchString:(NSString *)searchString;




- (NSInteger)browser:(NSBrowser *)browser nextTypeSelectMatchFromRow:(NSInteger)startRow toRow:(NSInteger)endRow inColumn:(NSInteger)column forString:(NSString *)searchString;






- (NSViewController *)browser:(NSBrowser *)browser previewViewControllerForLeafItem:(id)item;




- (NSViewController *)browser:(NSBrowser *)browser headerViewControllerForItem:(id)item;



- (void)browser:(NSBrowser *)browser didChangeLastColumn:(NSInteger)oldLastColumn toColumn:(NSInteger)column;



- (NSIndexSet *)browser:(NSBrowser *)browser selectionIndexesForProposedSelection:(NSIndexSet *)proposedSelectionIndexes inColumn:(NSInteger)column;

@end




@interface NSBrowser(NSDeprecated)



- (void)setAcceptsArrowKeys:(BOOL)flag;
- (BOOL)acceptsArrowKeys;



- (void)displayColumn:(NSInteger)column;



- (void)displayAllColumns;



- (void)scrollViaScroller:(NSScroller *)sender;



- (void)updateScroller;

@end
