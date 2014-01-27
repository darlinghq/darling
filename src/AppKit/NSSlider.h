#import <AppKit/NSControl.h>
#import <AppKit/NSSliderCell.h>

@interface NSSlider : NSControl

- (double)minValue;
- (void)setMinValue:(double)aDouble;
- (double)maxValue;
- (void)setMaxValue:(double)aDouble;
- (void)setAltIncrementValue:(double)incValue;
- (double)altIncrementValue;
- (void)setTitleCell:(NSCell *)aCell;
- (id)titleCell;
- (void)setTitleColor:(NSColor *)newColor;
- (NSColor *)titleColor;
- (void)setTitleFont:(NSFont *)fontObj;
- (NSFont *)titleFont;
- (NSString *)title;
- (void)setTitle:(NSString *)aString;
- (void)setKnobThickness:(CGFloat)aFloat;
- (CGFloat)knobThickness;
- (void)setImage:(NSImage *)backgroundImage;
- (NSImage *)image;
- (NSInteger)isVertical;
- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent;

@end

@interface NSSlider(NSTickMarkSupport)



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
