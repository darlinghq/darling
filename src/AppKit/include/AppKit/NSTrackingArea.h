#import <Foundation/NSObject.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSGeometry.h>


@class NSDictionary;


enum {
    NSTrackingMouseEnteredAndExited = 0x01,
    NSTrackingMouseMoved = 0x02,
    NSTrackingCursorUpdate = 0x04,
};


enum {
    NSTrackingActiveWhenFirstResponder = 0x10,
    NSTrackingActiveInKeyWindow = 0x20,
    NSTrackingActiveInActiveApp = 0x40,
    NSTrackingActiveAlways = 0x80,
};


enum {
    NSTrackingAssumeInside = 0x100,
    NSTrackingInVisibleRect = 0x200,
    NSTrackingEnabledDuringMouseDrag = 0x400
};

typedef NSUInteger NSTrackingAreaOptions;


@interface NSTrackingArea : NSObject <NSCopying, NSCoding>
{
@private
    NSRect _rect;
    __weak id _owner;
    NSDictionary * _userInfo;
    NSTrackingAreaOptions _options;
    NSInteger _privateFlags;
    void *_reserved;
}
- (id)initWithRect:(NSRect)rect options:(NSTrackingAreaOptions)options owner:(id)owner userInfo:(NSDictionary *)userInfo;
- (NSRect)rect;
- (NSTrackingAreaOptions)options;
- (id)owner;
- (NSDictionary *)userInfo;
@end
