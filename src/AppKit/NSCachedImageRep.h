#import <AppKit/NSImageRep.h>
#import <AppKit/NSGraphics.h>

@class NSWindow;


@interface NSCachedImageRep : NSImageRep {

    NSPoint _origin;
    NSWindow *_window;
    __strong void *_cache;
}


- (id)initWithWindow:(NSWindow *)win rect:(NSRect)rect;


- (id)initWithSize:(NSSize)size depth:(NSWindowDepth)depth separate:(BOOL)flag alpha:(BOOL)alpha;

- (NSWindow *)window;
- (NSRect)rect;

@end
