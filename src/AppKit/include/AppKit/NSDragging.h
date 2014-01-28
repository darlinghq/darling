#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <limits.h>

@class NSDraggingItem, NSDraggingSession, NSImage, NSPasteboard, NSView, NSWindow;
@class NSURL, NSArray, NSDictionary;
@protocol NSPasteboardWriting;


typedef NSUInteger NSDragOperation;

enum {
    NSDragOperationNone = 0,
    NSDragOperationCopy = 1,
    NSDragOperationLink = 2,
    NSDragOperationGeneric = 4,
    NSDragOperationPrivate = 8,
    NSDragOperationAll_Obsolete = 15,
    NSDragOperationMove = 16,
    NSDragOperationDelete = 32,
    NSDragOperationEvery = NSUIntegerMax
};


#define NSDragOperationAll NSDragOperationAll_Obsolete

#if MAC_OS_X_VERSION_10_7 <= MAC_OS_X_VERSION_MAX_ALLOWED
enum {
    NSDraggingFormationDefault = 0,
    NSDraggingFormationNone,
    NSDraggingFormationPile,
    NSDraggingFormationList,
    NSDraggingFormationStack
};
#endif
typedef NSInteger NSDraggingFormation;

#if MAC_OS_X_VERSION_10_7 <= MAC_OS_X_VERSION_MAX_ALLOWED
enum {
    NSDraggingContextOutsideApplication = 0,
    NSDraggingContextWithinApplication
};
#endif
typedef NSInteger NSDraggingContext;

#if MAC_OS_X_VERSION_10_7 <= MAC_OS_X_VERSION_MAX_ALLOWED
enum {
    NSDraggingItemEnumerationConcurrent = NSEnumerationConcurrent,

    NSDraggingItemEnumerationClearNonenumeratedImages = (1UL << 16),
};
#endif
typedef NSUInteger NSDraggingItemEnumerationOptions;





@protocol NSDraggingInfo <NSObject>
@required
- (NSWindow *)draggingDestinationWindow;
- (NSDragOperation)draggingSourceOperationMask;
- (NSPoint)draggingLocation;



- (NSPoint)draggedImageLocation;
- (NSImage *)draggedImage;
- (NSPasteboard *)draggingPasteboard;
- (id)draggingSource;
- (NSInteger)draggingSequenceNumber;
- (void)slideDraggedImageTo:(NSPoint)screenPoint;
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination;


@property NSDraggingFormation draggingFormation;



@property BOOL animatesToDestination;



@property NSInteger numberOfValidItemsForDrop;





#if NS_BLOCKS_AVAILABLE


- (void)enumerateDraggingItemsWithOptions:(NSDraggingItemEnumerationOptions)enumOpts forView:(NSView *)view classes:(NSArray *)classArray searchOptions:(NSDictionary *)searchOptions usingBlock:(void (^)(NSDraggingItem *draggingItem, NSInteger idx, BOOL *stop))block;
#endif
@end






@protocol NSDraggingDestination <NSObject>
@optional
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender;
- (void)draggingExited:(id <NSDraggingInfo>)sender;
- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
- (void)concludeDragOperation:(id <NSDraggingInfo>)sender;

- (void)draggingEnded:(id <NSDraggingInfo>)sender;

- (BOOL)wantsPeriodicDraggingUpdates;



- (void)updateDraggingItemsForDrag:(id <NSDraggingInfo>)sender;
@end


@protocol NSDraggingSource <NSObject>



@required
- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context;

@optional
- (void)draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint;
- (void)draggingSession:(NSDraggingSession *)session movedToPoint:(NSPoint)screenPoint;
- (void)draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation;

- (BOOL)ignoreModifierKeysForDraggingSession:(NSDraggingSession *)session;

@end

@interface NSObject(NSDraggingSourceDeprecated)


- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)flag;
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination;
- (void)draggedImage:(NSImage *)image beganAt:(NSPoint)screenPoint;
- (void)draggedImage:(NSImage *)image endedAt:(NSPoint)screenPoint operation:(NSDragOperation)operation;
- (void)draggedImage:(NSImage *)image movedTo:(NSPoint)screenPoint;
- (BOOL)ignoreModifierKeysWhileDragging;

- (void)draggedImage:(NSImage *)image endedAt:(NSPoint)screenPoint deposited:(BOOL)flag;


- (void)draggedImage:(NSImage *)image endedAt:(NSPoint)screenPoint deposited:(BOOL)flag;
@end
