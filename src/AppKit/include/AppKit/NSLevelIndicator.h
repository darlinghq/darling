#import <AppKit/NSControl.h>
#import <AppKit/NSLevelIndicatorCell.h>


@interface NSLevelIndicator : NSControl {
}

- (double)minValue;
- (void)setMinValue:(double)minValue;

- (double)maxValue;
- (void)setMaxValue:(double)maxValue;

- (double)warningValue;
- (void)setWarningValue:(double)warningValue;

- (double)criticalValue;
- (void)setCriticalValue:(double)criticalValue;

- (NSTickMarkPosition)tickMarkPosition;
- (void)setTickMarkPosition:(NSTickMarkPosition)position;

- (NSInteger)numberOfTickMarks;
- (void)setNumberOfTickMarks:(NSInteger)count;

- (NSInteger)numberOfMajorTickMarks;
- (void)setNumberOfMajorTickMarks:(NSInteger)count;

- (double)tickMarkValueAtIndex:(NSInteger)index;
- (NSRect)rectOfTickMarkAtIndex:(NSInteger)index;

@end
