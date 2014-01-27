#import <AppKit/NSControl.h>
#import <AppKit/NSDatePickerCell.h>


@class NSCalendar, NSLocale, NSTimeZone;

@interface NSDatePicker : NSControl
{
}



- (NSDatePickerStyle)datePickerStyle;
- (void)setDatePickerStyle:(NSDatePickerStyle)newStyle;

- (BOOL)isBezeled;
- (void)setBezeled:(BOOL)flag;

- (BOOL)isBordered;
- (void)setBordered:(BOOL)flag;

- (BOOL)drawsBackground;
- (void)setDrawsBackground:(BOOL)flag;

- (NSColor *)backgroundColor;
- (void)setBackgroundColor:(NSColor *)color;

- (NSColor *)textColor;
- (void)setTextColor:(NSColor *)color;



- (NSDatePickerMode)datePickerMode;
- (void)setDatePickerMode:(NSDatePickerMode)newMode;



- (NSDatePickerElementFlags)datePickerElements;
- (void)setDatePickerElements:(NSDatePickerElementFlags)elementFlags;

- (NSCalendar *)calendar;
- (void)setCalendar:(NSCalendar *)newCalendar;

- (NSLocale *)locale;
- (void)setLocale:(NSLocale *)newLocale;

- (NSTimeZone *)timeZone;
- (void)setTimeZone:(NSTimeZone *)newTimeZone;



- (NSDate *)dateValue;
- (void)setDateValue:(NSDate *)newStartDate;

- (NSTimeInterval)timeInterval;
- (void)setTimeInterval:(NSTimeInterval)newTimeInterval;



- (NSDate *)minDate;
- (void)setMinDate:(NSDate *)date;

- (NSDate *)maxDate;
- (void)setMaxDate:(NSDate *)date;



- (id <NSDatePickerCellDelegate>)delegate;
- (void)setDelegate:(id <NSDatePickerCellDelegate>)anObject;

@end
