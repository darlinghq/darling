#import <AppKit/NSText.h>

@class NSTextTable;
@class NSLayoutManager;
@class NSTextContainer;



enum {
    NSTextBlockAbsoluteValueType = 0,
    NSTextBlockPercentageValueType = 1
};
typedef NSUInteger NSTextBlockValueType;


enum {
    NSTextBlockWidth = 0,
    NSTextBlockMinimumWidth = 1,
    NSTextBlockMaximumWidth = 2,
    NSTextBlockHeight = 4,
    NSTextBlockMinimumHeight = 5,
    NSTextBlockMaximumHeight = 6
};
typedef NSUInteger NSTextBlockDimension;


enum {
    NSTextBlockPadding = -1,
    NSTextBlockBorder = 0,
    NSTextBlockMargin = 1
};
typedef NSInteger NSTextBlockLayer;


enum {
    NSTextBlockTopAlignment = 0,
    NSTextBlockMiddleAlignment = 1,
    NSTextBlockBottomAlignment = 2,
    NSTextBlockBaselineAlignment = 3
};
typedef NSUInteger NSTextBlockVerticalAlignment;


enum {
    NSTextTableAutomaticLayoutAlgorithm = 0,
    NSTextTableFixedLayoutAlgorithm = 1
};
typedef NSUInteger NSTextTableLayoutAlgorithm;


@interface NSTextBlock : NSObject <NSCoding, NSCopying> {

    void *_propVals;
    NSUInteger _propMask;
    NSUInteger _typeMask;
    id _primParamVal;
    id _otherParamVals;
    void *_blockPrimary;
    void *_blockSecondary;
}

- (id)init;


- (void)setValue:(CGFloat)val type:(NSTextBlockValueType)type forDimension:(NSTextBlockDimension)dimension;
- (CGFloat)valueForDimension:(NSTextBlockDimension)dimension;
- (NSTextBlockValueType)valueTypeForDimension:(NSTextBlockDimension)dimension;


- (void)setContentWidth:(CGFloat)val type:(NSTextBlockValueType)type;
- (CGFloat)contentWidth;
- (NSTextBlockValueType)contentWidthValueType;


- (void)setWidth:(CGFloat)val type:(NSTextBlockValueType)type forLayer:(NSTextBlockLayer)layer edge:(NSRectEdge)edge;
- (void)setWidth:(CGFloat)val type:(NSTextBlockValueType)type forLayer:(NSTextBlockLayer)layer;
- (CGFloat)widthForLayer:(NSTextBlockLayer)layer edge:(NSRectEdge)edge;
- (NSTextBlockValueType)widthValueTypeForLayer:(NSTextBlockLayer)layer edge:(NSRectEdge)edge;


- (void)setVerticalAlignment:(NSTextBlockVerticalAlignment)alignment;
- (NSTextBlockVerticalAlignment)verticalAlignment;


- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;
- (void)setBorderColor:(NSColor *)color forEdge:(NSRectEdge)edge;
- (void)setBorderColor:(NSColor *)color;
- (NSColor *)borderColorForEdge:(NSRectEdge)edge;


- (NSRect)rectForLayoutAtPoint:(NSPoint)startingPoint inRect:(NSRect)rect textContainer:(NSTextContainer *)textContainer characterRange:(NSRange)charRange;
- (NSRect)boundsRectForContentRect:(NSRect)contentRect inRect:(NSRect)rect textContainer:(NSTextContainer *)textContainer characterRange:(NSRange)charRange;


- (void)drawBackgroundWithFrame:(NSRect)frameRect inView:(NSView *)controlView characterRange:(NSRange)charRange layoutManager:(NSLayoutManager *)layoutManager;

@end


@interface NSTextTableBlock : NSTextBlock {
    NSTextTable *_table;
    NSInteger _rowNum;
    NSInteger _colNum;
    NSInteger _rowSpan;
    NSInteger _colSpan;
    void *_tableBlockPrimary;
    void *_tableBlockSecondary;
}

- (id)initWithTable:(NSTextTable *)table startingRow:(NSInteger)row rowSpan:(NSInteger)rowSpan startingColumn:(NSInteger)col columnSpan:(NSInteger)colSpan;


- (NSTextTable *)table;
- (NSInteger)startingRow;
- (NSInteger)rowSpan;
- (NSInteger)startingColumn;
- (NSInteger)columnSpan;

@end


@interface NSTextTable : NSTextBlock {
    NSUInteger _numCols;
    NSUInteger _tableFlags;
    id _lcache;
    void *_tablePrimary;
    void *_tableSecondary;
}


- (NSUInteger)numberOfColumns;
- (void)setNumberOfColumns:(NSUInteger)numCols;
- (NSTextTableLayoutAlgorithm)layoutAlgorithm;
- (void)setLayoutAlgorithm:(NSTextTableLayoutAlgorithm)algorithm;
- (BOOL)collapsesBorders;
- (void)setCollapsesBorders:(BOOL)flag;
- (BOOL)hidesEmptyCells;
- (void)setHidesEmptyCells:(BOOL)flag;


- (NSRect)rectForBlock:(NSTextTableBlock *)block layoutAtPoint:(NSPoint)startingPoint inRect:(NSRect)rect textContainer:(NSTextContainer *)textContainer characterRange:(NSRange)charRange;
- (NSRect)boundsRectForBlock:(NSTextTableBlock *)block contentRect:(NSRect)contentRect inRect:(NSRect)rect textContainer:(NSTextContainer *)textContainer characterRange:(NSRange)charRange;
- (void)drawBackgroundForBlock:(NSTextTableBlock *)block withFrame:(NSRect)frameRect inView:(NSView *)controlView characterRange:(NSRange)charRange layoutManager:(NSLayoutManager *)layoutManager;

@end
