#import <AppKit/NSControl.h>
#import <AppKit/NSCell.h>

enum {
    NSScrollerArrowsMaxEnd = 0,
    NSScrollerArrowsMinEnd = 1,
    NSScrollerArrowsDefaultSetting = 0,
    NSScrollerArrowsNone = 2
};
typedef NSUInteger NSScrollArrowPosition;

enum {
    NSNoScrollerParts = 0,
    NSOnlyScrollerArrows = 1,
    NSAllScrollerParts = 2
};
typedef NSUInteger NSUsableScrollerParts;

enum {
    NSScrollerNoPart = 0,
    NSScrollerDecrementPage = 1,
    NSScrollerKnob = 2,
    NSScrollerIncrementPage = 3,
    NSScrollerDecrementLine = 4,
    NSScrollerIncrementLine = 5,
    NSScrollerKnobSlot = 6
};
typedef NSUInteger NSScrollerPart;

enum {
    NSScrollerIncrementArrow = 0,
    NSScrollerDecrementArrow = 1
};
typedef NSUInteger NSScrollerArrow;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
enum {
    NSScrollerStyleLegacy = 0,
    NSScrollerStyleOverlay = 1
};
#endif
typedef NSInteger NSScrollerStyle;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
enum {
    NSScrollerKnobStyleDefault = 0,
    NSScrollerKnobStyleDark = 1,
    NSScrollerKnobStyleLight = 2
};
#endif
typedef NSInteger NSScrollerKnobStyle;

@interface NSScroller : NSControl
{

    CGFloat _curValue;
    CGFloat _percent;
    CGFloat _knobSize;
    struct __sFlags2 {
        unsigned int hitPart:4;
        unsigned int controlSize:2;
        unsigned int inMaxEnd:1;
        unsigned int setFloatValueOverridden:1;
        unsigned int setFloatValueKnobProportionOverridden:1;
        unsigned int style:1;
        unsigned int styleCompatibility:1;
        unsigned int overlayScrollerState:2;
        unsigned int knobStyle:2;
        unsigned int sbPaused:1;
        unsigned int isAnimatingKnob:1;
        unsigned int reserved:15;
    } _sFlags2;
    id _target;
    SEL _action;
    struct _sFlags {
        unsigned int isHoriz:1;
        NSScrollArrowPosition arrowsLoc:2;
        NSUsableScrollerParts partsUsable:2;
        unsigned int fine:1;
        unsigned int needsEnableFlush:1;
        unsigned int thumbing:1;
        unsigned int slotDrawn:1;
        unsigned int knobDrawn:1;
        unsigned int lit:1;
        unsigned int knobLit:1;
        unsigned int reserved:1;
        unsigned int controlTint:3;
        unsigned int repeatCount:16;
    } sFlags;
}
+ (BOOL)isCompatibleWithOverlayScrollers;



+ (CGFloat)scrollerWidthForControlSize:(NSControlSize)controlSize scrollerStyle:(NSScrollerStyle)scrollerStyle;



+ (CGFloat)scrollerWidthForControlSize:(NSControlSize)controlSize;



+ (CGFloat)scrollerWidth;



+ (NSScrollerStyle)preferredScrollerStyle;



- (NSScrollerStyle)scrollerStyle;
- (void)setScrollerStyle:(NSScrollerStyle)newScrollerStyle;



- (NSScrollerKnobStyle)knobStyle;
- (void)setKnobStyle:(NSScrollerKnobStyle)newKnobStyle;

- (void)drawParts;
- (NSRect)rectForPart:(NSScrollerPart)partCode;
- (void)checkSpaceForParts;
- (NSUsableScrollerParts)usableParts;
- (void)setArrowsPosition:(NSScrollArrowPosition)where;
- (NSScrollArrowPosition)arrowsPosition;
- (void)setControlTint:(NSControlTint)controlTint;
- (NSControlTint)controlTint;
- (void)setControlSize:(NSControlSize)controlSize;
- (NSControlSize)controlSize;
- (void)drawArrow:(NSScrollerArrow)whichArrow highlight:(BOOL)flag;
- (void)drawKnob;
- (void)drawKnobSlotInRect:(NSRect)slotRect highlight:(BOOL)flag;
- (void)highlight:(BOOL)flag;
- (NSScrollerPart)testPart:(NSPoint)thePoint;
- (void)trackKnob:(NSEvent *)theEvent;
- (void)trackScrollButtons:(NSEvent *)theEvent;
- (NSScrollerPart)hitPart;
- (CGFloat)knobProportion;
- (void)setKnobProportion:(CGFloat)proportion;

@end

@interface NSScroller(NSDeprecated)


- (void)setFloatValue:(float)aFloat knobProportion:(CGFloat)proportion;
@end



EXTERN_C NSString *const NSPreferredScrollerStyleDidChangeNotification;
