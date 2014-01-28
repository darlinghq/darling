#import <AppKit/NSButtonCell.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSMenuItem.h>

#if ! __LP64__
@class NSMenuView;
#endif

@interface NSMenuItemCell : NSButtonCell {

    @private
    id _extraData;
    NSSize _stateImageSize;
    NSSize _imageSize;
    NSSize _titleSize;
    NSSize _keyEquivalentSize;
    NSSize _size;
    struct __micFlags {
        unsigned int needsSizing:1;
        unsigned int reserved:1;
        unsigned int needsDisplay:1;
        unsigned int keyEquivGlyphWidth:16;
 unsigned int uniqueAgainstMain:1;
        unsigned int RESERVED:12;
    } _micFlags;
}

- (void)setMenuItem:(NSMenuItem *)item;
- (NSMenuItem *)menuItem;

#if ! __LP64__
- (void)setMenuView:(NSMenuView *)menuView;
- (NSMenuView *)menuView;
#endif

- (void)setNeedsSizing:(BOOL)flag;
- (BOOL)needsSizing;
- (void)calcSize;

- (void)setNeedsDisplay:(BOOL)flag;
- (BOOL)needsDisplay;

- (CGFloat)stateImageWidth;
- (CGFloat)imageWidth;
- (CGFloat)titleWidth;
- (CGFloat)keyEquivalentWidth;

- (NSRect)stateImageRectForBounds:(NSRect)cellFrame;
- (NSRect)titleRectForBounds:(NSRect)cellFrame;
- (NSRect)keyEquivalentRectForBounds:(NSRect)cellFrame;

- (void)drawSeparatorItemWithFrame:(NSRect)cellFrame inView:(NSView *)controlView;
- (void)drawStateImageWithFrame:(NSRect)cellFrame inView:(NSView *)controlView;
- (void)drawImageWithFrame:(NSRect)cellFrame inView:(NSView *)controlView;
- (void)drawTitleWithFrame:(NSRect)cellFrame inView:(NSView *)controlView;
- (void)drawKeyEquivalentWithFrame:(NSRect)cellFrame inView:(NSView *)controlView;
- (void)drawBorderAndBackgroundWithFrame:(NSRect)cellFrame inView:(NSView *)controlView;

- (NSInteger)tag;

@end
