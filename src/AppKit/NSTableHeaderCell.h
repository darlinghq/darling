#import <AppKit/NSTextFieldCell.h>

@interface NSTableHeaderCell : NSTextFieldCell {

}



- (void)drawSortIndicatorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView ascending:(BOOL)ascending priority:(NSInteger)priority;



- (NSRect)sortIndicatorRectForBounds:(NSRect)theRect;

@end
