#import <AppKit/NSView.h>
#import <AppKit/NSCell.h>
#import <Foundation/NSDate.h>

struct __NSProgressIndicatorThreadInfo;
typedef struct __NSProgressIndicatorThreadInfo _NSProgressIndicatorThreadInfo;
enum {
    NSProgressIndicatorPreferredThickness = 14,
    NSProgressIndicatorPreferredSmallThickness = 10,
    NSProgressIndicatorPreferredLargeThickness = 18,
    NSProgressIndicatorPreferredAquaThickness = 12
};
typedef NSUInteger NSProgressIndicatorThickness;


enum {
    NSProgressIndicatorBarStyle = 0,
    NSProgressIndicatorSpinningStyle = 1
};
typedef NSUInteger NSProgressIndicatorStyle;




@interface NSProgressIndicator : NSView
{
@private

    BOOL _isBezeled;
    BOOL _isIndeterminate;
    BOOL _threadedAnimation;

    double _minimum;
    double _maximum;


    double _value;

    unsigned int _animationIndex;
    NSTimeInterval _animationDelay;

    id _timer;

    CGFloat _drawingWidth;

    id _roundColor;
    id _reserved;

    volatile struct __progressIndicatorFlags {
        unsigned int isSpinning:1;
        unsigned int isVector:1;
        unsigned int isLocked:1;
        unsigned int controlTint:3;
        unsigned int controlSize:2;
 unsigned int style:1;
        unsigned int _delayedStartup:1;
  unsigned int hideWhenStopped:1;
        unsigned int revive:1;
        unsigned int _temporarilyBlockHeartBeating:1;
        unsigned int _isHidden:1;
        unsigned int _isHeartBeatInstalled:1;
        unsigned int _customRenderer:1;
        unsigned int _lastFrame:8;
        unsigned int _isDetaching:1;
        unsigned int RESERVED:7;
    } _progressIndicatorFlags;


    id _NSProgressIndicatorReserved1;
}



- (BOOL)isIndeterminate;
- (void)setIndeterminate:(BOOL)flag;

- (BOOL)isBezeled;
- (void)setBezeled:(BOOL)flag;

- (NSControlTint)controlTint;
- (void)setControlTint:(NSControlTint)tint;

- (NSControlSize)controlSize;
- (void)setControlSize:(NSControlSize)size;



- (double)doubleValue;
- (void)setDoubleValue:(double)doubleValue;

- (void)incrementBy:(double)delta;

- (double)minValue;
- (double)maxValue;
- (void)setMinValue:(double)newMinimum;
- (void)setMaxValue:(double)newMaximum;



- (BOOL)usesThreadedAnimation;
- (void)setUsesThreadedAnimation:(BOOL)threadedAnimation;

- (void)startAnimation:(id)sender;
- (void)stopAnimation:(id)sender;


- (void) setStyle: (NSProgressIndicatorStyle) style;
- (NSProgressIndicatorStyle) style;



- (void) sizeToFit;

- (BOOL) isDisplayedWhenStopped;
- (void) setDisplayedWhenStopped: (BOOL) isDisplayed;

@end

@interface NSProgressIndicator (NSProgressIndicatorDeprecated)
- (NSTimeInterval)animationDelay;
- (void)setAnimationDelay:(NSTimeInterval)delay;
- (void)animate:(id)sender;
@end
