#import <AppKit/NSControl.h>
#import <AppKit/NSTextFieldCell.h>
#import <AppKit/NSUserInterfaceValidation.h>

@protocol NSTextFieldDelegate;

@interface NSTextField : NSControl <NSUserInterfaceValidations>
{

    id _delegate;
    SEL _errorAction;
#if __LP64__
    CGFloat _maxLayoutWidth;
#endif
}

- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;
- (void)setDrawsBackground:(BOOL)flag;
- (BOOL)drawsBackground;
- (void)setTextColor:(NSColor *)color;
- (NSColor *)textColor;
- (BOOL)isBordered;
- (void)setBordered:(BOOL)flag;
- (BOOL)isBezeled;
- (void)setBezeled:(BOOL)flag;
- (BOOL)isEditable;
- (void)setEditable:(BOOL)flag;
- (BOOL)isSelectable;
- (void)setSelectable:(BOOL)flag;
- (void)selectText:(id)sender;
- (id <NSTextFieldDelegate>)delegate;
- (void)setDelegate:(id <NSTextFieldDelegate>)anObject;
- (BOOL)textShouldBeginEditing:(NSText *)textObject;
- (BOOL)textShouldEndEditing:(NSText *)textObject;
- (void)textDidBeginEditing:(NSNotification *)notification;
- (void)textDidEndEditing:(NSNotification *)notification;
- (void)textDidChange:(NSNotification *)notification;
- (BOOL)acceptsFirstResponder;

- (void)setBezelStyle:(NSTextFieldBezelStyle)style;
- (NSTextFieldBezelStyle)bezelStyle;



- (void)setPreferredMaxLayoutWidth:(CGFloat)width;
- (CGFloat)preferredMaxLayoutWidth;

@end

@interface NSTextField(NSTextFieldAttributedStringMethods)
- (BOOL)allowsEditingTextAttributes;
- (void)setAllowsEditingTextAttributes:(BOOL)flag;
- (BOOL)importsGraphics;
- (void)setImportsGraphics:(BOOL)flag;
@end

@protocol NSTextFieldDelegate <NSControlTextEditingDelegate> @end

@interface NSTextField(NSDeprecated)



- (void)setTitleWithMnemonic:(NSString *)stringWithAmpersand;

@end
