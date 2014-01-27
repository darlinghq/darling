#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitDefines.h>

@class NSColor, NSView;




enum {
    NSCompositeClear = 0,
    NSCompositeCopy = 1,
    NSCompositeSourceOver = 2,
    NSCompositeSourceIn = 3,
    NSCompositeSourceOut = 4,
    NSCompositeSourceAtop = 5,
    NSCompositeDestinationOver = 6,
    NSCompositeDestinationIn = 7,
    NSCompositeDestinationOut = 8,
    NSCompositeDestinationAtop = 9,
    NSCompositeXOR = 10,
    NSCompositePlusDarker = 11,
    NSCompositeHighlight = 12,
    NSCompositePlusLighter = 13
};
typedef NSUInteger NSCompositingOperation;


enum {
    NSBackingStoreRetained = 0,
    NSBackingStoreNonretained = 1,
    NSBackingStoreBuffered = 2
};
typedef NSUInteger NSBackingStoreType;


enum {
    NSWindowAbove = 1,
    NSWindowBelow = -1,
    NSWindowOut = 0
};
typedef NSInteger NSWindowOrderingMode;


enum {
    NSFocusRingOnly = 0,
    NSFocusRingBelow = 1,
    NSFocusRingAbove = 2
};
typedef NSUInteger NSFocusRingPlacement;


enum {
    NSFocusRingTypeDefault = 0,
    NSFocusRingTypeNone = 1,
    NSFocusRingTypeExterior = 2
};
typedef NSUInteger NSFocusRingType;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
enum {
    NSColorRenderingIntentDefault,
    NSColorRenderingIntentAbsoluteColorimetric,
    NSColorRenderingIntentRelativeColorimetric,
    NSColorRenderingIntentPerceptual,
    NSColorRenderingIntentSaturation
};
#endif
typedef NSInteger NSColorRenderingIntent;




EXTERN_C NSString *NSCalibratedWhiteColorSpace;
EXTERN_C NSString *NSCalibratedRGBColorSpace;
EXTERN_C NSString *NSDeviceWhiteColorSpace;
EXTERN_C NSString *NSDeviceRGBColorSpace;
EXTERN_C NSString *NSDeviceCMYKColorSpace;
EXTERN_C NSString *NSNamedColorSpace;
EXTERN_C NSString *NSPatternColorSpace;
EXTERN_C NSString *NSCustomColorSpace;



EXTERN_C NSString *NSCalibratedBlackColorSpace;
EXTERN_C NSString *NSDeviceBlackColorSpace;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
enum {
    NSWindowDepthTwentyfourBitRGB = 0x208,
    NSWindowDepthSixtyfourBitRGB = 0x210,
    NSWindowDepthOnehundredtwentyeightBitRGB = 0x220
};
#endif

typedef int NSWindowDepth;

EXTERN_C NSWindowDepth NSBestDepth (NSString *colorSpace, NSInteger bps, NSInteger bpp, BOOL planar, BOOL *exactMatch);
EXTERN_C BOOL NSPlanarFromDepth (NSWindowDepth depth);
EXTERN_C NSString *NSColorSpaceFromDepth (NSWindowDepth depth);
EXTERN_C NSInteger NSBitsPerSampleFromDepth(NSWindowDepth depth);
EXTERN_C NSInteger NSBitsPerPixelFromDepth(NSWindowDepth depth);
EXTERN_C NSInteger NSNumberOfColorComponents(NSString *colorSpaceName);
EXTERN_C const NSWindowDepth *NSAvailableWindowDepths (void);




EXTERN_C const CGFloat NSWhite;
EXTERN_C const CGFloat NSLightGray;
EXTERN_C const CGFloat NSDarkGray;
EXTERN_C const CGFloat NSBlack;




EXTERN_C NSString *NSDeviceResolution;
EXTERN_C NSString *NSDeviceColorSpaceName;
EXTERN_C NSString *NSDeviceBitsPerSample;
EXTERN_C NSString *NSDeviceIsScreen;
EXTERN_C NSString *NSDeviceIsPrinter;
EXTERN_C NSString *NSDeviceSize;




EXTERN_C void NSRectFill(NSRect aRect);
EXTERN_C void NSRectFillList(const NSRect *rects, NSInteger count);
EXTERN_C void NSRectFillListWithGrays(const NSRect *rects, const CGFloat *grays, NSInteger num);
EXTERN_C void NSRectFillListWithColors(const NSRect *rects, NSColor * const *colors, NSInteger num);
EXTERN_C void NSRectFillUsingOperation(NSRect aRect, NSCompositingOperation op);
EXTERN_C void NSRectFillListUsingOperation(const NSRect *rects, NSInteger count, NSCompositingOperation op);
EXTERN_C void NSRectFillListWithColorsUsingOperation(const NSRect *rects, NSColor * const *colors, NSInteger num, NSCompositingOperation op);
EXTERN_C void NSFrameRect(NSRect aRect);
EXTERN_C void NSFrameRectWithWidth(NSRect aRect, CGFloat frameWidth);
EXTERN_C void NSFrameRectWithWidthUsingOperation(NSRect aRect, CGFloat frameWidth, NSCompositingOperation op);
EXTERN_C void NSRectClip(NSRect aRect);
EXTERN_C void NSRectClipList(const NSRect *rects, NSInteger count);
EXTERN_C NSRect NSDrawTiledRects(NSRect boundsRect, NSRect clipRect, const NSRectEdge *sides, const CGFloat *grays, NSInteger count);
EXTERN_C void NSDrawGrayBezel(NSRect aRect, NSRect clipRect);
EXTERN_C void NSDrawGroove(NSRect aRect, NSRect clipRect);
EXTERN_C void NSDrawWhiteBezel(NSRect aRect, NSRect clipRect);
EXTERN_C void NSDrawButton(NSRect aRect, NSRect clipRect);
EXTERN_C void NSEraseRect(NSRect aRect);
EXTERN_C NSColor *NSReadPixel(NSPoint passedPoint);
EXTERN_C void NSDrawBitmap(NSRect rect, NSInteger width, NSInteger height, NSInteger bps, NSInteger spp, NSInteger bpp, NSInteger bpr, BOOL isPlanar, BOOL hasAlpha, NSString *colorSpaceName, const unsigned char *const data[5]);
EXTERN_C void NSCopyBits(NSInteger srcGState, NSRect srcRect, NSPoint destPoint);
EXTERN_C void NSHighlightRect(NSRect aRect);
EXTERN_C void NSBeep(void);


EXTERN_C NSInteger NSGetWindowServerMemory(NSInteger context, NSInteger *virtualMemory, NSInteger *windowBackingMemory, NSString **windowDumpString);

EXTERN_C NSRect NSDrawColorTiledRects(NSRect boundsRect, NSRect clipRect, const NSRectEdge *sides, NSColor **colors, NSInteger count);
EXTERN_C void NSDrawDarkBezel(NSRect aRect, NSRect clipRect);
EXTERN_C void NSDrawLightBezel(NSRect aRect, NSRect clipRect);
EXTERN_C void NSDottedFrameRect(NSRect aRect);

EXTERN_C void NSDrawWindowBackground(NSRect aRect);
EXTERN_C void NSSetFocusRingStyle(NSFocusRingPlacement placement);
EXTERN_C void NSDisableScreenUpdates(void);
EXTERN_C void NSEnableScreenUpdates(void);
enum {


 NSAnimationEffectDisappearingItemDefault = 0,


 NSAnimationEffectPoof = 10
};
typedef NSUInteger NSAnimationEffect;

EXTERN_C void NSShowAnimationEffect(NSAnimationEffect animationEffect, NSPoint centerLocation, NSSize size, id animationDelegate, SEL didEndSelector, void *contextInfo);



EXTERN_C void NSCountWindows(NSInteger *count);
EXTERN_C void NSWindowList(NSInteger size, NSInteger list[]);
EXTERN_C void NSCountWindowsForContext(NSInteger context, NSInteger *count);
EXTERN_C void NSWindowListForContext(NSInteger context, NSInteger size, NSInteger list[]);
