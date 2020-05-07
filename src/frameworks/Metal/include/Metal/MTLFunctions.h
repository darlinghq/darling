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

void* MTLAddDevice(void);
void* MTLAddDeviceClass(void);
void* MTLCPUCacheModeString(void);
void* MTLCopyAllDevices(void);
void* MTLCreateSystemDefaultDevice(void);
void* MTLFailureTypeGetEnabled(void);
void* MTLFailureTypeGetErrorModeType(void);
void* MTLFailureTypeSetErrorModeType(void);
void* MTLFeatureSetSupportsSamplingFromPixelFormat(void);
void* MTLGetReportFailureBlock(void);
void* MTLGetTextureLevelInfoForDevice(void);
void* MTLGetTextureLevelInfoForDeviceWithOptions(void);
void* MTLGetWarningMode(void);
void* MTLIOAccelCommandBufferStorageAllocResourceAtIndex(void);
void* MTLIOAccelCommandBufferStorageBeginSegment(void);
void* MTLIOAccelCommandBufferStorageCreate(void);
void* MTLIOAccelCommandBufferStorageCreateExt(void);
void* MTLIOAccelCommandBufferStorageDealloc(void);
void* MTLIOAccelCommandBufferStorageEndSegment(void);
void* MTLIOAccelCommandBufferStorageGrowKernelCommandBuffer(void);
void* MTLIOAccelCommandBufferStorageGrowSegmentList(void);
void* MTLIOAccelCommandBufferStoragePoolCreate(void);
void* MTLIOAccelCommandBufferStoragePoolCreateStorage(void);
void* MTLIOAccelCommandBufferStoragePoolDealloc(void);
void* MTLIOAccelCommandBufferStoragePoolPurge(void);
void* MTLIOAccelCommandBufferStoragePoolReturnStorage(void);
void* MTLIOAccelCommandBufferStorageReleaseAllResources(void);
void* MTLIOAccelCommandBufferStorageReleaseDeviceShmems(void);
void* MTLIOAccelCommandBufferStorageReset(void);
void* MTLIOAccelDeviceShmemRelease(void);
void* MTLIOAccelPooledResourceRelease(void);
void* MTLIOAccelResourcePoolCreatePooledResource(void);
void* MTLPixelFormatCompatibilityString(void);
void* MTLPixelFormatGetInfo(void);
void* MTLPixelFormatGetInfoForDevice(void);
void* MTLPixelFormatGetName(void);
void* MTLPurgeableStateString(void);
void* MTLRangeAllocatorAllocate(void);
void* MTLRangeAllocatorAllocateRange(void);
void* MTLRangeAllocatorDeallocate(void);
void* MTLRangeAllocatorDestroy(void);
void* MTLRangeAllocatorGetFragmentCapacity(void);
void* MTLRangeAllocatorGetFragmentCount(void);
void* MTLRangeAllocatorGetFreeCount(void);
void* MTLRangeAllocatorGetMaxFreeSize(void);
void* MTLRangeAllocatorInit(void);
void* MTLRangeAllocatorSetFragmentCapacityIncrement(void);
void* MTLReleaseAssertionFailure(void);
void* MTLRenderPipelineColorAttachmentDescriptorDescription(void);
void* MTLReportFailure(void);
void* MTLReportFailureTypeEnabled(void);
void* MTLResourceListAddResource(void);
void* MTLResourceListPoolCreateResourceList(void);
void* MTLResourceListRelease(void);
void* MTLResourceOptionsString(void);
void* MTLSetReportFailureBlock(void);
void* MTLSetWarningMode(void);
void* MTLStorageModeString(void);
void* MTLTextureTypeString(void);
void* MTLTextureUsageString(void);
void* MTLUseInterpose(void);
void* MTLValidationEnabled(void);
void* _MTLAdjustMTLSize(void);
void* _MTLGetAttachmentSize(void);
void* _MTLTextureValidateGetBytes(void);
void* _MTLTextureValidateReplaceRegion(void);
void* _Z17MTLDataTypeString11MTLDataType(void);
void* _Z20_MTLConstantDataSize11MTLDataType(void);
void* _Z24MTLDataTypeGetSignedType11MTLDataType(void);
void* _Z27MTLDataTypeGetComponentType11MTLDataType(void);
void* _Z28MTLDataTypeGetComponentCount11MTLDataType(void);
void* _Z28MTLDataTypeGetShaderTypeName11MTLDataTypej(void);
void* _Z28MTLDataTypeGetVectorDataType11MTLDataTypej(void);
void* _Z43_MTLCompatibleTextureDataTypeAndPixelFormat11MTLDataType14MTLPixelFormat(void);
void* _ZN13MTLStatistics10cvtGPU_CPUEy(void);
void* _ZN13MTLStatistics12newTimeTupleEyy(void);
void* _ZN13MTLStatistics16copySampleToUserEPyS0_Pc(void);
void* _ZN13MTLStatistics23getAllAvailableCountersEjP11StatInfoRec(void);
void* _ZN13MTLStatistics28validateAllRequestedCountersEPU29objcproto18MTLCommandQueueSPI22MTLIOAccelCommandQueuejP7NSArrayjP11StatInfoRec(void);
void* _ZN28MTLCompilerConnectionManager31createCompilerConnectionManagerEv(void);
void* getShaderCachePath(void);
void* setShaderCachePath(void);
void* validateReadPixels(void);
void* validateWritePixels(void);
