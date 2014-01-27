#import <AppKit/NSImageRep.h>

@class NSPDFImageRep;

@interface NSEPSImageRep : NSImageRep {

    NSPoint _bBoxOrigin;
    NSData *_epsData;
    NSPDFImageRep* _pdfImageRep;
}

+ (id)imageRepWithData:(NSData *)epsData;
- (id)initWithData:(NSData *)epsData;

- (void)prepareGState;

- (NSData *)EPSRepresentation;

- (NSRect)boundingBox;

@end
