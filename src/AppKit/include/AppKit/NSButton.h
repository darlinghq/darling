#import <AppKit/NSControl.h>
#import <AppKit/NSButtonCell.h>
#import <AppKit/NSUserInterfaceValidation.h>

@class NSSound;

@interface NSButton : NSControl <NSUserInterfaceValidations>

- (NSString *)title;
- (void)setTitle:(NSString *)aString;
- (NSString *)alternateTitle;
- (void)setAlternateTitle:(NSString *)aString;
- (NSImage *)image;
- (void)setImage:(NSImage *)image;
- (NSImage *)alternateImage;
- (void)setAlternateImage:(NSImage *)image;
- (NSCellImagePosition)imagePosition;
- (void)setImagePosition:(NSCellImagePosition)aPosition;
- (void)setButtonType:(NSButtonType)aType;
- (NSInteger)state;
- (void)setState:(NSInteger)value;
- (BOOL)isBordered;
- (void)setBordered:(BOOL)flag;
- (BOOL)isTransparent;
- (void)setTransparent:(BOOL)flag;
- (void)setPeriodicDelay:(float)delay interval:(float)interval;
- (void)getPeriodicDelay:(float *)delay interval:(float *)interval;
- (NSString *)keyEquivalent;
- (void)setKeyEquivalent:(NSString *)charCode;
- (NSUInteger)keyEquivalentModifierMask;
- (void)setKeyEquivalentModifierMask:(NSUInteger)mask;
- (void)highlight:(BOOL)flag;
- (BOOL)performKeyEquivalent:(NSEvent *)key;

@end

@interface NSButton(NSButtonAttributedStringMethods)
- (NSAttributedString *)attributedTitle;
- (void)setAttributedTitle:(NSAttributedString *)aString;
- (NSAttributedString *)attributedAlternateTitle;
- (void)setAttributedAlternateTitle:(NSAttributedString *)obj;
@end

@interface NSButton(NSButtonBezelStyles)
- (void) setBezelStyle:(NSBezelStyle)bezelStyle;
- (NSBezelStyle)bezelStyle;
@end

@interface NSButton(NSButtonMixedState)
- (void)setAllowsMixedState:(BOOL)flag;
- (BOOL)allowsMixedState;
- (void)setNextState;
@end

@interface NSButton(NSButtonBorder)
- (void) setShowsBorderOnlyWhileMouseInside:(BOOL)show;
- (BOOL) showsBorderOnlyWhileMouseInside;
@end

@interface NSButton (NSButtonSoundExtensions)
- (void)setSound:(NSSound *)aSound;
- (NSSound *)sound;
@end


@interface NSButton(NSKeyboardUI)



- (void)setTitleWithMnemonic:(NSString *)stringWithAmpersand;

@end
