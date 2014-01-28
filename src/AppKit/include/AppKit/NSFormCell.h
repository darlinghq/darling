#import <AppKit/NSActionCell.h>

@interface NSFormCell : NSActionCell
{

    CGFloat _titleWidth;
    id _titleCell;
    CGFloat _titleEndPoint;
#if __LP64__
    CGFloat _preferredTextFieldWidth;
#endif
}

- (id)initTextCell:(NSString *)aString;

- (CGFloat)titleWidth:(NSSize)aSize;
- (CGFloat)titleWidth;
- (void)setTitleWidth:(CGFloat)width;
- (NSString *)title;
- (void)setTitle:(NSString *)aString;
- (NSFont *)titleFont;
- (void)setTitleFont:(NSFont *)fontObj;
- (NSTextAlignment)titleAlignment;
- (void)setTitleAlignment:(NSTextAlignment)mode;
- (BOOL)isOpaque;

- (void)setPreferredTextFieldWidth:(CGFloat)preferredWidth;
- (CGFloat)preferredTextFieldWidth;
- (void)setPlaceholderString:(NSString*)string;
- (NSString*)placeholderString;
- (void)setPlaceholderAttributedString:(NSAttributedString*)string;
- (NSAttributedString*)placeholderAttributedString;

- (NSWritingDirection)titleBaseWritingDirection;
- (void)setTitleBaseWritingDirection:(NSWritingDirection)writingDirection;
- (void)setPreferredTextFieldWidth:(CGFloat)width;
- (CGFloat)preferredTextFieldWidth;

@end

@interface NSFormCell(NSKeyboardUI)


- (void)setTitleWithMnemonic:(NSString *)stringWithAmpersand;
@end

@interface NSFormCell(NSFormCellAttributedStringMethods)
- (NSAttributedString *)attributedTitle;
- (void)setAttributedTitle:(NSAttributedString *)obj;
@end
