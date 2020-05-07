/*
 * Copyright (c) 1998-2016 Apple Inc. All rights reserved.
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
#ifndef _IOBUFFERMEMORYDESCRIPTOR_H
#define _IOBUFFERMEMORYDESCRIPTOR_H

#include <IOKit/IOMemoryDescriptor.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>

enum {
	kIOMemoryPhysicallyContiguous       = 0x00000010,
	kIOMemoryPageable                   = 0x00000020,
	kIOMemoryPurgeable                  = 0x00000040,
	kIOMemoryHostPhysicallyContiguous   = 0x00000080,
	kIOMemorySharingTypeMask            = 0x000f0000,
	kIOMemoryUnshared                   = 0x00000000,
	kIOMemoryKernelUserShared           = 0x00010000,
	// shared IOMemoryDescriptor options for IOBufferMemoryDescriptor:
	kIOBufferDescriptorMemoryFlags      = kIOMemoryDirectionMask
#ifdef XNU_KERNEL_PRIVATE
	    | kIOMemoryAutoPrepare
#endif
	    | kIOMemoryThreadSafe
	    | kIOMemoryClearEncrypt
	    | kIOMemoryMapperNone
	    | kIOMemoryUseReserve
};

#define _IOBUFFERMEMORYDESCRIPTOR_INTASKWITHOPTIONS_            1
#define _IOBUFFERMEMORYDESCRIPTOR_HOSTPHYSICALLYCONTIGUOUS_     1
#define IOBUFFERMEMORYDESCRIPTOR_SUPPORTS_INTASKWITHOPTIONS_TAGS        1
/*!
 *   @class IOBufferMemoryDescriptor
 *   @abstract Provides a simple memory descriptor that allocates its own buffer memory.
 */

class IOBufferMemoryDescriptor : public IOGeneralMemoryDescriptor
{
	OSDeclareDefaultStructorsWithDispatch(IOBufferMemoryDescriptor);

private:
/*! @struct ExpansionData
 *   @discussion This structure will be used to expand the capablilties of this class in the future.
 */
	struct ExpansionData {
		IOMemoryMap *   map;
	};

/*! @var reserved
 *   Reserved for future use.  (Internal use only)  */
	APPLE_KEXT_WSHADOW_PUSH;
	ExpansionData * reserved;

protected:
	void *               _buffer;
	vm_size_t            _capacity;
	vm_offset_t          _alignment;
	IOOptionBits         _options;
private:
	uintptr_t            _internalReserved;
	unsigned             _internalFlags;
	APPLE_KEXT_WSHADOW_POP;

private:
#ifndef __LP64__
	virtual bool initWithOptions(
		IOOptionBits options,
		vm_size_t    capacity,
		vm_offset_t  alignment,
		task_t       inTask) APPLE_KEXT_DEPRECATED;                /* use withOptions() instead */
#endif /* !__LP64__ */

public:
	virtual bool initWithPhysicalMask(
		task_t            inTask,
		IOOptionBits      options,
		mach_vm_size_t    capacity,
		mach_vm_address_t alignment,
		mach_vm_address_t physicalMask);

#ifdef __LP64__
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 0);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 1);
#else /* !__LP64__ */
	OSMetaClassDeclareReservedUsed(IOBufferMemoryDescriptor, 0);
	OSMetaClassDeclareReservedUsed(IOBufferMemoryDescriptor, 1);
#endif /* !__LP64__ */
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 2);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 3);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 4);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 5);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 6);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 7);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 8);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 9);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 10);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 11);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 12);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 13);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 14);
	OSMetaClassDeclareReservedUnused(IOBufferMemoryDescriptor, 15);

protected:
	virtual void free() APPLE_KEXT_OVERRIDE;

public:

/*
 * withOptions:
 *
 * Returns a new IOBufferMemoryDescriptor with a buffer large enough to
 * hold capacity bytes.  The descriptor's length is initially set to the
 * capacity.
 */
#ifndef __LP64__
	virtual bool initWithOptions(   IOOptionBits options,
	    vm_size_t    capacity,
	    vm_offset_t  alignment) APPLE_KEXT_DEPRECATED;                         /* use withOptions() instead */
#endif /* !__LP64__ */

	static IOBufferMemoryDescriptor * withCopy(
		task_t            inTask,
		IOOptionBits      options,
		vm_map_t          sourceMap,
		mach_vm_address_t source,
		mach_vm_size_t    size);

	static IOBufferMemoryDescriptor * withOptions(  IOOptionBits options,
	    vm_size_t    capacity,
	    vm_offset_t  alignment = 1);

/*! @function inTaskWithOptions
 *   @abstract Creates a memory buffer with memory descriptor for that buffer.
 *   @discussion Added in Mac OS X 10.2, this method allocates a memory buffer with a given size and alignment in the task's address space specified, and returns a memory descriptor instance representing the memory. It is recommended that memory allocated for I/O or sharing via mapping be created via IOBufferMemoryDescriptor. Options passed with the request specify the kind of memory to be allocated - pageablity and sharing are specified with option bits. This function may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param inTask The task the buffer will be allocated in.
 *   @param options Options for the allocation:<br>
 *   kIODirectionOut, kIODirectionIn - set the direction of the I/O transfer.<br>
 *   kIOMemoryPhysicallyContiguous - pass to request memory be physically contiguous. This option is heavily discouraged. The request may fail if memory is fragmented, may cause large amounts of paging activity, and may take a very long time to execute.<br>
 *   kIOMemoryPageable - pass to request memory be non-wired - the default for kernel allocated memory is wired.<br>
 *   kIOMemoryPurgeable - pass to request memory that may later have its purgeable state set with IOMemoryDescriptor::setPurgeable. Only supported for kIOMemoryPageable allocations.<br>
 *   kIOMemoryKernelUserShared - pass to request memory that will be mapped into both the kernel and client applications.<br>
 *   kIOMapInhibitCache - allocate memory with inhibited cache setting. <br>
 *   kIOMapWriteThruCache - allocate memory with writethru cache setting. <br>
 *   kIOMapCopybackCache - allocate memory with copyback cache setting. <br>
 *   kIOMapWriteCombineCache - allocate memory with writecombined cache setting.
 *   @param capacity The number of bytes to allocate.
 *   @param alignment The minimum required alignment of the buffer in bytes - 1 is the default for no required alignment. For example, pass 256 to get memory allocated at an address with bits 0-7 zero.
 *   @result Returns an instance of class IOBufferMemoryDescriptor to be released by the caller, which will free the memory desriptor and associated buffer. */

	static IOBufferMemoryDescriptor * inTaskWithOptions(
		task_t       inTask,
		IOOptionBits options,
		vm_size_t    capacity,
		vm_offset_t  alignment = 1);

/*! @function inTaskWithOptions
 *   @abstract Creates a memory buffer with memory descriptor for that buffer.
 *   @discussion Added in Mac OS X 10.2, this method allocates a memory buffer with a given size and alignment in the task's address space specified, and returns a memory descriptor instance representing the memory. It is recommended that memory allocated for I/O or sharing via mapping be created via IOBufferMemoryDescriptor. Options passed with the request specify the kind of memory to be allocated - pageablity and sharing are specified with option bits. This function may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param inTask The task the buffer will be allocated in.
 *   @param options Options for the allocation:<br>
 *   kIODirectionOut, kIODirectionIn - set the direction of the I/O transfer.<br>
 *   kIOMemoryPhysicallyContiguous - pass to request memory be physically contiguous. This option is heavily discouraged. The request may fail if memory is fragmented, may cause large amounts of paging activity, and may take a very long time to execute.<br>
 *   kIOMemoryPageable - pass to request memory be non-wired - the default for kernel allocated memory is wired.<br>
 *   kIOMemoryPurgeable - pass to request memory that may later have its purgeable state set with IOMemoryDescriptor::setPurgeable. Only supported for kIOMemoryPageable allocations.<br>
 *   kIOMemoryKernelUserShared - pass to request memory that will be mapped into both the kernel and client applications.<br>
 *   kIOMapInhibitCache - allocate memory with inhibited cache setting. <br>
 *   kIOMapWriteThruCache - allocate memory with writethru cache setting. <br>
 *   kIOMapCopybackCache - allocate memory with copyback cache setting. <br>
 *   kIOMapWriteCombineCache - allocate memory with writecombined cache setting.
 *   @param capacity The number of bytes to allocate.
 *   @param alignment The minimum required alignment of the buffer in bytes - 1 is the default for no required alignment. For example, pass 256 to get memory allocated at an address with bits 0-7 zero.
 *   @param kernTag The kernel memory tag
 *   @param userTag The user memory tag
 *   @result Returns an instance of class IOBufferMemoryDescriptor to be released by the caller, which will free the memory desriptor and associated buffer. */

	static IOBufferMemoryDescriptor * inTaskWithOptions(
		task_t       inTask,
		IOOptionBits options,
		vm_size_t    capacity,
		vm_offset_t  alignment,
		uint32_t     kernTag,
		uint32_t     userTag);

/*! @function inTaskWithPhysicalMask
 *   @abstract Creates a memory buffer with memory descriptor for that buffer.
 *   @discussion Added in Mac OS X 10.5, this method allocates a memory buffer with a given size and alignment in the task's address space specified, and returns a memory descriptor instance representing the memory. It is recommended that memory allocated for I/O or sharing via mapping be created via IOBufferMemoryDescriptor. Options passed with the request specify the kind of memory to be allocated - pageablity and sharing are specified with option bits. This function may block and so should not be called from interrupt level or while a simple lock is held.
 *   @param inTask The task the buffer will be mapped in. Pass NULL to create memory unmapped in any task (eg. for use as a DMA buffer).
 *   @param options Options for the allocation:<br>
 *   kIODirectionOut, kIODirectionIn - set the direction of the I/O transfer.<br>
 *   kIOMemoryPhysicallyContiguous - pass to request memory be physically contiguous. This option is heavily discouraged. The request may fail if memory is fragmented, may cause large amounts of paging activity, and may take a very long time to execute.<br>
 *   kIOMemoryKernelUserShared - pass to request memory that will be mapped into both the kernel and client applications.<br>
 *   kIOMapInhibitCache - allocate memory with inhibited cache setting. <br>
 *   kIOMapWriteThruCache - allocate memory with writethru cache setting. <br>
 *   kIOMapCopybackCache - allocate memory with copyback cache setting. <br>
 *   kIOMapWriteCombineCache - allocate memory with writecombined cache setting.
 *   @param capacity The number of bytes to allocate.
 *   @param physicalMask The buffer will be allocated with pages such that physical addresses will only have bits set present in physicalMask. For example, pass 0x00000000FFFFFFFFULL for a buffer to be accessed by hardware that has 32 address bits.
 *   @result Returns an instance of class IOBufferMemoryDescriptor to be released by the caller, which will free the memory desriptor and associated buffer. */

	static IOBufferMemoryDescriptor * inTaskWithPhysicalMask(
		task_t            inTask,
		IOOptionBits      options,
		mach_vm_size_t    capacity,
		mach_vm_address_t physicalMask);

/*
 * withCapacity:
 *
 * Returns a new IOBufferMemoryDescriptor with a buffer large enough to
 * hold capacity bytes.  The descriptor's length is initially set to the
 * capacity.
 */
	static IOBufferMemoryDescriptor * withCapacity(
		vm_size_t    capacity,
		IODirection  withDirection,
		bool         withContiguousMemory = false);
#ifndef __LP64__
	virtual bool initWithBytes(const void * bytes,
	    vm_size_t    withLength,
	    IODirection  withDirection,
	    bool         withContiguousMemory = false) APPLE_KEXT_DEPRECATED;                    /* use withBytes() instead */
#endif /* !__LP64__ */

/*
 * withBytes:
 *
 * Returns a new IOBufferMemoryDescriptor preloaded with bytes (copied).
 * The descriptor's length and capacity are set to the input buffer's size.
 */
	static IOBufferMemoryDescriptor * withBytes(
		const void * bytes,
		vm_size_t    withLength,
		IODirection  withDirection,
		bool         withContiguousMemory = false);

/*
 * setLength:
 *
 * Change the buffer length of the memory descriptor.  When a new buffer
 * is created, the initial length of the buffer is set to be the same as
 * the capacity.  The length can be adjusted via setLength for a shorter
 * transfer (there is no need to create more buffer descriptors when you
 * can reuse an existing one, even for different transfer sizes).   Note
 * that the specified length must not exceed the capacity of the buffer.
 */
	virtual void setLength(vm_size_t length);

/*
 * setDirection:
 *
 * Change the direction of the transfer.  This method allows one to redirect
 * the descriptor's transfer direction.  This eliminates the need to destroy
 * and create new buffers when different transfer directions are needed.
 */
	virtual void setDirection(IODirection direction);

/*
 * getCapacity:
 *
 * Get the buffer capacity
 */
	virtual vm_size_t getCapacity() const;

/*
 * getBytesNoCopy:
 *
 * Return the virtual address of the beginning of the buffer
 */
	virtual void *getBytesNoCopy();

/*
 * getBytesNoCopy:
 *
 * Return the virtual address of an offset from the beginning of the buffer
 */
	virtual void *getBytesNoCopy(vm_size_t start, vm_size_t withLength);

/*
 * appendBytes:
 *
 * Add some data to the end of the buffer.  This method automatically
 * maintains the memory descriptor buffer length.  Note that appendBytes
 * will not copy past the end of the memory descriptor's current capacity.
 */
	virtual bool appendBytes(const void *bytes, vm_size_t withLength);

#ifndef __LP64__
	virtual void * getVirtualSegment(IOByteCount offset,
	    IOByteCount * length) APPLE_KEXT_OVERRIDE APPLE_KEXT_DEPRECATED;                             /* use getBytesNoCopy() instead */
#endif /* !__LP64__ */
};

#endif /* !_IOBUFFERMEMORYDESCRIPTOR_H */
