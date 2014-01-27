#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSUserInterfaceValidation.h>
#import <AppKit/NSView.h>

@class NSMenu;
@class NSImage, NSAttributedString, NSView;

@interface NSMenuItem : NSObject <NSCopying, NSCoding, NSValidatedUserInterfaceItem>
{

    @private
    NSMenu *_menu;
    NSString *_title;
    NSString *_keyEquivalent;
#if __LP64__
    NSString *_uiid;
    id _repObject;
    NSMenu *_submenu;
    id _extraData;
    id _target;
    SEL _action;
    NSInteger _tag;
    unsigned char _keyEquivalentModifierMask;
#else
    NSUInteger _keyEquivalentModifierMask;
    NSInteger _userKEGenerationCount;
    NSInteger _state;
    NSImage *_image;
    NSMenu *_submenu;
    NSString *_uiid;
    id _repObject;
    id _target;
    SEL _action;
    NSInteger _tag;
    id _extraData;
#endif
    struct __miFlags {
        unsigned int keGenerationCount:8;
        unsigned int disabled:1;
        unsigned int isSeparator:1;
        unsigned int hidden:1;
        unsigned int alternate:1;
        unsigned int moreAlternate:1;
        unsigned int singleAlternate:1;
        unsigned int indent:4;
        unsigned int keShareMode:3;
        unsigned int state:2;
        unsigned int RESERVED1:2;
        unsigned int limitedView:1;
        unsigned int nextItemIsAlternate:1;
        unsigned int blockKE:1;
        unsigned int ignoredForAccessibility:1;
        unsigned int hiddenActiveKE:1;
        unsigned int noRepeatKEs:1;
        unsigned int RESERVED2:1;
    } _miFlags;
}

+ (void)setUsesUserKeyEquivalents:(BOOL)flag;
+ (BOOL)usesUserKeyEquivalents;

+ (NSMenuItem *)separatorItem;

- (id)initWithTitle:(NSString *)aString action:(SEL)aSelector keyEquivalent:(NSString *)charCode;

- (void)setMenu:(NSMenu *)menu;
- (NSMenu *)menu;



- (BOOL)hasSubmenu;
- (void)setSubmenu:(NSMenu *)submenu;
- (NSMenu *)submenu;



- (NSMenuItem *)parentItem;

- (void)setTitle:(NSString *)aString;
- (NSString *)title;
- (void)setAttributedTitle:(NSAttributedString*)string;
- (NSAttributedString*)attributedTitle;

- (BOOL)isSeparatorItem;

- (void)setKeyEquivalent:(NSString *)aKeyEquivalent;
- (NSString *)keyEquivalent;
- (void)setKeyEquivalentModifierMask:(NSUInteger)mask;
- (NSUInteger)keyEquivalentModifierMask;

- (NSString *)userKeyEquivalent;

- (void)setTitleWithMnemonic:(NSString *)stringWithAmpersand;

- (void)setImage:(NSImage *)menuImage;
- (NSImage *)image;

- (void)setState:(NSInteger)state;
- (NSInteger)state;
- (void)setOnStateImage:(NSImage *)image;
- (NSImage *)onStateImage;
- (void)setOffStateImage:(NSImage *)image;
- (NSImage *)offStateImage;
- (void)setMixedStateImage:(NSImage *)image;
- (NSImage *)mixedStateImage;

- (void)setEnabled:(BOOL)flag;
- (BOOL)isEnabled;


- (void)setAlternate:(BOOL)isAlternate;
- (BOOL)isAlternate;

- (void)setIndentationLevel:(NSInteger)indentationLevel;
- (NSInteger)indentationLevel;

- (void)setTarget:(id)anObject;
- (id)target;
- (void)setAction:(SEL)aSelector;
- (SEL)action;

- (void)setTag:(NSInteger)anInt;
- (NSInteger)tag;

- (void)setRepresentedObject:(id)anObject;
- (id)representedObject;
- (void)setView:(NSView *)view;
- (NSView *)view;


- (BOOL)isHighlighted;


- (void)setHidden:(BOOL)hidden;
- (BOOL)isHidden;
- (BOOL)isHiddenOrHasHiddenAncestor;


- (void) setToolTip:(NSString*)toolTip;
- (NSString*)toolTip;

@end

@interface NSView (NSViewEnclosingMenuItem)

- (NSMenuItem *)enclosingMenuItem;
@end


#if defined(__GNUC__) && (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 2)

#endif
@protocol NSMenuItem;


@interface NSMenuItem (NSDeprecated)

- (void)setMnemonicLocation:(NSUInteger)location;
- (NSUInteger)mnemonicLocation;
- (NSString *)mnemonic;

@end
