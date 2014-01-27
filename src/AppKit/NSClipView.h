#import <AppKit/NSView.h>

@class NSColor, NSNotification;

@interface NSClipView : NSView
{

    NSColor *_backgroundColor;
    NSView *_docView;
    NSRect _docRect;
    NSRect _oldDocFrame;
    NSCursor *_cursor;
    id _scrollAnimationHelper;
    struct __cvFlags {
 unsigned int isFlipped:1;
 unsigned int onlyUncovered:1;
 unsigned int reflectScroll:1;
 unsigned int usedByCell:1;
 unsigned int scrollClipTo:1;
 unsigned int noCopyOnScroll:1;
        unsigned int drawsBackground:1;
        unsigned int scrollInProgress:1;
        unsigned int skipRemoveSuperviewCheck:1;
        unsigned int animateCurrentScroll:1;
        unsigned int canAnimateScrolls:1;
        unsigned int nextScrollRelativeToCurrentPosition:1;
        unsigned int viewBoundsChangedOverridden:1;
        unsigned int viewFrameChangedOverridden:1;
        unsigned int documentViewAlignment:4;
        unsigned int redrawnWhileScrolling:1;
        unsigned int dontConstrainScroll:1;
        unsigned int lastAtEdgesState:4;
        unsigned int showOverlayScrollersForScrollStep:1;
        unsigned int scrollerKnobFlashSpecifier:2;
        unsigned int drawsContentShadow:1;
        unsigned int dontConstrainBoundsChange:1;
        unsigned int RESERVED:3;
    } _cvFlags;
}

- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;
- (void)setDrawsBackground:(BOOL)flag;
- (BOOL)drawsBackground;
- (void)setDocumentView:(NSView *)aView;
- (id)documentView;
- (NSRect)documentRect;
- (void)setDocumentCursor:(NSCursor *)anObj;
- (NSCursor *)documentCursor;
- (NSRect)documentVisibleRect;
- (void)viewFrameChanged:(NSNotification *)notification;
- (void)viewBoundsChanged:(NSNotification *)notification;
- (void)setCopiesOnScroll:(BOOL)flag;
- (BOOL)copiesOnScroll;
- (BOOL)autoscroll:(NSEvent *)theEvent;
- (NSPoint)constrainScrollPoint:(NSPoint)newOrigin;
- (void)scrollToPoint:(NSPoint)newOrigin;

@end

@interface NSView(NSClipViewSuperview)
- (void)reflectScrolledClipView:(NSClipView *)aClipView;
- (void)scrollClipView:(NSClipView *)aClipView toPoint:(NSPoint)aPoint;
@end
