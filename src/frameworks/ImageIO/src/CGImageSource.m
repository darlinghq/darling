#import <ImageIO/CGImageSource.h>
#import <ImageIO/CGImageProperties.h>
#import <Onyx2D/O2ImageSource.h>
#import <Onyx2D/O2BitmapContext.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2Image.h>
#include <math.h>
#include <string.h>

const CFStringRef kCGImageSourceCreateThumbnailFromImageAlways = CFSTR("kCGImageSourceCreateThumbnailFromImageAlways");
const CFStringRef kCGImageSourceCreateThumbnailFromImageIfAbsent = CFSTR("kCGImageSourceCreateThumbnailFromImageIfAbsent");
const CFStringRef kCGImageSourceCreateThumbnailWithTransform = CFSTR("kCGImageSourceCreateThumbnailWithTransform");
const CFStringRef kCGImageSourceShouldAllowFloat = CFSTR("kCGImageSourceShouldAllowFloat");
const CFStringRef kCGImageSourceShouldCache = CFSTR("kCGImageSourceShouldCache");
const CFStringRef kCGImageSourceThumbnailMaxPixelSize = CFSTR("kCGImageSourceThumbnailMaxPixelSize");

@interface _O2ImageSource : O2ImageSource
@end

CGImageSourceRef CGImageSourceCreateWithData(CFDataRef data,CFDictionaryRef options) {
	return (CGImageSourceRef)[O2ImageSource newImageSourceWithData:data options:options];
}

CGImageSourceRef CGImageSourceCreateWithURL(CFURLRef url,CFDictionaryRef options) {
	return (CGImageSourceRef)[O2ImageSource newImageSourceWithURL:(NSURL *)url options:options];
}

size_t CGImageSourceGetCount(CGImageSourceRef self) {
   return [self count];
}

CGImageRef CGImageSourceCreateImageAtIndex(CGImageSourceRef self,size_t index,CFDictionaryRef options) {
   return (CGImageRef)[self createImageAtIndex:index options:options];
}

CFDictionaryRef CGImageSourceCopyPropertiesAtIndex(CGImageSourceRef self, size_t index,CFDictionaryRef options) {
   return (CFDictionaryRef)[self copyPropertiesAtIndex:index options:options];
}

static bool thumbnailOptionIsTrue(CFDictionaryRef options, CFStringRef key) {
    if (options == NULL)
        return false;
    CFTypeRef value = CFDictionaryGetValue(options, key);
    return value != NULL && CFGetTypeID(value) == CFBooleanGetTypeID() && CFBooleanGetValue((CFBooleanRef)value);
}

/* Source pixel, in the stored image (sw x sh, row 0 at the top), of the displayed
   pixel (x, y) for an EXIF/TIFF orientation (1-8). The definitions say where the
   stored 0th row and 0th column go: e.g. 6 = 0th row on the right, 0th column at the
   top (the stored image is rotated 90 degrees counter-clockwise). */
static void orientedSource(int orientation, size_t sw, size_t sh, size_t x, size_t y,
                           size_t *sx, size_t *sy)
{
    switch (orientation) {
    case 2: *sx = sw - 1 - x; *sy = y; break;
    case 3: *sx = sw - 1 - x; *sy = sh - 1 - y; break;
    case 4: *sx = x; *sy = sh - 1 - y; break;
    case 5: *sx = y; *sy = x; break;
    case 6: *sx = y; *sy = sh - 1 - x; break;
    case 7: *sx = sw - 1 - y; *sy = sh - 1 - x; break;
    case 8: *sx = sw - 1 - y; *sy = x; break;
    default: *sx = x; *sy = y; break;
    }
}

CGImageRef CGImageSourceCreateThumbnailAtIndex(CGImageSourceRef self, size_t index, CFDictionaryRef options) {
    /* The Onyx2D decoders expose no embedded thumbnails, so the only thumbnail there
       can be is one made from the full image, which the caller has to allow. */
    if (!thumbnailOptionIsTrue(options, kCGImageSourceCreateThumbnailFromImageAlways) &&
        !thumbnailOptionIsTrue(options, kCGImageSourceCreateThumbnailFromImageIfAbsent))
        return NULL;

    O2ImageRef image = (O2ImageRef)CGImageSourceCreateImageAtIndex(self, index, NULL);
    if (image == NULL)
        return NULL;
    size_t width = O2ImageGetWidth(image);
    size_t height = O2ImageGetHeight(image);

    int orientation = 1;
    if (thumbnailOptionIsTrue(options, kCGImageSourceCreateThumbnailWithTransform)) {
        CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(self, index, NULL);
        if (properties != NULL) {
            CFTypeRef value = CFDictionaryGetValue(properties, kCGImagePropertyOrientation);
            if (value != NULL && CFGetTypeID(value) == CFNumberGetTypeID())
                CFNumberGetValue((CFNumberRef)value, kCFNumberIntType, &orientation);
            CFRelease(properties);
        }
        if (orientation < 1 || orientation > 8)
            orientation = 1;
    }
    bool transposed = orientation >= 5;
    double displayW = transposed ? height : width;
    double displayH = transposed ? width : height;

    /* Without kCGImageSourceThumbnailMaxPixelSize the thumbnail may be as big as the
       image; it is never scaled up. */
    double scale = 1.0;
    CFTypeRef maxValue = options ? CFDictionaryGetValue(options, kCGImageSourceThumbnailMaxPixelSize) : NULL;
    if (maxValue != NULL && CFGetTypeID(maxValue) == CFNumberGetTypeID()) {
        double maxPixelSize = 0;
        CFNumberGetValue((CFNumberRef)maxValue, kCFNumberDoubleType, &maxPixelSize);
        double longest = fmax(displayW, displayH);
        if (maxPixelSize > 0 && longest > maxPixelSize)
            scale = maxPixelSize / longest;
    }
    if (orientation == 1 && scale == 1.0)
        return (CGImageRef)image;

    /* Scale upright first, then move the pixels for the orientation: the scaled draw
       is the plain path every image takes, and the remap is exact. */
    size_t storedW = (size_t)fmax(1, lround(width * scale));
    size_t storedH = (size_t)fmax(1, lround(height * scale));
    size_t thumbW = transposed ? storedH : storedW;
    size_t thumbH = transposed ? storedW : storedH;
    O2ColorSpaceRef colorSpace = O2ColorSpaceCreateDeviceRGB();
    O2BitmapInfo info = kO2BitmapByteOrder32Big | kO2ImageAlphaPremultipliedLast;
    O2ContextRef upright = O2BitmapContextCreate(NULL, storedW, storedH, 8, storedW * 4, colorSpace, info);
    O2ContextRef oriented = (upright != NULL && orientation != 1)
            ? O2BitmapContextCreate(NULL, thumbW, thumbH, 8, thumbW * 4, colorSpace, info)
            : NULL;
    O2ColorSpaceRelease(colorSpace);
    if (upright == NULL || (orientation != 1 && oriented == NULL)) {
        if (upright != NULL)
            O2ContextRelease(upright);
        O2ImageRelease(image);
        return NULL;
    }
    O2ContextDrawImage(upright, O2RectMake(0, 0, storedW, storedH), image);
    O2ImageRelease(image);

    O2ImageRef thumbnail;
    if (oriented == NULL) {
        thumbnail = O2BitmapContextCreateImage(upright);
    } else {
        const uint8_t *src = O2BitmapContextGetData(upright);
        uint8_t *dst = O2BitmapContextGetData(oriented);
        size_t srcRow = O2BitmapContextGetBytesPerRow(upright);
        size_t dstRow = O2BitmapContextGetBytesPerRow(oriented);
        for (size_t y = 0; y < thumbH; y++) {
            for (size_t x = 0; x < thumbW; x++) {
                size_t sx, sy;
                orientedSource(orientation, storedW, storedH, x, y, &sx, &sy);
                memcpy(dst + y * dstRow + x * 4, src + sy * srcRow + sx * 4, 4);
            }
        }
        thumbnail = O2BitmapContextCreateImage(oriented);
        O2ContextRelease(oriented);
    }
    O2ContextRelease(upright);
    return (CGImageRef)thumbnail;
}

CFStringRef CGImageSourceGetType(CGImageSourceRef self)
{
    return [(O2ImageSource*)self type];
}

CGImageSourceRef CGImageSourceCreateWithDataProvider(CGDataProviderRef provider, CFDictionaryRef options)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return nil;
}
