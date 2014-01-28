#import <AppKit/NSActionCell.h>
#import <AppKit/NSSliderCell.h>


enum {
    NSRelevancyLevelIndicatorStyle,
    NSContinuousCapacityLevelIndicatorStyle,
    NSDiscreteCapacityLevelIndicatorStyle,
    NSRatingLevelIndicatorStyle
};
typedef NSUInteger NSLevelIndicatorStyle;

@interface NSLevelIndicatorCell : NSActionCell {
  @private
    double _value;
    double _minValue;
    double _maxValue;
    double _warningValue;
    double _criticalValue;
    int _numberOfTickMarks;
    int _numberOfMajorTickMarks;
    struct {
 unsigned int indicatorStyle:4;
        unsigned int tickMarkPosition:1;
        unsigned int selectable:1;
 unsigned int ratingPlaceholder:1;
 unsigned int reserved:25;
    } _liFlags;
    NSRect _cellFrame;
    int _reserved1;
    int _reserved2;
    int _reserved3;
    int _reserved4;
}

- (id)initWithLevelIndicatorStyle:(NSLevelIndicatorStyle)levelIndicatorStyle;

- (NSLevelIndicatorStyle)levelIndicatorStyle;
- (void)setLevelIndicatorStyle:(NSLevelIndicatorStyle)levelIndicatorStyle;

- (double)minValue;
- (void)setMinValue:(double)minValue;

- (double)maxValue;
- (void)setMaxValue:(double)maxValue;

- (double)warningValue;
- (void)setWarningValue:(double)warningValue;

- (double)criticalValue;
- (void)setCriticalValue:(double)criticalValue;

- (void)setTickMarkPosition:(NSTickMarkPosition)position;
- (NSTickMarkPosition)tickMarkPosition;

- (void)setNumberOfTickMarks:(NSInteger)count;
- (NSInteger)numberOfTickMarks;

- (void)setNumberOfMajorTickMarks:(NSInteger)count;
- (NSInteger)numberOfMajorTickMarks;

- (NSRect)rectOfTickMarkAtIndex:(NSInteger)index;
- (double)tickMarkValueAtIndex:(NSInteger)index;

- (void)setImage:(NSImage*)image;

@end
