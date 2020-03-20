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


#ifndef _IOSurface_H_
#define _IOSurface_H_

#import <Foundation/Foundation.h>

@interface IOSurface : NSObject

@end

void* IOSurfaceAlignProperty(void);
void* IOSurfaceAllowsPixelSizeCasting(void);
void* IOSurfaceBindAccel(void);
void* IOSurfaceBindAccelOnPlane(void);
void* IOSurfaceClientAlignProperty(void);
void* IOSurfaceClientAllowsPixelSizeCasting(void);
void* IOSurfaceClientBindAccel(void);
void* IOSurfaceClientCopyAllValues(void);
void* IOSurfaceClientCopyDataValueBytes(void);
void* IOSurfaceClientCopyValue(void);
void* IOSurfaceClientCreate(void);
void* IOSurfaceClientCreateMachPort(void);
void* IOSurfaceClientDecrementUseCount(void);
void* IOSurfaceClientFlushProcessorCaches(void);
void* IOSurfaceClientGetAllocSize(void);
void* IOSurfaceClientGetBaseAddress(void);
void* IOSurfaceClientGetBaseAddressOfPlane(void);
void* IOSurfaceClientGetBitDepthOfComponentOfPlane(void);
void* IOSurfaceClientGetBitOffsetOfComponentOfPlane(void);
void* IOSurfaceClientGetBulkAttachments(void);
void* IOSurfaceClientGetBytesPerElement(void);
void* IOSurfaceClientGetBytesPerElementOfPlane(void);
void* IOSurfaceClientGetBytesPerRow(void);
void* IOSurfaceClientGetBytesPerRowOfPlane(void);
void* IOSurfaceClientGetCacheMode(void);
void* IOSurfaceClientGetElementHeight(void);
void* IOSurfaceClientGetElementHeightOfPlane(void);
void* IOSurfaceClientGetElementWidth(void);
void* IOSurfaceClientGetElementWidthOfPlane(void);
void* IOSurfaceClientGetHeight(void);
void* IOSurfaceClientGetHeightOfPlane(void);
void* IOSurfaceClientGetID(void);
void* IOSurfaceClientGetNameOfComponentOfPlane(void);
void* IOSurfaceClientGetNumberOfComponentsOfPlane(void);
void* IOSurfaceClientGetPixelFormat(void);
void* IOSurfaceClientGetPlaneCount(void);
void* IOSurfaceClientGetPropertyAlignment(void);
void* IOSurfaceClientGetPropertyMaximum(void);
void* IOSurfaceClientGetProtectionOptions(void);
void* IOSurfaceClientGetRangeOfComponentOfPlane(void);
void* IOSurfaceClientGetSeed(void);
void* IOSurfaceClientGetSubsampling(void);
void* IOSurfaceClientGetTypeOfComponentOfPlane(void);
void* IOSurfaceClientGetUseCount(void);
void* IOSurfaceClientGetWidth(void);
void* IOSurfaceClientGetWidthOfPlane(void);
void* IOSurfaceClientGetYCbCrMatrix(void);
void* IOSurfaceClientIncrementUseCount(void);
void* IOSurfaceClientIsDisplayable(void);
void* IOSurfaceClientIsInUse(void);
void* IOSurfaceClientLock(void);
void* IOSurfaceClientLookup(void);
void* IOSurfaceClientLookupFromMachPort(void);
void* IOSurfaceClientNeedsBindAccel(void);
void* IOSurfaceClientRelease(void);
void* IOSurfaceClientRemoveSurfaceNotify(void);
void* IOSurfaceClientRemoveValue(void);
void* IOSurfaceClientRetain(void);
void* IOSurfaceClientSetBulkAttachments(void);
void* IOSurfaceClientSetPurgeable(void);
void* IOSurfaceClientSetSurfaceNotify(void);
void* IOSurfaceClientSetValue(void);
void* IOSurfaceClientSetYCbCrMatrix(void);
void* IOSurfaceClientSupportsProtectionOptions(void);
void* IOSurfaceClientUnlock(void);
void* IOSurfaceCopyAllValues(void);
void* IOSurfaceCopyDataValueBytes(void);
void* IOSurfaceCopyValue(void);
void* IOSurfaceCreate(void);
void* IOSurfaceCreateMachPort(void);
void* IOSurfaceCreateXPCObject(void);
void* IOSurfaceDecrementUseCount(void);
void* IOSurfaceFlushProcessorCaches(void);
void* IOSurfaceGetAllocSize(void);
void* IOSurfaceGetBaseAddress(void);
void* IOSurfaceGetBaseAddressOfPlane(void);
void* IOSurfaceGetBitDepthOfComponentOfPlane(void);
void* IOSurfaceGetBitOffsetOfComponentOfPlane(void);
void* IOSurfaceGetBitsPerBlock(void);
void* IOSurfaceGetBitsPerBlockOfPlane(void);
void* IOSurfaceGetBlockHeightOfPlane(void);
void* IOSurfaceGetBlockWidthOfPlane(void);
void* IOSurfaceGetBulkAttachments(void);
void* IOSurfaceGetBytesPerElement(void);
void* IOSurfaceGetBytesPerElementOfPlane(void);
void* IOSurfaceGetBytesPerRow(void);
void* IOSurfaceGetBytesPerRowOfPlane(void);
void* IOSurfaceGetCacheMode(void);
void* IOSurfaceGetElementHeight(void);
void* IOSurfaceGetElementHeightOfPlane(void);
void* IOSurfaceGetElementWidth(void);
void* IOSurfaceGetElementWidthOfPlane(void);
void* IOSurfaceGetGraphicsCommPageAddress(void);
void* IOSurfaceGetHeight(void);
void* IOSurfaceGetHeightOfPlane(void);
void* IOSurfaceGetID(void);
void* IOSurfaceGetNameOfComponentOfPlane(void);
void* IOSurfaceGetNumberOfComponentsOfPlane(void);
void* IOSurfaceGetPixelFormat(void);
void* IOSurfaceGetPlaneCount(void);
void* IOSurfaceGetPropertyAlignment(void);
void* IOSurfaceGetPropertyMaximum(void);
void* IOSurfaceGetProtectionOptions(void);
void* IOSurfaceGetRangeOfComponentOfPlane(void);
void* IOSurfaceGetSeed(void);
void* IOSurfaceGetSeedOfPlane(void);
void* IOSurfaceGetSubsampling(void);
void* IOSurfaceGetTypeID(void);
void* IOSurfaceGetTypeOfComponentOfPlane(void);
void* IOSurfaceGetUseCount(void);
void* IOSurfaceGetWidth(void);
void* IOSurfaceGetWidthOfPlane(void);
void* IOSurfaceGetYCbCrMatrix(void);
void* IOSurfaceIncrementUseCount(void);
void* IOSurfaceIsDisplayable(void);
void* IOSurfaceIsInUse(void);
void* IOSurfaceLock(void);
void* IOSurfaceLockPlane(void);
void* IOSurfaceLog(void);
void* IOSurfaceLookup(void);
void* IOSurfaceLookupFromMachPort(void);
void* IOSurfaceLookupFromXPCObject(void);
void* IOSurfaceNeedsBindAccel(void);
void* IOSurfaceNeedsBindAccelOnPlane(void);
void* IOSurfaceNotifierCreate(void);
void* IOSurfaceNotifierGetMachPort(void);
void* IOSurfaceNotifierGetRunLoopSource(void);
void* IOSurfaceNotifierGetTypeID(void);
void* IOSurfaceNotifierSetDispatchQueue(void);
void* IOSurfaceRemoveAllValues(void);
void* IOSurfaceRemoveValue(void);
void* IOSurfaceSetBulkAttachments(void);
void* IOSurfaceSetBulkAttachments2(void);
void* IOSurfaceSetPurgeable(void);
void* IOSurfaceSetValue(void);
void* IOSurfaceSetValues(void);
void* IOSurfaceSetYCbCrMatrix(void);
void* IOSurfaceSupportsProtectionOptions(void);
void* IOSurfaceUnlock(void);
void* IOSurfaceUnlockPlane(void);
void* _ioSurfaceAddClientRef(void);

#endif
