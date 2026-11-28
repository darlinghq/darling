// Copyright (C) 2026 Lubos Dolezel
//
// IOSurface component metadata comes from the surface's attachments. When an attachment is present its arrays
// define the components; when it is absent the accessors fall back to one component spanning the element's bits.
// Invalid planes and components return zero or the unknown enum value. Property maximums are nonzero only for
// properties with a compatibility limit.
// Build: clang -framework IOSurface -framework Foundation iosurface-component-metadata.m
#import <IOSurface/IOSurface.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

static void Set(IOSurfaceRef surface, CFStringRef key, NSArray *values)
{
    IOSurfaceSetValue(surface, key, (CFTypeRef) values);
}

int main(void)
{
    @autoreleasepool {
        NSDictionary *properties = [NSDictionary dictionaryWithObjectsAndKeys:
                [NSNumber numberWithInt: 16], (id) kIOSurfaceWidth,
                [NSNumber numberWithInt: 4], (id) kIOSurfaceHeight,
                [NSNumber numberWithInt: 64], (id) kIOSurfaceBytesPerRow,
                [NSNumber numberWithInt: 4], (id) kIOSurfaceBytesPerElement,
                [NSNumber numberWithInt: 256], (id) kIOSurfaceAllocSize,
                nil];
        IOSurfaceRef surface = IOSurfaceCreate((CFDictionaryRef) properties);
        CHECK(surface != NULL);
        if (surface == NULL)
            return 1;

        CHECK(IOSurfaceGetNumberOfComponentsOfPlane(surface, 1) == 0);

        Set(surface, kIOSurfacePlaneComponentBitDepths, @[@8, @8, @8, @8]);
        Set(surface, kIOSurfacePlaneComponentBitOffsets, @[@0, @8, @16, @24]);
        Set(surface, kIOSurfacePlaneComponentNames, @[
                @(kIOSurfaceComponentNameRed), @(kIOSurfaceComponentNameGreen),
                @(kIOSurfaceComponentNameBlue), @(kIOSurfaceComponentNameAlpha)]);
        Set(surface, kIOSurfacePlaneComponentTypes, @[
                @(kIOSurfaceComponentTypeUnsignedInteger), @(kIOSurfaceComponentTypeUnsignedInteger),
                @(kIOSurfaceComponentTypeUnsignedInteger), @(kIOSurfaceComponentTypeUnsignedInteger)]);
        Set(surface, kIOSurfacePlaneComponentRanges, @[
                @(kIOSurfaceComponentRangeFullRange), @(kIOSurfaceComponentRangeFullRange),
                @(kIOSurfaceComponentRangeFullRange), @(kIOSurfaceComponentRangeFullRange)]);
        IOSurfaceSetValue(surface, kIOSurfaceSubsampling,
                          (CFTypeRef) [NSNumber numberWithInt: kIOSurfaceSubsampling420]);

        CHECK(IOSurfaceGetNumberOfComponentsOfPlane(surface, 0) == 4);
        CHECK(IOSurfaceGetBitDepthOfComponentOfPlane(surface, 0, 2) == 8);
        CHECK(IOSurfaceGetBitOffsetOfComponentOfPlane(surface, 0, 3) == 24);
        CHECK(IOSurfaceGetNameOfComponentOfPlane(surface, 0, 0) == kIOSurfaceComponentNameRed);
        CHECK(IOSurfaceGetTypeOfComponentOfPlane(surface, 0, 1) == kIOSurfaceComponentTypeUnsignedInteger);
        CHECK(IOSurfaceGetRangeOfComponentOfPlane(surface, 0, 2) == kIOSurfaceComponentRangeFullRange);
        CHECK(IOSurfaceGetSubsampling(surface) == kIOSurfaceSubsampling420);

        CHECK(IOSurfaceGetBitDepthOfComponentOfPlane(surface, 0, 4) == 0);
        CHECK(IOSurfaceGetNameOfComponentOfPlane(surface, 0, 4) == kIOSurfaceComponentNameUnknown);

        IOSurfaceRemoveValue(surface, kIOSurfacePlaneComponentBitDepths);
        IOSurfaceRemoveValue(surface, kIOSurfacePlaneComponentBitOffsets);
        IOSurfaceRemoveValue(surface, kIOSurfacePlaneComponentNames);
        IOSurfaceRemoveValue(surface, kIOSurfacePlaneComponentTypes);
        IOSurfaceRemoveValue(surface, kIOSurfacePlaneComponentRanges);
        IOSurfaceRemoveValue(surface, kIOSurfaceSubsampling);

        CHECK(IOSurfaceGetNumberOfComponentsOfPlane(surface, 0) == 1);
        CHECK(IOSurfaceGetBitDepthOfComponentOfPlane(surface, 0, 0) == 32);
        CHECK(IOSurfaceGetBitOffsetOfComponentOfPlane(surface, 0, 0) == 0);
        CHECK(IOSurfaceGetNameOfComponentOfPlane(surface, 0, 0) == kIOSurfaceComponentNameUnknown);
        CHECK(IOSurfaceGetTypeOfComponentOfPlane(surface, 0, 0) == kIOSurfaceComponentTypeUnknown);
        CHECK(IOSurfaceGetRangeOfComponentOfPlane(surface, 0, 0) == kIOSurfaceComponentRangeUnknown);
        CHECK(IOSurfaceGetSubsampling(surface) == kIOSurfaceSubsamplingNone);

        size_t maximumWidth = IOSurfaceGetPropertyMaximum(kIOSurfaceWidth);
        CHECK(maximumWidth >= 1024 && maximumWidth <= 32768);
        CHECK(IOSurfaceGetPropertyMaximum(kIOSurfaceBytesPerRow) >= 64);
        CHECK(IOSurfaceGetPropertyMaximum(kIOSurfaceAllocSize) == 0);
        CHECK(IOSurfaceGetPropertyMaximum(kIOSurfacePixelFormat) == 0);
        CHECK(IOSurfaceGetPropertyMaximum(NULL) == 0);

        CFRelease(surface);
    }

    if (failures == 0)
        printf("PASS: IOSurface component metadata\n");
    return failures ? 1 : 0;
}
