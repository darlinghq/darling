/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _CMBLOCKBUFFER_H_
#define _CMBLOCKBUFFER_H_

#include <CoreMedia/CMBase.h>

CF_IMPLICIT_BRIDGING_ENABLED

enum {
	kCMBlockBufferNoErr = 0,
	kCMBlockBufferStructureAllocationFailedErr = -12700,
	kCMBlockBufferBlockAllocationFailedErr = -12701,
	kCMBlockBufferBadCustomBlockSourceErr = -12702,
	kCMBlockBufferBadOffsetParameterErr = -12703,
	kCMBlockBufferBadLengthParameterErr = -12704,
	kCMBlockBufferBadPointerParameterErr = -12705,
	kCMBlockBufferEmptyBBufErr = -12706,
	kCMBlockBufferUnallocatedBlockErr = -12707,
	kCMBlockBufferInsufficientSpaceErr = -12708,
};

typedef uint32_t CMBlockBufferFlags;
enum {
	kCMBlockBufferAssureMemoryNowFlag = (1L << 0),
	kCMBlockBufferAlwaysCopyDataFlag = (1L << 1),
	kCMBlockBufferDontOptimizeDepthFlag = (1L << 2),
	kCMBlockBufferPermitEmptyReferenceFlag = (1L << 3),
};

typedef struct CF_BRIDGED_TYPE(id) OpaqueCMBlockBuffer *CMBlockBufferRef;

#pragma pack(push, 4)
typedef struct {
	uint32_t version;
	void *(*AllocateBlock)(void *refCon, size_t sizeInBytes);
	void (*FreeBlock)(void *refCon, void *doomedMemoryBlock, size_t sizeInBytes);
	void *refCon;
} CMBlockBufferCustomBlockSource;
#pragma pack(pop)

enum { kCMBlockBufferCustomBlockSourceVersion = 0 };

CM_EXPORT CFTypeID CMBlockBufferGetTypeID(void);
CM_EXPORT OSStatus CMBlockBufferCreateWithMemoryBlock(CFAllocatorRef structureAllocator, void *memoryBlock,
	size_t blockLength, CFAllocatorRef blockAllocator, const CMBlockBufferCustomBlockSource *customBlockSource,
	size_t offsetToData, size_t dataLength, CMBlockBufferFlags flags,
	CF_RETURNS_RETAINED CMBlockBufferRef *blockBufferOut);
CM_EXPORT size_t CMBlockBufferGetDataLength(CMBlockBufferRef theBuffer);
CM_EXPORT OSStatus CMBlockBufferGetDataPointer(CMBlockBufferRef theBuffer, size_t offset,
	size_t *lengthAtOffsetOut, size_t *totalLengthOut, char **dataPointerOut);

CF_IMPLICIT_BRIDGING_DISABLED

#endif
