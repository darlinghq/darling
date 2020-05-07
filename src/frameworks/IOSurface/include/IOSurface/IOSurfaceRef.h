#ifndef IOSURFACE_REF_H
#define IOSURFACE_REF_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOSurface/IOSurfaceTypes.h>

__BEGIN_DECLS

typedef struct  __IOSurface *IOSurfaceRef;


extern const CFStringRef kIOSurfaceAllocSize;
extern const CFStringRef kIOSurfaceWidth;
extern const CFStringRef kIOSurfaceHeight;
extern const CFStringRef kIOSurfaceBytesPerRow;
extern const CFStringRef kIOSurfaceBytesPerElement;
extern const CFStringRef kIOSurfaceElementWidth;
extern const CFStringRef kIOSurfaceElementHeight;
extern const CFStringRef kIOSurfaceOffset;
extern const CFStringRef kIOSurfacePlaneInfo;
extern const CFStringRef kIOSurfacePlaneWidth;
extern const CFStringRef kIOSurfacePlaneHeight;
extern const CFStringRef kIOSurfacePlaneBytesPerRow;
extern const CFStringRef kIOSurfacePlaneOffset;
extern const CFStringRef kIOSurfacePlaneSize;
extern const CFStringRef kIOSurfacePlaneBase;
extern const CFStringRef kIOSurfacePlaneBitsPerElement;
extern const CFStringRef kIOSurfacePlaneBytesPerElement;
extern const CFStringRef kIOSurfacePlaneElementWidth;
extern const CFStringRef kIOSurfacePlaneElementHeight;
extern const CFStringRef kIOSurfaceCacheMode;
extern const CFStringRef kIOSurfaceIsGlobal;
extern const CFStringRef kIOSurfacePixelFormat;
extern const CFStringRef kIOSurfacePixelSizeCastingAllowed;
extern const CFStringRef kIOSurfacePlaneComponentBitDepths;
extern const CFStringRef kIOSurfacePlaneComponentBitOffsets;

typedef int32_t IOSurfaceComponentName; enum {
    kIOSurfaceComponentNameUnknown = 0,
    kIOSurfaceComponentNameAlpha = 1,
    kIOSurfaceComponentNameRed = 2,
    kIOSurfaceComponentNameGreen = 3,
    kIOSurfaceComponentNameBlue = 4,
    kIOSurfaceComponentNameLuma = 5,
    kIOSurfaceComponentNameChromaRed = 6,
    kIOSurfaceComponentNameChromaBlue = 7,
};



extern const CFStringRef kIOSurfacePlaneComponentNames;

typedef int32_t IOSurfaceComponentType; enum {
    kIOSurfaceComponentTypeUnknown = 0,
    kIOSurfaceComponentTypeUnsignedInteger = 1,
    kIOSurfaceComponentTypeSignedInteger = 2,
    kIOSurfaceComponentTypeFloat = 3,
};


extern const CFStringRef kIOSurfacePlaneComponentTypes;

typedef int32_t IOSurfaceComponentRange; enum {
    kIOSurfaceComponentRangeUnknown = 0,
    kIOSurfaceComponentRangeFullRange = 1,
    kIOSurfaceComponentRangeVideoRange = 2,
    kIOSurfaceComponentRangeWideRange = 3,
};


extern const CFStringRef kIOSurfacePlaneComponentRanges;

typedef int32_t IOSurfaceSubsampling; enum {
    kIOSurfaceSubsamplingUnknown = 0,
    kIOSurfaceSubsamplingNone = 1,
    kIOSurfaceSubsampling422 = 2,
    kIOSurfaceSubsampling420 = 3,
    kIOSurfaceSubsampling411 = 4,
};


extern const CFStringRef kIOSurfaceSubsampling;

CFTypeID IOSurfaceGetTypeID(void);

IOSurfaceRef _Nullable IOSurfaceCreate(CFDictionaryRef properties);

IOSurfaceRef _Nullable IOSurfaceLookup(IOSurfaceID csid);

IOSurfaceID IOSurfaceGetID(IOSurfaceRef buffer);
kern_return_t IOSurfaceLock(IOSurfaceRef buffer, IOSurfaceLockOptions options, uint32_t * _Nullable seed);
kern_return_t IOSurfaceUnlock(IOSurfaceRef buffer, IOSurfaceLockOptions options, uint32_t * _Nullable seed);


size_t IOSurfaceGetAllocSize(IOSurfaceRef buffer);

size_t IOSurfaceGetWidth(IOSurfaceRef buffer);

size_t IOSurfaceGetHeight(IOSurfaceRef buffer);

size_t IOSurfaceGetBytesPerElement(IOSurfaceRef buffer);

size_t IOSurfaceGetBytesPerRow(IOSurfaceRef buffer);

void *IOSurfaceGetBaseAddress(IOSurfaceRef buffer);

size_t IOSurfaceGetElementWidth(IOSurfaceRef buffer);

size_t IOSurfaceGetElementHeight(IOSurfaceRef buffer);

OSType IOSurfaceGetPixelFormat(IOSurfaceRef buffer);


uint32_t IOSurfaceGetSeed(IOSurfaceRef buffer);


size_t IOSurfaceGetPlaneCount(IOSurfaceRef buffer);
size_t IOSurfaceGetWidthOfPlane(IOSurfaceRef buffer, size_t planeIndex);

size_t IOSurfaceGetHeightOfPlane(IOSurfaceRef buffer, size_t planeIndex);

size_t IOSurfaceGetBytesPerElementOfPlane(IOSurfaceRef buffer, size_t planeIndex);

size_t IOSurfaceGetBytesPerRowOfPlane(IOSurfaceRef buffer, size_t planeIndex);

void *IOSurfaceGetBaseAddressOfPlane(IOSurfaceRef buffer, size_t planeIndex);

size_t IOSurfaceGetElementWidthOfPlane(IOSurfaceRef buffer, size_t planeIndex);

size_t IOSurfaceGetElementHeightOfPlane(IOSurfaceRef buffer, size_t planeIndex);


size_t IOSurfaceGetNumberOfComponentsOfPlane(IOSurfaceRef buffer, size_t planeIndex);

IOSurfaceComponentName IOSurfaceGetNameOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex, size_t componentIndex);

IOSurfaceComponentType IOSurfaceGetTypeOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex, size_t componentIndex);

IOSurfaceComponentRange IOSurfaceGetRangeOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex, size_t componentIndex);

size_t IOSurfaceGetBitDepthOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex, size_t componentIndex);

size_t IOSurfaceGetBitOffsetOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex, size_t componentIndex);

IOSurfaceSubsampling IOSurfaceGetSubsampling(IOSurfaceRef buffer);


void IOSurfaceSetValue(IOSurfaceRef buffer, CFStringRef key, CFTypeRef value);

CFTypeRef _Nullable IOSurfaceCopyValue(IOSurfaceRef buffer, CFStringRef key);

void IOSurfaceRemoveValue(IOSurfaceRef buffer, CFStringRef key);

void IOSurfaceSetValues(IOSurfaceRef buffer, CFDictionaryRef keysAndValues);

CFDictionaryRef _Nullable IOSurfaceCopyAllValues(IOSurfaceRef buffer);

void IOSurfaceRemoveAllValues(IOSurfaceRef buffer);


mach_port_t IOSurfaceCreateMachPort(IOSurfaceRef buffer);


IOSurfaceRef _Nullable IOSurfaceLookupFromMachPort(mach_port_t port);
size_t IOSurfaceGetPropertyMaximum(CFStringRef property);
size_t IOSurfaceGetPropertyAlignment(CFStringRef property);


size_t IOSurfaceAlignProperty(CFStringRef property, size_t value);
void IOSurfaceIncrementUseCount(IOSurfaceRef buffer);

void IOSurfaceDecrementUseCount(IOSurfaceRef buffer);

int32_t IOSurfaceGetUseCount(IOSurfaceRef buffer);

Boolean IOSurfaceIsInUse(IOSurfaceRef buffer);


Boolean IOSurfaceAllowsPixelSizeCasting(IOSurfaceRef buffer);
kern_return_t IOSurfaceSetPurgeable(IOSurfaceRef buffer, uint32_t newState, uint32_t * _Nullable oldState);

__END_DECLS

#endif
