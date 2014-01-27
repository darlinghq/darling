#import <AppKit/NSView.h>

@class NSFont;

enum {
    NSNoTitle = 0,
    NSAboveTop = 1,
    NSAtTop = 2,
    NSBelowTop = 3,
    NSAboveBottom = 4,
    NSAtBottom = 5,
    NSBelowBottom = 6
};
typedef NSUInteger NSTitlePosition;

enum {
    NSBoxPrimary = 0,
    NSBoxSecondary = 1,
    NSBoxSeparator = 2,
    NSBoxOldStyle = 3,
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
    NSBoxCustom = 4
#endif
};
typedef NSUInteger NSBoxType;

@interface NSBox : NSView
{

    id _titleCell;
    id _contentView;
    NSSize _offsets;
    NSRect _borderRect;
    NSRect _titleRect;
    struct __bFlags {
        NSBorderType borderType:2;
        NSTitlePosition titlePosition:3;
        unsigned int backgroundTransparent:1;
        unsigned int orientation:2;
        unsigned int needsTile:1;
        unsigned int transparent:1;
        unsigned int colorAltInterpretation:1;
        unsigned int boxType:3;
        unsigned int useSuperAddSubview:1;
        unsigned int _RESERVED:17;
    } _bFlags;
    id _unused;
}

- (NSBorderType)borderType;
- (NSTitlePosition)titlePosition;
- (void)setBorderType:(NSBorderType)aType;
- (void)setBoxType:(NSBoxType)boxType;
- (NSBoxType)boxType;
- (void)setTitlePosition:(NSTitlePosition)aPosition;
- (NSString *)title;
- (void)setTitle:(NSString *)aString;
- (NSFont *)titleFont;
- (void)setTitleFont:(NSFont *)fontObj;
- (NSRect)borderRect;
- (NSRect)titleRect;
- (id)titleCell;
- (void)sizeToFit;
- (NSSize)contentViewMargins;
- (void)setContentViewMargins:(NSSize)offsetSize;
- (void)setFrameFromContentFrame:(NSRect)contentFrame;
- (id)contentView;
- (void)setContentView:(NSView *)aView;


- (BOOL)isTransparent;
- (void)setTransparent:(BOOL)flag;

@end

@interface NSBox (NSCustomBoxTypeProperties)



- (CGFloat)borderWidth;
- (void)setBorderWidth:(CGFloat)borderWidth;

- (CGFloat)cornerRadius;
- (void)setCornerRadius:(CGFloat)cornerRadius;

- (NSColor *)borderColor;
- (void)setBorderColor:(NSColor *)borderColor;

- (NSColor *)fillColor;
- (void)setFillColor:(NSColor *)fillColor;

@end

@interface NSBox(NSKeyboardUI)


- (void)setTitleWithMnemonic:(NSString *)stringWithAmpersand;
@end
