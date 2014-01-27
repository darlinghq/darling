#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSColor, NSEvent, NSImage;

#define NSAppKitVersionNumberWithCursorSizeSupport 682.0

@interface NSCursor : NSObject <NSCoding> {

    NSPoint _hotSpot;
    struct _cursorFlags {
 unsigned int onMouseExited:1;
 unsigned int onMouseEntered:1;
        unsigned int cursorType:8;
 unsigned int :22;
    } _flags;
    id _image;
}



+ (NSCursor *)currentCursor;







+ (NSCursor *)currentSystemCursor;

+ (NSCursor *)arrowCursor;
+ (NSCursor *)IBeamCursor;
+ (NSCursor *)pointingHandCursor;
+ (NSCursor *)closedHandCursor;
+ (NSCursor *)openHandCursor;
+ (NSCursor *)resizeLeftCursor;
+ (NSCursor *)resizeRightCursor;
+ (NSCursor *)resizeLeftRightCursor;
+ (NSCursor *)resizeUpCursor;
+ (NSCursor *)resizeDownCursor;
+ (NSCursor *)resizeUpDownCursor;
+ (NSCursor *)crosshairCursor;
+ (NSCursor *)disappearingItemCursor;
+ (NSCursor *)operationNotAllowedCursor;
+ (NSCursor *)dragLinkCursor;
+ (NSCursor *)dragCopyCursor;
+ (NSCursor *)contextualMenuCursor;
+ (NSCursor *)IBeamCursorForVerticalLayout;

- (id)initWithImage:(NSImage *)newImage hotSpot:(NSPoint)aPoint;
- (id)initWithImage:(NSImage *)newImage foregroundColorHint:(NSColor *)fg backgroundColorHint:(NSColor *)bg hotSpot:(NSPoint)hotSpot;

+ (void)hide;
+ (void)unhide;
+ (void)setHiddenUntilMouseMoves:(BOOL)flag;
+ (void)pop;

- (NSImage *)image;
- (NSPoint)hotSpot;
- (void)push;
- (void)pop;
- (void)set;
- (void)setOnMouseExited:(BOOL)flag;
- (void)setOnMouseEntered:(BOOL)flag;
- (BOOL)isSetOnMouseExited;
- (BOOL)isSetOnMouseEntered;
- (void)mouseEntered:(NSEvent *)theEvent;
- (void)mouseExited:(NSEvent *)theEvent;

@end
