#import <AppKit/NSActionCell.h>

@interface NSStepperCell : NSActionCell {
  @private
    double _value;
    double _minValue;
    double _maxValue;
    double _increment;
    struct {
        unsigned int valueWraps:1;
        unsigned int autorepeat:1;
        unsigned int drawing:1;
        unsigned int reserved:29;
    } _stFlags;
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
