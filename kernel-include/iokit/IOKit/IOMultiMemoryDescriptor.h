/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _IOMULTIMEMORYDESCRIPTOR_H
#define _IOMULTIMEMORYDESCRIPTOR_H

#include <IOKit/IOMemoryDescriptor.h>

/*! @class IOMultiMemoryDescriptor : public IOMemoryDescriptor
 *   @abstract The IOMultiMemoryDescriptor object describes a memory area made up of several other IOMemoryDescriptors.
 *   @discussion The IOMultiMemoryDescriptor object represents multiple ranges of memory, specified as an ordered list of IOMemoryDescriptors.  The descriptors are chained end-to-end to make up a single contiguous buffer. */

class IOMultiMemoryDescriptor : public IOMemoryDescriptor
{
	OSDeclareDefaultStructors(IOMultiMemoryDescriptor);

protected:

	IOMemoryDescriptor ** _descriptors;
	UInt32                _descriptorsCount;
	bool                  _descriptorsIsAllocated;

	virtual void free() APPLE_KEXT_OVERRIDE;

public:

/*! @function withDescriptors
 *   @abstract Create an IOMultiMemoryDescriptor to describe a memory area made up of several other IOMemoryDescriptors.
 *   @discussion This method creates and initializes an IOMultiMemoryDescriptor for memory consisting of a number of other IOMemoryDescriptors, chained end-to-end (in the order they appear in the array) to represent a single contiguous memory buffer.  Passing the descriptor array as a reference will avoid an extra allocation.
 *   @param descriptors An array of IOMemoryDescriptors which make up the memory to be described.
 *   @param withCount The object count for the descriptors array.
 *   @param withDirection An I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures.
 *   @param asReference If false, the IOMultiMemoryDescriptor object will make a copy of the descriptors array, otherwise, the array will be used in situ, avoiding an extra allocation.
 *   @result The created IOMultiMemoryDescriptor on success, to be released by the caller, or zero on failure. */

	static IOMultiMemoryDescriptor * withDescriptors(
		IOMemoryDescriptor ** descriptors,
		UInt32                withCount,
		IODirection           withDirection,
		bool                  asReference = false );

/*! @function withDescriptors
 *   @abstract Initialize an IOMultiMemoryDescriptor to describe a memory area made up of several other IOMemoryDescriptors.
 *   @discussion This method initializes an IOMultiMemoryDescriptor for memory consisting of a number of other IOMemoryDescriptors, chained end-to-end (in the order they appear in the array) to represent a single contiguous memory buffer.  Passing the descriptor array as a reference will avoid an extra allocation.
 *   @param descriptors An array of IOMemoryDescriptors which make up the memory to be described.
 *   @param withCount The object count for the descriptors array.
 *   @param withDirection An I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures.
 *   @param asReference If false, the IOMultiMemoryDescriptor object will make a copy of the descriptors array, otherwise, the array will be used in situ, avoiding an extra allocation.
 *   @result The created IOMultiMemoryDescriptor on success, to be released by the caller, or zero on failure. */

	virtual bool initWithDescriptors(
		IOMemoryDescriptor ** descriptors,
		UInt32                withCount,
		IODirection           withDirection,
		bool                  asReference = false );

/*! @function getPhysicalSegment
 *   @abstract Break a memory descriptor into its physically contiguous segments.
 *   @discussion This method returns the physical address of the byte at the given offset into the memory, and optionally the length of the physically contiguous segment from that offset.
 *   @param offset A byte offset into the memory whose physical address to return.
 *   @param length If non-zero, getPhysicalSegment will store here the length of the physically contiguous segement at the given offset.
 *   @result A physical address, or zero if the offset is beyond the length of the memory. */

	virtual addr64_t getPhysicalSegment( IOByteCount   offset,
	    IOByteCount * length,
	    IOOptionBits  options = 0 ) APPLE_KEXT_OVERRIDE;

/*! @function prepare
 *   @abstract Prepare the memory for an I/O transfer.
 *   @discussion This involves paging in the memory, if necessary, and wiring it down for the duration of the transfer.  The complete() method completes the processing of the memory after the I/O transfer finishes.  This method needn't called for non-pageable memory.
 *   @param forDirection The direction of the I/O just completed, or kIODirectionNone for the direction specified by the memory descriptor.
 *   @result An IOReturn code. */

	virtual IOReturn prepare(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;

/*! @function complete
 *   @abstract Complete processing of the memory after an I/O transfer finishes.
 *   @discussion This method should not be called unless a prepare was previously issued; the prepare() and complete() must occur in pairs, before and after an I/O transfer involving pageable memory.
 *   @param forDirection The direction of the I/O just completed, or kIODirectionNone for the direction specified by the memory descriptor.
 *   @result An IOReturn code. */

	virtual IOReturn complete(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;

	virtual IOReturn setPurgeable(IOOptionBits newState, IOOptionBits * oldState) APPLE_KEXT_OVERRIDE;

	IOReturn setOwnership(task_t newOwner, int newLedgerTag, IOOptionBits newOptions);

/*! @function getPageCounts
 *   @abstract Retrieve the number of resident and/or dirty pages encompassed by an IOMemoryDescriptor.
 *   @discussion This method returns the number of resident and/or dirty pages encompassed by an IOMemoryDescriptor.
 *   @param residentPageCount - If non-null, a pointer to a byte count that will return the number of resident pages encompassed by this IOMemoryDescriptor.
 *   @param dirtyPageCount - If non-null, a pointer to a byte count that will return the number of dirty pages encompassed by this IOMemoryDescriptor.
 *   @result An IOReturn code. */

	IOReturn getPageCounts(IOByteCount * residentPageCount,
	    IOByteCount * dirtyPageCount);

	virtual uint64_t getPreparationID( void ) APPLE_KEXT_OVERRIDE;

#define IOMULTIMEMORYDESCRIPTOR_SUPPORTS_GETPAGECOUNTS  1

private:
	virtual IOReturn doMap(vm_map_t           addressMap,
	    IOVirtualAddress * atAddress,
	    IOOptionBits       options,
	    IOByteCount        sourceOffset = 0,
	    IOByteCount        length = 0 ) APPLE_KEXT_OVERRIDE;
};

#endif /* !_IOMULTIMEMORYDESCRIPTOR_H */
