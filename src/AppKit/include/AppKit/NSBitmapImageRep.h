#import <AppKit/NSImageRep.h>

@class CIImage;
@class NSColor;
@class NSColorSpace;

enum {
    NSTIFFCompressionNone = 1,
    NSTIFFCompressionCCITTFAX3 = 3,
    NSTIFFCompressionCCITTFAX4 = 4,
    NSTIFFCompressionLZW = 5,
    NSTIFFCompressionJPEG = 6,
    NSTIFFCompressionNEXT = 32766,
    NSTIFFCompressionPackBits = 32773,
    NSTIFFCompressionOldJPEG = 32865
};
typedef NSUInteger NSTIFFCompression;

enum {
    NSTIFFFileType,
    NSBMPFileType,
    NSGIFFileType,
    NSJPEGFileType,
    NSPNGFileType,
    NSJPEG2000FileType
};
typedef NSUInteger NSBitmapImageFileType;

enum {
    NSImageRepLoadStatusUnknownType = -1,
    NSImageRepLoadStatusReadingHeader = -2,
    NSImageRepLoadStatusWillNeedAllData = -3,
    NSImageRepLoadStatusInvalidData = -4,
    NSImageRepLoadStatusUnexpectedEOF = -5,
    NSImageRepLoadStatusCompleted = -6
};
typedef NSInteger NSImageRepLoadStatus;

enum {
    NSAlphaFirstBitmapFormat = 1 << 0,
    NSAlphaNonpremultipliedBitmapFormat = 1 << 1,
    NSFloatingPointSamplesBitmapFormat = 1 << 2
};
typedef NSUInteger NSBitmapFormat;

EXTERN_C NSString* NSImageCompressionMethod;
EXTERN_C NSString* NSImageCompressionFactor;
EXTERN_C NSString* NSImageDitherTransparency;
EXTERN_C NSString* NSImageRGBColorTable;
EXTERN_C NSString* NSImageInterlaced;
EXTERN_C NSString* NSImageColorSyncProfileData;
EXTERN_C NSString* NSImageFrameCount;
EXTERN_C NSString* NSImageCurrentFrame;
EXTERN_C NSString* NSImageCurrentFrameDuration;
EXTERN_C NSString* NSImageLoopCount ;
EXTERN_C NSString* NSImageGamma ;
EXTERN_C NSString* NSImageProgressive ;
EXTERN_C NSString* NSImageEXIFData ;
EXTERN_C NSString* NSImageFallbackBackgroundColor;

@interface NSBitmapImageRep : NSImageRep {

    struct __bitmapRepFlags {
        unsigned int bitsPerPixel:8;
 unsigned int isPlanar:1;
 unsigned int explicitPlanes:1;
 unsigned int imageSourceIsIndexed:1;
 unsigned int dataLoaded:1;
        unsigned int colorModel:4;
        unsigned int tierTwoInfoIsLoaded:1;
        unsigned int respectO:1;
        unsigned int compressionFactor:14;
        unsigned int imageNumber:8;
        unsigned int bitmapFormat:3;
        unsigned int cgImageIsPrimary:1;
 unsigned int compression:20;
    } _moreRepFlags;
    unsigned int _bytesPerRow;
    volatile id _dataObj;
    NSData *_tiffData;
    id _properties;
}

- (id)initWithFocusedViewRect:(NSRect)rect;

- (id)initWithBitmapDataPlanes:(unsigned char **)planes pixelsWide:(NSInteger)width pixelsHigh:(NSInteger)height bitsPerSample:(NSInteger)bps samplesPerPixel:(NSInteger)spp hasAlpha:(BOOL)alpha isPlanar:(BOOL)isPlanar colorSpaceName:(NSString *)colorSpaceName bytesPerRow:(NSInteger)rBytes bitsPerPixel:(NSInteger)pBits;
- (id)initWithBitmapDataPlanes:(unsigned char **)planes pixelsWide:(NSInteger)width pixelsHigh:(NSInteger)height bitsPerSample:(NSInteger)bps samplesPerPixel:(NSInteger)spp hasAlpha:(BOOL)alpha isPlanar:(BOOL)isPlanar colorSpaceName:(NSString *)colorSpaceName bitmapFormat:(NSBitmapFormat)bitmapFormat bytesPerRow:(NSInteger)rBytes bitsPerPixel:(NSInteger)pBits;
- (id)initWithCGImage:(CGImageRef)cgImage;
- (id)initWithCIImage:(CIImage *)ciImage;

+ (NSArray *)imageRepsWithData:(NSData *)data;

+ (id)imageRepWithData:(NSData *)data;
- (id)initWithData:(NSData *)data;

- (unsigned char *)bitmapData ;
- (void)getBitmapDataPlanes:(unsigned char **)data;
- (BOOL)isPlanar;
- (NSInteger)samplesPerPixel;
- (NSInteger)bitsPerPixel;
- (NSInteger)bytesPerRow;
- (NSInteger)bytesPerPlane;
- (NSInteger)numberOfPlanes;
- (NSBitmapFormat)bitmapFormat;

- (void)getCompression:(NSTIFFCompression *)compression factor:(float *)factor;
- (void)setCompression:(NSTIFFCompression)compression factor:(float)factor;

- (NSData *)TIFFRepresentation;
- (NSData *)TIFFRepresentationUsingCompression:(NSTIFFCompression)comp factor:(float)factor;

+ (NSData *)TIFFRepresentationOfImageRepsInArray:(NSArray *)array;
+ (NSData *)TIFFRepresentationOfImageRepsInArray:(NSArray *)array usingCompression:(NSTIFFCompression)comp factor:(float)factor;

+ (void)getTIFFCompressionTypes:(const NSTIFFCompression **)list count:(NSInteger *)numTypes;
+ (NSString *)localizedNameForTIFFCompressionType:(NSTIFFCompression)compression;
- (BOOL)canBeCompressedUsing:(NSTIFFCompression)compression;





- (void)colorizeByMappingGray:(CGFloat)midPoint toColor:(NSColor *)midPointColor blackMapping:(NSColor *)shadowColor whiteMapping:(NSColor *)lightColor;

- (id)initForIncrementalLoad;
- (NSInteger)incrementalLoadFromData:(NSData*)data complete:(BOOL)complete;

- (void)setColor:(NSColor*)color atX:(NSInteger)x y:(NSInteger)y;
- (NSColor*)colorAtX:(NSInteger)x y:(NSInteger)y;

- (void)getPixel:(NSUInteger[])p atX:(NSInteger)x y:(NSInteger)y;
- (void)setPixel:(NSUInteger[])p atX:(NSInteger)x y:(NSInteger)y;

- (CGImageRef)CGImage;


- (NSColorSpace *)colorSpace;

- (NSBitmapImageRep *)bitmapImageRepByConvertingToColorSpace:(NSColorSpace *)targetSpace renderingIntent:(NSColorRenderingIntent)renderingIntent;
- (NSBitmapImageRep *)bitmapImageRepByRetaggingWithColorSpace:(NSColorSpace *)newSpace;

@end


@interface NSBitmapImageRep (NSBitmapImageFileTypeExtensions)

+ (NSData *)representationOfImageRepsInArray:(NSArray *)imageReps usingType:(NSBitmapImageFileType)storageType properties:(NSDictionary *)properties;

- (NSData *)representationUsingType:(NSBitmapImageFileType)storageType properties:(NSDictionary *)properties;

- (void)setProperty:(NSString *)property withValue:(id)value;
- (id)valueForProperty:(NSString *)property;

@end
