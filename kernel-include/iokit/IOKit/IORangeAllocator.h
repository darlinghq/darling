/*
 * Copyright (c) 1998-2019 Apple Inc. All rights reserved.
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
/*
 * Copyright (c) 1999 Apple Computer, Inc.
 *
 *
 * HISTORY
 *
 * sdouglas 05 Nov 99 - created.
 */

#ifndef _IOKIT_IORANGEALLOCATOR_H
#define _IOKIT_IORANGEALLOCATOR_H

#include <libkern/c++/OSObject.h>
#include <IOKit/IOTypes.h>

typedef IOByteCount IORangeScalar;

/*! @class IORangeAllocator
 *   @abstract A utility class to manage allocations from a range.
 *   @discussion The IORangeAllocator class provides functions for allocating ranges, at a fixed or any offset, and freeing them back to a free list. It is useful for describing ranges of memory or address space without requiring storage in the memory - information describing the free elements is kept elsewhere. Ranges are described by a start offset and a size. IORangeAllocator is optionally protected against multithreaded access.
 */

class IORangeAllocator : public OSObject {
	OSDeclareDefaultStructors(IORangeAllocator);

protected:
	UInt32              numElements;
	UInt32              capacity;
	UInt32              capacityIncrement;
	IORangeScalar       defaultAlignmentMask;
	IOOptionBits        options;

	struct IORangeAllocatorElement *    elements;

private:
	virtual bool allocElement( UInt32 index );

	virtual void deallocElement( UInt32 index );

public:
	enum {
		kLocking        = 0x00000001
	};

/*! @function init
 *   @abstract Standard initializer for IORangeAllocator.
 *   @discussion This method initializes an IORangeAllocator and optionally sets the free list to contain one fragment, from zero to an endOfRange parameter. The capacity in terms of free fragments and locking options are set for the instance.
 *   @param endOfRange If the free list is to contain an initial fragment, set endOfRange to the last offset in the range, ie. size - 1, to create a free fragment for the range zero to endOfRange inclusive. If zero is passed, the free list will be initialized empty, and can be populated with calls to the deallocate method.
 *   @param defaultAlignment If this parameter is non-zero it specifies a required alignment for all allocations, for example pass 256 to align allocations on 256 byte boundaries. Zero or one specify unaligned allocations.
 *   @param capacity Sets the initial size of the free list in number of noncontiguous fragments. This value is also used for the capacityIncrement.
 *   @param options Pass kLocking if the instance can be used by multiple threads.
 *   @result Returns true if the instance is successfully initialized, false on failure. */

	virtual bool init( IORangeScalar endOfRange,
	    IORangeScalar defaultAlignment,
	    UInt32 capacity,
	    IOOptionBits options );

/*! @function withRange
 *   @abstract Standard factory method for IORangeAllocator.
 *   @discussion This method allocates and initializes an IORangeAllocator and optionally sets the free list to contain one fragment, from zero to an endOfRange parameter. The capacity in terms of free fragments and locking options are set for the instance.
 *   @param endOfRange If the free list is to contain an initial fragment, set endOfRange to the last offset in the range, ie. size - 1, to create a free fragment for the range zero to endOfRange inclusive. If zero is passed the free list will be initialized empty, and can be populated with calls to the deallocate method.
 *   @param defaultAlignment If this parameter is non-zero it specifies a required alignment for all allocations, for example pass 256 to align allocations on 256 byte boundaries. Zero or one specify unaligned allocations.
 *   @param capacity Sets the initial size of the free list in number of non-contiguous fragments. This value is also used for the capacityIncrement.
 *   @param options Pass kLocking if the instance can be used by multiple threads.
 *   @result Returns the new IORangeAllocator instance, to be released by the caller, or zero on failure. */

	static IORangeAllocator * withRange( IORangeScalar endOfRange,
	    IORangeScalar defaultAlignment = 0,
	    UInt32 capacity = 0,
	    IOOptionBits options = 0 );

	virtual void free() APPLE_KEXT_OVERRIDE;
	virtual bool serialize(OSSerialize *s) const APPLE_KEXT_OVERRIDE;

/*! @function getFragmentCount
 *   @abstract Accessor to return the number of free fragments in the range.
 *   @discussion This method returns a count of free fragments. Each fragment describes a non-contiguous free range - deallocations will merge contiguous fragments together.
 *   @result Returns the count of free fragments.
 */

	virtual UInt32 getFragmentCount( void );

/*! @function getFragmentCapacity
 *   @abstract Accessor to return the number of free fragments in the range.
 *   @discussion This method returns the current capacity of the free fragment list.
 *   @result Returns the current capacity of free fragment list.
 */

	virtual UInt32 getFragmentCapacity( void );

/*! @function setFragmentCapacityIncrement
 *   @abstract Sets the count of fragments the free list will increase by when full.
 *   @discussion This method sets the number of extra fragments the free list will expand to when full. It defaults to the initial capacity.
 *   @param count The number of fragments to increment the capacity by when the free list is full.
 */

	virtual void setFragmentCapacityIncrement( UInt32 count );

/*! @function getFreeCount
 *   @abstract Totals the sizes of the free fragments.
 *   @discussion This method returns the total of the sizes of the fragments on the free list.
 *   @result Returns the total of the free fragments sizes.
 */

	virtual IORangeScalar getFreeCount( void );

/*! @function allocate
 *   @abstract Allocates from the free list, at any offset.
 *   @discussion This method allocates a range from the free list. The alignment will default to the alignment set when the allocator was created or may be set here.
 *   @param size The size of the range requested.
 *   @param result The beginning of the range allocated is returned here on success.
 *   @param alignment If zero is passed, default to the allocators alignment, otherwise pass an alignment required for the allocation, for example 4096 to page align.
 *   @result Returns true if the allocation was successful, else false.
 */

	virtual bool allocate( IORangeScalar size,
	    IORangeScalar * result,
	    IORangeScalar alignment = 0 );

/*! @function allocateRange
 *   @abstract Allocates from the free list, at a set offset.
 *   @discussion This method allocates a range from the free list, given a set offset passed in.
 *   @param start The beginning of the range requested.
 *   @param size The size of the range requested.
 *   @result Returns true if the allocation was successful, else false.
 */

	virtual bool allocateRange( IORangeScalar start,
	    IORangeScalar size );

/*! @function deallocate
 *   @abstract Deallocates a range to the free list.
 *   @discussion This method deallocates a range to the free list, given a the start offset and length passed in.
 *   @param start The beginning of the range requested.
 *   @param size Returns the size of the range requested.
 */

	virtual void deallocate( IORangeScalar start,
	    IORangeScalar size );
};

#endif /* _IOKIT_IORANGEALLOCATOR_H */
