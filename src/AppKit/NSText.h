#import <AppKit/NSView.h>
#import <AppKit/NSSpellProtocol.h>

@class NSColor, NSFont, NSNotification;
@protocol NSTextDelegate;


enum {
    NSEnterCharacter = 0x0003,
    NSBackspaceCharacter = 0x0008,
    NSTabCharacter = 0x0009,
    NSNewlineCharacter = 0x000a,
    NSFormFeedCharacter = 0x000c,
    NSCarriageReturnCharacter = 0x000d,
    NSBackTabCharacter = 0x0019,
    NSDeleteCharacter = 0x007f,
    NSLineSeparatorCharacter = 0x2028,
    NSParagraphSeparatorCharacter = 0x2029
};


enum {
    NSLeftTextAlignment = 0,
    NSRightTextAlignment = 1,
    NSCenterTextAlignment = 2,
    NSJustifiedTextAlignment = 3,
    NSNaturalTextAlignment = 4
};
typedef NSUInteger NSTextAlignment;


enum {
    NSWritingDirectionNatural = -1,
    NSWritingDirectionLeftToRight = 0,
    NSWritingDirectionRightToLeft = 1
};
typedef NSInteger NSWritingDirection;


enum {
    NSTextWritingDirectionEmbedding = (0 << 1),
    NSTextWritingDirectionOverride = (1 << 1)
};



enum {
    NSIllegalTextMovement = 0,
    NSReturnTextMovement = 0x10,
    NSTabTextMovement = 0x11,
    NSBacktabTextMovement = 0x12,
    NSLeftTextMovement = 0x13,
    NSRightTextMovement = 0x14,
    NSUpTextMovement = 0x15,
    NSDownTextMovement = 0x16,
    NSCancelTextMovement = 0x17,
    NSOtherTextMovement = 0
};

@interface NSText : NSView <NSChangeSpelling, NSIgnoreMisspelledWords> {

    id _ivars;
}

- (NSString *)string;
- (void)setString:(NSString *)string;

- (void)replaceCharactersInRange:(NSRange)range withString:(NSString *)aString;
- (void)replaceCharactersInRange:(NSRange)range withRTF:(NSData *)rtfData;
- (void)replaceCharactersInRange:(NSRange)range withRTFD:(NSData *)rtfdData;

- (NSData *)RTFFromRange:(NSRange)range;
- (NSData *)RTFDFromRange:(NSRange)range;

- (BOOL)writeRTFDToFile:(NSString *)path atomically:(BOOL)flag;
- (BOOL)readRTFDFromFile:(NSString *)path;

- (id <NSTextDelegate>)delegate;
- (void)setDelegate:(id <NSTextDelegate>)anObject;

- (BOOL)isEditable;
- (void)setEditable:(BOOL)flag;
- (BOOL)isSelectable;
- (void)setSelectable:(BOOL)flag;
- (BOOL)isRichText;
- (void)setRichText:(BOOL)flag;
- (BOOL)importsGraphics;
- (void)setImportsGraphics:(BOOL)flag;
- (BOOL)isFieldEditor;
- (void)setFieldEditor:(BOOL)flag;
- (BOOL)usesFontPanel;
- (void)setUsesFontPanel:(BOOL)flag;
- (BOOL)drawsBackground;
- (void)setDrawsBackground:(BOOL)flag;
- (NSColor *)backgroundColor;
- (void)setBackgroundColor:(NSColor *)color;

- (BOOL)isRulerVisible;

- (NSRange)selectedRange;
- (void)setSelectedRange:(NSRange)range;

- (void)scrollRangeToVisible:(NSRange)range;

- (void)setFont:(NSFont *)obj;
- (NSFont *)font;
- (void)setTextColor:(NSColor *)color;
- (NSColor *)textColor;
- (NSTextAlignment)alignment;
- (void)setAlignment:(NSTextAlignment)mode;
- (NSWritingDirection)baseWritingDirection;
- (void)setBaseWritingDirection:(NSWritingDirection)writingDirection;

- (void)setTextColor:(NSColor *)color range:(NSRange)range;
- (void)setFont:(NSFont *)font range:(NSRange)range;

- (NSSize)maxSize;
- (void)setMaxSize:(NSSize)newMaxSize;
- (NSSize)minSize;
- (void)setMinSize:(NSSize)newMinSize;

- (BOOL)isHorizontallyResizable;
- (void)setHorizontallyResizable:(BOOL)flag;
- (BOOL)isVerticallyResizable;
- (void)setVerticallyResizable:(BOOL)flag;

- (void)sizeToFit;

- (void)copy:(id)sender;
- (void)copyFont:(id)sender;
- (void)copyRuler:(id)sender;
- (void)cut:(id)sender;
- (void)delete:(id)sender;
- (void)paste:(id)sender;
- (void)pasteFont:(id)sender;
- (void)pasteRuler:(id)sender;
- (void)selectAll:(id)sender;
- (void)changeFont:(id)sender;
- (void)alignLeft:(id)sender;
- (void)alignRight:(id)sender;
- (void)alignCenter:(id)sender;
- (void)subscript:(id)sender;
- (void)superscript:(id)sender;
- (void)underline:(id)sender;
- (void)unscript:(id)sender;
- (void)showGuessPanel:(id)sender;
- (void)checkSpelling:(id)sender;
- (void)toggleRuler:(id)sender;

@end

@protocol NSTextDelegate <NSObject>
@optional
- (BOOL)textShouldBeginEditing:(NSText *)textObject;
- (BOOL)textShouldEndEditing:(NSText *)textObject;
- (void)textDidBeginEditing:(NSNotification *)notification;
- (void)textDidEndEditing:(NSNotification *)notification;
- (void)textDidChange:(NSNotification *)notification;
@end


EXTERN_C NSString *NSTextDidBeginEditingNotification;
EXTERN_C NSString *NSTextDidEndEditingNotification;
EXTERN_C NSString *NSTextDidChangeNotification;
