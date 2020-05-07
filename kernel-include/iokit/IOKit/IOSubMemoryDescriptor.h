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

#ifndef _IOSUBMEMORYDESCRIPTOR_H
#define _IOSUBMEMORYDESCRIPTOR_H

#include <IOKit/IOMemoryDescriptor.h>

/*! @class IOSubMemoryDescriptor : public IOMemoryDescriptor
 *   @abstract The IOSubMemoryDescriptor object describes a memory area made up of a portion of another IOMemoryDescriptor.
 *   @discussion The IOSubMemoryDescriptor object represents a subrange of memory, specified as a portion of another IOMemoryDescriptor. */

class IOSubMemoryDescriptor : public IOMemoryDescriptor
{
	OSDeclareDefaultStructors(IOSubMemoryDescriptor);

protected:
	IOMemoryDescriptor * _parent;
	IOByteCount          _start;

	virtual void free() APPLE_KEXT_OVERRIDE;

public:
/*! @function withSubRange
 *   @abstract Create an IOMemoryDescriptor to describe a subrange of an existing descriptor.
 *   @discussion  This method creates and initializes an IOMemoryDescriptor for memory consisting of a subrange of the specified memory descriptor. The parent memory descriptor is retained by the new descriptor.
 *   @param of The parent IOMemoryDescriptor of which a subrange is to be used for the new descriptor, which will be retained by the subrange IOMemoryDescriptor.
 *   @param offset A byte offset into the parent memory descriptor's memory.
 *   @param length The length of the subrange.
 *   @param options
 *       kIOMemoryDirectionMask (options:direction)	This nibble indicates the I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures.
 *   @result The created IOMemoryDescriptor on success, to be released by the caller, or zero on failure. */

	static IOSubMemoryDescriptor *      withSubRange(IOMemoryDescriptor *of,
	    IOByteCount offset,
	    IOByteCount length,
	    IOOptionBits options);

/*
 * Initialize or reinitialize an IOSubMemoryDescriptor to describe
 * a subrange of an existing descriptor.
 *
 * An IOSubMemoryDescriptor can be re-used by calling initSubRange
 * again on an existing instance -- note that this behavior is not
 * commonly supported in other IOKit classes, although it is here.
 */
	virtual bool initSubRange( IOMemoryDescriptor * parent,
	    IOByteCount offset, IOByteCount length,
	    IODirection withDirection );

/*
 * IOMemoryDescriptor required methods
 */

	virtual addr64_t getPhysicalSegment( IOByteCount   offset,
	    IOByteCount * length,
	    IOOptionBits  options = 0 ) APPLE_KEXT_OVERRIDE;

	virtual IOReturn prepare(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;

	virtual IOReturn complete(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;

#ifdef __LP64__
	virtual IOReturn redirect( task_t safeTask, bool redirect ) APPLE_KEXT_OVERRIDE;
#else
	IOReturn redirect( task_t safeTask, bool redirect );
#endif /* __LP64__ */

	virtual IOReturn setPurgeable( IOOptionBits newState,
	    IOOptionBits * oldState ) APPLE_KEXT_OVERRIDE;

	IOReturn setOwnership( task_t newOwner,
	    int newLedgerTag,
	    IOOptionBits newLedgerOptions );

// support map() on kIOMemoryTypeVirtual without prepare()
	virtual IOMemoryMap *       makeMapping(
		IOMemoryDescriptor *    owner,
		task_t                  intoTask,
		IOVirtualAddress        atAddress,
		IOOptionBits            options,
		IOByteCount             offset,
		IOByteCount             length ) APPLE_KEXT_OVERRIDE;

	virtual uint64_t getPreparationID( void ) APPLE_KEXT_OVERRIDE;

/*! @function getPageCounts
 *   @abstract Retrieve the number of resident and/or dirty pages encompassed by an IOMemoryDescriptor.
 *   @discussion This method returns the number of resident and/or dirty pages encompassed by an IOMemoryDescriptor.
 *   @param residentPageCount - If non-null, a pointer to a byte count that will return the number of resident pages encompassed by this IOMemoryDescriptor.
 *   @param dirtyPageCount - If non-null, a pointer to a byte count that will return the number of dirty pages encompassed by this IOMemoryDescriptor.
 *   @result An IOReturn code. */

	IOReturn getPageCounts(IOByteCount * residentPageCount,
	    IOByteCount * dirtyPageCount);
};

#endif /* !_IOSUBMEMORYDESCRIPTOR_H */
