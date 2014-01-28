#import <AppKit/NSResponder.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSRange.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSAnimation.h>
#import <AppKit/NSUserInterfaceItemIdentification.h>
#import <AppKit/NSDragging.h>

@class NSBitmapImageRep, NSCursor, NSDraggingSession, NSGraphicsContext, NSImage, NSPasteboard, NSScrollView, NSTextInputContext, NSWindow, NSAttributedString;
@class CIFilter, CALayer, NSDictionary, NSScreen, NSShadow, NSTrackingArea;
@protocol NSDraggingSource;

enum {
    NSViewNotSizable = 0,
    NSViewMinXMargin = 1,
    NSViewWidthSizable = 2,
    NSViewMaxXMargin = 4,
    NSViewMinYMargin = 8,
    NSViewHeightSizable = 16,
    NSViewMaxYMargin = 32
};

enum {
    NSNoBorder = 0,
    NSLineBorder = 1,
    NSBezelBorder = 2,
    NSGrooveBorder = 3
};
typedef NSUInteger NSBorderType;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
enum {

    NSViewLayerContentsRedrawNever = 0,

    NSViewLayerContentsRedrawOnSetNeedsDisplay = 1,

    NSViewLayerContentsRedrawDuringViewResize = 2,

    NSViewLayerContentsRedrawBeforeViewResize = 3
};
#endif
typedef NSInteger NSViewLayerContentsRedrawPolicy;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
enum {
    NSViewLayerContentsPlacementScaleAxesIndependently = 0,
    NSViewLayerContentsPlacementScaleProportionallyToFit = 1,
    NSViewLayerContentsPlacementScaleProportionallyToFill = 2,
    NSViewLayerContentsPlacementCenter = 3,
    NSViewLayerContentsPlacementTop = 4,
    NSViewLayerContentsPlacementTopRight = 5,
    NSViewLayerContentsPlacementRight = 6,
    NSViewLayerContentsPlacementBottomRight = 7,
    NSViewLayerContentsPlacementBottom = 8,
    NSViewLayerContentsPlacementBottomLeft = 9,
    NSViewLayerContentsPlacementLeft = 10,
    NSViewLayerContentsPlacementTopLeft = 11
};
#endif
typedef NSInteger NSViewLayerContentsPlacement;

typedef struct __VFlags {
#ifdef __BIG_ENDIAN__
 unsigned int rotatedFromBase:1;
 unsigned int rotatedOrScaledFromBase:1;
 unsigned int autosizing:6;
 unsigned int autoresizeSubviews:1;
 unsigned int wantsGState:1;
 unsigned int needsDisplay:1;
 unsigned int validGState:1;
 unsigned int newGState:1;
 unsigned int noVerticalAutosizing:1;
 unsigned int frameChangeNotesSuspended:1;
 unsigned int needsFrameChangeNote:1;
 unsigned int focusChangeNotesSuspended:1;
 unsigned int boundsChangeNotesSuspended:1;
 unsigned int needsBoundsChangeNote:1;
 unsigned int removingWithoutInvalidation:1;
 unsigned int isFlipped:1;
 unsigned int needsDisplayForBounds:1;
 unsigned int specialArchiving:1;
 unsigned int ignoreHitTest:1;
 unsigned int retainCount:6;
 unsigned int isOpaque:1;
 unsigned int aboutToResize:1;
#else
 unsigned int aboutToResize:1;
 unsigned int isOpaque:1;
 unsigned int retainCount:6;
 unsigned int ignoreHitTest:1;
 unsigned int specialArchiving:1;
 unsigned int needsDisplayForBounds:1;
 unsigned int isFlipped:1;
 unsigned int removingWithoutInvalidation:1;
 unsigned int needsBoundsChangeNote:1;
 unsigned int boundsChangeNotesSuspended:1;
 unsigned int focusChangeNotesSuspended:1;
 unsigned int needsFrameChangeNote:1;
 unsigned int frameChangeNotesSuspended:1;
 unsigned int noVerticalAutosizing:1;
 unsigned int newGState:1;
 unsigned int validGState:1;
 unsigned int needsDisplay:1;
 unsigned int wantsGState:1;
 unsigned int autoresizeSubviews:1;
 unsigned int autosizing:6;
 unsigned int rotatedOrScaledFromBase:1;
 unsigned int rotatedFromBase:1;
#endif
} _VFlags;

typedef NSInteger NSTrackingRectTag;
typedef NSInteger NSToolTipTag;

@class _NSViewAuxiliary;

@interface NSView : NSResponder <NSAnimatablePropertyContainer, NSUserInterfaceItemIdentification, NSDraggingDestination>
{

    NSRect _frame;
    NSRect _bounds;
    id _superview;
    id _subviews;
    NSWindow *_window;
    id _gState;
    id _frameMatrix;
    CALayer *_layer;
    id _dragTypes;
    _NSViewAuxiliary *_viewAuxiliary;
    _VFlags _vFlags;
    struct __VFlags2 {
        unsigned int nextKeyViewRefCount:14;
        unsigned int previousKeyViewRefCount:14;
        unsigned int isVisibleRect:1;
        unsigned int hasToolTip:1;
        unsigned int cachedIsFlipped:1;
        unsigned int menuWasSet:1;
    } _vFlags2;
}

- (id)initWithFrame:(NSRect)frameRect;

- (NSWindow *)window;
- (NSView *)superview;
- (NSArray *)subviews;
- (BOOL)isDescendantOf:(NSView *)aView;
- (NSView *)ancestorSharedWithView:(NSView *)aView;
- (NSView *)opaqueAncestor;
- (void)setHidden:(BOOL)flag;
- (BOOL)isHidden;
- (BOOL)isHiddenOrHasHiddenAncestor;

- (void)getRectsBeingDrawn:(const NSRect **)rects count:(NSInteger *)count;
- (BOOL)needsToDrawRect:(NSRect)aRect;
- (BOOL)wantsDefaultClipping;
- (void)viewDidHide;
- (void)viewDidUnhide;

- (void)setSubviews:(NSArray *)newSubviews;
- (void)addSubview:(NSView *)aView;
- (void)addSubview:(NSView *)aView positioned:(NSWindowOrderingMode)place relativeTo:(NSView *)otherView;
- (void)sortSubviewsUsingFunction:(NSComparisonResult (*)(id, id, void *))compare context:(void *)context;
- (void)viewWillMoveToWindow:(NSWindow *)newWindow;
- (void)viewDidMoveToWindow;
- (void)viewWillMoveToSuperview:(NSView *)newSuperview;
- (void)viewDidMoveToSuperview;
- (void)didAddSubview:(NSView *)subview;
- (void)willRemoveSubview:(NSView *)subview;
- (void)removeFromSuperview;
- (void)replaceSubview:(NSView *)oldView with:(NSView *)newView;
- (void)removeFromSuperviewWithoutNeedingDisplay;
- (void)viewDidChangeBackingProperties;

- (void)setPostsFrameChangedNotifications:(BOOL)flag;
- (BOOL)postsFrameChangedNotifications;
- (void)resizeSubviewsWithOldSize:(NSSize)oldSize;
- (void)resizeWithOldSuperviewSize:(NSSize)oldSize;
- (void)setAutoresizesSubviews:(BOOL)flag;
- (BOOL)autoresizesSubviews;
- (void)setAutoresizingMask:(NSUInteger)mask;
- (NSUInteger)autoresizingMask;

- (void)setFrameOrigin:(NSPoint)newOrigin;
- (void)setFrameSize:(NSSize)newSize;
- (void)setFrame:(NSRect)frameRect;
- (NSRect)frame;
- (void)setFrameRotation:(CGFloat)angle;
- (CGFloat)frameRotation;
- (void)setFrameCenterRotation:(CGFloat)angle;
- (CGFloat)frameCenterRotation;

- (void)setBoundsOrigin:(NSPoint)newOrigin;
- (void)setBoundsSize:(NSSize)newSize;
- (void)setBoundsRotation:(CGFloat)angle;
- (CGFloat)boundsRotation;
- (void)translateOriginToPoint:(NSPoint)translation;
- (void)scaleUnitSquareToSize:(NSSize)newUnitSize;
- (void)rotateByAngle:(CGFloat)angle;
- (void)setBounds:(NSRect)aRect;
- (NSRect)bounds;

- (BOOL)isFlipped;
- (BOOL)isRotatedFromBase;
- (BOOL)isRotatedOrScaledFromBase;



- (BOOL)isOpaque;

- (NSPoint)convertPoint:(NSPoint)aPoint fromView:(NSView *)aView;
- (NSPoint)convertPoint:(NSPoint)aPoint toView:(NSView *)aView;
- (NSSize)convertSize:(NSSize)aSize fromView:(NSView *)aView;
- (NSSize)convertSize:(NSSize)aSize toView:(NSView *)aView;
- (NSRect)convertRect:(NSRect)aRect fromView:(NSView *)aView;
- (NSRect)convertRect:(NSRect)aRect toView:(NSView *)aView;



- (NSRect)backingAlignedRect:(NSRect)aRect options:(NSAlignmentOptions)options;
- (NSRect)centerScanRect:(NSRect)aRect;



- (NSPoint)convertPointToBacking:(NSPoint)aPoint;
- (NSPoint)convertPointFromBacking:(NSPoint)aPoint;
- (NSSize)convertSizeToBacking:(NSSize)aSize;
- (NSSize)convertSizeFromBacking:(NSSize)aSize;
- (NSRect)convertRectToBacking:(NSRect)aRect;
- (NSRect)convertRectFromBacking:(NSRect)aRect;



- (NSPoint)convertPointToLayer:(NSPoint)aPoint;
- (NSPoint)convertPointFromLayer:(NSPoint)aPoint;
- (NSSize)convertSizeToLayer:(NSSize)aSize;
- (NSSize)convertSizeFromLayer:(NSSize)aSize;
- (NSRect)convertRectToLayer:(NSRect)aRect;
- (NSRect)convertRectFromLayer:(NSRect)aRect;



- (NSPoint)convertPointToBase:(NSPoint)aPoint;
- (NSPoint)convertPointFromBase:(NSPoint)aPoint;
- (NSSize)convertSizeToBase:(NSSize)aSize;
- (NSSize)convertSizeFromBase:(NSSize)aSize;
- (NSRect)convertRectToBase:(NSRect)aRect;
- (NSRect)convertRectFromBase:(NSRect)aRect;



- (BOOL)canDrawConcurrently;


- (void)setCanDrawConcurrently:(BOOL)flag;

- (BOOL)canDraw;
- (void)setNeedsDisplay:(BOOL)flag;
- (void)setNeedsDisplayInRect:(NSRect)invalidRect;
- (BOOL)needsDisplay;
- (void)lockFocus;
- (void)unlockFocus;
- (BOOL)lockFocusIfCanDraw;
- (BOOL)lockFocusIfCanDrawInContext:(NSGraphicsContext *)context;
+ (NSView *)focusView;
- (NSRect)visibleRect;

- (void)display;
- (void)displayIfNeeded;
- (void)displayIfNeededIgnoringOpacity;
- (void)displayRect:(NSRect)rect;
- (void)displayIfNeededInRect:(NSRect)rect;
- (void)displayRectIgnoringOpacity:(NSRect)rect;
- (void)displayIfNeededInRectIgnoringOpacity:(NSRect)rect;
- (void)drawRect:(NSRect)dirtyRect;
- (void)displayRectIgnoringOpacity:(NSRect)aRect inContext:(NSGraphicsContext *)context;

- (NSBitmapImageRep *)bitmapImageRepForCachingDisplayInRect:(NSRect)rect;
- (void)cacheDisplayInRect:(NSRect)rect toBitmapImageRep:(NSBitmapImageRep *)bitmapImageRep;
- (void)viewWillDraw;

- (NSInteger)gState;
- (void)allocateGState;
- (oneway void)releaseGState;
- (void)setUpGState;
- (void)renewGState;

- (void)scrollPoint:(NSPoint)aPoint;
- (BOOL)scrollRectToVisible:(NSRect)aRect;
- (BOOL)autoscroll:(NSEvent *)theEvent;
- (NSRect)adjustScroll:(NSRect)newVisible;
- (void)scrollRect:(NSRect)aRect by:(NSSize)delta;
- (void)translateRectsNeedingDisplayInRect:(NSRect)clipRect by:(NSSize)delta;

- (NSView *)hitTest:(NSPoint)aPoint;
- (BOOL)mouse:(NSPoint)aPoint inRect:(NSRect)aRect;
- (id)viewWithTag:(NSInteger)aTag;
- (NSInteger)tag;
- (BOOL)performKeyEquivalent:(NSEvent *)theEvent;
- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent;
- (BOOL)shouldDelayWindowOrderingForEvent:(NSEvent *)theEvent;
- (BOOL)needsPanelToBecomeKey;
- (BOOL)mouseDownCanMoveWindow;



- (void)setAcceptsTouchEvents:(BOOL)flag;
- (BOOL)acceptsTouchEvents;



- (void)setWantsRestingTouches:(BOOL)flag;
- (BOOL)wantsRestingTouches;

- (void)addCursorRect:(NSRect)aRect cursor:(NSCursor *)anObj;
- (void)removeCursorRect:(NSRect)aRect cursor:(NSCursor *)anObj;
- (void)discardCursorRects;
- (void)resetCursorRects;

- (NSTrackingRectTag)addTrackingRect:(NSRect)aRect owner:(id)anObject userData:(void *)data assumeInside:(BOOL)flag;
- (void)removeTrackingRect:(NSTrackingRectTag)tag;

- (CALayer *)makeBackingLayer;



- (NSViewLayerContentsRedrawPolicy)layerContentsRedrawPolicy;
- (void)setLayerContentsRedrawPolicy:(NSViewLayerContentsRedrawPolicy)newPolicy;

- (NSViewLayerContentsPlacement)layerContentsPlacement;
- (void)setLayerContentsPlacement:(NSViewLayerContentsPlacement)newPlacement;



- (void)setWantsLayer:(BOOL)flag;
- (BOOL)wantsLayer;



- (void)setLayer:(CALayer *)newLayer;
- (CALayer *)layer;



- (BOOL)wantsUpdateLayer;



- (void)updateLayer;

- (void)setAlphaValue:(CGFloat)viewAlpha;
- (CGFloat)alphaValue;

- (void)setBackgroundFilters:(NSArray *)filters;
- (NSArray *)backgroundFilters;

- (void)setCompositingFilter:(CIFilter *)filter;
- (CIFilter *)compositingFilter;

- (void)setContentFilters:(NSArray *)filters;
- (NSArray *)contentFilters;

- (void)setShadow:(NSShadow *)shadow;
- (NSShadow *)shadow;



- (void)addTrackingArea:(NSTrackingArea *)trackingArea;
- (void)removeTrackingArea:(NSTrackingArea *)trackingArea;
- (NSArray *)trackingAreas;



- (void)updateTrackingAreas;


- (BOOL)shouldDrawColor;

- (void)setPostsBoundsChangedNotifications:(BOOL)flag;
- (BOOL)postsBoundsChangedNotifications;

- (NSScrollView *)enclosingScrollView;

- (NSMenu *)menuForEvent:(NSEvent *)event;
+ (NSMenu *)defaultMenu;

- (void)setToolTip:(NSString *)string;
- (NSString *)toolTip;
- (NSToolTipTag)addToolTipRect:(NSRect)aRect owner:(id)anObject userData:(void *)data;
- (void)removeToolTip:(NSToolTipTag)tag;
- (void)removeAllToolTips;



- (void)viewWillStartLiveResize;

- (void)viewDidEndLiveResize;

- (BOOL)inLiveResize;

- (BOOL)preservesContentDuringLiveResize;

- (NSRect)rectPreservedDuringLiveResize;

- (void)getRectsExposedDuringLiveResize:(NSRect[4])exposedRects count:(NSInteger *)count;




- (NSTextInputContext *)inputContext;







- (NSRect)rectForSmartMagnificationAtPoint:(NSPoint)location inRect:(NSRect)visibleRect;

@end

@interface NSObject (NSLayerDelegateContentsScaleUpdating)



- (BOOL)layer:(CALayer *)layer shouldInheritContentsScale:(CGFloat)newScale fromWindow:(NSWindow *)window;

@end

@interface NSObject(NSToolTipOwner)
- (NSString *)view:(NSView *)view stringForToolTip:(NSToolTipTag)tag point:(NSPoint)point userData:(void *)data;
@end

@interface NSView(NSKeyboardUI)
- (void)setNextKeyView:(NSView *)next;
- (NSView *)nextKeyView;
- (NSView *)previousKeyView;
- (NSView *)nextValidKeyView;
- (NSView *)previousValidKeyView;

- (BOOL)canBecomeKeyView;

- (void)setKeyboardFocusRingNeedsDisplayInRect:(NSRect)rect;

- (void)setFocusRingType:(NSFocusRingType)focusRingType;
- (NSFocusRingType)focusRingType;
+ (NSFocusRingType)defaultFocusRingType;

- (void)drawFocusRingMask;
- (NSRect)focusRingMaskBounds;
- (void)noteFocusRingMaskChanged;

@end

@interface NSView(NSPrinting)


- (void)writeEPSInsideRect:(NSRect)rect toPasteboard:(NSPasteboard *)pasteboard;
- (NSData *)dataWithEPSInsideRect:(NSRect)rect;
- (void)writePDFInsideRect:(NSRect)rect toPasteboard:(NSPasteboard *)pasteboard;
- (NSData *)dataWithPDFInsideRect:(NSRect)rect;


- (void)print:(id)sender;


- (BOOL)knowsPageRange:(NSRangePointer)range;
- (CGFloat)heightAdjustLimit;
- (CGFloat)widthAdjustLimit;
- (void)adjustPageWidthNew:(CGFloat *)newRight left:(CGFloat)oldLeft right:(CGFloat)oldRight limit:(CGFloat)rightLimit;
- (void)adjustPageHeightNew:(CGFloat *)newBottom top:(CGFloat)oldTop bottom:(CGFloat)oldBottom limit:(CGFloat)bottomLimit;
- (NSRect)rectForPage:(NSInteger)page;
- (NSPoint)locationOfPrintRect:(NSRect)aRect;
- (void)drawPageBorderWithSize:(NSSize)borderSize;
- (NSAttributedString *)pageHeader;
- (NSAttributedString *)pageFooter;


- (void)drawSheetBorderWithSize:(NSSize)borderSize;



- (NSString *)printJobTitle;
- (void)beginDocument;
- (void)endDocument;

- (void)beginPageInRect:(NSRect)aRect atPlacement:(NSPoint)location;
- (void)endPage;
@end


@interface NSView(NSDrag)


- (NSDraggingSession *)beginDraggingSessionWithItems:(NSArray *)items event:(NSEvent *)event source:(id<NSDraggingSource>)source;



- (void)dragImage:(NSImage *)anImage at:(NSPoint)viewLocation offset:(NSSize)initialOffset event:(NSEvent *)event pasteboard:(NSPasteboard *)pboard source:(id)sourceObj slideBack:(BOOL)slideFlag;

- (NSArray *)registeredDraggedTypes;
- (void)registerForDraggedTypes:(NSArray *)newTypes;
- (void)unregisterDraggedTypes;

- (BOOL)dragFile:(NSString *)filename fromRect:(NSRect)rect slideBack:(BOOL)aFlag event:(NSEvent *)event;
- (BOOL)dragPromisedFilesOfTypes:(NSArray *)typeArray fromRect:(NSRect)rect source:(id)sourceObject slideBack:(BOOL)aFlag event:(NSEvent *)event;
@end

@interface NSView (NSFullScreenMode)
- (BOOL)enterFullScreenMode:(NSScreen *)screen withOptions:(NSDictionary *)options;
- (void)exitFullScreenModeWithOptions:(NSDictionary *)options;
- (BOOL)isInFullScreenMode;
@end


EXTERN_C NSString * const NSFullScreenModeAllScreens;
EXTERN_C NSString * const NSFullScreenModeSetting;
EXTERN_C NSString * const NSFullScreenModeWindowLevel;
EXTERN_C NSString * const NSFullScreenModeApplicationPresentationOptions;

@interface NSView(NSDefinition)






- (void)showDefinitionForAttributedString:(NSAttributedString *)attrString atPoint:(NSPoint)textBaselineOrigin;

#if NS_BLOCKS_AVAILABLE
- (void)showDefinitionForAttributedString:(NSAttributedString *)attrString range:(NSRange)targetRange options:(NSDictionary *)options baselineOriginProvider:(NSPoint (^)(NSRange adjustedRange))originProvider;
#endif

@end



EXTERN_C NSString * const NSDefinitionPresentationTypeKey;
EXTERN_C NSString * const NSDefinitionPresentationTypeOverlay;
EXTERN_C NSString * const NSDefinitionPresentationTypeDictionaryApplication;


@interface NSView(NSFindIndicator)



- (BOOL)isDrawingFindIndicator;

@end

@interface NSView(NSDeprecated)



- (BOOL)performMnemonic:(NSString *)theString;

@end




EXTERN_C NSString *NSViewFrameDidChangeNotification;
EXTERN_C NSString *NSViewFocusDidChangeNotification;
EXTERN_C NSString *NSViewBoundsDidChangeNotification;

EXTERN_C NSString *NSViewGlobalFrameDidChangeNotification;


EXTERN_C NSString *NSViewDidUpdateTrackingAreasNotification;
