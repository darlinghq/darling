#import <AppKit/NSTableView.h>
#import <AppKit/AppKitDefines.h>
#import <CoreFoundation/CFDictionary.h>

@class NSButtonCell;
@class NSTableView;
@class NSTableHeaderView;
@class NSTableColumn;
@class NSMouseTracker;
@class NSNotification;
@class NSString;
@protocol NSOutlineViewDelegate, NSOutlineViewDataSource;

typedef struct __OvFlags {
#ifdef __BIG_ENDIAN__
    unsigned int delegateWillDisplayCell:1;
    unsigned int delegateShouldEditTableColumn:1;
    unsigned int delegateShouldSelectItem:1;
    unsigned int delegateShouldSelectTableColumn:1;
    unsigned int delegateSelectionShouldChangeInOutlineView:1;
    unsigned int delegateShouldCollapseItem:1;
    unsigned int delegateShouldExpandItem:1;
    unsigned int autoresizesOutlineColumn:1;
    unsigned int autoSaveExpandItems:1;
    unsigned int enableExpandNotifications:1;
    unsigned int delegateWillDisplayOutlineCell:1;
    unsigned int subclassRowForItem:1;
    unsigned int selectionAdjustmentDisabled:1;
    unsigned int autoExpandFlashState:1;
    unsigned int animateExpandAndCollapse:1;
    unsigned int delegateHeightOfRowByItem:1;
    unsigned int delayRowEntryFreeDisabled:1;
    unsigned int numberOfRowsDataExpandEntered:1;
    unsigned int validDataSourceMethods:1;
    unsigned int reloadingData:1;
    unsigned int dataSourceDraggingEndedAt:1;
    unsigned int dataSourceDraggedImageMovedTo:1;
    unsigned int delegateShouldShowOutlineCellForItem:1;
    unsigned int delegateAutoExpandItem:1;
    unsigned int delegateAutoCollapseItem:1;
    unsigned int delegateShouldAutoExpandItem:1;
    unsigned int flashingEntireRow:1;
    unsigned int flashingOutlineCell:1;
    unsigned int dontRedisplayOnFrameChange:1;
    unsigned int allowAutomaticAnimations:1;
    unsigned int dataSourceObjectValueByItem:1;
    unsigned int allItemsLoaded:1;
#else
    unsigned int allItemsLoaded:1;
    unsigned int dataSourceObjectValueByItem:1;
    unsigned int allowAutomaticAnimations:1;
    unsigned int dontRedisplayOnFrameChange:1;
    unsigned int flashingOutlineCell:1;
    unsigned int flashingEntireRow:1;
    unsigned int delegateShouldAutoExpandItem:1;
    unsigned int delegateAutoCollapseItem:1;
    unsigned int delegateAutoExpandItem:1;
    unsigned int delegateShouldShowOutlineCellForItem:1;
    unsigned int dataSourceDraggedImageMovedTo:1;
    unsigned int dataSourceDraggingEndedAt:1;
    unsigned int reloadingData:1;
    unsigned int validDataSourceMethods:1;
    unsigned int numberOfRowsDataExpandEntered:1;
    unsigned int delayRowEntryFreeDisabled:1;
    unsigned int delegateHeightOfRowByItem:1;
    unsigned int animateExpandAndCollapse:1;
    unsigned int autoExpandFlashState:1;
    unsigned int selectionAdjustmentDisabled:1;
    unsigned int subclassRowForItem:1;
    unsigned int delegateWillDisplayOutlineCell:1;
    unsigned int enableExpandNotifications:1;
    unsigned int autoSaveExpandItems:1;
    unsigned int autoresizesOutlineColumn:1;
    unsigned int delegateShouldExpandItem:1;
    unsigned int delegateShouldCollapseItem:1;
    unsigned int delegateSelectionShouldChangeInOutlineView:1;
    unsigned int delegateShouldSelectTableColumn:1;
    unsigned int delegateShouldSelectItem:1;
    unsigned int delegateShouldEditTableColumn:1;
    unsigned int delegateWillDisplayCell:1;
#endif
} _OVFlags;



enum { NSOutlineViewDropOnItemIndex = -1 };

@class _NSOVRowEntry;

@interface NSOutlineView : NSTableView {
  @private
    NSInteger _numberOfRows;
    _NSOVRowEntry *_rowEntryTree;
    NSMapTable *_itemToEntryMap;
    __strong CFMutableArrayRef _rowEntryArray;
    NSInteger _firstRowIndexDrawn;
    id _autoExpandTimerItem;
    NSTableColumn *_outlineTableColumn;
    BOOL _initedRows;
    BOOL _indentationMarkerInCell;
    NSInteger _indentationPerLevel;
    NSButtonCell *_outlineCell;
    NSRect _trackingOutlineFrame;
    id _expandingItem;
    NSInteger _unused;
    _OVFlags _ovFlags;
    id _ovLock;
    __strong long *_indentArray;
    long _originalWidth;
    id _expandSet;
    id _expandSetToExpandItemsInto;
    long _indentArraySize;
    NSButtonCell *_trackingOutlineCell;
    NSInteger _trackingRow;
    id _ovReserved;
}

- (void)setDelegate:(id <NSOutlineViewDelegate>)anObject;
- (id <NSOutlineViewDelegate>)delegate;

- (void)setDataSource:(id <NSOutlineViewDataSource>)aSource;
- (id <NSOutlineViewDataSource>)dataSource;



- (void)setOutlineTableColumn:(NSTableColumn *)outlineTableColumn;
- (NSTableColumn *)outlineTableColumn;



- (BOOL)isExpandable:(id)item;



- (void)expandItem:(id)item expandChildren:(BOOL)expandChildren;



- (void)expandItem:(id)item;



- (void)collapseItem:(id)item collapseChildren:(BOOL)collapseChildren;



- (void)collapseItem:(id)item;



- (void)reloadItem:(id)item reloadChildren:(BOOL)reloadChildren;



- (void)reloadItem:(id)item;



- (id)parentForItem:(id)item;



- (id)itemAtRow:(NSInteger)row;
- (NSInteger)rowForItem:(id)item;



- (NSInteger)levelForItem:(id)item;
- (NSInteger)levelForRow:(NSInteger)row;
- (BOOL)isItemExpanded:(id)item;



- (void)setIndentationPerLevel:(CGFloat)indentationPerLevel;
- (CGFloat)indentationPerLevel;



- (void)setIndentationMarkerFollowsCell:(BOOL)drawInCell;
- (BOOL)indentationMarkerFollowsCell;



- (void)setAutoresizesOutlineColumn:(BOOL)resize;
- (BOOL)autoresizesOutlineColumn;



- (NSRect)frameOfOutlineCellAtRow:(NSInteger)row;






- (void)setDropItem:(id)item dropChildIndex:(NSInteger)index;



- (BOOL)shouldCollapseAutoExpandedItemsForDeposited:(BOOL)deposited;



- (BOOL)autosaveExpandedItems;
- (void)setAutosaveExpandedItems:(BOOL)save;






- (void)insertItemsAtIndexes:(NSIndexSet *)indexes inParent:(id)parent withAnimation:(NSTableViewAnimationOptions)animationOptions;



- (void)removeItemsAtIndexes:(NSIndexSet *)indexes inParent:(id)parent withAnimation:(NSTableViewAnimationOptions)animationOptions;



- (void)moveItemAtIndex:(NSInteger)fromIndex inParent:(id)oldParent toIndex:(NSInteger)toIndex inParent:(id)newParent;



- (void)insertRowsAtIndexes:(NSIndexSet *)indexes withAnimation:(NSTableViewAnimationOptions)animationOptions UNAVAILABLE_ATTRIBUTE;
- (void)removeRowsAtIndexes:(NSIndexSet *)indexes withAnimation:(NSTableViewAnimationOptions)animationOptions UNAVAILABLE_ATTRIBUTE;
- (void)moveRowAtIndex:(NSInteger)oldIndex toIndex:(NSInteger)newIndex UNAVAILABLE_ATTRIBUTE;

@end





@protocol NSOutlineViewDataSource <NSObject>
@optional




- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item;
- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item;
- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item;



- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item;






- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item;

- (id)outlineView:(NSOutlineView *)outlineView itemForPersistentObject:(id)object;
- (id)outlineView:(NSOutlineView *)outlineView persistentObjectForItem:(id)item;




- (void)outlineView:(NSOutlineView *)outlineView sortDescriptorsDidChange:(NSArray *)oldDescriptors;






- (id <NSPasteboardWriting>)outlineView:(NSOutlineView *)outlineView pasteboardWriterForItem:(id)item;



- (void)outlineView:(NSOutlineView *)outlineView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forItems:(NSArray *)draggedItems;



- (void)outlineView:(NSOutlineView *)outlineView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation;



- (BOOL)outlineView:(NSOutlineView *)outlineView writeItems:(NSArray *)items toPasteboard:(NSPasteboard *)pasteboard;



- (void)outlineView:(NSOutlineView *)outlineView updateDraggingItemsForDrag:(id <NSDraggingInfo>)draggingInfo;



- (NSDragOperation)outlineView:(NSOutlineView *)outlineView validateDrop:(id <NSDraggingInfo>)info proposedItem:(id)item proposedChildIndex:(NSInteger)index;



- (BOOL)outlineView:(NSOutlineView *)outlineView acceptDrop:(id <NSDraggingInfo>)info item:(id)item childIndex:(NSInteger)index;



- (NSArray *)outlineView:(NSOutlineView *)outlineView namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination forDraggedItems:(NSArray *)items;

@end



@protocol NSOutlineViewDelegate <NSControlTextEditingDelegate>
@optional






- (NSView *)outlineView:(NSOutlineView *)outlineView viewForTableColumn:(NSTableColumn *)tableColumn item:(id)item;



- (NSTableRowView *)outlineView:(NSOutlineView *)outlineView rowViewForItem:(id)item;



- (void)outlineView:(NSOutlineView *)outlineView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row;



- (void)outlineView:(NSOutlineView *)outlineView didRemoveRowView:(NSTableRowView *)rowView forRow:(NSInteger)row;






- (void)outlineView:(NSOutlineView *)outlineView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item;
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item;
- (BOOL)selectionShouldChangeInOutlineView:(NSOutlineView *)outlineView;



- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item;



- (NSIndexSet *)outlineView:(NSOutlineView *)outlineView selectionIndexesForProposedSelection:(NSIndexSet *)proposedSelectionIndexes;

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectTableColumn:(NSTableColumn *)tableColumn;

- (void)outlineView:(NSOutlineView *)outlineView mouseDownInHeaderOfTableColumn:(NSTableColumn *)tableColumn;
- (void)outlineView:(NSOutlineView *)outlineView didClickTableColumn:(NSTableColumn *)tableColumn;
- (void)outlineView:(NSOutlineView *)outlineView didDragTableColumn:(NSTableColumn *)tableColumn;




- (NSString *)outlineView:(NSOutlineView *)outlineView toolTipForCell:(NSCell *)cell rect:(NSRectPointer)rect tableColumn:(NSTableColumn *)tableColumn item:(id)item mouseLocation:(NSPoint)mouseLocation;




- (CGFloat)outlineView:(NSOutlineView *)outlineView heightOfRowByItem:(id)item;




- (NSString *)outlineView:(NSOutlineView *)outlineView typeSelectStringForTableColumn:(NSTableColumn *)tableColumn item:(id)item;




- (id)outlineView:(NSOutlineView *)outlineView nextTypeSelectMatchFromItem:(id)startItem toItem:(id)endItem forString:(NSString *)searchString;




- (BOOL)outlineView:(NSOutlineView *)outlineView shouldTypeSelectForEvent:(NSEvent *)event withCurrentSearchString:(NSString *)searchString;




- (BOOL)outlineView:(NSOutlineView *)outlineView shouldShowCellExpansionForTableColumn:(NSTableColumn *)tableColumn item:(id)item;




- (BOOL)outlineView:(NSOutlineView *)outlineView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item;






- (NSCell *)outlineView:(NSOutlineView *)outlineView dataCellForTableColumn:(NSTableColumn *)tableColumn item:(id)item;




- (BOOL)outlineView:(NSOutlineView *)outlineView isGroupItem:(id)item;




- (BOOL)outlineView:(NSOutlineView *)outlineView shouldExpandItem:(id)item;




- (BOOL)outlineView:(NSOutlineView *)outlineView shouldCollapseItem:(id)item;




- (void)outlineView:(NSOutlineView *)outlineView willDisplayOutlineCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item;





- (CGFloat)outlineView:(NSOutlineView *)outlineView sizeToFitWidthOfColumn:(NSInteger)column;




- (BOOL)outlineView:(NSOutlineView *)outlineView shouldReorderColumn:(NSInteger)columnIndex toColumn:(NSInteger)newColumnIndex;




- (BOOL)outlineView:(NSOutlineView *)outlineView shouldShowOutlineCellForItem:(id)item;



- (void)outlineViewSelectionDidChange:(NSNotification *)notification;
- (void)outlineViewColumnDidMove:(NSNotification *)notification;
- (void)outlineViewColumnDidResize:(NSNotification *)notification;
- (void)outlineViewSelectionIsChanging:(NSNotification *)notification;
- (void)outlineViewItemWillExpand:(NSNotification *)notification;
- (void)outlineViewItemDidExpand:(NSNotification *)notification;
- (void)outlineViewItemWillCollapse:(NSNotification *)notification;
- (void)outlineViewItemDidCollapse:(NSNotification *)notification;

@end




EXTERN_C NSString *NSOutlineViewSelectionDidChangeNotification;
EXTERN_C NSString *NSOutlineViewColumnDidMoveNotification;
EXTERN_C NSString *NSOutlineViewColumnDidResizeNotification;
EXTERN_C NSString *NSOutlineViewSelectionIsChangingNotification;





EXTERN_C NSString *NSOutlineViewItemWillExpandNotification;
EXTERN_C NSString *NSOutlineViewItemDidExpandNotification;
EXTERN_C NSString *NSOutlineViewItemWillCollapseNotification;
EXTERN_C NSString *NSOutlineViewItemDidCollapseNotification;
