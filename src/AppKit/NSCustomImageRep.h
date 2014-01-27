#import <AppKit/NSImageRep.h>

@interface NSCustomImageRep : NSImageRep {

    SEL _drawMethod;
    id _drawObject;
    unsigned int _reserved;
}

- (id)initWithSize:(NSSize)size flipped:(BOOL)drawingHandlerShouldBeCalledWithFlippedContext drawingHandler:(BOOL (^)(NSRect dstRect))drawingHandler;
- (BOOL (^)(NSRect dstRect))drawingHandler;

- (id)initWithDrawSelector:(SEL)aMethod delegate:(id)anObject;
- (SEL)drawSelector;
- (id)delegate;

@end
