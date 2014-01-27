#import <AppKit/AppKitDefines.h>
#import <Foundation/Foundation.h>


@class NSArray, NSGraphicsContext, NSMutableArray, NSString, NSTimer;
@protocol NSAnimationDelegate;

enum {
    NSAnimationEaseInOut,
    NSAnimationEaseIn,
    NSAnimationEaseOut,
    NSAnimationLinear
};
typedef NSUInteger NSAnimationCurve;

enum {
    NSAnimationBlocking,
    NSAnimationNonblocking,
    NSAnimationNonblockingThreaded
};
typedef NSUInteger NSAnimationBlockingMode;

typedef float NSAnimationProgress;

extern NSString *NSAnimationProgressMarkNotification;
extern NSString *NSAnimationProgressMark;

@interface NSAnimation : NSObject <NSCopying, NSCoding> {
  @private
    NSTimeInterval _duration;
    NSAnimationProgress _currentProgress;
    float _framesPerSecond;
    id _delegate;
    NSTimer *_timer;
    NSTimeInterval _startTime;
    NSMutableArray *_progressMarks;
    NSAnimation *_startAnimation;
    NSAnimation *_stopAnimation;
    int _nextProgressMark;
    struct __aFlags {
 unsigned int delegateAnimationShouldStart:1;
 unsigned int delegateAnimationDidStop:1;
 unsigned int delegateAnimationDidEnd:1;
 unsigned int delegateAnimationValueForProgress:1;
 unsigned int delegateAnimationDidReachProgressMark:1;
 unsigned int animating:1;
 unsigned int blocking:1;
 unsigned int reserved:25;
    } _aFlags;
    struct __aSettings {
 unsigned int animationCurve:8;
 unsigned int animationBlockingMode:2;
 unsigned int reserved:22;
    } _aSettings;
    NSRunLoop *_scheduledRunLoop;
    NSInteger _reserved2;
    NSInteger _reserved3;
    NSInteger _reserved4;
}

- (id)initWithDuration:(NSTimeInterval)duration animationCurve:(NSAnimationCurve)animationCurve;

- (void)startAnimation;
- (void)stopAnimation;
- (BOOL)isAnimating;

- (NSAnimationProgress)currentProgress;
- (void)setCurrentProgress:(NSAnimationProgress)progress;

- (void)setDuration:(NSTimeInterval)duration;
- (NSTimeInterval)duration;

- (NSAnimationBlockingMode)animationBlockingMode;
- (void)setAnimationBlockingMode:(NSAnimationBlockingMode)animationBlockingMode;

- (void)setFrameRate:(float)framesPerSecond;
- (float)frameRate;

- (void)setAnimationCurve:(NSAnimationCurve)curve;
- (NSAnimationCurve)animationCurve;

- (float)currentValue;

- (void)setDelegate:(id <NSAnimationDelegate>)delegate;
- (id <NSAnimationDelegate>)delegate;

- (NSArray *)progressMarks;
- (void)setProgressMarks:(NSArray *)progressMarks;

- (void)addProgressMark:(NSAnimationProgress)progressMark;
- (void)removeProgressMark:(NSAnimationProgress)progressMark;

- (void)startWhenAnimation:(NSAnimation*)animation reachesProgress:(NSAnimationProgress)startProgress;
- (void)stopWhenAnimation:(NSAnimation*)animation reachesProgress:(NSAnimationProgress)stopProgress;

- (void)clearStartAnimation;
- (void)clearStopAnimation;

- (NSArray *)runLoopModesForAnimating;

@end

@protocol NSAnimationDelegate <NSObject>
@optional
- (BOOL)animationShouldStart:(NSAnimation*)animation;
- (void)animationDidStop:(NSAnimation*)animation;
- (void)animationDidEnd:(NSAnimation*)animation;
- (float)animation:(NSAnimation*)animation valueForProgress:(NSAnimationProgress)progress;
- (void)animation:(NSAnimation*)animation didReachProgressMark:(NSAnimationProgress)progress;
@end



EXTERN_C NSString* NSViewAnimationTargetKey;
EXTERN_C NSString* NSViewAnimationStartFrameKey;
EXTERN_C NSString* NSViewAnimationEndFrameKey;
EXTERN_C NSString* NSViewAnimationEffectKey;
EXTERN_C NSString* NSViewAnimationFadeInEffect;
EXTERN_C NSString* NSViewAnimationFadeOutEffect;

@interface NSViewAnimation : NSAnimation {
  @private
    NSArray* _viewAnimations;
    id _viewAnimationInfo;
    id _windowAnimationInfo;
    NSUInteger _reserved4a;
    NSUInteger _reserved4b;
    NSUInteger _reserved4c;
    struct __vaFlags {
 unsigned int reserved:32;
    } _vaFlags;
    NSUInteger _reserved5;
    NSUInteger _reserved6;
    NSUInteger _reserved7;
    NSUInteger _reserved8;
}

- (id)initWithViewAnimations:(NSArray*)viewAnimations;

- (NSArray*)viewAnimations;
- (void)setViewAnimations:(NSArray*)viewAnimations;

@end


@protocol NSAnimatablePropertyContainer
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5





- (id)animator;



- (NSDictionary *)animations;
- (void)setAnimations:(NSDictionary *)animations;



- (id)animationForKey:(NSString *)key;
+ (id)defaultAnimationForKey:(NSString *)key;

#endif
@end

EXTERN_C NSString *NSAnimationTriggerOrderIn;
EXTERN_C NSString *NSAnimationTriggerOrderOut;
