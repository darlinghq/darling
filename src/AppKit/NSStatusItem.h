#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSAttributedString;
@class NSImage;
@class NSMenu;
@class NSStatusBar;
@class NSString;
@class NSView;
@class NSWindow;


@interface NSStatusItem : NSObject
{
 @private
    NSStatusBar* _fStatusBar;
    CGFloat _fLength;
    NSWindow* _fWindow;
    NSView* _fView;
    int _fPriority;
    struct {
 unsigned int customView:1;
 unsigned int highlightMode:1;
 unsigned int hasAlternateImage:1;
 unsigned int hidden:1;
 unsigned int backgroundStyle:4;
 unsigned int inAdjustLength:1;
 unsigned int reserved:23;
    } _fFlags;
    id _statusItemMenu;
    id _fReserved2;
    id _fReserved3;
    id _fReserved4;
}

- (NSStatusBar* )statusBar;

- (CGFloat)length;
- (void)setLength:(CGFloat)length;

@end

@interface NSStatusItem (NSStatusItemCommon)

- (SEL)action;
- (void)setAction:(SEL)action;

- (SEL)doubleAction;
- (void)setDoubleAction:(SEL)action;

- (id)target;
- (void)setTarget:(id)target;

- (NSString* )title;
- (void)setTitle:(NSString*)title;

- (NSAttributedString* )attributedTitle;
- (void)setAttributedTitle:(NSAttributedString*)title;

- (NSImage* )image;
- (void)setImage:(NSImage*)image;

- (NSImage *)alternateImage;
- (void)setAlternateImage:(NSImage*)image;

- (NSMenu* )menu;
- (void)setMenu:(NSMenu*)menu;

- (BOOL)isEnabled;
- (void)setEnabled:(BOOL)enabled;

- (NSString* )toolTip;
- (void)setToolTip:(NSString*)toolTip;

- (void)setHighlightMode:(BOOL)highlightMode;
- (BOOL)highlightMode;

- (NSInteger)sendActionOn:(NSInteger)mask;

- (void)popUpStatusItemMenu:(NSMenu*)menu;
- (void)drawStatusBarBackgroundInRect:(NSRect)rect withHighlight:(BOOL)highlight;

@end

@interface NSStatusItem (NSStatusItemView)

- (NSView* )view;
- (void)setView:(NSView*)view;

@end
