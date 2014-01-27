#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSGraphics.h>

@class NSArray, NSColorSpace;

typedef struct NSScreenAuxiliary NSScreenAuxiliaryOpaque;

@interface NSScreen : NSObject {

@private
    NSRect _frame;
    NSWindowDepth _depth;
    int _screenNumber;
    id _auxiliaryStorage;
}

+ (NSArray *)screens;
+ (NSScreen *)mainScreen;
+ (NSScreen *)deepestScreen;

- (NSWindowDepth)depth;
- (NSRect)frame;
- (NSRect)visibleFrame;
- (NSDictionary *)deviceDescription;
- (NSColorSpace *)colorSpace;

- (const NSWindowDepth *)supportedWindowDepths ;



- (NSRect)convertRectToBacking:(NSRect)aRect;
- (NSRect)convertRectFromBacking:(NSRect)aRect;



- (NSRect)backingAlignedRect:(NSRect)aRect options:(NSAlignmentOptions)options;



- (CGFloat)backingScaleFactor;

@end


EXTERN_C NSString * const NSScreenColorSpaceDidChangeNotification;


@interface NSScreen(NSDeprecated)




- (CGFloat)userSpaceScaleFactor;

@end
