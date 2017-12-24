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

#include <stdlib.h>
#include <stdio.h>
#import <IOSurface/IOSurface.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

@implementation IOSurface

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

sting(void) {
    if (verbose) puts("STUB: IOSurfaceAllowsPixelSizeCasting called");
    return NULL;
}

void* IOSurfaceBindAccel(void) {
    if (verbose) puts("STUB: IOSurfaceBindAccel called");
    return NULL;
}

void* IOSurfaceBindAccelOnPlane(void) {
    if (verbose) puts("STUB: IOSurfaceBindAccelOnPlane called");
    return NULL;
}

void* IOSurfaceClientAlignProperty(void) {
    if (verbose) puts("STUB: IOSurfaceClientAlignProperty called");
    return NULL;
}

void* IOSurfaceClientAllowsPixelSizeCasting(void) {
    if (verbose) puts("STUB: IOSurfaceClientAllowsPixelSizeCasting called");
    return NULL;
}

void* IOSurfaceClientBindAccel(void) {
    if (verbose) puts("STUB: IOSurfaceClientBindAccel called");
    return NULL;
}

void* IOSurfaceClientCopyAllValues(void) {
    if (verbose) puts("STUB: IOSurfaceClientCopyAllValues called");
    return NULL;
}

void* IOSurfaceClientCopyDataValueBytes(void) {
    if (verbose) puts("STUB: IOSurfaceClientCopyDataValueBytes called");
    return NULL;
}

void* IOSurfaceClientCopyValue(void) {
    if (verbose) puts("STUB: IOSurfaceClientCopyValue called");
    return NULL;
}

void* IOSurfaceClientCreate(void) {
    if (verbose) puts("STUB: IOSurfaceClientCreate called");
    return NULL;
}

void* IOSurfaceClientCreateMachPort(void) {
    if (verbose) puts("STUB: IOSurfaceClientCreateMachPort called");
    return NULL;
}

void* IOSurfaceClientDecrementUseCount(void) {
    if (verbose) puts("STUB: IOSurfaceClientDecrementUseCount called");
    return NULL;
}

void* IOSurfaceClientFlushProcessorCaches(void) {
    if (verbose) puts("STUB: IOSurfaceClientFlushProcessorCaches called");
    return NULL;
}

void* IOSurfaceClientGetAllocSize(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetAllocSize called");
    return NULL;
}

void* IOSurfaceClientGetBaseAddress(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetBaseAddress called");
    return NULL;
}

void* IOSurfaceClientGetBaseAddressOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetBaseAddressOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetBitDepthOfComponentOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetBitDepthOfComponentOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetBitOffsetOfComponentOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetBitOffsetOfComponentOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetBulkAttachments(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetBulkAttachments called");
    return NULL;
}

void* IOSurfaceClientGetBytesPerElement(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetBytesPerElement called");
    return NULL;
}

void* IOSurfaceClientGetBytesPerElementOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetBytesPerElementOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetBytesPerRow(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetBytesPerRow called");
    return NULL;
}

void* IOSurfaceClientGetBytesPerRowOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetBytesPerRowOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetCacheMode(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetCacheMode called");
    return NULL;
}

void* IOSurfaceClientGetElementHeight(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetElementHeight called");
    return NULL;
}

void* IOSurfaceClientGetElementHeightOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetElementHeightOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetElementWidth(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetElementWidth called");
    return NULL;
}

void* IOSurfaceClientGetElementWidthOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetElementWidthOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetHeight(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetHeight called");
    return NULL;
}

void* IOSurfaceClientGetHeightOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetHeightOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetID(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetID called");
    return NULL;
}

void* IOSurfaceClientGetNameOfComponentOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetNameOfComponentOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetNumberOfComponentsOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetNumberOfComponentsOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetPixelFormat(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetPixelFormat called");
    return NULL;
}

void* IOSurfaceClientGetPlaneCount(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetPlaneCount called");
    return NULL;
}

void* IOSurfaceClientGetPropertyAlignment(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetPropertyAlignment called");
    return NULL;
}

void* IOSurfaceClientGetPropertyMaximum(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetPropertyMaximum called");
    return NULL;
}

void* IOSurfaceClientGetProtectionOptions(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetProtectionOptions called");
    return NULL;
}

void* IOSurfaceClientGetRangeOfComponentOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetRangeOfComponentOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetSeed(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetSeed called");
    return NULL;
}

void* IOSurfaceClientGetSubsampling(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetSubsampling called");
    return NULL;
}

void* IOSurfaceClientGetTypeOfComponentOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetTypeOfComponentOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetUseCount(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetUseCount called");
    return NULL;
}

void* IOSurfaceClientGetWidth(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetWidth called");
    return NULL;
}

void* IOSurfaceClientGetWidthOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetWidthOfPlane called");
    return NULL;
}

void* IOSurfaceClientGetYCbCrMatrix(void) {
    if (verbose) puts("STUB: IOSurfaceClientGetYCbCrMatrix called");
    return NULL;
}

void* IOSurfaceClientIncrementUseCount(void) {
    if (verbose) puts("STUB: IOSurfaceClientIncrementUseCount called");
    return NULL;
}

void* IOSurfaceClientIsDisplayable(void) {
    if (verbose) puts("STUB: IOSurfaceClientIsDisplayable called");
    return NULL;
}

void* IOSurfaceClientIsInUse(void) {
    if (verbose) puts("STUB: IOSurfaceClientIsInUse called");
    return NULL;
}

void* IOSurfaceClientLock(void) {
    if (verbose) puts("STUB: IOSurfaceClientLock called");
    return NULL;
}

void* IOSurfaceClientLookup(void) {
    if (verbose) puts("STUB: IOSurfaceClientLookup called");
    return NULL;
}

void* IOSurfaceClientLookupFromMachPort(void) {
    if (verbose) puts("STUB: IOSurfaceClientLookupFromMachPort called");
    return NULL;
}

void* IOSurfaceClientNeedsBindAccel(void) {
    if (verbose) puts("STUB: IOSurfaceClientNeedsBindAccel called");
    return NULL;
}

void* IOSurfaceClientRelease(void) {
    if (verbose) puts("STUB: IOSurfaceClientRelease called");
    return NULL;
}

void* IOSurfaceClientRemoveSurfaceNotify(void) {
    if (verbose) puts("STUB: IOSurfaceClientRemoveSurfaceNotify called");
    return NULL;
}

void* IOSurfaceClientRemoveValue(void) {
    if (verbose) puts("STUB: IOSurfaceClientRemoveValue called");
    return NULL;
}

void* IOSurfaceClientRetain(void) {
    if (verbose) puts("STUB: IOSurfaceClientRetain called");
    return NULL;
}

void* IOSurfaceClientSetBulkAttachments(void) {
    if (verbose) puts("STUB: IOSurfaceClientSetBulkAttachments called");
    return NULL;
}

void* IOSurfaceClientSetPurgeable(void) {
    if (verbose) puts("STUB: IOSurfaceClientSetPurgeable called");
    return NULL;
}

void* IOSurfaceClientSetSurfaceNotify(void) {
    if (verbose) puts("STUB: IOSurfaceClientSetSurfaceNotify called");
    return NULL;
}

void* IOSurfaceClientSetValue(void) {
    if (verbose) puts("STUB: IOSurfaceClientSetValue called");
    return NULL;
}

void* IOSurfaceClientSetYCbCrMatrix(void) {
    if (verbose) puts("STUB: IOSurfaceClientSetYCbCrMatrix called");
    return NULL;
}

void* IOSurfaceClientSupportsProtectionOptions(void) {
    if (verbose) puts("STUB: IOSurfaceClientSupportsProtectionOptions called");
    return NULL;
}

void* IOSurfaceClientUnlock(void) {
    if (verbose) puts("STUB: IOSurfaceClientUnlock called");
    return NULL;
}

void* IOSurfaceCopyAllValues(void) {
    if (verbose) puts("STUB: IOSurfaceCopyAllValues called");
    return NULL;
}

void* IOSurfaceCopyDataValueBytes(void) {
    if (verbose) puts("STUB: IOSurfaceCopyDataValueBytes called");
    return NULL;
}

void* IOSurfaceCopyValue(void) {
    if (verbose) puts("STUB: IOSurfaceCopyValue called");
    return NULL;
}

void* IOSurfaceCreate(void) {
    if (verbose) puts("STUB: IOSurfaceCreate called");
    return NULL;
}

void* IOSurfaceCreateMachPort(void) {
    if (verbose) puts("STUB: IOSurfaceCreateMachPort called");
    return NULL;
}

void* IOSurfaceCreateXPCObject(void) {
    if (verbose) puts("STUB: IOSurfaceCreateXPCObject called");
    return NULL;
}

void* IOSurfaceDecrementUseCount(void) {
    if (verbose) puts("STUB: IOSurfaceDecrementUseCount called");
    return NULL;
}

void* IOSurfaceFlushProcessorCaches(void) {
    if (verbose) puts("STUB: IOSurfaceFlushProcessorCaches called");
    return NULL;
}

void* IOSurfaceGetAllocSize(void) {
    if (verbose) puts("STUB: IOSurfaceGetAllocSize called");
    return NULL;
}

void* IOSurfaceGetBaseAddress(void) {
    if (verbose) puts("STUB: IOSurfaceGetBaseAddress called");
    return NULL;
}

void* IOSurfaceGetBaseAddressOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetBaseAddressOfPlane called");
    return NULL;
}

void* IOSurfaceGetBitDepthOfComponentOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetBitDepthOfComponentOfPlane called");
    return NULL;
}

void* IOSurfaceGetBitOffsetOfComponentOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetBitOffsetOfComponentOfPlane called");
    return NULL;
}

void* IOSurfaceGetBitsPerBlock(void) {
    if (verbose) puts("STUB: IOSurfaceGetBitsPerBlock called");
    return NULL;
}

void* IOSurfaceGetBitsPerBlockOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetBitsPerBlockOfPlane called");
    return NULL;
}

void* IOSurfaceGetBlockHeightOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetBlockHeightOfPlane called");
    return NULL;
}

void* IOSurfaceGetBlockWidthOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetBlockWidthOfPlane called");
    return NULL;
}

void* IOSurfaceGetBulkAttachments(void) {
    if (verbose) puts("STUB: IOSurfaceGetBulkAttachments called");
    return NULL;
}

void* IOSurfaceGetBytesPerElement(void) {
    if (verbose) puts("STUB: IOSurfaceGetBytesPerElement called");
    return NULL;
}

void* IOSurfaceGetBytesPerElementOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetBytesPerElementOfPlane called");
    return NULL;
}

void* IOSurfaceGetBytesPerRow(void) {
    if (verbose) puts("STUB: IOSurfaceGetBytesPerRow called");
    return NULL;
}

void* IOSurfaceGetBytesPerRowOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetBytesPerRowOfPlane called");
    return NULL;
}

void* IOSurfaceGetCacheMode(void) {
    if (verbose) puts("STUB: IOSurfaceGetCacheMode called");
    return NULL;
}

void* IOSurfaceGetElementHeight(void) {
    if (verbose) puts("STUB: IOSurfaceGetElementHeight called");
    return NULL;
}

void* IOSurfaceGetElementHeightOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetElementHeightOfPlane called");
    return NULL;
}

void* IOSurfaceGetElementWidth(void) {
    if (verbose) puts("STUB: IOSurfaceGetElementWidth called");
    return NULL;
}

void* IOSurfaceGetElementWidthOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetElementWidthOfPlane called");
    return NULL;
}

void* IOSurfaceGetGraphicsCommPageAddress(void) {
    if (verbose) puts("STUB: IOSurfaceGetGraphicsCommPageAddress called");
    return NULL;
}

void* IOSurfaceGetHeight(void) {
    if (verbose) puts("STUB: IOSurfaceGetHeight called");
    return NULL;
}

void* IOSurfaceGetHeightOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetHeightOfPlane called");
    return NULL;
}

void* IOSurfaceGetID(void) {
    if (verbose) puts("STUB: IOSurfaceGetID called");
    return NULL;
}

void* IOSurfaceGetNameOfComponentOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetNameOfComponentOfPlane called");
    return NULL;
}

void* IOSurfaceGetNumberOfComponentsOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetNumberOfComponentsOfPlane called");
    return NULL;
}

void* IOSurfaceGetPixelFormat(void) {
    if (verbose) puts("STUB: IOSurfaceGetPixelFormat called");
    return NULL;
}

void* IOSurfaceGetPlaneCount(void) {
    if (verbose) puts("STUB: IOSurfaceGetPlaneCount called");
    return NULL;
}

void* IOSurfaceGetPropertyAlignment(void) {
    if (verbose) puts("STUB: IOSurfaceGetPropertyAlignment called");
    return NULL;
}

void* IOSurfaceGetPropertyMaximum(void) {
    if (verbose) puts("STUB: IOSurfaceGetPropertyMaximum called");
    return NULL;
}

void* IOSurfaceGetProtectionOptions(void) {
    if (verbose) puts("STUB: IOSurfaceGetProtectionOptions called");
    return NULL;
}

void* IOSurfaceGetRangeOfComponentOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetRangeOfComponentOfPlane called");
    return NULL;
}

void* IOSurfaceGetSeed(void) {
    if (verbose) puts("STUB: IOSurfaceGetSeed called");
    return NULL;
}

void* IOSurfaceGetSeedOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetSeedOfPlane called");
    return NULL;
}

void* IOSurfaceGetSubsampling(void) {
    if (verbose) puts("STUB: IOSurfaceGetSubsampling called");
    return NULL;
}

void* IOSurfaceGetTypeID(void) {
    if (verbose) puts("STUB: IOSurfaceGetTypeID called");
    return NULL;
}

void* IOSurfaceGetTypeOfComponentOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetTypeOfComponentOfPlane called");
    return NULL;
}

void* IOSurfaceGetUseCount(void) {
    if (verbose) puts("STUB: IOSurfaceGetUseCount called");
    return NULL;
}

void* IOSurfaceGetWidth(void) {
    if (verbose) puts("STUB: IOSurfaceGetWidth called");
    return NULL;
}

void* IOSurfaceGetWidthOfPlane(void) {
    if (verbose) puts("STUB: IOSurfaceGetWidthOfPlane called");
    return NULL;
}

void* IOSurfaceGetYCbCrMatrix(void) {
    if (verbose) puts("STUB: IOSurfaceGetYCbCrMatrix called");
    return NULL;
}

void* IOSurfaceIncrementUseCount(void) {
    if (verbose) puts("STUB: IOSurfaceIncrementUseCount called");
    return NULL;
}

void* IOSurfaceIsDisplayable(void) {
    if (verbose) puts("STUB: IOSurfaceIsDisplayable called");
    return NULL;
}

void* IOSurfaceIsInUse(void) {
    if (verbose) puts("STUB: IOSurfaceIsInUse called");
    return NULL;
}

void* IOSurfaceLock(void) {
    if (verbose) puts("STUB: IOSurfaceLock called");
    return NULL;
}

void* IOSurfaceLockPlane(void) {
    if (verbose) puts("STUB: IOSurfaceLockPlane called");
    return NULL;
}

void* IOSurfaceLog(void) {
    if (verbose) puts("STUB: IOSurfaceLog called");
    return NULL;
}

void* IOSurfaceLookup(void) {
    if (verbose) puts("STUB: IOSurfaceLookup called");
    return NULL;
}

void* IOSurfaceLookupFromMachPort(void) {
    if (verbose) puts("STUB: IOSurfaceLookupFromMachPort called");
    return NULL;
}

void* IOSurfaceLookupFromXPCObject(void) {
    if (verbose) puts("STUB: IOSurfaceLookupFromXPCObject called");
    return NULL;
}

void* IOSurfaceNeedsBindAccel(void) {
    if (verbose) puts("STUB: IOSurfaceNeedsBindAccel called");
    return NULL;
}

void* IOSurfaceNeedsBindAccelOnPlane(void) {
    if (verbose) puts("STUB: IOSurfaceNeedsBindAccelOnPlane called");
    return NULL;
}

void* IOSurfaceNotifierCreate(void) {
    if (verbose) puts("STUB: IOSurfaceNotifierCreate called");
    return NULL;
}

void* IOSurfaceNotifierGetMachPort(void) {
    if (verbose) puts("STUB: IOSurfaceNotifierGetMachPort called");
    return NULL;
}

void* IOSurfaceNotifierGetRunLoopSource(void) {
    if (verbose) puts("STUB: IOSurfaceNotifierGetRunLoopSource called");
    return NULL;
}

void* IOSurfaceNotifierGetTypeID(void) {
    if (verbose) puts("STUB: IOSurfaceNotifierGetTypeID called");
    return NULL;
}

void* IOSurfaceNotifierSetDispatchQueue(void) {
    if (verbose) puts("STUB: IOSurfaceNotifierSetDispatchQueue called");
    return NULL;
}

void* IOSurfaceRemoveAllValues(void) {
    if (verbose) puts("STUB: IOSurfaceRemoveAllValues called");
    return NULL;
}

void* IOSurfaceRemoveValue(void) {
    if (verbose) puts("STUB: IOSurfaceRemoveValue called");
    return NULL;
}

void* IOSurfaceSetBulkAttachments(void) {
    if (verbose) puts("STUB: IOSurfaceSetBulkAttachments called");
    return NULL;
}

void* IOSurfaceSetBulkAttachments2(void) {
    if (verbose) puts("STUB: IOSurfaceSetBulkAttachments2 called");
    return NULL;
}

void* IOSurfaceSetPurgeable(void) {
    if (verbose) puts("STUB: IOSurfaceSetPurgeable called");
    return NULL;
}

void* IOSurfaceSetValue(void) {
    if (verbose) puts("STUB: IOSurfaceSetValue called");
    return NULL;
}

void* IOSurfaceSetValues(void) {
    if (verbose) puts("STUB: IOSurfaceSetValues called");
    return NULL;
}

void* IOSurfaceSetYCbCrMatrix(void) {
    if (verbose) puts("STUB: IOSurfaceSetYCbCrMatrix called");
    return NULL;
}

void* IOSurfaceSupportsProtectionOptions(void) {
    if (verbose) puts("STUB: IOSurfaceSupportsProtectionOptions called");
    return NULL;
}

void* IOSurfaceUnlock(void) {
    if (verbose) puts("STUB: IOSurfaceUnlock called");
    return NULL;
}

void* IOSurfaceUnlockPlane(void) {
    if (verbose) puts("STUB: IOSurfaceUnlockPlane called");
    return NULL;
}

void* _ioSurfaceAddClientRef(void) {
    if (verbose) puts("STUB: _ioSurfaceAddClientRef called");
    return NULL;
}
