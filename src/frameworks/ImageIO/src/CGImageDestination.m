#import <ImageIO/CGImageDestination.h>
#import <Onyx2D/O2ImageDestination.h>
#import <Onyx2D/O2ImageSource.h>
#import <Onyx2D/O2BitmapContext.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2Image.h>

#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <objc/runtime.h>
#include <stdint.h>

// Set on a destination (associated object) when an image could not be added; Finalize then fails.
static char ImageIOFailureKey;

const CFStringRef kCGImageDestinationLossyCompressionQuality=(CFStringRef)@"kCGImageDestinationLossyCompressionQuality";
const CFStringRef kCGImageDestinationBackgroundColor=(CFStringRef)@"kCGImageDestinationBackgroundColor";

// Each value is the constant's own name (read on macOS 26.7.3).
// Test: tests/src/imageio_property_names.c
//
// kCGImageDestinationImageMaxPixelSize is a CFNumber that Apple documents (ImageIO, kCGImageDestinationImageMaxPixelSize)
// as: if present, the destination rescales the image to fit within the maximum width and height; if absent, the native size
// is kept. Observed on macOS 26.7.3 and 10.14.4 (x86_64 and i386), where the documentation is silent, and followed here:
//  - the image is scaled so that its longer side is exactly the limit, so a limit above the image's size enlarges it (a 40x20
//    image at 100 is written as 100x50), and an image whose longer side is the limit is added unchanged;
//  - the other side is the exact proportion truncated, with a minimum of one pixel (40x20 at 7 is 7x3, 40x1 at 10 is 10x1);
//  - CGImageDestinationAddImageFromSource does the same as AddImage (10.14.4; 26.7.3 only reduces there and rounds the
//    other side to nearest, which is not followed);
//  - the key is read from the per-image properties of those two calls, not from the options of CGImageDestinationCreate...
//    and not from CGImageDestinationSetProperties; every format of the destination honours it;
//  - the value is a number: a double is truncated to an integer (10.5 is 10), and a number below 1, a NaN, and a number from
//    about 1e10 up (the cutoff is not known; 2^31 is used here) leave the image alone; a string and a boolean are ignored
//    (26.7.3 reads a numeric string and a boolean as numbers, which is not followed because 10.14.4 ignores them).
// Chosen where macOS gives no answer: the scaled copy is a device RGB bitmap, and an image that cannot be scaled (the bitmap
// cannot be allocated) is not added at full size and Finalize returns false.
// Test: tests/imageio-destination-max-pixel-size.c
// OptimizeColorForSharing and PreserveGainMap are accepted and have no effect yet.
const CFStringRef kCGImageDestinationImageMaxPixelSize = CFSTR("kCGImageDestinationImageMaxPixelSize");
const CFStringRef kCGImageDestinationOptimizeColorForSharing = CFSTR("kCGImageDestinationOptimizeColorForSharing");
const CFStringRef kCGImageDestinationPreserveGainMap = CFSTR("kCGImageDestinationPreserveGainMap");

// The limit the properties hold under kCGImageDestinationImageMaxPixelSize: a number of at least 1 (truncated) and below
// 2^31, else 0 (no limit).
static size_t maxPixelSizeLimit(CFDictionaryRef properties) {
   CFTypeRef value=properties!=NULL?CFDictionaryGetValue(properties,kCGImageDestinationImageMaxPixelSize):NULL;
   double requested=0;
   if (value==NULL || CFGetTypeID(value)!=CFNumberGetTypeID() || !CFNumberGetValue((CFNumberRef)value,kCFNumberDoubleType,&requested) || !(requested>=1) || !(requested<2147483648.0)) {
      return 0;
   }
   return (size_t)requested;
}

static size_t longerSide(O2ImageRef image) {
   const size_t width=O2ImageGetWidth(image),height=O2ImageGetHeight(image);
   return width>height?width:height;
}

// A new RGBA copy of `image` whose longer side is `limit` pixels, aspect ratio kept, the other side truncated to at least one
// pixel. The caller has checked that the longer side is not `limit` already. NULL when the bitmap cannot be made (the bitmap
// allocation can also raise).
static O2ImageRef createImageWithin(O2ImageRef image,size_t limit) {
   size_t width=O2ImageGetWidth(image),height=O2ImageGetHeight(image);
   const size_t longer=longerSide(image);
   // One division of an exact product, so a proportion that is a whole number comes out whole.
   width=width==longer?limit:(size_t)fmax(1,floor((double)width*(double)limit/(double)longer));
   height=height==longer?limit:(size_t)fmax(1,floor((double)height*(double)limit/(double)longer));
   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceRGB();
   O2ContextRef context=O2BitmapContextCreate(NULL,width,height,8,width*4,colorSpace,kO2ImageAlphaPremultipliedLast|kO2BitmapByteOrder32Big);
   O2ColorSpaceRelease(colorSpace);
   O2ImageRef result=NULL;
   if (context!=NULL) {
      O2ContextSetInterpolationQuality(context,kO2InterpolationHigh);
      O2ContextDrawImage(context,O2RectMake(0,0,width,height),image);
      result=O2BitmapContextCreateImage(context);
      O2ContextRelease(context);
   }
   return result;
}

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
   size_t limit=maxPixelSizeLimit(properties);
   if (limit!=0 && image!=NULL && longerSide((O2ImageRef)image)!=limit) {
      O2ImageRef scaled=NULL;
      @try {
         scaled=createImageWithin((O2ImageRef)image,limit);
      } @catch (NSException *exception) {
         scaled=NULL;
      }
      if (scaled==NULL) {
         // Never fall back to the full-size image: the caller asked for a bounded one.
         objc_setAssociatedObject((id)self,&ImageIOFailureKey,(id)kCFBooleanTrue,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
         return;
      }
      O2ImageDestinationAddImage((O2ImageDestinationRef)self,scaled,properties);
      O2ImageRelease(scaled);
      return;
   }
   O2ImageDestinationAddImage((O2ImageDestinationRef)self,(O2ImageRef)image,properties);
}

void CGImageDestinationAddImageFromSource(CGImageDestinationRef self,CGImageSourceRef imageSource,size_t index,CFDictionaryRef properties) {
   if (maxPixelSizeLimit(properties)!=0) {
      O2ImageRef image=O2ImageSourceCreateImageAtIndex((O2ImageSourceRef)imageSource,index,NULL);
      CGImageDestinationAddImage(self,(CGImageRef)image,properties);
      O2ImageRelease(image);
      return;
   }
   O2ImageDestinationAddImageFromSource((O2ImageDestinationRef)self,(O2ImageSourceRef)imageSource,index,properties);
}


bool CGImageDestinationFinalize(CGImageDestinationRef self) {
   if (objc_getAssociatedObject((id)self,&ImageIOFailureKey)!=nil) {
      return false;
   }
   return O2ImageDestinationFinalize((O2ImageDestinationRef)self);
}

// The CGImageMetadata functions below are limited: tags are kept in a plain
// dictionary keyed by path and are not written into the encoded image, so
// AddImageAndMetadata adds the image and drops the metadata.
void CGImageDestinationAddImageAndMetadata(CGImageDestinationRef self, CGImageRef image, CGImageMetadataRef metadata, CFDictionaryRef properties) {
   CGImageDestinationAddImage(self, image, properties);
}

CGMutableImageMetadataRef CGImageMetadataCreateMutable(void) {
   return (CGMutableImageMetadataRef)CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
}

CGImageMetadataTagRef CGImageMetadataTagCreate(CFStringRef xmlns, CFStringRef prefix, CFStringRef name, int type, CFTypeRef value) {
   return (CGImageMetadataTagRef)CFRetain(value ? value : (CFTypeRef)kCFNull);
}

bool CGImageMetadataSetTagWithPath(CGMutableImageMetadataRef metadata, CGImageMetadataTagRef parent, CFStringRef path, CGImageMetadataTagRef tag) {
   if (metadata && path && tag) {
      CFDictionarySetValue((CFMutableDictionaryRef)metadata, path, (const void*)tag);
      return true;
   }
   return false;
}
