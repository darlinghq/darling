#import <AppKit/AppKitDefines.h>
#import <AppKit/NSGraphics.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>

@class NSDictionary;
@class NSString;
@class NSWindow;
@class NSBitmapImageRep;


EXTERN_C NSString *NSGraphicsContextDestinationAttributeName;


EXTERN_C NSString *NSGraphicsContextRepresentationFormatAttributeName;


EXTERN_C NSString *NSGraphicsContextPSFormat;
EXTERN_C NSString *NSGraphicsContextPDFFormat;

enum {
   NSImageInterpolationDefault = 0,
   NSImageInterpolationNone = 1,
   NSImageInterpolationLow = 2,
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
   NSImageInterpolationMedium = 4,
#endif
   NSImageInterpolationHigh = 3
};
typedef NSUInteger NSImageInterpolation;

@interface NSGraphicsContext : NSObject {
}


+ (NSGraphicsContext *)graphicsContextWithAttributes:(NSDictionary *)attributes;


+ (NSGraphicsContext *)graphicsContextWithWindow:(NSWindow *)window;



+ (NSGraphicsContext *)graphicsContextWithBitmapImageRep:(NSBitmapImageRep *)bitmapRep;



+ (NSGraphicsContext *)graphicsContextWithGraphicsPort:(void *)graphicsPort flipped:(BOOL)initialFlippedState;


+ (NSGraphicsContext *)currentContext;
+ (void)setCurrentContext:(NSGraphicsContext *)context;


+ (BOOL)currentContextDrawingToScreen;



+ (void)saveGraphicsState;

+ (void)restoreGraphicsState;

+ (void)setGraphicsState:(NSInteger)gState;


- (NSDictionary *)attributes;


- (BOOL)isDrawingToScreen;


- (void)saveGraphicsState;
- (void)restoreGraphicsState;

- (void)flushGraphics;


- (void *)graphicsPort ;



- (BOOL)isFlipped;
@end

@interface NSGraphicsContext(NSGraphicsContext_RenderingOptions)

- (void)setShouldAntialias:(BOOL)antialias;
- (BOOL)shouldAntialias;
- (void)setImageInterpolation:(NSImageInterpolation)interpolation;
- (NSImageInterpolation)imageInterpolation;

- (void)setPatternPhase:(NSPoint)phase;
- (NSPoint)patternPhase;
- (void)setCompositingOperation:(NSCompositingOperation)operation;
- (NSCompositingOperation)compositingOperation;
- (NSColorRenderingIntent)colorRenderingIntent;
- (void)setColorRenderingIntent:(NSColorRenderingIntent)renderingIntent;
@end

@class CIContext;
@interface NSGraphicsContext (NSQuartzCoreAdditions)
- (CIContext *)CIContext;
@end



#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
@interface NSGraphicsContext (NSGraphicsContextDeprecated)
- (id)focusStack;
- (void)setFocusStack:(id)stack;
@end
#endif
