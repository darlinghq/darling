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

#ifndef _IOINTERLEAVEDMEMORYDESCRIPTOR_H
#define _IOINTERLEAVEDMEMORYDESCRIPTOR_H

#include <IOKit/IOMemoryDescriptor.h>

/*! @class IOInterleavedMemoryDescriptor : public IOMemoryDescriptor
    @abstract The IOInterleavedMemoryDescriptor object describes a memory area made up of portions of several other IOMemoryDescriptors.
    @discussion The IOInterleavedMemoryDescriptor object represents interleaved ranges of memory, specified as an ordered list of portions of individual IOMemoryDescriptors.  The portions are chained end-to-end to make up a single contiguous buffer. */

class IOInterleavedMemoryDescriptor : public IOMemoryDescriptor
{
    OSDeclareDefaultStructors(IOInterleavedMemoryDescriptor);

protected:

    IOByteCount           _descriptorCapacity;
    UInt32                _descriptorCount;
    IOMemoryDescriptor ** _descriptors;
    IOByteCount         * _descriptorOffsets;
    IOByteCount         * _descriptorLengths;
    bool                  _descriptorPrepared;

    virtual void free() APPLE_KEXT_OVERRIDE;

public:

/*! @function withCapacity
    @abstract Create an IOInterleavedMemoryDescriptor to describe a memory area made up of several other IOMemoryDescriptors.
    @discussion This method creates and initializes an IOInterleavedMemoryDescriptor for memory consisting of portions of a number of other IOMemoryDescriptors, chained end-to-end (in the order they appear in the array) to represent a single contiguous memory buffer.
    @param capacity The maximum number of IOMemoryDescriptors that may be subsequently added to this IOInterleavedMemoryDescriptor.
    @param direction An I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures.
    @result The created IOInterleavedMemoryDescriptor on success, to be released by the caller, or zero on failure. */

    static IOInterleavedMemoryDescriptor * withCapacity( IOByteCount	       capacity,
                                                         IODirection           direction);

/*! @function initWithCapacity
    @abstract Initialize an IOInterleavedMemoryDescriptor to describe a memory area made up of several other IOMemoryDescriptors.
    @discussion This method initializes an IOInterleavedMemoryDescriptor for memory consisting of portions of a number of other IOMemoryDescriptors, chained end-to-end (in the order they appear in the array) to represent a single contiguous memory buffer.
    @param capacity The maximum number of IOMemoryDescriptors that may be subsequently added to this IOInterleavedMemoryDescriptor.
    @param direction An I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures.
    @result The created IOInterleavedMemoryDescriptor on success, to be released by the caller, or zero on failure. */

    virtual bool initWithCapacity( IOByteCount           capacity,
                                   IODirection           direction );

/*! @function clearMemoryDescriptors
    @abstract Clear all of the IOMemoryDescriptors currently contained in and reset the IOInterleavedMemoryDescriptor.
    @discussion Clears each IOMemoryDescriptor by completing (if needed) and releasing.  The IOInterleavedMemoryDescriptor is then reset and may accept new descriptors up to the capacity specified when it was created.
    @param direction An I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures. */

    virtual void clearMemoryDescriptors( IODirection direction = kIODirectionNone );

/*! @function setMemoryDescriptor
    @abstract Add a portion of an IOMemoryDescriptor to the IOInterleavedMemoryDescriptor.
    @discussion This method adds the portion of an IOMemoryDescriptor described by the offset and length parameters to the end of the IOInterleavedMemoryDescriptor.  A single IOMemoryDescriptor may be added as many times as there is room for it.  The offset and length must describe a portion entirely within the IOMemoryDescriptor.
    @param descriptor An IOMemoryDescriptor to be added to the IOInterleavedMemoryDescriptor.  Its direction must be compatible with that of the IOInterleavedMemoryDescriptor.
    @param offset The offset into the IOMemoryDescriptor of the portion that will be added to the virtualized buffer.
    @param length The length of the portion of the IOMemoryDescriptor to be added to the virtualized buffer. 
    @result Returns true the portion was successfully added. */

    virtual bool setMemoryDescriptor( IOMemoryDescriptor * descriptor,
                                      IOByteCount offset,
                                      IOByteCount length );

/*! @function getPhysicalSegment
    @abstract Break a memory descriptor into its physically contiguous segments.
    @discussion This method returns the physical address of the byte at the given offset into the memory, and optionally the length of the physically contiguous segment from that offset.
    @param offset A byte offset into the memory whose physical address to return.
    @param length If non-zero, getPhysicalSegment will store here the length of the physically contiguous segement at the given offset.
    @result A physical address, or zero if the offset is beyond the length of the memory. */

    virtual addr64_t getPhysicalSegment( IOByteCount   offset,
                                         IOByteCount * length,
                                         IOOptionBits  options = 0 ) APPLE_KEXT_OVERRIDE;

/*! @function prepare
    @abstract Prepare the memory for an I/O transfer.
    @discussion This involves paging in the memory, if necessary, and wiring it down for the duration of the transfer.  The complete() method completes the processing of the memory after the I/O transfer finishes.  This method need not called for non-pageable memory.
    @param forDirection The direction of the I/O to be performed, or kIODirectionNone for the direction specified by the memory descriptor.
    @result An IOReturn code. */

    virtual IOReturn prepare(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;

/*! @function complete
    @abstract Complete processing of the memory after an I/O transfer finishes.
    @discussion This method should not be called unless a prepare was previously issued; the prepare() and complete() must occur in pairs, before and after an I/O transfer involving pageable memory.
    @param forDirection The direction of the I/O just completed, or kIODirectionNone for the direction specified by the memory descriptor.
    @result An IOReturn code. */

    virtual IOReturn complete(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;
};

#endif /* !_IOINTERLEAVEDMEMORYDESCRIPTOR_H */
