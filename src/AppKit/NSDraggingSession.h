#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSDragging.h>

@class NSPasteboard, NSImage, NSDraggingItem;
@protocol NSDraggingSource, NSPasteboardWriting;


@interface NSDraggingSession : NSObject {
@private
    void *_private;
    NSPoint _dragLocation;
    NSPoint _offset;
    id<NSDraggingSource> _source;
    id _alternateDragSource;
    id<NSDraggingSource> _filePromiseDragSource;
    NSPasteboard *_pboard;
    NSImage *_compositeImageCache;
    NSInteger _leaderIndex;
    BOOL _animatesOnCancelOrFail;
    BOOL _reservedBOOL1;
    BOOL _reservedBOOL2;
    BOOL _reservedBOOL3;
    BOOL _reservedBOOL4;
    BOOL _reservedBOOL5;
    BOOL _reservedBOOL6;
    BOOL _reservedBOOL7;
    NSInteger _reservedInteger1;
    NSInteger _reservedInteger2;
}


@property NSDraggingFormation draggingFormation;



@property BOOL animatesToStartingPositionsOnCancelOrFail;



@property NSInteger draggingLeaderIndex;


@property(readonly) NSPasteboard *draggingPasteboard;


@property(readonly) NSInteger draggingSequenceNumber;


@property(readonly) NSPoint draggingLocation;





#if NS_BLOCKS_AVAILABLE


- (void)enumerateDraggingItemsWithOptions:(NSDraggingItemEnumerationOptions)enumOpts forView:(NSView *)view classes:(NSArray *)classArray searchOptions:(NSDictionary *)searchOptions usingBlock:(void (^)(NSDraggingItem *draggingItem, NSInteger idx, BOOL *stop))block;
#endif
@end
