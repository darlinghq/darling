// Copyright (C) 2026 Lubos Dolezel
//
// CGImageSourceCreateIncremental and CGImageSourceUpdateData (Apple's
// CGImageSource reference): an incremental source starts with no data, is given
// all the data received so far on each update, and has its images once that
// data is enough to decode them; it is complete once the final data is given.
// Also CGImageSourceGetStatusAtIndex and CGImageSourceGetPrimaryImageIndex
// (0 for formats that have no primary image, which is all but HEIF), and the
// private CGImageSourceGetTypeWithData and CGImageSourceSetAllowableTypes.
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <stdio.h>

// private API that WebKit uses (its ImageIOSPI.h)
CFStringRef CGImageSourceGetTypeWithData(CFDataRef, CFStringRef, bool *);
OSStatus CGImageSourceSetAllowableTypes(CFArrayRef allowableTypes);

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } else printf("PASS %s\n", #cond); } while (0)

static CFDataRef pngOf4x4(void) {
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(NULL, 4, 4, 8, 0, space, kCGImageAlphaPremultipliedLast);
    CGContextSetRGBFillColor(context, 0, 1, 0, 1);
    CGContextFillRect(context, CGRectMake(0, 0, 4, 4));
    CGImageRef image = CGBitmapContextCreateImage(context);
    CFMutableDataRef data = CFDataCreateMutable(NULL, 0);
    CGImageDestinationRef dest = CGImageDestinationCreateWithData(data, CFSTR("public.png"), 1, NULL);
    CGImageDestinationAddImage(dest, image, NULL);
    CGImageDestinationFinalize(dest);
    CFRelease(dest);
    CGImageRelease(image);
    CGContextRelease(context);
    CGColorSpaceRelease(space);
    return data;
}

int main(void) {
    CFDataRef png = pngOf4x4();
    CHECK(CFDataGetLength(png) > 8);

    CGImageSourceRef source = CGImageSourceCreateIncremental(NULL);
    CHECK(source != NULL);
    CHECK(CGImageSourceGetCount(source) == 0);
    CHECK(CGImageSourceGetStatus(source) == kCGImageStatusReadingHeader);
    CHECK(CGImageSourceCreateImageAtIndex(source, 0, NULL) == NULL);

    // too little to tell the type
    CFDataRef start = CFDataCreate(NULL, CFDataGetBytePtr(png), 4);
    CGImageSourceUpdateData(source, start, false);
    CHECK(CGImageSourceGetStatus(source) == kCGImageStatusReadingHeader);
    CHECK(CGImageSourceGetStatusAtIndex(source, 0) == kCGImageStatusReadingHeader);
    CFRelease(start);

    // everything, and final
    CGImageSourceUpdateData(source, png, true);
    CHECK(CGImageSourceGetStatus(source) == kCGImageStatusComplete);
    CHECK(CGImageSourceGetCount(source) == 1);
    CHECK(CGImageSourceGetStatusAtIndex(source, 0) == kCGImageStatusComplete);
    CHECK(CGImageSourceGetStatusAtIndex(source, 1) == kCGImageStatusInvalidData);
    CHECK(CGImageSourceGetType(source) != NULL && CFEqual(CGImageSourceGetType(source), CFSTR("public.png")));
    CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
    CHECK(image != NULL && CGImageGetWidth(image) == 4 && CGImageGetHeight(image) == 4);
    if (image != NULL)
        CGImageRelease(image);
    CHECK(CGImageSourceGetPrimaryImageIndex(source) == 0);
    CFRelease(source);

    // data that is final but no image format
    source = CGImageSourceCreateIncremental(NULL);
    CFDataRef junk = CFDataCreate(NULL, (const UInt8 *) "not an image at all", 19);
    CGImageSourceUpdateData(source, junk, true);
    CHECK(CGImageSourceGetCount(source) == 0);
    CHECK(CGImageSourceGetStatus(source) == kCGImageStatusUnknownType);
    CFRelease(junk);
    CFRelease(source);

    // a source made from complete data
    source = CGImageSourceCreateWithData(png, NULL);
    CHECK(CGImageSourceGetStatusAtIndex(source, 0) == kCGImageStatusComplete);
    CHECK(CGImageSourceGetStatusAtIndex(NULL, 0) == kCGImageStatusInvalidData);
    CFRelease(source);

    // the type of the data, and the decodable types limited
    CHECK(CGImageSourceGetTypeWithData(png, NULL, NULL) != NULL
          && CFEqual(CGImageSourceGetTypeWithData(png, NULL, NULL), CFSTR("public.png")));
    CHECK(CGImageSourceGetTypeWithData(NULL, NULL, NULL) == NULL);
    const void *jpegOnly[] = { CFSTR("public.jpeg") };
    CFArrayRef allowed = CFArrayCreate(NULL, jpegOnly, 1, &kCFTypeArrayCallBacks);
    CHECK(CGImageSourceSetAllowableTypes(allowed) == 0);
    CHECK(CGImageSourceCreateWithData(png, NULL) == NULL);
    CHECK(CGImageSourceSetAllowableTypes(NULL) == 0);
    source = CGImageSourceCreateWithData(png, NULL);
    CHECK(source != NULL);
    CFRelease(source);
    CFRelease(allowed);
    CFRelease(png);

    printf("%s\n", failures ? "FAILED" : "ALL PASSED");
    return failures != 0;
}
