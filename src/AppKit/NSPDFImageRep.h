#import <AppKit/NSImageRep.h>

@interface NSPDFImageRep : NSImageRep
{
  @private
    NSData* _pdfData;
    int _reserved1;
    int _reserved2;

    id _private;
}

+ (id)imageRepWithData:(NSData*)pdfData;
- (id)initWithData:(NSData*)pdfData;

- (NSData*)PDFRepresentation;
- (NSRect)bounds;

- (void) setCurrentPage:(NSInteger)page;
- (NSInteger) currentPage;
- (NSInteger) pageCount;

@end
