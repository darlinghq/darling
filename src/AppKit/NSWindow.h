#import <Foundation/NSGeometry.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSResponder.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/AppKitDefines.h>
#import <Foundation/NSDate.h>
#import <AppKit/NSUserInterfaceValidation.h>
#import <AppKit/NSUserInterfaceItemIdentification.h>
#import <AppKit/NSAnimation.h>

@class NSButton, NSButtonCell, NSColor, NSImage, NSPasteboard, NSScreen;
@class NSNotification, NSText, NSView, NSMutableSet, NSSet, NSDate;
@class NSToolbar, NSGraphicsContext, NSURL, NSColorSpace;
@class NSDockTile;

@protocol NSWindowDelegate;

#define NSAppKitVersionNumberWithCustomSheetPosition 686.0
#define NSAppKitVersionNumberWithDeferredWindowDisplaySupport 1019.0

enum {
    NSBorderlessWindowMask = 0,
    NSTitledWindowMask = 1 << 0,
    NSClosableWindowMask = 1 << 1,
    NSMiniaturizableWindowMask = 1 << 2,
    NSResizableWindowMask = 1 << 3

};



enum {
    NSTexturedBackgroundWindowMask = 1 << 8
};



enum {
    NSUnscaledWindowMask = 1 << 11
};



enum {
    NSUnifiedTitleAndToolbarWindowMask = 1 << 12
};


#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
enum {
    NSFullScreenWindowMask = 1 << 14
};
#endif



enum {
    NSDisplayWindowRunLoopOrdering = 600000,
    NSResetCursorRectsRunLoopOrdering = 700000
};

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
enum {
 NSWindowSharingNone = 0,
 NSWindowSharingReadOnly = 1,
 NSWindowSharingReadWrite = 2
};
#endif
typedef NSUInteger NSWindowSharingType;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
enum {
 NSWindowBackingLocationDefault = 0,
 NSWindowBackingLocationVideoMemory = 1,
 NSWindowBackingLocationMainMemory = 2
};
#endif
typedef NSUInteger NSWindowBackingLocation;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
enum {
  NSWindowCollectionBehaviorDefault = 0,
  NSWindowCollectionBehaviorCanJoinAllSpaces = 1 << 0,
  NSWindowCollectionBehaviorMoveToActiveSpace = 1 << 1
};
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6

enum {
 NSWindowCollectionBehaviorManaged = 1 << 2,
 NSWindowCollectionBehaviorTransient = 1 << 3,
 NSWindowCollectionBehaviorStationary = 1 << 4,
};


enum {
  NSWindowCollectionBehaviorParticipatesInCycle = 1 << 5,
  NSWindowCollectionBehaviorIgnoresCycle = 1 << 6
};
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7

enum {
    NSWindowCollectionBehaviorFullScreenPrimary = 1 << 7,
    NSWindowCollectionBehaviorFullScreenAuxiliary = 1 << 8
};
#endif


typedef NSUInteger NSWindowCollectionBehavior;


#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
enum {
    NSWindowAnimationBehaviorDefault = 0,
    NSWindowAnimationBehaviorNone = 2,

    NSWindowAnimationBehaviorDocumentWindow = 3,
    NSWindowAnimationBehaviorUtilityWindow = 4,
    NSWindowAnimationBehaviorAlertPanel = 5
};
#endif
typedef NSInteger NSWindowAnimationBehavior;


#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6

enum {
    NSWindowNumberListAllApplications = 1 << 0,
    NSWindowNumberListAllSpaces = 1 << 4
};
#endif

typedef NSUInteger NSWindowNumberListOptions;

#define NSNormalWindowLevel kCGNormalWindowLevel
#define NSFloatingWindowLevel kCGFloatingWindowLevel
#define NSSubmenuWindowLevel kCGTornOffMenuWindowLevel
#define NSTornOffMenuWindowLevel kCGTornOffMenuWindowLevel
#define NSMainMenuWindowLevel kCGMainMenuWindowLevel
#define NSStatusWindowLevel kCGStatusWindowLevel

#define NSDockWindowLevel kCGDockWindowLevel
#define NSModalPanelWindowLevel kCGModalPanelWindowLevel
#define NSPopUpMenuWindowLevel kCGPopUpMenuWindowLevel
#define NSScreenSaverWindowLevel kCGScreenSaverWindowLevel


enum {
    NSDirectSelection = 0,
    NSSelectingNext,
    NSSelectingPrevious
};
typedef NSUInteger NSSelectionDirection;


enum {
    NSWindowCloseButton,
    NSWindowMiniaturizeButton,
    NSWindowZoomButton,
    NSWindowToolbarButton,
    NSWindowDocumentIconButton
};

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
enum {
    NSWindowDocumentVersionsButton = 6,
    NSWindowFullScreenButton
};
#endif

typedef NSUInteger NSWindowButton;

@class NSWindowAuxiliary;
@class NSEvent;
@class NSWindowController;

@interface NSWindow : NSResponder <NSAnimatablePropertyContainer, NSUserInterfaceValidations, NSUserInterfaceItemIdentification>
{

    NSRect _frame;
    id _contentView;
    id _delegate;
    NSResponder *_firstResponder;
    NSView *_lastLeftHit;
    NSView *_lastRightHit;
    id _counterpart;
    id _fieldEditor;
    int _winEventMask;
    NSInteger _windowNum;
    int _level;
    NSColor *_backgroundColor;
    id _borderView;
    unsigned char _postingDisabled;
    unsigned char _styleMask;
    unsigned char _flushDisabled;
    unsigned char _reservedWindow1;
    void *_cursorRects;
    void *_trectTable;
    NSImage *_miniIcon;
    int32_t _bamboo;
    NSMutableSet *_dragTypes;
    NSURL *_representedURL;
    NSSize *_sizeLimits;
    NSString *_frameSaveName;
    NSSet *_regDragTypes;
    struct __wFlags {
        unsigned int backing:2;
        unsigned int visible:1;
        unsigned int isMainWindow:1;
        unsigned int isKeyWindow:1;
        unsigned int hidesOnDeactivate:1;
        unsigned int dontFreeWhenClosed:1;
        unsigned int oneShot:1;
        unsigned int deferred:1;
        unsigned int cursorRectsDisabled:1;
        unsigned int haveFreeCursorRects:1;
        unsigned int validCursorRects:1;
        unsigned int docEdited:1;
        unsigned int dynamicDepthLimit:1;
        unsigned int worksWhenModal:1;
        unsigned int limitedBecomeKey:1;
        unsigned int needsFlush:1;
        unsigned int viewsNeedDisplay:1;
        unsigned int ignoredFirstMouse:1;
        unsigned int repostedFirstMouse:1;
        unsigned int windowDying:1;
        unsigned int tempHidden:1;
        unsigned int floatingPanel:1;
        unsigned int wantsToBeOnMainScreen:1;
        unsigned int optimizedDrawingOk:1;
        unsigned int optimizeDrawing:1;
        unsigned int titleIsRepresentedFilename:1;
        unsigned int excludedFromWindowsMenu:1;
        unsigned int depthLimit:4;
        unsigned int delegateReturnsValidRequestor:1;
        unsigned int lmouseupPending:1;
        unsigned int rmouseupPending:1;
        unsigned int wantsToDestroyRealWindow:1;
        unsigned int wantsToRegDragTypes:1;
        unsigned int sentInvalidateCursorRectsMsg:1;
        unsigned int avoidsActivation:1;
        unsigned int frameSavedUsingTitle:1;
        unsigned int didRegDragTypes:1;
        unsigned int delayedOneShot:1;
        unsigned int postedNeedsDisplayNote:1;
        unsigned int postedInvalidCursorRectsNote:1;
        unsigned int initialFirstResponderTempSet:1;
        unsigned int autodisplay:1;
        unsigned int tossedFirstEvent:1;
        unsigned int isImageCache:1;
        unsigned int _unused:3;
        unsigned int keyViewSelectionDirection:2;
        unsigned int defaultButtonCellKETemporarilyDisabled:1;
        unsigned int defaultButtonCellKEDisabled:1;
        unsigned int menuHasBeenSet:1;
        unsigned int wantsToBeModal:1;
        unsigned int showingModalFrame:1;
        unsigned int isTerminating:1;
        unsigned int win32MouseActivationInProgress:1;
        unsigned int makingFirstResponderForMouseDown:1;
        unsigned int needsZoom:1;
        unsigned int sentWindowNeedsDisplayMsg:1;
        unsigned int liveResizeActive:1;
    } _wFlags;
    id _defaultButtonCell;
    NSView *_initialFirstResponder;
    NSWindowAuxiliary *_auxiliaryStorage;
}

+ (NSRect)frameRectForContentRect:(NSRect)cRect styleMask:(NSUInteger)aStyle;
+ (NSRect)contentRectForFrameRect:(NSRect)fRect styleMask:(NSUInteger)aStyle;
+ (CGFloat)minFrameWidthWithTitle:(NSString *)aTitle styleMask:(NSUInteger)aStyle;
+ (NSWindowDepth)defaultDepthLimit;

- (NSRect)frameRectForContentRect:(NSRect)contentRect;
- (NSRect)contentRectForFrameRect:(NSRect)frameRect;

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag;
- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag screen:(NSScreen *)screen;

- (NSString *)title;
- (void)setTitle:(NSString *)aString;




- (void)setRepresentedURL:(NSURL *)url;
- (NSURL *)representedURL;
- (NSString *)representedFilename;
- (void)setRepresentedFilename:(NSString *)aString;
- (void)setTitleWithRepresentedFilename:(NSString *)filename;
- (void)setExcludedFromWindowsMenu:(BOOL)flag;
- (BOOL)isExcludedFromWindowsMenu;
- (void)setContentView:(NSView *)aView;
- (id)contentView;
- (void)setDelegate:(id <NSWindowDelegate>)anObject;
- (id <NSWindowDelegate>)delegate;
- (NSInteger)windowNumber;
- (NSUInteger)styleMask;

- (void)setStyleMask:(NSUInteger)styleMask;
- (NSText *)fieldEditor:(BOOL)createFlag forObject:(id)anObject;
- (void)endEditingFor:(id)anObject;

- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen *)screen;
- (void)setFrame:(NSRect)frameRect display:(BOOL)flag;
- (void)setContentSize:(NSSize)aSize;
- (void)setFrameOrigin:(NSPoint)aPoint;
- (void)setFrameTopLeftPoint:(NSPoint)aPoint;
- (NSPoint)cascadeTopLeftFromPoint:(NSPoint)topLeftPoint;
- (NSRect)frame;




- (NSTimeInterval)animationResizeTime:(NSRect)newFrame;


- (void)setFrame:(NSRect)frameRect display:(BOOL)displayFlag animate:(BOOL)animateFlag;

- (BOOL)inLiveResize;


- (void)setShowsResizeIndicator:(BOOL)show;
- (BOOL)showsResizeIndicator;

- (void)setResizeIncrements:(NSSize)increments;
- (NSSize)resizeIncrements;
- (void)setAspectRatio:(NSSize)ratio;
- (NSSize)aspectRatio;

- (void)setContentResizeIncrements:(NSSize)increments;
- (NSSize)contentResizeIncrements;
- (void)setContentAspectRatio:(NSSize)ratio;
- (NSSize)contentAspectRatio;

- (void)useOptimizedDrawing:(BOOL)flag;
- (void)disableFlushWindow;
- (void)enableFlushWindow;
- (BOOL)isFlushWindowDisabled;
- (void)flushWindow;
- (void)flushWindowIfNeeded;
- (void)setViewsNeedDisplay:(BOOL)flag;
- (BOOL)viewsNeedDisplay;
- (void)displayIfNeeded;
- (void)display;
- (void)setAutodisplay:(BOOL)flag;
- (BOOL)isAutodisplay;

- (BOOL)preservesContentDuringLiveResize;
- (void)setPreservesContentDuringLiveResize:(BOOL)flag;

- (void)update;

- (BOOL)makeFirstResponder:(NSResponder *)aResponder;

- (NSResponder *)firstResponder;

- (NSInteger)resizeFlags;
- (void)keyDown:(NSEvent *)theEvent;
- (void)close;
- (void)setReleasedWhenClosed:(BOOL)flag;
- (BOOL)isReleasedWhenClosed;
- (void)miniaturize:(id)sender;
- (void)deminiaturize:(id)sender;
- (BOOL)isZoomed;
- (void)zoom:(id)sender;
- (BOOL)isMiniaturized;
- (BOOL)tryToPerform:(SEL)anAction with:(id)anObject;
- (id)validRequestorForSendType:(NSString *)sendType returnType:(NSString *)returnType;
- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;



- (void)setContentBorderThickness:(CGFloat)thickness forEdge:(NSRectEdge)edge;
- (CGFloat)contentBorderThicknessForEdge:(NSRectEdge)edge;

- (void)setAutorecalculatesContentBorderThickness:(BOOL)flag forEdge:(NSRectEdge)edge;
- (BOOL)autorecalculatesContentBorderThicknessForEdge:(NSRectEdge)edge;


- (void)setMovable:(BOOL)flag;
- (BOOL)isMovable;

- (void)setMovableByWindowBackground:(BOOL)flag;
- (BOOL)isMovableByWindowBackground;

- (void)setHidesOnDeactivate:(BOOL)flag;
- (BOOL)hidesOnDeactivate;


- (void)setCanHide:(BOOL)flag;
- (BOOL)canHide;

- (void)center;
- (void)makeKeyAndOrderFront:(id)sender;
- (void)orderFront:(id)sender;
- (void)orderBack:(id)sender;
- (void)orderOut:(id)sender;
- (void)orderWindow:(NSWindowOrderingMode)place relativeTo:(NSInteger)otherWin;
- (void)orderFrontRegardless;

- (void)setMiniwindowImage:(NSImage *)image;
- (void)setMiniwindowTitle:(NSString *)title;
- (NSImage *)miniwindowImage;
- (NSString *)miniwindowTitle;

- (NSDockTile *)dockTile;

- (void)setDocumentEdited:(BOOL)flag;
- (BOOL)isDocumentEdited;
- (BOOL)isVisible;
- (BOOL)isKeyWindow;
- (BOOL)isMainWindow;
- (BOOL)canBecomeKeyWindow;
- (BOOL)canBecomeMainWindow;
- (void)makeKeyWindow;
- (void)makeMainWindow;
- (void)becomeKeyWindow;
- (void)resignKeyWindow;
- (void)becomeMainWindow;
- (void)resignMainWindow;

- (BOOL)worksWhenModal;


- (BOOL)preventsApplicationTerminationWhenModal;
- (void)setPreventsApplicationTerminationWhenModal:(BOOL)flag;


- (NSRect)convertRectToScreen:(NSRect)aRect;
- (NSRect)convertRectFromScreen:(NSRect)aRect;


- (NSRect)convertRectToBacking:(NSRect)aRect;
- (NSRect)convertRectFromBacking:(NSRect)aRect;


- (NSRect)backingAlignedRect:(NSRect)aRect options:(NSAlignmentOptions)options;


- (CGFloat)backingScaleFactor;


- (NSPoint)convertBaseToScreen:(NSPoint)aPoint;
- (NSPoint)convertScreenToBase:(NSPoint)aPoint;

- (void)performClose:(id)sender;
- (void)performMiniaturize:(id)sender;
- (void)performZoom:(id)sender;
- (NSInteger)gState;
- (void)setOneShot:(BOOL)flag;
- (BOOL)isOneShot;
- (NSData *)dataWithEPSInsideRect:(NSRect)rect;
- (NSData *)dataWithPDFInsideRect:(NSRect)rect;
- (void)print:(id)sender;

- (void)disableCursorRects;
- (void)enableCursorRects;
- (void)discardCursorRects;
- (BOOL)areCursorRectsEnabled;
- (void)invalidateCursorRectsForView:(NSView *)aView;
- (void)resetCursorRects;

- (void)setAllowsToolTipsWhenApplicationIsInactive:(BOOL)allowWhenInactive;


- (BOOL)allowsToolTipsWhenApplicationIsInactive;


- (void)setBackingType:(NSBackingStoreType)bufferingType;
- (NSBackingStoreType)backingType;
- (void)setLevel:(NSInteger)newLevel;
- (NSInteger)level;
- (void)setDepthLimit:(NSWindowDepth)limit;
- (NSWindowDepth)depthLimit;
- (void)setDynamicDepthLimit:(BOOL)flag;
- (BOOL)hasDynamicDepthLimit;
- (NSScreen *)screen;
- (NSScreen *)deepestScreen;
- (BOOL)canStoreColor;
- (void)setHasShadow:(BOOL)hasShadow;
- (BOOL)hasShadow;
- (void)invalidateShadow;
- (void)setAlphaValue:(CGFloat)windowAlpha;
- (CGFloat)alphaValue;
- (void)setOpaque:(BOOL)isOpaque;
- (BOOL)isOpaque;




- (void)setSharingType:(NSWindowSharingType)type;
- (NSWindowSharingType)sharingType;



- (void)setPreferredBackingLocation:(NSWindowBackingLocation)backingLocation;


- (NSWindowBackingLocation)preferredBackingLocation;


- (NSWindowBackingLocation)backingLocation;




- (BOOL)allowsConcurrentViewDrawing;



- (void)setAllowsConcurrentViewDrawing:(BOOL)flag;

- (BOOL)displaysWhenScreenProfileChanges;
- (void)setDisplaysWhenScreenProfileChanges:(BOOL)flag;

- (void)disableScreenUpdatesUntilFlush;



- (BOOL)canBecomeVisibleWithoutLogin;
- (void)setCanBecomeVisibleWithoutLogin:(BOOL)flag;


- (void)setCollectionBehavior:(NSWindowCollectionBehavior)behavior;
- (NSWindowCollectionBehavior)collectionBehavior;




- (NSWindowAnimationBehavior)animationBehavior;
- (void)setAnimationBehavior:(NSWindowAnimationBehavior)newAnimationBehavior;




-(BOOL)canBeVisibleOnAllSpaces;
-(void)setCanBeVisibleOnAllSpaces:(BOOL)flag;


- (BOOL)isOnActiveSpace;

- (void)toggleFullScreen:(id)sender;

- (NSString *)stringWithSavedFrame;
- (void)setFrameFromString:(NSString *)string;
- (void)saveFrameUsingName:(NSString *)name;

- (BOOL)setFrameUsingName:(NSString *)name force:(BOOL)force;
- (BOOL)setFrameUsingName:(NSString *)name;
- (BOOL)setFrameAutosaveName:(NSString *)name;
- (NSString *)frameAutosaveName;
+ (void)removeFrameUsingName:(NSString *)name;


- (void)cacheImageInRect:(NSRect)aRect;
- (void)restoreCachedImage;
- (void)discardCachedImage;

- (NSSize)minSize;
- (NSSize)maxSize;
- (void)setMinSize:(NSSize)size;
- (void)setMaxSize:(NSSize)size;
- (NSSize)contentMinSize;
- (NSSize)contentMaxSize;
- (void)setContentMinSize:(NSSize)size;
- (void)setContentMaxSize:(NSSize)size;
- (NSEvent *)nextEventMatchingMask:(NSUInteger)mask;
- (NSEvent *)nextEventMatchingMask:(NSUInteger)mask untilDate:(NSDate *)expiration inMode:(NSString *)mode dequeue:(BOOL)deqFlag;
- (void)discardEventsMatchingMask:(NSUInteger)mask beforeEvent:(NSEvent *)lastEvent;
- (void)postEvent:(NSEvent *)event atStart:(BOOL)flag;
- (NSEvent *)currentEvent;
- (void)setAcceptsMouseMovedEvents:(BOOL)flag;
- (BOOL)acceptsMouseMovedEvents;
- (void)setIgnoresMouseEvents:(BOOL)flag;
- (BOOL)ignoresMouseEvents;
- (NSDictionary *)deviceDescription;
- (void)sendEvent:(NSEvent *)theEvent;
- (NSPoint)mouseLocationOutsideOfEventStream;
+ (void)menuChanged:(NSMenu *)menu;

- (id)windowController;
- (void)setWindowController:(NSWindowController *)windowController;

- (BOOL)isSheet;
- (NSWindow *)attachedSheet;

+ (NSButton *)standardWindowButton:(NSWindowButton)b forStyleMask:(NSUInteger)styleMask;
- (NSButton *)standardWindowButton:(NSWindowButton)b;

- (void)addChildWindow:(NSWindow *)childWin ordered:(NSWindowOrderingMode)place;
- (void)removeChildWindow:(NSWindow *)childWin;
- (NSArray *)childWindows;

- (NSWindow *)parentWindow;
- (void)setParentWindow:(NSWindow *)window;



- (NSGraphicsContext *)graphicsContext;




- (CGFloat)userSpaceScaleFactor;


- (void)setColorSpace:(NSColorSpace *)colorSpace;
- (NSColorSpace *)colorSpace;
+ (NSArray *)windowNumbersWithOptions:(NSWindowNumberListOptions)options;



+ (NSInteger)windowNumberAtPoint:(NSPoint)point belowWindowWithWindowNumber:(NSInteger)windowNumber;

@end

@interface NSWindow(NSKeyboardUI)
- (void)setInitialFirstResponder:(NSView *)view;
- (NSView *)initialFirstResponder;
- (void)selectNextKeyView:(id)sender;
- (void)selectPreviousKeyView:(id)sender;
- (void)selectKeyViewFollowingView:(NSView *)aView;
- (void)selectKeyViewPrecedingView:(NSView *)aView;
- (NSSelectionDirection)keyViewSelectionDirection;
- (void)setDefaultButtonCell:(NSButtonCell *)defButt;
- (NSButtonCell *)defaultButtonCell;
- (void)disableKeyEquivalentForDefaultButtonCell;
- (void)enableKeyEquivalentForDefaultButtonCell;
- (void)setAutorecalculatesKeyViewLoop:(BOOL)flag;
- (BOOL)autorecalculatesKeyViewLoop;
- (void)recalculateKeyViewLoop;
@end

@interface NSWindow (NSToolbarSupport)
- (void)setToolbar:(NSToolbar*)toolbar;
- (NSToolbar *)toolbar;
- (void)toggleToolbarShown:(id)sender;
- (void)runToolbarCustomizationPalette:(id)sender;
- (void)setShowsToolbarButton:(BOOL)show;
- (BOOL)showsToolbarButton;
@end

@interface NSWindow(NSDrag)
- (void)dragImage:(NSImage *)anImage at:(NSPoint)baseLocation offset:(NSSize)initialOffset event:(NSEvent *)event pasteboard:(NSPasteboard *)pboard source:(id)sourceObj slideBack:(BOOL)slideFlag;

- (void)registerForDraggedTypes:(NSArray *)newTypes;
- (void)unregisterDraggedTypes;
@end

@interface NSWindow(NSCarbonExtensions)

- (NSWindow *)initWithWindowRef:(void * )windowRef;

- (void * )windowRef ;
@end


@protocol NSWindowDelegate <NSObject>
@optional
- (BOOL)windowShouldClose:(id)sender;
- (id)windowWillReturnFieldEditor:(NSWindow *)sender toObject:(id)client;
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize;
- (NSRect)windowWillUseStandardFrame:(NSWindow *)window defaultFrame:(NSRect)newFrame;
- (BOOL)windowShouldZoom:(NSWindow *)window toFrame:(NSRect)newFrame;
- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window;
- (NSRect)window:(NSWindow *)window willPositionSheet:(NSWindow *)sheet usingRect:(NSRect)rect;


- (BOOL)window:(NSWindow *)window shouldPopUpDocumentPathMenu:(NSMenu *)menu;



- (BOOL)window:(NSWindow *)window shouldDragDocumentWithEvent:(NSEvent *)event from:(NSPoint)dragImageLocation withPasteboard:(NSPasteboard *)pasteboard;

- (NSSize)window:(NSWindow *)window willUseFullScreenContentSize:(NSSize)proposedSize;

- (NSApplicationPresentationOptions)window:(NSWindow *)window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions;



- (NSArray *)customWindowsToEnterFullScreenForWindow:(NSWindow *)window;



- (void)window:(NSWindow *)window startCustomAnimationToEnterFullScreenWithDuration:(NSTimeInterval)duration;



- (void)windowDidFailToEnterFullScreen:(NSWindow *)window;



- (NSArray *)customWindowsToExitFullScreenForWindow:(NSWindow *)window;



- (void)window:(NSWindow *)window startCustomAnimationToExitFullScreenWithDuration:(NSTimeInterval)duration;



- (void)windowDidFailToExitFullScreen:(NSWindow *)window;




- (NSSize)window:(NSWindow *)window willResizeForVersionBrowserWithMaxPreferredSize:(NSSize)maxPreferredFrameSize maxAllowedSize:(NSSize)maxAllowedFrameSize;




- (void)window:(NSWindow *)window willEncodeRestorableState:(NSCoder *)state;



- (void)window:(NSWindow *)window didDecodeRestorableState:(NSCoder *)state;



- (void)windowDidResize:(NSNotification *)notification;
- (void)windowDidExpose:(NSNotification *)notification;
- (void)windowWillMove:(NSNotification *)notification;
- (void)windowDidMove:(NSNotification *)notification;
- (void)windowDidBecomeKey:(NSNotification *)notification;
- (void)windowDidResignKey:(NSNotification *)notification;
- (void)windowDidBecomeMain:(NSNotification *)notification;
- (void)windowDidResignMain:(NSNotification *)notification;
- (void)windowWillClose:(NSNotification *)notification;
- (void)windowWillMiniaturize:(NSNotification *)notification;
- (void)windowDidMiniaturize:(NSNotification *)notification;
- (void)windowDidDeminiaturize:(NSNotification *)notification;
- (void)windowDidUpdate:(NSNotification *)notification;
- (void)windowDidChangeScreen:(NSNotification *)notification;
- (void)windowDidChangeScreenProfile:(NSNotification *)notification;
- (void)windowDidChangeBackingProperties:(NSNotification *)notification;
- (void)windowWillBeginSheet:(NSNotification *)notification;
- (void)windowDidEndSheet:(NSNotification *)notification;
- (void)windowWillStartLiveResize:(NSNotification *)notification;
- (void)windowDidEndLiveResize:(NSNotification *)notification;
- (void)windowWillEnterFullScreen:(NSNotification *)notification;
- (void)windowDidEnterFullScreen:(NSNotification *)notification;
- (void)windowWillExitFullScreen:(NSNotification *)notification;
- (void)windowDidExitFullScreen:(NSNotification *)notification;
- (void)windowWillEnterVersionBrowser:(NSNotification *)notification;
- (void)windowDidEnterVersionBrowser:(NSNotification *)notification;
- (void)windowWillExitVersionBrowser:(NSNotification *)notification;
- (void)windowDidExitVersionBrowser:(NSNotification *)notification;

@end



EXTERN_C NSString *NSWindowDidBecomeKeyNotification;
EXTERN_C NSString *NSWindowDidBecomeMainNotification;
EXTERN_C NSString *NSWindowDidChangeScreenNotification;
EXTERN_C NSString *NSWindowDidDeminiaturizeNotification;
EXTERN_C NSString *NSWindowDidExposeNotification;
EXTERN_C NSString *NSWindowDidMiniaturizeNotification;
EXTERN_C NSString *NSWindowDidMoveNotification;
EXTERN_C NSString *NSWindowDidResignKeyNotification;
EXTERN_C NSString *NSWindowDidResignMainNotification;
EXTERN_C NSString *NSWindowDidResizeNotification;
EXTERN_C NSString *NSWindowDidUpdateNotification;
EXTERN_C NSString *NSWindowWillCloseNotification;
EXTERN_C NSString *NSWindowWillMiniaturizeNotification;
EXTERN_C NSString *NSWindowWillMoveNotification;
EXTERN_C NSString *NSWindowWillBeginSheetNotification;
EXTERN_C NSString *NSWindowDidEndSheetNotification;



EXTERN_C NSString * const NSWindowDidChangeBackingPropertiesNotification;

EXTERN_C NSString * const NSBackingPropertyOldScaleFactorKey;
EXTERN_C NSString * const NSBackingPropertyOldColorSpaceKey;




EXTERN_C NSString *NSWindowDidChangeScreenProfileNotification;


EXTERN_C NSString * const NSWindowWillStartLiveResizeNotification;

EXTERN_C NSString * const NSWindowDidEndLiveResizeNotification;
EXTERN_C NSString * const NSWindowWillEnterFullScreenNotification;
EXTERN_C NSString * const NSWindowDidEnterFullScreenNotification;
EXTERN_C NSString * const NSWindowWillExitFullScreenNotification;
EXTERN_C NSString * const NSWindowDidExitFullScreenNotification;
EXTERN_C NSString * const NSWindowWillEnterVersionBrowserNotification;
EXTERN_C NSString * const NSWindowDidEnterVersionBrowserNotification;
EXTERN_C NSString * const NSWindowWillExitVersionBrowserNotification;
EXTERN_C NSString * const NSWindowDidExitVersionBrowserNotification;
