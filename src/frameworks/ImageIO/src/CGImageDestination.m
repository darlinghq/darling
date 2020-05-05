#import <ImageIO/CGImageDestination.h>
#import <Onyx2D/O2ImageDestination.h>
#import <Onyx2D/O2ImageSource.h>

#include <CoreFoundation/CoreFoundation.h>

const CFStringRef kCGImageDestinationLossyCompressionQuality=(CFStringRef)@"kCGImageDestinationLossyCompressionQuality";
const CFStringRef kCGImageDestinationBackgroundColor=(CFStringRef)@"kCGImageDestinationBackgroundColor";
const CFStringRef kCGImageDestinationDPI=(CFStringRef)@"kCGImageDestinationDPI";

CFTypeID CGImageDestinationGetTypeID(void) {
   return O2ImageDestinationGetTypeID();
}

CFArrayRef CGImageDestinationCopyTypeIdentifiers(void) {
   return O2ImageDestinationCopyTypeIdentifiers();
}

CGImageDestinationRef CGImageDestinationCreateWithData(CFMutableDataRef data,CFStringRef type,size_t imageCount,CFDictionaryRef options) {
   return (CGImageDestinationRef)O2ImageDestinationCreateWithData(data,type,imageCount,options);
}

CGImageDestinationRef CGImageDestinationCreateWithDataConsumer(CGDataConsumerRef dataConsumer,CFStringRef type,size_t imageCount,CFDictionaryRef options) {
   return (CGImageDestinationRef)O2ImageDestinationCreateWithDataConsumer((O2DataConsumerRef)dataConsumer,type,imageCount,options);
}

CGImageDestinationRef CGImageDestinationCreateWithURL(CFURLRef url,CFStringRef type,size_t imageCount,CFDictionaryRef options) {
   return (CGImageDestinationRef)O2ImageDestinationCreateWithURL(url,type,imageCount,options);
}

void CGImageDestinationSetProperties(CGImageDestinationRef self,CFDictionaryRef properties) {
   O2ImageDestinationSetProperties((O2ImageDestinationRef)self,properties);
}

void CGImageDestinationAddImage(CGImageDestinationRef self,CGImageRef image,CFDictionaryRef properties) {
   O2ImageDestinationAddImage((O2ImageDestinationRef)self,(O2ImageRef)image,properties);
}

void CGImageDestinationAddImageFromSource(CGImageDestinationRef self,CGImageSourceRef imageSource,size_t index,CFDictionaryRef properties) {
   O2ImageDestinationAddImageFromSource((O2ImageDestinationRef)self,(O2ImageSourceRef)imageSource,index,properties);
}


bool CGImageDestinationFinalize(CGImageDestinationRef self) {
   return O2ImageDestinationFinalize((O2ImageDestinationRef)self);
}
