// Copyright (C) 2026 Lubos Dolezel
//
// CGImageSourceCopyTypeIdentifiers, CGImageSourceGetStatus and
// CGImageSourceCopyAuxiliaryDataInfoAtIndex. On macOS 26.7.3 the type list has
// 62 entries and contains every type below, a PNG source reports status 0
// (complete), NULL reports -4 (invalid data), and a PNG has no auxiliary data of
// any type (NULL).
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static CFDataRef encode(CGImageRef image, CFStringRef type) {
    CFMutableDataRef data = CFDataCreateMutable(NULL, 0);
    CGImageDestinationRef dest = CGImageDestinationCreateWithData(data, type, 1, NULL);
    if (dest == NULL) {
        CFRelease(data);
        return NULL;
    }
    CGImageDestinationAddImage(dest, image, NULL);
    bool ok = CGImageDestinationFinalize(dest);
    CFRelease(dest);
    // An encoder that produces nothing for this image (for example icns, which
    // only holds the standard icon sizes) has nothing to read back.
    if (!ok || CFDataGetLength(data) == 0) {
        CFRelease(data);
        return NULL;
    }
    return data;
}

int main(void) {
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(NULL, 4, 4, 8, 0, space, kCGImageAlphaPremultipliedLast);
    CGContextSetRGBFillColor(context, 0, 1, 0, 1);
    CGContextFillRect(context, CGRectMake(0, 0, 4, 4));
    CGImageRef image = CGBitmapContextCreateImage(context);

    CFArrayRef types = CGImageSourceCopyTypeIdentifiers();
    CHECK(types != NULL);
    // Every listed type is a type this implementation can decode: where the
    // destination can write it, the source must recognise the data as that type.
    // Each type listed here must be one the source recognises from data of that
    // format (the formats the destination can write are round-tripped).
    CFStringRef listed[] = { CFSTR("public.png"), CFSTR("public.tiff"), CFSTR("public.jpeg"),
                             CFSTR("com.microsoft.bmp"), CFSTR("com.compuserve.gif"), CFSTR("com.apple.icns") };
    CFStringRef required[] = { CFSTR("public.png"), CFSTR("public.tiff"), CFSTR("public.jpeg") };
    for (size_t i = 0; i < 3; i++)
        CHECK(types != NULL && CFArrayContainsValue(types, CFRangeMake(0, CFArrayGetCount(types)), required[i]));
    for (size_t i = 0; i < 6; i++) {
        if (types == NULL || !CFArrayContainsValue(types, CFRangeMake(0, CFArrayGetCount(types)), listed[i]))
            continue;
        CFDataRef data = encode(image, listed[i]);
        if (data == NULL)
            continue;
        CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
        CHECK(source != NULL);
        if (source != NULL) {
            CHECK(CFEqual(CGImageSourceGetType(source), listed[i]));
            CFRelease(source);
        }
        CFRelease(data);
    }

    CHECK(CGImageSourceGetStatus(NULL) == kCGImageStatusInvalidData);
    CFDataRef png = encode(image, CFSTR("public.png"));
    CHECK(png != NULL);
    CGImageSourceRef source = png ? CGImageSourceCreateWithData(png, NULL) : NULL;
    CHECK(source != NULL);
    if (source != NULL) {
        CHECK(CGImageSourceGetStatus(source) == kCGImageStatusComplete);
        CFStringRef kinds[] = { kCGImageAuxiliaryDataTypeDepth, kCGImageAuxiliaryDataTypeDisparity,
                                kCGImageAuxiliaryDataTypeHDRGainMap, kCGImageAuxiliaryDataTypeISOGainMap };
        for (size_t i = 0; i < 4; i++)
            CHECK(CGImageSourceCopyAuxiliaryDataInfoAtIndex(source, 0, kinds[i]) == NULL);
        CFRelease(source);
    }
    if (failures == 0)
        printf("PASS: ImageIO source queries\n");
    return failures ? 1 : 0;
}
