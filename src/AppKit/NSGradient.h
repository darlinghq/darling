#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>


@class NSArray, NSBezierPath, NSColor, NSColorSpace;

typedef NSUInteger NSGradientDrawingOptions;

enum {
    NSGradientDrawsBeforeStartingLocation = (1 << 0),
    NSGradientDrawsAfterEndingLocation = (1 << 1),
};

@interface NSGradient : NSObject <NSCopying, NSCoding> {
@private
    NSArray *_colorArray;
    NSColorSpace *_colorSpace;
    __strong void *_functionRef;
    void *_componentArray;
    void *_reserved1;
    void *_reserved2;
    void *_reserved3;
}




- (id)initWithStartingColor:(NSColor *)startingColor endingColor:(NSColor *)endingColor;




- (id)initWithColors:(NSArray *)colorArray;
- (id)initWithColorsAndLocations:(NSColor *)firstColor, ... NS_REQUIRES_NIL_TERMINATION;




- (id)initWithColors:(NSArray *)colorArray atLocations:(const CGFloat *)locations colorSpace:(NSColorSpace *)colorSpace;






- (void)drawFromPoint:(NSPoint)startingPoint toPoint:(NSPoint)endingPoint options:(NSGradientDrawingOptions)options;




- (void)drawInRect:(NSRect)rect angle:(CGFloat)angle;




- (void)drawInBezierPath:(NSBezierPath *)path angle:(CGFloat)angle;






- (void)drawFromCenter:(NSPoint)startCenter radius:(CGFloat)startRadius toCenter:(NSPoint)endCenter radius:(CGFloat)endRadius options:(NSGradientDrawingOptions)options;






- (void)drawInRect:(NSRect)rect relativeCenterPosition:(NSPoint)relativeCenterPosition;






- (void)drawInBezierPath:(NSBezierPath *)path relativeCenterPosition:(NSPoint)relativeCenterPosition;
- (NSColorSpace *)colorSpace;



- (NSInteger)numberOfColorStops;



- (void)getColor:(NSColor **)color location:(CGFloat *)location atIndex:(NSInteger)index;






- (NSColor *)interpolatedColorAtLocation:(CGFloat)location;


@end
