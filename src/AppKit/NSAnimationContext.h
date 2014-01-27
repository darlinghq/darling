#import <AppKit/AppKitDefines.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSDate.h>

@class CAMediaTimingFunction;


@interface NSAnimationContext : NSObject {
@private
    NSTimeInterval _duration;
    id _reserved[4];
    unsigned int _allowsImplicitAnimation:1;
    unsigned int _isImplicitContext:1;
    unsigned int __reserved:30;

}
+ (void)runAnimationGroup:(void (^)(NSAnimationContext *context))changes completionHandler:(void (^)(void))completionHandler;

+ (void)beginGrouping;
+ (void)endGrouping;

+ (NSAnimationContext *)currentContext;

@property NSTimeInterval duration;
@property(retain) CAMediaTimingFunction *timingFunction;

#if NS_BLOCKS_AVAILABLE
@property(copy) void (^completionHandler)(void);
#endif



@property BOOL allowsImplicitAnimation;

@end
