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
static int verbose = 0;
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}
void* MTLAddDevice(void) { if (verbose) puts("STUB: MTLAddDevice called"); return NULL; }
void* MTLAddDeviceClass(void) { if (verbose) puts("STUB: MTLAddDeviceClass called"); return NULL; }
void* MTLCPUCacheModeString(void) { if (verbose) puts("STUB: MTLCPUCacheModeString called"); return NULL; }
void* MTLCopyAllDevices(void) { if (verbose) puts("STUB: MTLCopyAllDevices called"); return NULL; }
void* MTLCreateSystemDefaultDevice(void) { if (verbose) puts("STUB: MTLCreateSystemDefaultDevice called"); return NULL; }
void* MTLFailureTypeGetEnabled(void) { if (verbose) puts("STUB: MTLFailureTypeGetEnabled called"); return NULL; }
void* MTLFailureTypeGetErrorModeType(void) { if (verbose) puts("STUB: MTLFailureTypeGetErrorModeType called"); return NULL; }
void* MTLFailureTypeSetErrorModeType(void) { if (verbose) puts("STUB: MTLFailureTypeSetErrorModeType called"); return NULL; }
void* MTLFeatureSetSupportsSamplingFromPixelFormat(void) { if (verbose) puts("STUB: MTLFeatureSetSupportsSamplingFromPixelFormat called"); return NULL; }
void* MTLGetReportFailureBlock(void) { if (verbose) puts("STUB: MTLGetReportFailureBlock called"); return NULL; }
void* MTLGetTextureLevelInfoForDevice(void) { if (verbose) puts("STUB: MTLGetTextureLevelInfoForDevice called"); return NULL; }
void* MTLGetTextureLevelInfoForDeviceWithOptions(void) { if (verbose) puts("STUB: MTLGetTextureLevelInfoForDeviceWithOptions called"); return NULL; }
void* MTLGetWarningMode(void) { if (verbose) puts("STUB: MTLGetWarningMode called"); return NULL; }
void* MTLIOAccelCommandBufferStorageAllocResourceAtIndex(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageAllocResourceAtIndex called"); return NULL; }
void* MTLIOAccelCommandBufferStorageBeginSegment(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageBeginSegment called"); return NULL; }
void* MTLIOAccelCommandBufferStorageCreate(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageCreate called"); return NULL; }
void* MTLIOAccelCommandBufferStorageCreateExt(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageCreateExt called"); return NULL; }
void* MTLIOAccelCommandBufferStorageDealloc(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageDealloc called"); return NULL; }
void* MTLIOAccelCommandBufferStorageEndSegment(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageEndSegment called"); return NULL; }
void* MTLIOAccelCommandBufferStorageGrowKernelCommandBuffer(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageGrowKernelCommandBuffer called"); return NULL; }
void* MTLIOAccelCommandBufferStorageGrowSegmentList(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageGrowSegmentList called"); return NULL; }
void* MTLIOAccelCommandBufferStoragePoolCreate(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStoragePoolCreate called"); return NULL; }
void* MTLIOAccelCommandBufferStoragePoolCreateStorage(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStoragePoolCreateStorage called"); return NULL; }
void* MTLIOAccelCommandBufferStoragePoolDealloc(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStoragePoolDealloc called"); return NULL; }
void* MTLIOAccelCommandBufferStoragePoolPurge(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStoragePoolPurge called"); return NULL; }
void* MTLIOAccelCommandBufferStoragePoolReturnStorage(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStoragePoolReturnStorage called"); return NULL; }
void* MTLIOAccelCommandBufferStorageReleaseAllResources(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageReleaseAllResources called"); return NULL; }
void* MTLIOAccelCommandBufferStorageReleaseDeviceShmems(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageReleaseDeviceShmems called"); return NULL; }
void* MTLIOAccelCommandBufferStorageReset(void) { if (verbose) puts("STUB: MTLIOAccelCommandBufferStorageReset called"); return NULL; }
void* MTLIOAccelDeviceShmemRelease(void) { if (verbose) puts("STUB: MTLIOAccelDeviceShmemRelease called"); return NULL; }
void* MTLIOAccelPooledResourceRelease(void) { if (verbose) puts("STUB: MTLIOAccelPooledResourceRelease called"); return NULL; }
void* MTLIOAccelResourcePoolCreatePooledResource(void) { if (verbose) puts("STUB: MTLIOAccelResourcePoolCreatePooledResource called"); return NULL; }
void* MTLPixelFormatCompatibilityString(void) { if (verbose) puts("STUB: MTLPixelFormatCompatibilityString called"); return NULL; }
void* MTLPixelFormatGetInfo(void) { if (verbose) puts("STUB: MTLPixelFormatGetInfo called"); return NULL; }
void* MTLPixelFormatGetInfoForDevice(void) { if (verbose) puts("STUB: MTLPixelFormatGetInfoForDevice called"); return NULL; }
void* MTLPixelFormatGetName(void) { if (verbose) puts("STUB: MTLPixelFormatGetName called"); return NULL; }
void* MTLPurgeableStateString(void) { if (verbose) puts("STUB: MTLPurgeableStateString called"); return NULL; }
void* MTLRangeAllocatorAllocate(void) { if (verbose) puts("STUB: MTLRangeAllocatorAllocate called"); return NULL; }
void* MTLRangeAllocatorAllocateRange(void) { if (verbose) puts("STUB: MTLRangeAllocatorAllocateRange called"); return NULL; }
void* MTLRangeAllocatorDeallocate(void) { if (verbose) puts("STUB: MTLRangeAllocatorDeallocate called"); return NULL; }
void* MTLRangeAllocatorDestroy(void) { if (verbose) puts("STUB: MTLRangeAllocatorDestroy called"); return NULL; }
void* MTLRangeAllocatorGetFragmentCapacity(void) { if (verbose) puts("STUB: MTLRangeAllocatorGetFragmentCapacity called"); return NULL; }
void* MTLRangeAllocatorGetFragmentCount(void) { if (verbose) puts("STUB: MTLRangeAllocatorGetFragmentCount called"); return NULL; }
void* MTLRangeAllocatorGetFreeCount(void) { if (verbose) puts("STUB: MTLRangeAllocatorGetFreeCount called"); return NULL; }
void* MTLRangeAllocatorGetMaxFreeSize(void) { if (verbose) puts("STUB: MTLRangeAllocatorGetMaxFreeSize called"); return NULL; }
void* MTLRangeAllocatorInit(void) { if (verbose) puts("STUB: MTLRangeAllocatorInit called"); return NULL; }
void* MTLRangeAllocatorSetFragmentCapacityIncrement(void) { if (verbose) puts("STUB: MTLRangeAllocatorSetFragmentCapacityIncrement called"); return NULL; }
void* MTLReleaseAssertionFailure(void) { if (verbose) puts("STUB: MTLReleaseAssertionFailure called"); return NULL; }
void* MTLRenderPipelineColorAttachmentDescriptorDescription(void) { if (verbose) puts("STUB: MTLRenderPipelineColorAttachmentDescriptorDescription called"); return NULL; }
void* MTLReportFailure(void) { if (verbose) puts("STUB: MTLReportFailure called"); return NULL; }
void* MTLReportFailureTypeEnabled(void) { if (verbose) puts("STUB: MTLReportFailureTypeEnabled called"); return NULL; }
void* MTLResourceListAddResource(void) { if (verbose) puts("STUB: MTLResourceListAddResource called"); return NULL; }
void* MTLResourceListPoolCreateResourceList(void) { if (verbose) puts("STUB: MTLResourceListPoolCreateResourceList called"); return NULL; }
void* MTLResourceListRelease(void) { if (verbose) puts("STUB: MTLResourceListRelease called"); return NULL; }
void* MTLResourceOptionsString(void) { if (verbose) puts("STUB: MTLResourceOptionsString called"); return NULL; }
void* MTLSetReportFailureBlock(void) { if (verbose) puts("STUB: MTLSetReportFailureBlock called"); return NULL; }
void* MTLSetWarningMode(void) { if (verbose) puts("STUB: MTLSetWarningMode called"); return NULL; }
void* MTLStorageModeString(void) { if (verbose) puts("STUB: MTLStorageModeString called"); return NULL; }
void* MTLTextureTypeString(void) { if (verbose) puts("STUB: MTLTextureTypeString called"); return NULL; }
void* MTLTextureUsageString(void) { if (verbose) puts("STUB: MTLTextureUsageString called"); return NULL; }
void* MTLUseInterpose(void) { if (verbose) puts("STUB: MTLUseInterpose called"); return NULL; }
void* MTLValidationEnabled(void) { if (verbose) puts("STUB: MTLValidationEnabled called"); return NULL; }
void* _MTLAdjustMTLSize(void) { if (verbose) puts("STUB: _MTLAdjustMTLSize called"); return NULL; }
void* _MTLGetAttachmentSize(void) { if (verbose) puts("STUB: _MTLGetAttachmentSize called"); return NULL; }
void* _MTLTextureValidateGetBytes(void) { if (verbose) puts("STUB: _MTLTextureValidateGetBytes called"); return NULL; }
void* _MTLTextureValidateReplaceRegion(void) { if (verbose) puts("STUB: _MTLTextureValidateReplaceRegion called"); return NULL; }
void* _Z17MTLDataTypeString11MTLDataType(void) { if (verbose) puts("STUB: _Z17MTLDataTypeString11MTLDataType called"); return NULL; }
void* _Z20_MTLConstantDataSize11MTLDataType(void) { if (verbose) puts("STUB: _Z20_MTLConstantDataSize11MTLDataType called"); return NULL; }
void* _Z24MTLDataTypeGetSignedType11MTLDataType(void) { if (verbose) puts("STUB: _Z24MTLDataTypeGetSignedType11MTLDataType called"); return NULL; }
void* _Z27MTLDataTypeGetComponentType11MTLDataType(void) { if (verbose) puts("STUB: _Z27MTLDataTypeGetComponentType11MTLDataType called"); return NULL; }
void* _Z28MTLDataTypeGetComponentCount11MTLDataType(void) { if (verbose) puts("STUB: _Z28MTLDataTypeGetComponentCount11MTLDataType called"); return NULL; }
void* _Z28MTLDataTypeGetShaderTypeName11MTLDataTypej(void) { if (verbose) puts("STUB: _Z28MTLDataTypeGetShaderTypeName11MTLDataTypej called"); return NULL; }
void* _Z28MTLDataTypeGetVectorDataType11MTLDataTypej(void) { if (verbose) puts("STUB: _Z28MTLDataTypeGetVectorDataType11MTLDataTypej called"); return NULL; }
void* _Z43_MTLCompatibleTextureDataTypeAndPixelFormat11MTLDataType14MTLPixelFormat(void) { if (verbose) puts("STUB: _Z43_MTLCompatibleTextureDataTypeAndPixelFormat11MTLDataType14MTLPixelFormat called"); return NULL; }
void* _ZN13MTLStatistics10cvtGPU_CPUEy(void) { if (verbose) puts("STUB: _ZN13MTLStatistics10cvtGPU_CPUEy called"); return NULL; }
void* _ZN13MTLStatistics12newTimeTupleEyy(void) { if (verbose) puts("STUB: _ZN13MTLStatistics12newTimeTupleEyy called"); return NULL; }
void* _ZN13MTLStatistics16copySampleToUserEPyS0_Pc(void) { if (verbose) puts("STUB: _ZN13MTLStatistics16copySampleToUserEPyS0_Pc called"); return NULL; }
void* _ZN13MTLStatistics23getAllAvailableCountersEjP11StatInfoRec(void) { if (verbose) puts("STUB: _ZN13MTLStatistics23getAllAvailableCountersEjP11StatInfoRec called"); return NULL; }
void* _ZN13MTLStatistics28validateAllRequestedCountersEPU29objcproto18MTLCommandQueueSPI22MTLIOAccelCommandQueuejP7NSArrayjP11StatInfoRec(void) { if (verbose) puts("STUB: _ZN13MTLStatistics28validateAllRequestedCountersEPU29objcproto18MTLCommandQueueSPI22MTLIOAccelCommandQueuejP7NSArrayjP11StatInfoRec called"); return NULL; }
void* _ZN28MTLCompilerConnectionManager31createCompilerConnectionManagerEv(void) { if (verbose) puts("STUB: _ZN28MTLCompilerConnectionManager31createCompilerConnectionManagerEv called"); return NULL; }
void* getShaderCachePath(void) { if (verbose) puts("STUB: getShaderCachePath called"); return NULL; }
void* setShaderCachePath(void) { if (verbose) puts("STUB: setShaderCachePath called"); return NULL; }
void* validateReadPixels(void) { if (verbose) puts("STUB: validateReadPixels called"); return NULL; }
void* validateWritePixels(void) { if (verbose) puts("STUB: validateWritePixels called"); return NULL; }
