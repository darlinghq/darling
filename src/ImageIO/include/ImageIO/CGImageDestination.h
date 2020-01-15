@class O2ImageDestination;
typedef O2ImageDestination *CGImageDestinationRef;

#include <ImageIO/ImageIOBase.h>

#import <CoreGraphics/CGImage.h>
#import <ImageIO/CGImageSource.h>
#import <CoreGraphics/CGDataConsumer.h>
#import <CoreFoundation/CFURL.h>

IMAGEIO_EXTERN const CFStringRef kCGImageDestinationLossyCompressionQuality;
IMAGEIO_EXTERN const CFStringRef kCGImageDestinationBackgroundColor;
IMAGEIO_EXTERN const CFStringRef kCGImageDestinationDPI;

IMAGEIO_EXTERN CFTypeID CGImageDestinationGetTypeID(void);

IMAGEIO_EXTERN CFArrayRef CGImageDestinationCopyTypeIdentifiers(void);

IMAGEIO_EXTERN CGImageDestinationRef CGImageDestinationCreateWithData(CFMutableDataRef data, CFStringRef type, size_t imageCount, CFDictionaryRef options);
IMAGEIO_EXTERN CGImageDestinationRef CGImageDestinationCreateWithDataConsumer(CGDataConsumerRef dataConsumer, CFStringRef type, size_t imageCount, CFDictionaryRef options);
IMAGEIO_EXTERN CGImageDestinationRef CGImageDestinationCreateWithURL(CFURLRef url, CFStringRef type, size_t imageCount, CFDictionaryRef options);

IMAGEIO_EXTERN void CGImageDestinationSetProperties(CGImageDestinationRef self, CFDictionaryRef properties);

IMAGEIO_EXTERN void CGImageDestinationAddImage(CGImageDestinationRef self, CGImageRef image, CFDictionaryRef properties);
IMAGEIO_EXTERN void CGImageDestinationAddImageFromSource(CGImageDestinationRef self, CGImageSourceRef imageSource, size_t index, CFDictionaryRef properties);

IMAGEIO_EXTERN bool CGImageDestinationFinalize(CGImageDestinationRef self);
