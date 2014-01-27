#import <AppKit/NSView.h>
#import <AppKit/NSViewController.h>
#import <AppKit/NSDragging.h>


@class NSCollectionView, NSImageView, NSIndexSet, NSMutableIndexSet, NSNib, NSTextField;
@protocol NSCollectionViewDelegate;

typedef NSInteger NSCollectionViewDropOperation;


@interface NSCollectionViewItem : NSViewController <NSCopying> {
@private
    NSCollectionView *_collectionView;
    struct {
        unsigned int isSelected:1;
        unsigned int suppressSelectionChangedNotification:1;
        unsigned int connectionsCopied:1;
        unsigned int reserved:29;
    } _cviFlags;
    NSData *_cachedArchive;
    void *_reserved2;
}

@property (readonly) NSCollectionView *collectionView;

@property (getter=isSelected) BOOL selected;

@property (assign) IBOutlet NSImageView *imageView;
@property (assign) IBOutlet NSTextField *textField;



@property (readonly) NSArray *draggingImageComponents;

@end


@interface NSCollectionView : NSView <NSDraggingSource, NSDraggingDestination> {
@private
    NSArray *_content;
    NSMutableIndexSet *_selectionIndexes;
    NSCollectionViewItem *_itemPrototype;
    NSSize _minItemSize;
    NSSize _maxItemSize;
    NSUInteger _maxGridRows;
    NSUInteger _maxGridColumns;
    NSArray *_backgroundColors;
    NSMutableArray *_displayedItems;
    NSTimeInterval _animationDuration;
    struct {
        unsigned int isFirstResponder:1;
        unsigned int invalidateItemViews:1;
        unsigned int selectable:1;
        unsigned int allowsMultipleSelection:1;
        unsigned int avoidsEmptySelection:1;
        unsigned int superviewIsClipView:1;
        unsigned int needsUpdateGrid:1;
        unsigned int needsUpdateBackground:1;
        unsigned int gridSettingsNeedUpdate:1;
        unsigned int guardSetFrameSize:1;
        unsigned int canDisplayItems:1;
        unsigned int animateForDrag:1;
        unsigned int unarchiving:1;
        unsigned int reserved:19;
    } _cvFlags;
    id _delegate;
    NSMutableArray *_backgroundLayers;
    NSSize _storedFrameSize;
    NSNib *_cachedNib;
    BOOL *_animTimerCancel;
    NSArray *_removedItems;
    NSArray *_addedItems;
    NSIndexSet *_addedItemIndexes;
    BOOL *_resizeTimerCancel;

    NSIndexSet *_draggedIndexes;
    NSDragOperation _draggingSourceOperationMaskForLocal;
    NSDragOperation _draggingSourceOperationMaskForNonLocal;
    NSInteger _currentDropIndex;
    NSInteger _currentDropSpaceIndex;
    NSInteger _shiftRow;
    NSDragOperation _currentDragOperation;
    NSCollectionViewDropOperation _currentDropOperation;
    id _draggingInfo;
    BOOL *_dragTimerCancel;

    id _private;
    void *_reserved[13];
}

- (void)setDelegate:(id <NSCollectionViewDelegate>)aDelegate;
- (id <NSCollectionViewDelegate>)delegate;

- (BOOL)isFirstResponder;

- (void)setContent:(NSArray *)content;
- (NSArray *)content;
- (void)setSelectable:(BOOL)flag;
- (BOOL)isSelectable;
- (void)setAllowsMultipleSelection:(BOOL)flag;
- (BOOL)allowsMultipleSelection;
- (void)setSelectionIndexes:(NSIndexSet *)indexes;
- (NSIndexSet *)selectionIndexes;

- (NSCollectionViewItem *)newItemForRepresentedObject:(id)object;

- (void)setItemPrototype:(NSCollectionViewItem *)prototype;
- (NSCollectionViewItem *)itemPrototype;

- (void)setMaxNumberOfRows:(NSUInteger)number;
- (NSUInteger)maxNumberOfRows;
- (void)setMaxNumberOfColumns:(NSUInteger)number;
- (NSUInteger)maxNumberOfColumns;
- (void)setMinItemSize:(NSSize)size;
- (NSSize)minItemSize;
- (void)setMaxItemSize:(NSSize)size;
- (NSSize)maxItemSize;

- (void)setBackgroundColors:(NSArray *)colors;
- (NSArray *)backgroundColors;



- (NSCollectionViewItem *)itemAtIndex:(NSUInteger)index;



- (NSRect)frameForItemAtIndex:(NSUInteger)index;



- (NSRect)frameForItemAtIndex:(NSUInteger)index withNumberOfItems:(NSUInteger)numberOfItems;





- (void)setDraggingSourceOperationMask:(NSDragOperation)dragOperationMask forLocal:(BOOL)localDestination;



- (NSImage *)draggingImageForItemsAtIndexes:(NSIndexSet *)indexes withEvent:(NSEvent *)event offset:(NSPointPointer)dragImageOffset;

@end

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6

enum {
    NSCollectionViewDropOn = 0,
    NSCollectionViewDropBefore = 1,
};

#endif

@protocol NSCollectionViewDelegate <NSObject>
@optional





- (BOOL)collectionView:(NSCollectionView *)collectionView canDragItemsAtIndexes:(NSIndexSet *)indexes withEvent:(NSEvent *)event;




- (BOOL)collectionView:(NSCollectionView *)collectionView writeItemsAtIndexes:(NSIndexSet *)indexes toPasteboard:(NSPasteboard *)pasteboard;



- (NSArray *)collectionView:(NSCollectionView *)collectionView namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropURL forDraggedItemsAtIndexes:(NSIndexSet *)indexes;



- (NSImage *)collectionView:(NSCollectionView *)collectionView draggingImageForItemsAtIndexes:(NSIndexSet *)indexes withEvent:(NSEvent *)event offset:(NSPointPointer)dragImageOffset;







- (NSDragOperation)collectionView:(NSCollectionView *)collectionView validateDrop:(id <NSDraggingInfo>)draggingInfo proposedIndex:(NSInteger *)proposedDropIndex dropOperation:(NSCollectionViewDropOperation *)proposedDropOperation;





- (BOOL)collectionView:(NSCollectionView *)collectionView acceptDrop:(id <NSDraggingInfo>)draggingInfo index:(NSInteger)index dropOperation:(NSCollectionViewDropOperation)dropOperation;






- (id <NSPasteboardWriting>)collectionView:(NSCollectionView *)collectionView pasteboardWriterForItemAtIndex:(NSUInteger)index;



- (void)collectionView:(NSCollectionView *)collectionView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forItemsAtIndexes:(NSIndexSet *)indexes;



- (void)collectionView:(NSCollectionView *)collectionView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint dragOperation:(NSDragOperation)operation;



- (void)collectionView:(NSCollectionView *)collectionView updateDraggingItemsForDrag:(id <NSDraggingInfo>)draggingInfo;

@end
