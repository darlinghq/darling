#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSBundle.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSPasteboard.h>

@class NSArray, NSColor, NSImageRep, NSGraphicsContext, NSURL;
@protocol NSImageDelegate;


enum {
    NSImageLoadStatusCompleted,
    NSImageLoadStatusCancelled,
    NSImageLoadStatusInvalidData,
    NSImageLoadStatusUnexpectedEOF,
    NSImageLoadStatusReadError
};
typedef NSUInteger NSImageLoadStatus;

enum {
    NSImageCacheDefault,
    NSImageCacheAlways,
    NSImageCacheBySize,
    NSImageCacheNever
};
typedef NSUInteger NSImageCacheMode;


@class _NSImageAuxiliary;

@interface NSImage : NSObject <NSCopying, NSCoding, NSPasteboardReading, NSPasteboardWriting>
{

    NSString *_name;
    NSSize _size;
    struct __imageFlags {
 unsigned int scalable:1;
 unsigned int dataRetained:1;
 unsigned int uniqueWindow:1;
 unsigned int sizeWasExplicitlySet:1;
 unsigned int builtIn:1;
 unsigned int needsToExpand:1;
 unsigned int useEPSOnResolutionMismatch:1;
 unsigned int matchesOnlyOnBestFittingAxis:1;
 unsigned int colorMatchPreferred:1;
 unsigned int multipleResolutionMatching:1;
 unsigned int focusedWhilePrinting:1;
 unsigned int archiveByName:1;
 unsigned int unboundedCacheDepth:1;
        unsigned int flipped:1;
        unsigned int aliased:1;
 unsigned int dirtied:1;
        unsigned int cacheMode:2;
        unsigned int sampleMode:3;
        unsigned int resMatchPreferred:1;
        unsigned int isTemplate:1;
        unsigned int failedToExpand:1;
        unsigned int reserved1:8;
    } _flags;
    volatile id _reps;
    _NSImageAuxiliary *_imageAuxiliary;
}

+ (id)imageNamed:(NSString *)name;

- (id)initWithSize:(NSSize)aSize;
- (id)initWithData:(NSData *)data;
- (id)initWithContentsOfFile:(NSString *)fileName;
- (id)initWithContentsOfURL:(NSURL *)url;
- (id)initByReferencingFile:(NSString *)fileName;
- (id)initByReferencingURL:(NSURL *)url;
- (id)initWithIconRef:(IconRef)iconRef;
- (id)initWithPasteboard:(NSPasteboard *)pasteboard;


- (id)initWithDataIgnoringOrientation:(NSData *)data;


+ (id)imageWithSize:(NSSize)size flipped:(BOOL)drawingHandlerShouldBeCalledWithFlippedContext drawingHandler:(BOOL (^)(NSRect dstRect))drawingHandler;

- (void)setSize:(NSSize)aSize;
- (NSSize)size;
- (BOOL)setName:(NSString *)string;
- (NSString *)name;
- (void)setBackgroundColor:(NSColor *)aColor;
- (NSColor *)backgroundColor;
- (void)setUsesEPSOnResolutionMismatch:(BOOL)flag;
- (BOOL)usesEPSOnResolutionMismatch;
- (void)setPrefersColorMatch:(BOOL)flag;
- (BOOL)prefersColorMatch;
- (void)setMatchesOnMultipleResolution:(BOOL)flag;
- (BOOL)matchesOnMultipleResolution;
- (BOOL)matchesOnlyOnBestFittingAxis;
- (void)setMatchesOnlyOnBestFittingAxis:(BOOL)flag;
- (void)drawAtPoint:(NSPoint)point fromRect:(NSRect)fromRect operation:(NSCompositingOperation)op fraction:(CGFloat)delta;
- (void)drawInRect:(NSRect)rect fromRect:(NSRect)fromRect operation:(NSCompositingOperation)op fraction:(CGFloat)delta;
- (void)drawInRect:(NSRect)dstSpacePortionRect fromRect:(NSRect)srcSpacePortionRect operation:(NSCompositingOperation)op fraction:(CGFloat)requestedAlpha respectFlipped:(BOOL)respectContextIsFlipped hints:(NSDictionary *)hints;
- (BOOL)drawRepresentation:(NSImageRep *)imageRep inRect:(NSRect)rect;
- (void)recache;
- (NSData *)TIFFRepresentation;
- (NSData *)TIFFRepresentationUsingCompression:(NSTIFFCompression)comp factor:(float)aFloat;

- (NSArray *)representations;
- (void)addRepresentations:(NSArray *)imageReps;
- (void)addRepresentation:(NSImageRep *)imageRep;
- (void)removeRepresentation:(NSImageRep *)imageRep;

- (BOOL)isValid;
- (void)lockFocus;
- (void)lockFocusFlipped:(BOOL)flipped;
- (void)unlockFocus;


- (NSImageRep *)bestRepresentationForDevice:(NSDictionary *)deviceDescription;

- (void)setDelegate:(id <NSImageDelegate>)anObject;
- (id <NSImageDelegate>)delegate;



+ (NSArray *)imageUnfilteredFileTypes;
+ (NSArray *)imageUnfilteredPasteboardTypes;
+ (NSArray *)imageFileTypes;
+ (NSArray *)imagePasteboardTypes;

+ (NSArray *)imageTypes;
+ (NSArray *)imageUnfilteredTypes;

+ (BOOL)canInitWithPasteboard:(NSPasteboard *)pasteboard;

- (void)cancelIncrementalLoad;

-(void)setCacheMode:(NSImageCacheMode)mode;
-(NSImageCacheMode)cacheMode;
- (NSRect)alignmentRect;
- (void)setAlignmentRect:(NSRect)rect;





- (BOOL)isTemplate;
- (void)setTemplate:(BOOL)isTemplate;



- (NSString *)accessibilityDescription;
- (void)setAccessibilityDescription:(NSString *)description;
- (id)initWithCGImage:(CGImageRef)cgImage size:(NSSize)size;
- (CGImageRef)CGImageForProposedRect:(NSRect *)proposedDestRect context:(NSGraphicsContext *)referenceContext hints:(NSDictionary *)hints;



- (NSImageRep *)bestRepresentationForRect:(NSRect)rect context:(NSGraphicsContext *)referenceContext hints:(NSDictionary *)hints;



- (BOOL)hitTestRect:(NSRect)testRectDestSpace withImageDestinationRect:(NSRect)imageRectDestSpace context:(NSGraphicsContext *)context hints:(NSDictionary *)hints flipped:(BOOL)flipped;

- (CGFloat)recommendedLayerContentsScale:(CGFloat)preferredContentsScale;
- (id)layerContentsForContentsScale:(CGFloat)layerContentsScale;
@end

EXTERN_C NSString *const NSImageHintCTM;
EXTERN_C NSString *const NSImageHintInterpolation;

@protocol NSImageDelegate <NSObject>
@optional

- (NSImage *)imageDidNotDraw:(id)sender inRect:(NSRect)aRect;

- (void)image:(NSImage *)image willLoadRepresentation:(NSImageRep *)rep;
- (void)image:(NSImage *)image didLoadRepresentationHeader:(NSImageRep *)rep;
- (void)image:(NSImage *)image didLoadPartOfRepresentation:(NSImageRep *)rep withValidRows:(NSInteger)rows;
- (void)image:(NSImage *)image didLoadRepresentation:(NSImageRep *)rep withStatus:(NSImageLoadStatus)status;
@end

@interface NSBundle(NSBundleImageExtension)
- (NSImage *)imageForResource:(NSString *)name;



- (NSString *)pathForImageResource:(NSString *)name;
- (NSURL *)URLForImageResource:(NSString *)name;
@end

@interface NSImage (NSDeprecated)


- (void)setFlipped:(BOOL)flag;
- (BOOL)isFlipped;


- (void)dissolveToPoint:(NSPoint)point fraction:(CGFloat)aFloat;
- (void)dissolveToPoint:(NSPoint)point fromRect:(NSRect)rect fraction:(CGFloat)aFloat;
- (void)compositeToPoint:(NSPoint)point operation:(NSCompositingOperation)op;
- (void)compositeToPoint:(NSPoint)point fromRect:(NSRect)rect operation:(NSCompositingOperation)op;
- (void)compositeToPoint:(NSPoint)point operation:(NSCompositingOperation)op fraction:(CGFloat)delta;
- (void)compositeToPoint:(NSPoint)point fromRect:(NSRect)rect operation:(NSCompositingOperation)op fraction:(CGFloat)delta;


- (void)lockFocusOnRepresentation:(NSImageRep *)imageRepresentation;


- (void)setScalesWhenResized:(BOOL)flag;
- (BOOL)scalesWhenResized;
- (void)setDataRetained:(BOOL)flag;
- (BOOL)isDataRetained;
- (void)setCachedSeparately:(BOOL)flag;
- (BOOL)isCachedSeparately;
- (void)setCacheDepthMatchesImageDepth:(BOOL)flag;
- (BOOL)cacheDepthMatchesImageDepth;

@end



EXTERN_C NSString *const NSImageNameQuickLookTemplate;
EXTERN_C NSString *const NSImageNameBluetoothTemplate;
EXTERN_C NSString *const NSImageNameIChatTheaterTemplate;
EXTERN_C NSString *const NSImageNameSlideshowTemplate;



EXTERN_C NSString *const NSImageNameActionTemplate;



EXTERN_C NSString *const NSImageNameSmartBadgeTemplate;



EXTERN_C NSString *const NSImageNameIconViewTemplate;
EXTERN_C NSString *const NSImageNameListViewTemplate;
EXTERN_C NSString *const NSImageNameColumnViewTemplate;
EXTERN_C NSString *const NSImageNameFlowViewTemplate;

EXTERN_C NSString *const NSImageNamePathTemplate;



EXTERN_C NSString *const NSImageNameInvalidDataFreestandingTemplate;
EXTERN_C NSString *const NSImageNameLockLockedTemplate;
EXTERN_C NSString *const NSImageNameLockUnlockedTemplate;



EXTERN_C NSString *const NSImageNameGoRightTemplate;
EXTERN_C NSString *const NSImageNameGoLeftTemplate;



EXTERN_C NSString *const NSImageNameRightFacingTriangleTemplate;
EXTERN_C NSString *const NSImageNameLeftFacingTriangleTemplate;

EXTERN_C NSString *const NSImageNameAddTemplate;
EXTERN_C NSString *const NSImageNameRemoveTemplate;
EXTERN_C NSString *const NSImageNameRevealFreestandingTemplate;
EXTERN_C NSString *const NSImageNameFollowLinkFreestandingTemplate;
EXTERN_C NSString *const NSImageNameEnterFullScreenTemplate;
EXTERN_C NSString *const NSImageNameExitFullScreenTemplate;
EXTERN_C NSString *const NSImageNameStopProgressTemplate;
EXTERN_C NSString *const NSImageNameStopProgressFreestandingTemplate;
EXTERN_C NSString *const NSImageNameRefreshTemplate;
EXTERN_C NSString *const NSImageNameRefreshFreestandingTemplate;

EXTERN_C NSString *const NSImageNameBonjour;
EXTERN_C NSString *const NSImageNameComputer;
EXTERN_C NSString *const NSImageNameFolderBurnable;
EXTERN_C NSString *const NSImageNameFolderSmart;
EXTERN_C NSString *const NSImageNameFolder;
EXTERN_C NSString *const NSImageNameNetwork;



EXTERN_C NSString *const NSImageNameDotMac;
EXTERN_C NSString *const NSImageNameMobileMe;



EXTERN_C NSString *const NSImageNameMultipleDocuments;



EXTERN_C NSString *const NSImageNameUserAccounts;
EXTERN_C NSString *const NSImageNamePreferencesGeneral;
EXTERN_C NSString *const NSImageNameAdvanced;



EXTERN_C NSString *const NSImageNameInfo;
EXTERN_C NSString *const NSImageNameFontPanel;
EXTERN_C NSString *const NSImageNameColorPanel;



EXTERN_C NSString *const NSImageNameUser;
EXTERN_C NSString *const NSImageNameUserGroup;
EXTERN_C NSString *const NSImageNameEveryone;
EXTERN_C NSString *const NSImageNameUserGuest;



EXTERN_C NSString *const NSImageNameMenuOnStateTemplate;
EXTERN_C NSString *const NSImageNameMenuMixedStateTemplate;



EXTERN_C NSString *const NSImageNameApplicationIcon;

EXTERN_C NSString *const NSImageNameTrashEmpty;
EXTERN_C NSString *const NSImageNameTrashFull;
EXTERN_C NSString *const NSImageNameHomeTemplate;
EXTERN_C NSString *const NSImageNameBookmarksTemplate;
EXTERN_C NSString *const NSImageNameCaution;
EXTERN_C NSString *const NSImageNameStatusAvailable;
EXTERN_C NSString *const NSImageNameStatusPartiallyAvailable;
EXTERN_C NSString *const NSImageNameStatusUnavailable;
EXTERN_C NSString *const NSImageNameStatusNone;

EXTERN_C NSString *const NSImageNameShareTemplate;
