#import <Foundation/NSDate.h>
#import <AppKit/NSView.h>
#import <AppKit/NSScroller.h>
#import <AppKit/NSTextFinder.h>

@class NSColor, NSClipView, NSRulerView, NSScroller;

#if MAC_OS_X_VERSION_10_7 <= MAC_OS_X_VERSION_MAX_ALLOWED
enum {
    NSScrollElasticityAutomatic = 0,
    NSScrollElasticityNone = 1,
    NSScrollElasticityAllowed = 2,
};
#endif
typedef NSInteger NSScrollElasticity;

typedef struct __SFlags {
#ifdef __BIG_ENDIAN__
    unsigned int vScrollerRequired:1;
    unsigned int hScrollerRequired:1;
    unsigned int vScrollerStatus:1;
    unsigned int hScrollerStatus:1;
    unsigned int noDynamicScrolling:1;
    NSBorderType borderType:2;
    unsigned int oldRulerInstalled:1;
    unsigned int showRulers:1;
    unsigned int hasHorizontalRuler:1;
    unsigned int hasVerticalRuler:1;
    unsigned int needsTile:1;
    unsigned int doesNotDrawBackground:1;
    unsigned int skipRemoveSuperviewCheck:1;
    unsigned int focusRingNeedsRedisplay:1;
    unsigned int hasCustomLineBorderColor:1;
    unsigned int autohidesScrollers:1;
    unsigned int autoforwardsScrollWheelEvents:1;
    unsigned int findBarVisible:1;
    unsigned int unused:1;
    unsigned int vContentElasticity:2;
    unsigned int hContentElasticity:2;
    unsigned int predominantAxisScrolling:1;
    unsigned int findBarPosition:2;
    unsigned int RESERVED:5;
#else
    unsigned int RESERVED:5;
    unsigned int findBarPosition:2;
    unsigned int predominantAxisScrolling:1;
    unsigned int hContentElasticity:2;
    unsigned int vContentElasticity:2;
    unsigned int unused:1;
    unsigned int findBarVisible:1;
    unsigned int autoforwardsScrollWheelEvents:1;
    unsigned int autohidesScrollers:1;
    unsigned int hasCustomLineBorderColor:1;
    unsigned int focusRingNeedsRedisplay:1;
    unsigned int skipRemoveSuperviewCheck:1;
    unsigned int doesNotDrawBackground:1;
    unsigned int needsTile:1;
    unsigned int hasVerticalRuler:1;
    unsigned int hasHorizontalRuler:1;
    unsigned int showRulers:1;
    unsigned int oldRulerInstalled:1;
    NSBorderType borderType:2;
    unsigned int noDynamicScrolling:1;
    unsigned int hScrollerStatus:1;
    unsigned int vScrollerStatus:1;
    unsigned int hScrollerRequired:1;
    unsigned int vScrollerRequired:1;
#endif
} _SFlags;

@interface NSScrollView : NSView <NSTextFinderBarContainer>
{

    NSScroller *_vScroller;
    NSScroller *_hScroller;
    NSClipView *_contentView;
    NSClipView *_headerClipView;
    NSView * _cornerView;
    id _ruler;
    _SFlags _sFlags;
    __strong void *_extraIvars;


    NSRulerView *_horizontalRuler;
    NSRulerView *_verticalRuler;
}



+ (NSSize)frameSizeForContentSize:(NSSize)cSize horizontalScrollerClass:(Class)horizontalScrollerClass verticalScrollerClass:(Class)verticalScrollerClass borderType:(NSBorderType)aType controlSize:(NSControlSize)controlSize scrollerStyle:(NSScrollerStyle)scrollerStyle;



+ (NSSize)contentSizeForFrameSize:(NSSize)fSize horizontalScrollerClass:(Class)horizontalScrollerClass verticalScrollerClass:(Class)verticalScrollerClass borderType:(NSBorderType)aType controlSize:(NSControlSize)controlSize scrollerStyle:(NSScrollerStyle)scrollerStyle;



+ (NSSize)frameSizeForContentSize:(NSSize)cSize hasHorizontalScroller:(BOOL)hFlag hasVerticalScroller:(BOOL)vFlag borderType:(NSBorderType)aType;



+ (NSSize)contentSizeForFrameSize:(NSSize)fSize hasHorizontalScroller:(BOOL)hFlag hasVerticalScroller:(BOOL)vFlag borderType:(NSBorderType)aType;

- (NSRect)documentVisibleRect;
- (NSSize)contentSize;

- (void)setDocumentView:(NSView *)aView;
- (id)documentView;
- (void)setContentView:(NSClipView *)contentView;
- (NSClipView *)contentView;
- (void)setDocumentCursor:(NSCursor *)anObj;
- (NSCursor *)documentCursor;
- (void)setBorderType:(NSBorderType)aType;
- (NSBorderType)borderType;
- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;
- (void)setDrawsBackground:(BOOL)flag;
- (BOOL)drawsBackground;
- (void)setHasVerticalScroller:(BOOL)flag;
- (BOOL)hasVerticalScroller;
- (void)setHasHorizontalScroller:(BOOL)flag;
- (BOOL)hasHorizontalScroller;
- (void)setVerticalScroller:(NSScroller *)anObject;
- (NSScroller *)verticalScroller;
- (void)setHorizontalScroller:(NSScroller *)anObject;
- (NSScroller *)horizontalScroller;
- (BOOL)autohidesScrollers;
- (void)setAutohidesScrollers:(BOOL)flag;
- (void)setHorizontalLineScroll:(CGFloat)value;
- (void)setVerticalLineScroll:(CGFloat)value;
- (void)setLineScroll:(CGFloat)value;
- (CGFloat)horizontalLineScroll;
- (CGFloat)verticalLineScroll;
- (CGFloat)lineScroll;
- (void)setHorizontalPageScroll:(CGFloat)value;
- (void)setVerticalPageScroll:(CGFloat)value;
- (void)setPageScroll:(CGFloat)value;
- (CGFloat)horizontalPageScroll;
- (CGFloat)verticalPageScroll;
- (CGFloat)pageScroll;
- (void)setScrollsDynamically:(BOOL)flag;
- (BOOL)scrollsDynamically;
- (void)tile;
- (void)reflectScrolledClipView:(NSClipView *)cView;
- (void)scrollWheel:(NSEvent *)theEvent;



- (NSScrollerStyle)scrollerStyle;
- (void)setScrollerStyle:(NSScrollerStyle)newScrollerStyle;



- (NSScrollerKnobStyle)scrollerKnobStyle;
- (void)setScrollerKnobStyle:(NSScrollerKnobStyle)newScrollerKnobStyle;



- (void)flashScrollers;



- (NSScrollElasticity)horizontalScrollElasticity;
- (void)setHorizontalScrollElasticity:(NSScrollElasticity)elasticity;



- (NSScrollElasticity)verticalScrollElasticity;
- (void)setVerticalScrollElasticity:(NSScrollElasticity)elasticity;



- (BOOL)usesPredominantAxisScrolling;
- (void)setUsesPredominantAxisScrolling:(BOOL)predominantAxisScrolling;



@property BOOL allowsMagnification;


@property CGFloat magnification;



@property CGFloat maxMagnification;



@property CGFloat minMagnification;



- (void)magnifyToFitRect:(NSRect)rect;



- (void)setMagnification:(CGFloat)magnification centeredAtPoint:(NSPoint)point;

@end



EXTERN_C NSString * const NSScrollViewWillStartLiveMagnifyNotification;



EXTERN_C NSString * const NSScrollViewDidEndLiveMagnifyNotification;


@interface NSScrollView(NSRulerSupport)

+ (void)setRulerViewClass:(Class)rulerViewClass;
+ (Class)rulerViewClass;

- (void)setRulersVisible:(BOOL)flag;
- (BOOL)rulersVisible;

- (void)setHasHorizontalRuler:(BOOL)flag;
- (BOOL)hasHorizontalRuler;
- (void)setHasVerticalRuler:(BOOL)flag;
- (BOOL)hasVerticalRuler;

- (void)setHorizontalRulerView:(NSRulerView *)ruler;
- (NSRulerView *)horizontalRulerView;
- (void)setVerticalRulerView:(NSRulerView *)ruler;
- (NSRulerView *)verticalRulerView;

@end

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
enum {
    NSScrollViewFindBarPositionAboveHorizontalRuler = 0,
    NSScrollViewFindBarPositionAboveContent = 1,
    NSScrollViewFindBarPositionBelowContent = 2
};
#endif
typedef NSInteger NSScrollViewFindBarPosition;

@interface NSScrollView(NSFindBarSupport)

- (void)setFindBarPosition:(NSScrollViewFindBarPosition)position;
- (NSScrollViewFindBarPosition)findBarPosition;

@end
