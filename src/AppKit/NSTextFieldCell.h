#import <AppKit/NSActionCell.h>

enum {
    NSTextFieldSquareBezel = 0,
    NSTextFieldRoundedBezel = 1
};
typedef NSUInteger NSTextFieldBezelStyle;

@class NSColor;

@interface NSTextFieldCell : NSActionCell {

    NSColor *_backgroundColor;
    NSColor *_textColor;
    struct __tfFlags {
        unsigned int drawsBackground:1;
        unsigned int bezelStyle:3;
        unsigned int thcSortDirection:2;
        unsigned int thcSortPriority:4;
        unsigned int mini:1;
        unsigned int textColorIgnoresNormalDisableFlag:1;
        unsigned int textColorDisableFlag:1;
        unsigned int thcForceHighlightForSort:1;
        unsigned int invalidTextColor:1;
        unsigned int notificationForMarkedText:1;
        unsigned int inToolbar:1;
        unsigned int hasTextLayer:1;
        unsigned int isButtonTitle:1;
        unsigned int reservedTextFieldCell:13;
    } _tfFlags;
}

- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;
- (void)setDrawsBackground:(BOOL)flag;
- (BOOL)drawsBackground;
- (void)setTextColor:(NSColor *)color;
- (NSColor *)textColor;
- (NSText *)setUpFieldEditorAttributes:(NSText *)textObj;

- (void)setBezelStyle:(NSTextFieldBezelStyle)style;
- (NSTextFieldBezelStyle)bezelStyle;

- (void)setPlaceholderString:(NSString*)string;
- (NSString*)placeholderString;
- (void)setPlaceholderAttributedString:(NSAttributedString*)string;
- (NSAttributedString*)placeholderAttributedString;

- (void)setWantsNotificationForMarkedText:(BOOL)flag;



- (NSArray *)allowedInputSourceLocales;
- (void)setAllowedInputSourceLocales:(NSArray *)localeIdentifiers;

@end
