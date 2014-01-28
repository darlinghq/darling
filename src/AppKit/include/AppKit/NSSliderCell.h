#import <AppKit/NSActionCell.h>

enum {
    NSTickMarkBelow = 0,
    NSTickMarkAbove = 1,
    NSTickMarkLeft = NSTickMarkAbove,
    NSTickMarkRight = NSTickMarkBelow
};
typedef NSUInteger NSTickMarkPosition;

enum {
    NSLinearSlider = 0,
    NSCircularSlider = 1
};
typedef NSUInteger NSSliderType;

@interface NSSliderCell : NSActionCell
{

    int _reserved[3];
    int _numberOfTickMarks;
    double _altIncValue;
    double _value;
    double _maxValue;
    double _minValue;
    NSRect _trackRect;
    struct __sliderCellFlags {
        unsigned int weAreVertical:1;
        unsigned int weAreVerticalSet:1;
        unsigned int reserved1:1;
        unsigned int isPressed:1;
        unsigned int allowsTickMarkValuesOnly:1;
        unsigned int tickMarkPosition:1;
        unsigned int sliderType:2;
        unsigned int drawing:1;
        unsigned int reserved2:23;
    } _scFlags;
}


+ (BOOL)prefersTrackingUntilMouseUp;


- (double)minValue;
- (void)setMinValue:(double)aDouble;
- (double)maxValue;
- (void)setMaxValue:(double)aDouble;
- (void)setAltIncrementValue:(double)incValue;
- (double)altIncrementValue;
- (NSInteger)isVertical;
- (void)setTitleColor:(NSColor *)newColor;
- (NSColor *)titleColor;
- (void)setTitleFont:(NSFont *)fontObj;
- (NSFont *)titleFont;
- (NSString *)title;
- (void)setTitle:(NSString *)aString;
- (void)setTitleCell:(NSCell *)aCell;
- (id)titleCell;
- (void)setKnobThickness:(CGFloat)aFloat;
- (CGFloat)knobThickness;
- (NSRect)knobRectFlipped:(BOOL)flipped;
- (void)drawKnob:(NSRect)knobRect;
- (void)drawKnob;
- (void)drawBarInside:(NSRect)aRect flipped:(BOOL)flipped;
- (NSRect)trackRect;

- (void) setSliderType:(NSSliderType)sliderType;
- (NSSliderType)sliderType;

@end

@interface NSSliderCell(NSTickMarkSupport)



- (void)setNumberOfTickMarks:(NSInteger)count;
- (NSInteger)numberOfTickMarks;



- (void)setTickMarkPosition:(NSTickMarkPosition)position;
- (NSTickMarkPosition)tickMarkPosition;




- (void)setAllowsTickMarkValuesOnly:(BOOL)yorn;
- (BOOL)allowsTickMarkValuesOnly;



- (double)tickMarkValueAtIndex:(NSInteger)index;



- (NSRect)rectOfTickMarkAtIndex:(NSInteger)index;




- (NSInteger)indexOfTickMarkAtPoint:(NSPoint)point;

- (double)closestTickMarkValueToValue:(double)value;

@end
