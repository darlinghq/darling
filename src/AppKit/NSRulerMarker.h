#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSEvent, NSImage, NSRulerView;

@interface NSRulerMarker : NSObject <NSCopying, NSCoding> {

    NSRulerView *_ruler;
    CGFloat _location;
    NSImage *_image;
    NSPoint _imageOrigin;

    struct __rFlags {
        unsigned int movable:1;
        unsigned int removable:1;
        unsigned int dragging:1;
        unsigned int pinned:1;
        unsigned int _reserved:28;
    } _rFlags;

    id _representedObject;
}



- (id)initWithRulerView:(NSRulerView *)ruler markerLocation:(CGFloat)location image:(NSImage *)image imageOrigin:(NSPoint)imageOrigin;




- (NSRulerView *)ruler;



- (void)setMarkerLocation:(CGFloat)location;
- (CGFloat)markerLocation;


- (void)setImage:(NSImage *)image;
- (NSImage *)image;


- (void)setImageOrigin:(NSPoint)imageOrigin;
- (NSPoint)imageOrigin;


- (void)setMovable:(BOOL)flag;
- (void)setRemovable:(BOOL)flag;
- (BOOL)isMovable;
- (BOOL)isRemovable;


- (BOOL)isDragging;


- (void)setRepresentedObject:(id <NSCopying>)representedObject;
- (id <NSCopying>)representedObject;




- (NSRect)imageRectInRuler;


- (CGFloat)thicknessRequiredInRuler;


- (void)drawRect:(NSRect)rect;


- (BOOL)trackMouse:(NSEvent *)mouseDownEvent adding:(BOOL)isAdding;


@end
