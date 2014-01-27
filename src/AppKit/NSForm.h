#import <AppKit/NSMatrix.h>

@class NSFormCell;

@interface NSForm : NSMatrix

- (NSInteger)indexOfSelectedItem;
- (void)setEntryWidth:(CGFloat)width;
- (void)setInterlineSpacing:(CGFloat)spacing;
- (void)setBordered:(BOOL)flag;
- (void)setBezeled:(BOOL)flag;
- (void)setTitleAlignment:(NSTextAlignment)mode;
- (void)setTextAlignment:(NSTextAlignment)mode;
- (void)setTitleFont:(NSFont *)fontObj;
- (void)setTextFont:(NSFont *)fontObj;
- (id)cellAtIndex:(NSInteger)index;
- (void)drawCellAtIndex:(NSInteger)index;
- (NSFormCell *)addEntry:(NSString *)title;
- (NSFormCell *)insertEntry:(NSString *)title atIndex:(NSInteger)index;
- (void)removeEntryAtIndex:(NSInteger)index;
- (NSInteger)indexOfCellWithTag:(NSInteger)aTag;
- (void)selectTextAtIndex:(NSInteger)index;
- (void)setFrameSize:(NSSize)newSize;
- (void)setTitleBaseWritingDirection:(NSWritingDirection)writingDirection;
- (void)setTextBaseWritingDirection:(NSWritingDirection)writingDirection;







- (void)setPreferredTextFieldWidth:(CGFloat)preferredWidth;
- (CGFloat)preferredTextFieldWidth;

@end
