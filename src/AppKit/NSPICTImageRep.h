#import <AppKit/NSImageRep.h>

@interface NSPICTImageRep : NSImageRep
{

    NSPoint _pictOrigin;
    NSData* _pictData;
#if !__LP64__
    unsigned int _reserved1;
    unsigned int _reserved2;
#else
    id _imageRep;
    NSUInteger _pictOffset;
    unsigned int _reserved1;
    unsigned int _reserved2;
#endif
}

+ (id)imageRepWithData:(NSData*)pictData;
- (id)initWithData:(NSData*)pictData;

- (NSData*) PICTRepresentation;
- (NSRect) boundingBox;

@end
