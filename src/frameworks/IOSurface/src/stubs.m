/*
  This file is part of Darling.

  Copyright (C) 2017 Lubos Dolezel

  Darling is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Darling is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// Component metadata is carried by ordinary IOSurface attachments. The public keys describe arrays of
// CFNumbers; a planar surface may nest one array per plane. When a surface has no metadata attachment, the
// conservative default is a single component with the plane's whole bit depth at offset zero. That keeps the
// accessors useful for simple pixel buffers without guessing a channel layout from a pixel format.

#include <stddef.h>
#include <stdint.h>
#import <IOSurface/IOSurface.h>
#include <CoreFoundation/CoreFoundation.h>

static BOOL ValidPlane(IOSurfaceRef buffer, size_t planeIndex)
{
    if (buffer == NULL)
        return NO;
    size_t count = IOSurfaceGetPlaneCount(buffer);
    return (planeIndex == 0 && count == 0) || planeIndex < count;
}

// Returns a retained array: either the plane's nested array or the whole attachment. Callers release it.
static CFArrayRef ComponentArrayForPlane(IOSurfaceRef buffer, CFStringRef key, size_t planeIndex)
{
    if (buffer == NULL || key == NULL)
        return NULL;

    CFTypeRef value = IOSurfaceCopyValue(buffer, key);
    if (value == NULL || CFGetTypeID(value) != CFArrayGetTypeID()) {
        if (value != NULL)
            CFRelease(value);
        return NULL;
    }

    CFArrayRef array = (CFArrayRef) value;
    CFIndex count = CFArrayGetCount(array);
    if (count > (CFIndex) planeIndex) {
        CFTypeRef element = CFArrayGetValueAtIndex(array, (CFIndex) planeIndex);
        if (element != NULL && CFGetTypeID(element) == CFArrayGetTypeID()) {
            CFRetain(element);
            CFRelease(value);
            return (CFArrayRef) element;
        }
    }
    return array;
}

static size_t ArrayCount(IOSurfaceRef buffer, CFStringRef key, size_t planeIndex)
{
    CFArrayRef array = ComponentArrayForPlane(buffer, key, planeIndex);
    if (array == NULL)
        return 0;
    size_t count = (size_t) CFArrayGetCount(array);
    CFRelease(array);
    return count;
}

static int64_t ArrayValue(IOSurfaceRef buffer, CFStringRef key, size_t planeIndex,
                          size_t componentIndex, int64_t fallback)
{
    CFArrayRef array = ComponentArrayForPlane(buffer, key, planeIndex);
    if (array == NULL) {
        return fallback;
    }

    int64_t value = fallback;
    if (componentIndex < (size_t) CFArrayGetCount(array)) {
        CFTypeRef element = CFArrayGetValueAtIndex(array, (CFIndex) componentIndex);
        if (element != NULL && CFGetTypeID(element) == CFNumberGetTypeID()
            && !CFNumberGetValue((CFNumberRef) element, kCFNumberSInt64Type, &value)) {
            value = fallback;
        }
    }
    CFRelease(array);
    return value;
}

size_t IOSurfaceGetNumberOfComponentsOfPlane(IOSurfaceRef buffer, size_t planeIndex)
{
    if (!ValidPlane(buffer, planeIndex))
        return 0;

    size_t count = ArrayCount(buffer, kIOSurfacePlaneComponentBitDepths, planeIndex);
    if (count == 0)
        count = ArrayCount(buffer, kIOSurfacePlaneComponentNames, planeIndex);
    if (count == 0)
        count = ArrayCount(buffer, kIOSurfacePlaneComponentTypes, planeIndex);
    if (count == 0)
        count = ArrayCount(buffer, kIOSurfacePlaneComponentRanges, planeIndex);
    if (count == 0)
        count = ArrayCount(buffer, kIOSurfacePlaneComponentBitOffsets, planeIndex);
    return count != 0 ? count : 1;
}

IOSurfaceComponentName IOSurfaceGetNameOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex,
                                                          size_t componentIndex)
{
    if (!ValidPlane(buffer, planeIndex) || componentIndex >= IOSurfaceGetNumberOfComponentsOfPlane(buffer, planeIndex))
        return kIOSurfaceComponentNameUnknown;
    return (IOSurfaceComponentName) ArrayValue(buffer, kIOSurfacePlaneComponentNames,
                                               planeIndex, componentIndex,
                                               kIOSurfaceComponentNameUnknown);
}

IOSurfaceComponentType IOSurfaceGetTypeOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex,
                                                         size_t componentIndex)
{
    if (!ValidPlane(buffer, planeIndex) || componentIndex >= IOSurfaceGetNumberOfComponentsOfPlane(buffer, planeIndex))
        return kIOSurfaceComponentTypeUnknown;
    return (IOSurfaceComponentType) ArrayValue(buffer, kIOSurfacePlaneComponentTypes,
                                               planeIndex, componentIndex,
                                               kIOSurfaceComponentTypeUnknown);
}

IOSurfaceComponentRange IOSurfaceGetRangeOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex,
                                                           size_t componentIndex)
{
    if (!ValidPlane(buffer, planeIndex) || componentIndex >= IOSurfaceGetNumberOfComponentsOfPlane(buffer, planeIndex))
        return kIOSurfaceComponentRangeUnknown;
    return (IOSurfaceComponentRange) ArrayValue(buffer, kIOSurfacePlaneComponentRanges,
                                                planeIndex, componentIndex,
                                                kIOSurfaceComponentRangeUnknown);
}

size_t IOSurfaceGetBitDepthOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex,
                                              size_t componentIndex)
{
    if (!ValidPlane(buffer, planeIndex) || componentIndex >= IOSurfaceGetNumberOfComponentsOfPlane(buffer, planeIndex))
        return 0;

    size_t bits = IOSurfaceGetBytesPerElementOfPlane(buffer, planeIndex) * 8;
    int64_t value = ArrayValue(buffer, kIOSurfacePlaneComponentBitDepths, planeIndex,
                               componentIndex, (int64_t) bits);
    return value > 0 ? (size_t) value : 0;
}

size_t IOSurfaceGetBitOffsetOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex,
                                               size_t componentIndex)
{
    if (!ValidPlane(buffer, planeIndex) || componentIndex >= IOSurfaceGetNumberOfComponentsOfPlane(buffer, planeIndex))
        return 0;
    int64_t value = ArrayValue(buffer, kIOSurfacePlaneComponentBitOffsets, planeIndex,
                               componentIndex, 0);
    return value >= 0 ? (size_t) value : 0;
}

IOSurfaceSubsampling IOSurfaceGetSubsampling(IOSurfaceRef buffer)
{
    if (buffer == NULL)
        return kIOSurfaceSubsamplingUnknown;

    CFTypeRef value = IOSurfaceCopyValue(buffer, kIOSurfaceSubsampling);
    if (value == NULL)
        return kIOSurfaceSubsamplingNone;
    if (CFGetTypeID(value) != CFNumberGetTypeID()) {
        CFRelease(value);
        return kIOSurfaceSubsamplingNone;
    }

    int64_t subsampling = kIOSurfaceSubsamplingNone;
    if (!CFNumberGetValue((CFNumberRef) value, kCFNumberSInt64Type, &subsampling)
        || subsampling < kIOSurfaceSubsamplingNone || subsampling > kIOSurfaceSubsampling411) {
        subsampling = kIOSurfaceSubsamplingNone;
    }
    CFRelease(value);
    return (IOSurfaceSubsampling) subsampling;
}

// The documented contract answers 0 for properties without a predefined limit. The nonzero answers are
// Darling's own conservative limits for a software-rendered surface: 16K pixels in either dimension (below the
// Core Animation limits public WebKit sources use) and enough row bytes for a 16-byte element at that width.
// Allocation sizes and offsets have no fixed limit here, so they answer 0.
static const size_t kMaximumSurfaceDimension = 16384;
static const size_t kMaximumBytesPerRow = kMaximumSurfaceDimension * 16;

size_t IOSurfaceGetPropertyMaximum(CFStringRef property)
{
    if (property == NULL)
        return 0;
    if (CFEqual(property, kIOSurfaceWidth) || CFEqual(property, kIOSurfaceHeight)
        || CFEqual(property, kIOSurfacePlaneWidth) || CFEqual(property, kIOSurfacePlaneHeight)) {
        return kMaximumSurfaceDimension;
    }
    if (CFEqual(property, kIOSurfaceBytesPerRow) || CFEqual(property, kIOSurfacePlaneBytesPerRow))
        return kMaximumBytesPerRow;
    return 0;
}
