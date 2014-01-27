#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSView;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
enum {
    NSTouchPhaseBegan = 1U << 0,
    NSTouchPhaseMoved = 1U << 1,
    NSTouchPhaseStationary = 1U << 2,
    NSTouchPhaseEnded = 1U << 3,
    NSTouchPhaseCancelled = 1U << 4,

    NSTouchPhaseTouching = NSTouchPhaseBegan | NSTouchPhaseMoved | NSTouchPhaseStationary,
    NSTouchPhaseAny = NSUIntegerMax
};
#endif
typedef NSUInteger NSTouchPhase;





@interface NSTouch : NSObject <NSCopying> {
@private
    NSInteger _index;
    id _identity;
    NSTouchPhase _phase;
    NSPoint _normalizedPosition;
    NSInteger _privateFlags;
    NSView *_view;
    id _reserved1;
    id _reserved2;
    id _reserved3;
    id _reserved4;

    id _device;
    NSSize _deviceSize;
    id _reserved5;
    id _reserved6;
    id _reserved7;
    id _reserved8;
}




@property(readonly, retain) id<NSObject, NSCopying> identity;
@property(readonly) NSTouchPhase phase;
@property(readonly) NSPoint normalizedPosition;
@property(readonly) BOOL isResting;


@property(readonly, retain) id device;



@property(readonly) NSSize deviceSize;

@end
