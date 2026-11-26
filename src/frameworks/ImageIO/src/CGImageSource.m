#import <ImageIO/CGImageSource.h>
#import <ImageIO/CGImageProperties.h>
#import <Onyx2D/O2ImageSource.h>
#import <Onyx2D/O2BitmapContext.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2Image.h>
#import <Onyx2D/O2DataProvider.h>
#import "ImageIOASTC.h"
#include <math.h>
#include <pthread.h>
#include <CarbonCore/MacErrors.h>
#include <string.h>

const CFStringRef kCGImageSourceCreateThumbnailFromImageAlways = CFSTR("kCGImageSourceCreateThumbnailFromImageAlways");
const CFStringRef kCGImageSourceCreateThumbnailFromImageIfAbsent = CFSTR("kCGImageSourceCreateThumbnailFromImageIfAbsent");
const CFStringRef kCGImageSourceCreateThumbnailWithTransform = CFSTR("kCGImageSourceCreateThumbnailWithTransform");
const CFStringRef kCGImageSourceShouldAllowFloat = CFSTR("kCGImageSourceShouldAllowFloat");
const CFStringRef kCGImageSourceShouldCache = CFSTR("kCGImageSourceShouldCache");
// Option value: the image is decoded when it is created. Decoding is always lazy here, so the option is
// accepted and has no effect. The string value is the name (WebCore ImageDecoderCG.cpp defines it so).
const CFStringRef kCGImageSourceTypeIdentifierHint = CFSTR("kCGImageSourceTypeIdentifierHint");
const CFStringRef kCGImageSourceShouldCacheImmediately = CFSTR("kCGImageSourceShouldCacheImmediately");
const CFStringRef kCGImageSourceThumbnailMaxPixelSize = CFSTR("kCGImageSourceThumbnailMaxPixelSize");
// Options the decoders here accept and ignore: they always decode at full size,
// to their own pixel format, and in software. The values follow the other
// options in being the constant's own name; for the private ones
// (ShouldPreferRGB32, SkipMetadata, UseHardwareAcceleration) that is inferred
// from their public siblings, not observed.
const CFStringRef kCGImageSourceSubsampleFactor = CFSTR("kCGImageSourceSubsampleFactor");
const CFStringRef kCGImageSourceShouldPreferRGB32 = CFSTR("kCGImageSourceShouldPreferRGB32");
const CFStringRef kCGImageSourceSkipMetadata = CFSTR("kCGImageSourceSkipMetadata");
const CFStringRef kCGImageSourceUseHardwareAcceleration = CFSTR("kCGImageSourceUseHardwareAcceleration");

// The types CGImageSourceSetAllowableTypes limits decoding to; NULL allows all.
static CFArrayRef allowableTypes;
static pthread_mutex_t allowableTypesLock = PTHREAD_MUTEX_INITIALIZER;

static BOOL typeIsAllowed(CFStringRef type) {
    pthread_mutex_lock(&allowableTypesLock);
    BOOL allowed = allowableTypes == NULL || (type != NULL &&
        CFArrayContainsValue(allowableTypes, CFRangeMake(0, CFArrayGetCount(allowableTypes)), type));
    pthread_mutex_unlock(&allowableTypesLock);
    return allowed;
}

// A source for data of a type that is not allowed is no source.
static O2ImageSource *allowedSource(O2ImageSource *source) {
    if (source != nil && !typeIsAllowed([source type])) {
        [source release];
        return nil;
    }
    return source;
}

// Each value is the constant's own name (read on macOS 26.7.3).
// Test: tests/src/imageio_property_names.c
const CFStringRef kCGImageAuxiliaryDataTypeDepth = CFSTR("kCGImageAuxiliaryDataTypeDepth");
const CFStringRef kCGImageAuxiliaryDataTypeDisparity = CFSTR("kCGImageAuxiliaryDataTypeDisparity");
const CFStringRef kCGImageAuxiliaryDataTypeHDRGainMap = CFSTR("kCGImageAuxiliaryDataTypeHDRGainMap");
const CFStringRef kCGImageAuxiliaryDataTypeISOGainMap = CFSTR("kCGImageAuxiliaryDataTypeISOGainMap");
const CFStringRef kCGImageAuxiliaryDataInfoData = CFSTR("kCGImageAuxiliaryDataInfoData");
const CFStringRef kCGImageAuxiliaryDataInfoDataDescription = CFSTR("kCGImageAuxiliaryDataInfoDataDescription");

@interface _O2ImageSource : O2ImageSource
@end

// The formats ImageIO decodes itself (ASTC) are tried first; everything else goes to the Onyx2D
// decoders, as before. The magic numbers do not overlap, so the order only saves work.
static O2ImageSourceRef newImageSource(O2DataProviderRef provider, CFDictionaryRef options) {
	O2ImageSourceRef source = ImageIOASTCSourceCreate(provider, options);
	if (source != nil)
		return source;
	return [O2ImageSource newImageSourceWithDataProvider:provider options:options];
}

static O2ImageSourceRef newImageSourceWithData(CFDataRef data, CFDictionaryRef options) {
	O2DataProviderRef provider = O2DataProviderCreateWithCFData(data);
	O2ImageSourceRef source = newImageSource(provider, options);
	O2DataProviderRelease(provider);
	return source;
}

// An incremental source (CGImageSourceCreateIncremental). It is given all the
// data received so far each time, and decodes it with a source made from that
// data; until then it has no images.
@interface _CGIncrementalImageSource : O2ImageSource {
    O2ImageSource *_decoder;
    BOOL _final;
}
- (void) updateData: (CFDataRef) data final: (BOOL) final;
- (CGImageSourceStatus) status;
- (CGImageSourceStatus) statusAtIndex: (size_t) index;
@end

@implementation _CGIncrementalImageSource

- initWithOptions: (CFDictionaryRef) options {
    [super init];
    _options = options != NULL ? [(NSDictionary *) options copy] : nil;
    return self;
}

- (void) dealloc {
    [_decoder release];
    [super dealloc];
}

- (void) updateData: (CFDataRef) data final: (BOOL) final {
    if (_final)
        return; // the data was already complete
    O2ImageSource *decoder = data != NULL
        ? allowedSource(newImageSourceWithData(data, (CFDictionaryRef) _options))
        : nil;
    [_decoder release];
    _decoder = decoder;
    _final = final;
}

- (NSUInteger) count {
    return _decoder != nil ? [_decoder count] : 0;
}

- (CFStringRef) type {
    return _decoder != nil ? [_decoder type] : NULL;
}

- (CFDictionaryRef) copyPropertiesAtIndex: (NSUInteger) index options: (CFDictionaryRef) options {
    return _decoder != nil ? [_decoder copyPropertiesAtIndex: index options: options] : NULL;
}

- (O2ImageRef) createImageAtIndex: (NSUInteger) index options: (CFDictionaryRef) options {
    return _decoder != nil ? [_decoder createImageAtIndex: index options: options] : NULL;
}

- (CGImageSourceStatus) status {
    if (_decoder == nil)
        return _final ? kCGImageStatusUnknownType : kCGImageStatusReadingHeader;
    if ([_decoder count] == 0)
        return _final ? kCGImageStatusInvalidData : kCGImageStatusReadingHeader;
    return _final ? kCGImageStatusComplete : kCGImageStatusIncomplete;
}

- (CGImageSourceStatus) statusAtIndex: (size_t) index {
    if (_decoder == nil || index >= [_decoder count])
        return _final ? kCGImageStatusInvalidData : kCGImageStatusReadingHeader;
    return _final ? kCGImageStatusComplete : kCGImageStatusIncomplete;
}

@end

CGImageSourceRef CGImageSourceCreateIncremental(CFDictionaryRef options) {
    return (CGImageSourceRef)[[_CGIncrementalImageSource alloc] initWithOptions: options];
}

void CGImageSourceUpdateData(CGImageSourceRef isrc, CFDataRef data, bool final) {
    if ([(id) isrc isKindOfClass: [_CGIncrementalImageSource class]])
        [(_CGIncrementalImageSource *) isrc updateData: data final: final];
}

CGImageSourceRef CGImageSourceCreateWithData(CFDataRef data,CFDictionaryRef options) {
	return (CGImageSourceRef)allowedSource(newImageSourceWithData(data, options));
}

CGImageSourceRef CGImageSourceCreateWithURL(CFURLRef url,CFDictionaryRef options) {
	O2DataProviderRef provider = [[O2DataProvider alloc] initWithURL:(NSURL *)url];
	if (provider == nil)
		return NULL;
	O2ImageSourceRef source = newImageSource(provider, options);
	O2DataProviderRelease(provider);
	return (CGImageSourceRef)allowedSource(source);
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

// Limited: file-level properties are not tracked separately, so this
// returns the properties of the first image, or NULL for an empty source.
CFDictionaryRef CGImageSourceCopyProperties(CGImageSourceRef self, CFDictionaryRef options) {
   if (!self) return NULL;
   if ([self count] > 0) {
      return CGImageSourceCopyPropertiesAtIndex(self, 0, options);
   }
   return NULL;
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

// No decoder here reads auxiliary data (depth, disparity, gain maps), so every
// source has none; the documented answer for that is NULL. macOS also answers
// NULL for a PNG for each of the four types.
CFDictionaryRef CGImageSourceCopyAuxiliaryDataInfoAtIndex(CGImageSourceRef isrc, size_t index, CFStringRef auxiliaryImageDataType)
{
    return NULL;
}

// The types the Onyx2D decoders behind O2ImageSource can read. macOS lists 62;
// all of these are among them. The gap is the formats we cannot decode.
// Test: tests/src/imageio_source_queries.c reads each type back from data.
CFArrayRef CGImageSourceCopyTypeIdentifiers(void)
{
    const void *types[] = {
        CFSTR("public.png"), CFSTR("public.tiff"), CFSTR("public.jpeg"),
        CFSTR("com.microsoft.bmp"), CFSTR("com.compuserve.gif"), CFSTR("com.apple.icns"),
        CFSTR("com.adobe.photoshop-image"), CFSTR("org.khronos.astc")
    };
    return CFArrayCreate(NULL, types, sizeof types / sizeof types[0], &kCFTypeArrayCallBacks);
}

// There are no incremental sources here, so a source that exists was created
// from complete data of a recognised type. macOS reports complete (0) for a PNG
// source and kCGImageStatusInvalidData (-4) for NULL (observed on macOS 26.7.3).
CGImageSourceStatus CGImageSourceGetStatus(CGImageSourceRef isrc)
{
    if (isrc == NULL)
        return kCGImageStatusInvalidData;
    if ([(id) isrc isKindOfClass: [_CGIncrementalImageSource class]])
        return [(_CGIncrementalImageSource *) isrc status];
    return kCGImageStatusComplete;
}

// A source made from complete data has each of its images complete.
CGImageSourceStatus CGImageSourceGetStatusAtIndex(CGImageSourceRef isrc, size_t index)
{
    if (isrc == NULL)
        return kCGImageStatusInvalidData;
    if ([(id) isrc isKindOfClass: [_CGIncrementalImageSource class]])
        return [(_CGIncrementalImageSource *) isrc statusAtIndex: index];
    return index < [(O2ImageSource *) isrc count] ? kCGImageStatusComplete : kCGImageStatusInvalidData;
}

// Only HEIF files name a primary image, and there is no HEIF decoder here, so
// it is always the first one.
size_t CGImageSourceGetPrimaryImageIndex(CGImageSourceRef isrc)
{
    return 0;
}

// Private API that WebKit uses (declared by its ImageIOSPI.h).

// The type of image the data holds, or NULL. The hint is for data no decoder
// recognises, which none of them here has use for, and the last argument is
// left alone. The type strings are constants, so the result needs no owner.
CFStringRef CGImageSourceGetTypeWithData(CFDataRef data, CFStringRef hint, bool *unused)
{
    (void) hint;
    (void) unused;
    if (data == NULL)
        return NULL;
    O2ImageSource *source = [O2ImageSource newImageSourceWithData: data options: NULL];
    CFStringRef type = [source type];
    [source release];
    return type;
}

// Limits the types later sources decode (an array of type identifiers); NULL
// lifts the limit.
OSStatus CGImageSourceSetAllowableTypes(CFArrayRef types)
{
    if (types != NULL && CFGetTypeID(types) != CFArrayGetTypeID())
        return paramErr;
    CFArrayRef copy = types != NULL ? CFArrayCreateCopy(NULL, types) : NULL;
    pthread_mutex_lock(&allowableTypesLock);
    CFArrayRef old = allowableTypes;
    allowableTypes = copy;
    pthread_mutex_unlock(&allowableTypesLock);
    if (old != NULL)
        CFRelease(old);
    return noErr;
}

// Decoding is always in software, so there is no hardware decoding to turn
// off.
OSStatus CGImageSourceDisableHardwareDecoding(void)
{
    return noErr;
}

// Restricted decoding limits a process to the decoders deemed safest. All the
// decoders here are of that kind already (none are hardware or plug-in based),
// so nothing changes.
OSStatus CGImageSourceEnableRestrictedDecoding(void)
{
    return noErr;
}
