#import <AppKit/NSControl.h>

@interface NSStepper : NSControl {
  @private
    unsigned int _reserved1;
    unsigned int _reserved2;
    unsigned int _reserved3;
    unsigned int _reserved4;
}

- (double)minValue;
- (void)setMinValue:(double)minValue;

- (double)maxValue;
- (void)setMaxValue:(double)maxValue;

- (double)increment;
- (void)setIncrement:(double)increment;

- (BOOL)valueWraps;
- (void)setValueWraps:(BOOL)valueWraps;

- (BOOL)autorepeat;
- (void)setAutorepeat:(BOOL)autorepeat;

@end
