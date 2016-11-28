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
#ifndef _IOMEMORYDESCRIPTOR_H
#define _IOMEMORYDESCRIPTOR_H

#include <sys/cdefs.h>

#include <IOKit/IOTypes.h>
#include <IOKit/IOLocks.h>
#include <libkern/c++/OSContainers.h>
#ifdef XNU_KERNEL_PRIVATE
#include <IOKit/IOKitDebug.h>
#endif

#include <mach/memory_object_types.h>

class IOMemoryMap;
class IOMapper;
class IOService;
class IODMACommand;

/*
 * Direction of transfer, with respect to the described memory.
 */
#ifdef __LP64__
enum
#else /* !__LP64__ */
enum IODirection
#endif /* !__LP64__ */
{
    kIODirectionNone  = 0x0,	//                    same as VM_PROT_NONE
    kIODirectionIn    = 0x1,	// User land 'read',  same as VM_PROT_READ
    kIODirectionOut   = 0x2,	// User land 'write', same as VM_PROT_WRITE
    kIODirectionOutIn = kIODirectionOut | kIODirectionIn,
    kIODirectionInOut = kIODirectionIn  | kIODirectionOut,

    // these flags are valid for the prepare() method only
    kIODirectionPrepareToPhys32   = 0x00000004,
    kIODirectionPrepareNoFault    = 0x00000008,
    kIODirectionPrepareReserved1  = 0x00000010,
#define IODIRECTIONPREPARENONCOHERENTDEFINED	1
    kIODirectionPrepareNonCoherent = 0x00000020,

     // these flags are valid for the complete() method only
#define IODIRECTIONCOMPLETEWITHERRORDEFINED		1
     kIODirectionCompleteWithError = 0x00000040,
#define IODIRECTIONCOMPLETEWITHDATAVALIDDEFINED	1
     kIODirectionCompleteWithDataValid = 0x00000080,
};
#ifdef __LP64__
typedef IOOptionBits IODirection;
#endif /* __LP64__ */

/*
 * IOOptionBits used in the withOptions variant
 */
enum {
    kIOMemoryDirectionMask	= 0x00000007,
#ifdef XNU_KERNEL_PRIVATE
    kIOMemoryAutoPrepare	= 0x00000008,	// Shared with Buffer MD
#endif

    kIOMemoryTypeVirtual	= 0x00000010,
    kIOMemoryTypePhysical	= 0x00000020,
    kIOMemoryTypeUPL		= 0x00000030,
    kIOMemoryTypePersistentMD	= 0x00000040,	// Persistent Memory Descriptor
    kIOMemoryTypeUIO		= 0x00000050,
#ifdef __LP64__
    kIOMemoryTypeVirtual64	= kIOMemoryTypeVirtual,
    kIOMemoryTypePhysical64	= kIOMemoryTypePhysical,
#else /* !__LP64__ */
    kIOMemoryTypeVirtual64	= 0x00000060,
    kIOMemoryTypePhysical64	= 0x00000070,
#endif /* !__LP64__ */
    kIOMemoryTypeMask		= 0x000000f0,

    kIOMemoryAsReference	= 0x00000100,
    kIOMemoryBufferPageable	= 0x00000400,
    kIOMemoryMapperNone		= 0x00000800,	// Shared with Buffer MD
    kIOMemoryHostOnly           = 0x00001000,   // Never DMA accessible
#ifdef XNU_KERNEL_PRIVATE
    kIOMemoryRedirected		= 0x00004000,
    kIOMemoryPreparedReadOnly	= 0x00008000,
#endif
    kIOMemoryPersistent		= 0x00010000,
#ifdef XNU_KERNEL_PRIVATE
    kIOMemoryReserved6156215	= 0x00020000,
#endif
    kIOMemoryThreadSafe		= 0x00100000,	// Shared with Buffer MD
    kIOMemoryClearEncrypt	= 0x00200000,	// Shared with Buffer MD

#ifdef XNU_KERNEL_PRIVATE
    kIOMemoryBufferPurgeable	= 0x00400000,
    kIOMemoryBufferCacheMask	= 0x70000000,
    kIOMemoryBufferCacheShift	= 28,
#endif
};

#define kIOMapperSystem	((IOMapper *) 0)

enum 
{
    kIOMemoryPurgeableKeepCurrent = 1,

    kIOMemoryPurgeableNonVolatile = 2,
    kIOMemoryPurgeableVolatile    = 3,
    kIOMemoryPurgeableEmpty       = 4,

    // modifiers for kIOMemoryPurgeableVolatile behavior
    kIOMemoryPurgeableVolatileGroup0           = VM_VOLATILE_GROUP_0,
    kIOMemoryPurgeableVolatileGroup1           = VM_VOLATILE_GROUP_1,
    kIOMemoryPurgeableVolatileGroup2           = VM_VOLATILE_GROUP_2,
    kIOMemoryPurgeableVolatileGroup3           = VM_VOLATILE_GROUP_3,
    kIOMemoryPurgeableVolatileGroup4           = VM_VOLATILE_GROUP_4,
    kIOMemoryPurgeableVolatileGroup5           = VM_VOLATILE_GROUP_5,
    kIOMemoryPurgeableVolatileGroup6           = VM_VOLATILE_GROUP_6,
    kIOMemoryPurgeableVolatileGroup7           = VM_VOLATILE_GROUP_7,
    kIOMemoryPurgeableVolatileBehaviorFifo     = VM_PURGABLE_BEHAVIOR_FIFO,
    kIOMemoryPurgeableVolatileBehaviorLifo     = VM_PURGABLE_BEHAVIOR_LIFO,
    kIOMemoryPurgeableVolatileOrderingObsolete = VM_PURGABLE_ORDERING_OBSOLETE,
    kIOMemoryPurgeableVolatileOrderingNormal   = VM_PURGABLE_ORDERING_NORMAL,
};
enum 
{
    kIOMemoryIncoherentIOFlush	 = 1,
    kIOMemoryIncoherentIOStore	 = 2,

    kIOMemoryClearEncrypted      = 50,
    kIOMemorySetEncrypted        = 51,
};

#define	IOMEMORYDESCRIPTOR_SUPPORTS_DMACOMMAND	1

struct IODMAMapSpecification
{
	uint64_t    alignment;
	IOService * device;
	uint32_t    options;
	uint8_t     numAddressBits;
	uint8_t     resvA[3];
	uint32_t    resvB[4];
};

struct IODMAMapPageList
{
    uint32_t                pageOffset;
    uint32_t                pageListCount;
    const upl_page_info_t * pageList;
};

// mapOptions for iovmMapMemory
enum
{
    kIODMAMapReadAccess           = 0x00000001,
    kIODMAMapWriteAccess          = 0x00000002,
    kIODMAMapPhysicallyContiguous = 0x00000010,
    kIODMAMapDeviceMemory         = 0x00000020,
    kIODMAMapPagingPath           = 0x00000040,
    kIODMAMapIdentityMap          = 0x00000080,

    kIODMAMapPageListFullyOccupied = 0x00000100,
    kIODMAMapFixedAddress          = 0x00000200,
};

#ifdef KERNEL_PRIVATE

// Used for dmaCommandOperation communications for IODMACommand and mappers

enum  {
    kIOMDWalkSegments             = 0x01000000,
    kIOMDFirstSegment	          = 1 | kIOMDWalkSegments,
    kIOMDGetCharacteristics       = 0x02000000,
    kIOMDGetCharacteristicsMapped = 1 | kIOMDGetCharacteristics,
    kIOMDDMAActive                = 0x03000000,
    kIOMDSetDMAActive             = 1 | kIOMDDMAActive,
    kIOMDSetDMAInactive           = kIOMDDMAActive,
    kIOMDAddDMAMapSpec            = 0x04000000,
    kIOMDDMAMap                   = 0x05000000,
    kIOMDDMACommandOperationMask  = 0xFF000000,
};
struct IOMDDMACharacteristics {
    UInt64 fLength;
    UInt32 fSGCount;
    UInt32 fPages;
    UInt32 fPageAlign;
    ppnum_t fHighestPage;
    IODirection fDirection;
    UInt8 fIsPrepared;
};

struct IOMDDMAMapArgs {
    IOMapper            * fMapper;
    IODMACommand        * fCommand;
    IODMAMapSpecification fMapSpec;
    uint64_t              fOffset;
    uint64_t              fLength;
    uint64_t              fAlloc;
    uint64_t              fAllocLength;
    uint8_t               fMapContig;
};

struct IOMDDMAWalkSegmentArgs {
    UInt64 fOffset;			// Input/Output offset
    UInt64 fIOVMAddr, fLength;		// Output variables
    UInt8 fMapped;			// Input Variable, Require mapped IOVMA
};
typedef UInt8 IOMDDMAWalkSegmentState[128];

#endif /* KERNEL_PRIVATE */

enum 
{
    kIOPreparationIDUnprepared = 0,
    kIOPreparationIDUnsupported = 1,
    kIOPreparationIDAlwaysPrepared = 2,
};

#ifdef XNU_KERNEL_PRIVATE
struct IOMemoryReference;
#endif


/*! @class IOMemoryDescriptor : public OSObject
    @abstract An abstract base class defining common methods for describing physical or virtual memory.
    @discussion The IOMemoryDescriptor object represents a buffer or range of memory, specified as one or more physical or virtual address ranges. It contains methods to return the memory's physically contiguous segments (fragments), for use with the IOMemoryCursor, and methods to map the memory into any address space with caching and placed mapping options. */

class IOMemoryDescriptor : public OSObject
{
    friend class IOMemoryMap;
    friend class IOMultiMemoryDescriptor;

    OSDeclareDefaultStructors(IOMemoryDescriptor);

protected:

/*! @var reserved
    Reserved for future use.  (Internal use only)  */
    struct IOMemoryDescriptorReserved * reserved;

protected:
    OSSet *		_mappings;
    IOOptionBits 	_flags;


#ifdef XNU_KERNEL_PRIVATE
public:
    struct IOMemoryReference *	_memRef;
protected:
#else
    void * __iomd_reserved5;
#endif

#ifdef __LP64__
    uint64_t		__iomd_reserved1;
    uint64_t		__iomd_reserved2;
    uint64_t		__iomd_reserved3;
    uint64_t		__iomd_reserved4;
#else /* !__LP64__ */
    IODirection         _direction;        /* use _flags instead */
#endif /* !__LP64__ */
    IOByteCount         _length;           /* length of all ranges */
    IOOptionBits 	_tag;

public:
typedef IOOptionBits DMACommandOps;
#ifndef __LP64__
    virtual IOPhysicalAddress getSourceSegment( IOByteCount offset,
						IOByteCount * length ) APPLE_KEXT_DEPRECATED;
#endif /* !__LP64__ */

/*! @function initWithOptions
    @abstract Master initialiser for all variants of memory descriptors.  For a more complete description see IOMemoryDescriptor::withOptions.
    @discussion Note this function can be used to re-init a previously created memory descriptor.
    @result true on success, false on failure. */
    virtual bool initWithOptions(void *		buffers,
                                 UInt32		count,
                                 UInt32		offset,
                                 task_t		task,
                                 IOOptionBits	options,
                                 IOMapper *	mapper = kIOMapperSystem);

#ifndef __LP64__
    virtual addr64_t getPhysicalSegment64( IOByteCount offset,
                                            IOByteCount * length ) APPLE_KEXT_DEPRECATED; /* use getPhysicalSegment() and kIOMemoryMapperNone instead */
#endif /* !__LP64__ */

/*! @function setPurgeable
    @abstract Control the purgeable status of a memory descriptors memory.
    @discussion Buffers may be allocated with the ability to have their purgeable status changed - IOBufferMemoryDescriptor with the kIOMemoryPurgeable option, VM_FLAGS_PURGEABLE may be passed to vm_allocate() in user space to allocate such buffers. The purgeable status of such a buffer may be controlled with setPurgeable(). The process of making a purgeable memory descriptor non-volatile and determining its previous state is atomic - if a purgeable memory descriptor is made nonvolatile and the old state is returned as kIOMemoryPurgeableVolatile, then the memory's previous contents are completely intact and will remain so until the memory is made volatile again.  If the old state is returned as kIOMemoryPurgeableEmpty then the memory was reclaimed while it was in a volatile state and its previous contents have been lost.
    @param newState - the desired new purgeable state of the memory:<br>
    kIOMemoryPurgeableKeepCurrent - make no changes to the memory's purgeable state.<br>
    kIOMemoryPurgeableVolatile    - make the memory volatile - the memory may be reclaimed by the VM system without saving its contents to backing store.<br>
    kIOMemoryPurgeableNonVolatile - make the memory nonvolatile - the memory is treated as with usual allocations and must be saved to backing store if paged.<br>
    kIOMemoryPurgeableEmpty       - make the memory volatile, and discard any pages allocated to it.
    @param oldState - if non-NULL, the previous purgeable state of the memory is returned here:<br>
    kIOMemoryPurgeableNonVolatile - the memory was nonvolatile.<br>
    kIOMemoryPurgeableVolatile    - the memory was volatile but its content has not been discarded by the VM system.<br>
    kIOMemoryPurgeableEmpty       - the memory was volatile and has been discarded by the VM system.<br>
    @result An IOReturn code. */

    virtual IOReturn setPurgeable( IOOptionBits newState,
                                    IOOptionBits * oldState );
    

/*! @function getPageCounts
    @abstract Retrieve the number of resident and/or dirty pages encompassed by an IOMemoryDescriptor.
    @discussion This method returns the number of resident and/or dirty pages encompassed by an IOMemoryDescriptor.
    @param residentPageCount - If non-null, a pointer to a byte count that will return the number of resident pages encompassed by this IOMemoryDescriptor.
    @param dirtyPageCount - If non-null, a pointer to a byte count that will return the number of dirty pages encompassed by this IOMemoryDescriptor.
    @result An IOReturn code. */

    IOReturn getPageCounts( IOByteCount * residentPageCount,
                            IOByteCount * dirtyPageCount);

/*! @function performOperation
    @abstract Perform an operation on the memory descriptor's memory.
    @discussion This method performs some operation on a range of the memory descriptor's memory. When a memory descriptor's memory is not mapped, it should be more efficient to use this method than mapping the memory to perform the operation virtually.
    @param options The operation to perform on the memory:<br>
    kIOMemoryIncoherentIOFlush - pass this option to store to memory and flush any data in the processor cache for the memory range, with synchronization to ensure the data has passed through all levels of processor cache. It may not be supported on all architectures. This type of flush may be used for non-coherent I/O such as AGP - it is NOT required for PCI coherent operations. The memory descriptor must have been previously prepared.<br>
    kIOMemoryIncoherentIOStore - pass this option to store to memory any data in the processor cache for the memory range, with synchronization to ensure the data has passed through all levels of processor cache. It may not be supported on all architectures. This type of flush may be used for non-coherent I/O such as AGP - it is NOT required for PCI coherent operations. The memory descriptor must have been previously prepared.
    @param offset A byte offset into the memory descriptor's memory.
    @param length The length of the data range.
    @result An IOReturn code. */

    virtual IOReturn performOperation( IOOptionBits options,
                                        IOByteCount offset, IOByteCount length );

    // Used for dedicated communications for IODMACommand
    virtual IOReturn dmaCommandOperation(DMACommandOps op, void *vData, UInt dataSize) const;

/*! @function getPhysicalSegment
    @abstract Break a memory descriptor into its physically contiguous segments.
    @discussion This method returns the physical address of the byte at the given offset into the memory, and optionally the length of the physically contiguous segment from that offset.
    @param offset A byte offset into the memory whose physical address to return.
    @param length If non-zero, getPhysicalSegment will store here the length of the physically contiguous segement at the given offset.
    @result A physical address, or zero if the offset is beyond the length of the memory. */

#ifdef __LP64__
    virtual addr64_t getPhysicalSegment( IOByteCount   offset,
                                         IOByteCount * length,
                                         IOOptionBits  options = 0 ) = 0;
#else /* !__LP64__ */
    virtual addr64_t getPhysicalSegment( IOByteCount   offset,
                                         IOByteCount * length,
                                         IOOptionBits  options );
#endif /* !__LP64__ */

    virtual uint64_t getPreparationID( void );
    void             setPreparationID( void );

#ifdef XNU_KERNEL_PRIVATE
    IOMemoryDescriptorReserved * getKernelReserved( void );
    IOReturn dmaMap(
	IOMapper                    * mapper,
	IODMACommand                * command,
	const IODMAMapSpecification * mapSpec,
	uint64_t                      offset,
	uint64_t                      length,
	uint64_t                    * mapAddress,
	uint64_t                    * mapLength);
#endif
	
private:
    OSMetaClassDeclareReservedUsed(IOMemoryDescriptor, 0);
#ifdef __LP64__
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 1);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 2);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 3);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 4);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 5);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 6);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 7);
#else /* !__LP64__ */
    OSMetaClassDeclareReservedUsed(IOMemoryDescriptor, 1);
    OSMetaClassDeclareReservedUsed(IOMemoryDescriptor, 2);
    OSMetaClassDeclareReservedUsed(IOMemoryDescriptor, 3);
    OSMetaClassDeclareReservedUsed(IOMemoryDescriptor, 4);
    OSMetaClassDeclareReservedUsed(IOMemoryDescriptor, 5);
    OSMetaClassDeclareReservedUsed(IOMemoryDescriptor, 6);
    OSMetaClassDeclareReservedUsed(IOMemoryDescriptor, 7);
#endif /* !__LP64__ */
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 8);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 9);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 10);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 11);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 12);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 13);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 14);
    OSMetaClassDeclareReservedUnused(IOMemoryDescriptor, 15);

protected:
    virtual void free() APPLE_KEXT_OVERRIDE;
public:
    static void initialize( void );

public:
/*! @function withAddress
    @abstract Create an IOMemoryDescriptor to describe one virtual range of the kernel task.
    @discussion This method creates and initializes an IOMemoryDescriptor for memory consisting of a single virtual memory range mapped into the kernel map.  This memory descriptor needs to be prepared before it can be used to extract data from the memory described.
    @param address The virtual address of the first byte in the memory.
    @param withLength The length of memory.
    @param withDirection An I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures.
    @result The created IOMemoryDescriptor on success, to be released by the caller, or zero on failure. */

    static IOMemoryDescriptor * withAddress(void *       address,
                                            IOByteCount  withLength,
                                            IODirection  withDirection);

#ifndef __LP64__
    static IOMemoryDescriptor * withAddress(IOVirtualAddress address,
                                            IOByteCount  withLength,
                                            IODirection  withDirection,
                                            task_t       withTask) APPLE_KEXT_DEPRECATED; /* use withAddressRange() and prepare() instead */
#endif /* !__LP64__ */

/*! @function withPhysicalAddress
    @abstract Create an IOMemoryDescriptor to describe one physical range.
    @discussion This method creates and initializes an IOMemoryDescriptor for memory consisting of a single physical memory range.
    @param address The physical address of the first byte in the memory.
    @param withLength The length of memory.
    @param withDirection An I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures.
    @result The created IOMemoryDescriptor on success, to be released by the caller, or zero on failure. */

    static IOMemoryDescriptor * withPhysicalAddress(
				IOPhysicalAddress	address,
				IOByteCount		withLength,
				IODirection      	withDirection );

#ifndef __LP64__
     static IOMemoryDescriptor * withRanges(IOVirtualRange * ranges,
                                            UInt32           withCount,
                                            IODirection      withDirection,
                                            task_t           withTask,
                                            bool             asReference = false) APPLE_KEXT_DEPRECATED; /* use withAddressRanges() instead */
#endif /* !__LP64__ */

/*! @function withAddressRange
    @abstract Create an IOMemoryDescriptor to describe one virtual range of the specified map.
    @discussion This method creates and initializes an IOMemoryDescriptor for memory consisting of a single virtual memory range mapped into the specified map.  This memory descriptor needs to be prepared before it can be used to extract data from the memory described.
    @param address The virtual address of the first byte in the memory.
    @param withLength The length of memory.
    @param options
        kIOMemoryDirectionMask (options:direction)	This nibble indicates the I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures. 
    @param task The task the virtual ranges are mapped into. Note that unlike IOMemoryDescriptor::withAddress(), kernel_task memory must be explicitly prepared when passed to this api. The task argument may be NULL to specify memory by physical address.
    @result The created IOMemoryDescriptor on success, to be released by the caller, or zero on failure. */

    static IOMemoryDescriptor * withAddressRange(
					mach_vm_address_t address,
					mach_vm_size_t	  length,
					IOOptionBits      options,
					task_t            task);

/*! @function withAddressRanges
    @abstract Create an IOMemoryDescriptor to describe one or more virtual ranges.
    @discussion This method creates and initializes an IOMemoryDescriptor for memory consisting of an array of virtual memory ranges each mapped into a specified source task.  This memory descriptor needs to be prepared before it can be used to extract data from the memory described.
    @param ranges An array of IOAddressRange structures which specify the virtual ranges in the specified map which make up the memory to be described. IOAddressRange is the 64bit version of IOVirtualRange.
    @param rangeCount The member count of the ranges array.
    @param options
        kIOMemoryDirectionMask (options:direction)	This nibble indicates the I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures. 
        kIOMemoryAsReference	For options:type = Virtual or Physical this indicate that the memory descriptor need not copy the ranges array into local memory.  This is an optimisation to try to minimise unnecessary allocations.
    @param task The task each of the virtual ranges are mapped into. Note that unlike IOMemoryDescriptor::withAddress(), kernel_task memory must be explicitly prepared when passed to this api. The task argument may be NULL to specify memory by physical address.
    @result The created IOMemoryDescriptor on success, to be released by the caller, or zero on failure. */

    static IOMemoryDescriptor * withAddressRanges(
					IOAddressRange * ranges,
					UInt32           rangeCount,
					IOOptionBits     options,
					task_t           task);

/*! @function withOptions
    @abstract Master initialiser for all variants of memory descriptors.
    @discussion This method creates and initializes an IOMemoryDescriptor for memory it has three main variants: Virtual, Physical & mach UPL.  These variants are selected with the options parameter, see below.  This memory descriptor needs to be prepared before it can be used to extract data from the memory described.


    @param buffers A pointer to an array of IOAddressRange when options:type is kIOMemoryTypeVirtual64 or kIOMemoryTypePhysical64 or a 64bit kernel. For type UPL it is a upl_t returned by the mach/memory_object_types.h apis, primarily used internally by the UBC. IOVirtualRanges or IOPhysicalRanges are 32 bit only types for use when options:type is kIOMemoryTypeVirtual or kIOMemoryTypePhysical on 32bit kernels.

    @param count options:type = Virtual or Physical count contains a count of the number of entires in the buffers array.  For options:type = UPL this field contains a total length.

    @param offset Only used when options:type = UPL, in which case this field contains an offset for the memory within the buffers upl.

    @param task Only used options:type = Virtual, The task each of the virtual ranges are mapped into.

    @param options
        kIOMemoryDirectionMask (options:direction)	This nibble indicates the I/O direction to be associated with the descriptor, which may affect the operation of the prepare and complete methods on some architectures. 
        kIOMemoryTypeMask (options:type)	kIOMemoryTypeVirtual64, kIOMemoryTypeVirtual, kIOMemoryTypePhysical64, kIOMemoryTypePhysical, kIOMemoryTypeUPL Indicates that what type of memory basic memory descriptor to use.  This sub-field also controls the interpretation of the buffers, count, offset & task parameters.
        kIOMemoryAsReference	For options:type = Virtual or Physical this indicate that the memory descriptor need not copy the ranges array into local memory.  This is an optimisation to try to minimise unnecessary allocations.
        kIOMemoryBufferPageable	Only used by the IOBufferMemoryDescriptor as an indication that the kernel virtual memory is in fact pageable and we need to use the kernel pageable submap rather than the default map.
    
    @param mapper Which IOMapper should be used to map the in-memory physical addresses into I/O space addresses.  Defaults to 0 which indicates that the system mapper is to be used, if present.  

    @result The created IOMemoryDescriptor on success, to be released by the caller, or zero on failure. */

    static IOMemoryDescriptor *withOptions(void *	buffers,
                                           UInt32	count,
                                           UInt32	offset,
                                           task_t	task,
                                           IOOptionBits	options,
                                           IOMapper *	mapper = kIOMapperSystem);

#ifndef __LP64__
    static IOMemoryDescriptor * withPhysicalRanges(
                                            IOPhysicalRange *	ranges,
                                            UInt32		withCount,
                                            IODirection 	withDirection,
                                            bool		asReference = false) APPLE_KEXT_DEPRECATED; /* use withOptions() and kIOMemoryTypePhysical instead */
#endif /* !__LP64__ */

#ifndef __LP64__
    static IOMemoryDescriptor *	withSubRange(IOMemoryDescriptor *of,
					     IOByteCount offset,
					     IOByteCount length,
					     IODirection withDirection) APPLE_KEXT_DEPRECATED; /* use IOSubMemoryDescriptor::withSubRange() and kIOMemoryThreadSafe instead */
#endif /* !__LP64__ */

/*! @function withPersistentMemoryDescriptor
    @abstract Copy constructor that generates a new memory descriptor if the backing memory for the same task's virtual address and length has changed.
    @discussion If the original memory descriptor's address and length is still backed by the same real memory, i.e. the user hasn't deallocated and the reallocated memory at the same address then the original memory descriptor is returned with a additional reference.  Otherwise we build a totally new memory descriptor with the same characteristics as the previous one but with a new view of the vm.  Note not legal to call this function with anything except an IOGeneralMemoryDescriptor that was created with the kIOMemoryPersistent option.
    @param originalMD The memory descriptor to be duplicated.
    @result Either the original memory descriptor with an additional retain or a new memory descriptor, 0 for a bad original memory descriptor or some other resource shortage. */
    static IOMemoryDescriptor *
	withPersistentMemoryDescriptor(IOMemoryDescriptor *originalMD);

#ifndef __LP64__
	// obsolete initializers
	// - initWithOptions is the designated initializer 
    virtual bool initWithAddress(void *       address,
                                 IOByteCount  withLength,
                                 IODirection  withDirection) APPLE_KEXT_DEPRECATED; /* use initWithOptions() instead */
    virtual bool initWithAddress(IOVirtualAddress address,
                                 IOByteCount  withLength,
                                 IODirection  withDirection,
                                 task_t       withTask) APPLE_KEXT_DEPRECATED; /* use initWithOptions() instead */
    virtual bool initWithPhysicalAddress(
				 IOPhysicalAddress	address,
				 IOByteCount		withLength,
				 IODirection      	withDirection ) APPLE_KEXT_DEPRECATED; /* use initWithOptions() instead */
    virtual bool initWithRanges(IOVirtualRange * ranges,
                                UInt32           withCount,
                                IODirection      withDirection,
                                task_t           withTask,
                                bool             asReference = false) APPLE_KEXT_DEPRECATED; /* use initWithOptions() instead */
    virtual bool initWithPhysicalRanges(IOPhysicalRange * ranges,
                                        UInt32           withCount,
                                        IODirection      withDirection,
                                        bool             asReference = false) APPLE_KEXT_DEPRECATED; /* use initWithOptions() instead */
#endif /* __LP64__ */

/*! @function getDirection
    @abstract Accessor to get the direction the memory descriptor was created with.
    @discussion This method returns the direction the memory descriptor was created with.
    @result The direction. */

    virtual IODirection getDirection() const;

/*! @function getLength
    @abstract Accessor to get the length of the memory descriptor (over all its ranges).
    @discussion This method returns the total length of the memory described by the descriptor, ie. the sum of its ranges' lengths.
    @result The byte count. */

    virtual IOByteCount getLength() const;

/*! @function setTag
    @abstract Set the tag for the memory descriptor.
    @discussion This method sets the tag for the memory descriptor. Tag bits are not interpreted by IOMemoryDescriptor.
    @param tag The tag. */

    virtual void setTag( IOOptionBits tag );

/*! @function getTag
    @abstract Accessor to the retrieve the tag for the memory descriptor.
    @discussion This method returns the tag for the memory descriptor. Tag bits are not interpreted by IOMemoryDescriptor.
    @result The tag. */

    virtual IOOptionBits getTag( void );

/*! @function readBytes
    @abstract Copy data from the memory descriptor's buffer to the specified buffer.
    @discussion This method copies data from the memory descriptor's memory at the given offset, to the caller's buffer.  The memory descriptor MUST have the kIODirectionOut direcction bit set  and be prepared.  kIODirectionOut means that this memory descriptor will be output to an external device, so readBytes is used to get memory into a local buffer for a PIO transfer to the device.
    @param offset A byte offset into the memory descriptor's memory.
    @param bytes The caller supplied buffer to copy the data to.
    @param withLength The length of the data to copy.
    @result The number of bytes copied, zero will be returned if the specified offset is beyond the length of the descriptor. Development/debug kernel builds will assert if the offset is beyond the length of the descriptor. */

    virtual IOByteCount readBytes(IOByteCount offset,
				void * bytes, IOByteCount withLength);

/*! @function writeBytes
    @abstract Copy data to the memory descriptor's buffer from the specified buffer.
    @discussion This method copies data to the memory descriptor's memory at the given offset, from the caller's buffer.  The memory descriptor MUST have the kIODirectionIn direcction bit set  and be prepared.  kIODirectionIn means that this memory descriptor will be input from an external device, so writeBytes is used to write memory into the descriptor for PIO drivers.
    @param offset A byte offset into the memory descriptor's memory.
    @param bytes The caller supplied buffer to copy the data from.
    @param withLength The length of the data to copy.
    @result The number of bytes copied, zero will be returned if the specified offset is beyond the length of the descriptor. Development/debug kernel builds will assert if the offset is beyond the length of the descriptor. */

    virtual IOByteCount writeBytes(IOByteCount offset,
				const void * bytes, IOByteCount withLength);

#ifndef __LP64__
    virtual IOPhysicalAddress getPhysicalSegment(IOByteCount offset,
						 IOByteCount * length);
#endif /* !__LP64__ */

/*! @function getPhysicalAddress
    @abstract Return the physical address of the first byte in the memory.
    @discussion This method returns the physical address of the  first byte in the memory. It is most useful on memory known to be physically contiguous.
    @result A physical address. */

    IOPhysicalAddress getPhysicalAddress();

#ifndef __LP64__
    virtual void * getVirtualSegment(IOByteCount offset,
					IOByteCount * length) APPLE_KEXT_DEPRECATED; /* use map() and getVirtualAddress() instead */
#endif /* !__LP64__ */

/*! @function prepare
    @abstract Prepare the memory for an I/O transfer.
    @discussion This involves paging in the memory, if necessary, and wiring it down for the duration of the transfer.  The complete() method completes the processing of the memory after the I/O transfer finishes.  Note that the prepare call is not thread safe and it is expected that the client will more easily be able to guarantee single threading a particular memory descriptor.
    @param forDirection The direction of the I/O just completed, or kIODirectionNone for the direction specified by the memory descriptor.
    @result An IOReturn code. */

    virtual IOReturn prepare(IODirection forDirection = kIODirectionNone) = 0;

/*! @function complete
    @abstract Complete processing of the memory after an I/O transfer finishes.
    @discussion This method should not be called unless a prepare was previously issued; the prepare() and complete() must occur in pairs, before and after an I/O transfer involving pageable memory.  In 10.3 or greater systems the direction argument to complete is not longer respected.  The direction is totally determined at prepare() time.
    @param forDirection DEPRECATED The direction of the I/O just completed, or kIODirectionNone for the direction specified by the memory descriptor.
    @result An IOReturn code. */

    virtual IOReturn complete(IODirection forDirection = kIODirectionNone) = 0;

    /*
     * Mapping functions.
     */

/*! @function createMappingInTask
    @abstract Maps a IOMemoryDescriptor into a task.
    @discussion This is the general purpose method to map all or part of the memory described by a memory descriptor into a task at any available address, or at a fixed address if possible. Caching & read-only options may be set for the mapping. The mapping is represented as a returned reference to a IOMemoryMap object, which may be shared if the mapping is compatible with an existing mapping of the IOMemoryDescriptor. The IOMemoryMap object returned should be released only when the caller has finished accessing the mapping, as freeing the object destroys the mapping. 
    @param intoTask Sets the target task for the mapping. Pass kernel_task for the kernel address space.
    @param atAddress If a placed mapping is requested, atAddress specifies its address, and the kIOMapAnywhere should not be set. Otherwise, atAddress is ignored.
    @param options Mapping options are defined in IOTypes.h,<br>
	kIOMapAnywhere should be passed if the mapping can be created anywhere. If not set, the atAddress parameter sets the location of the mapping, if it is available in the target map.<br>
	kIOMapDefaultCache to inhibit the cache in I/O areas, kIOMapCopybackCache in general purpose RAM.<br>
	kIOMapInhibitCache, kIOMapWriteThruCache, kIOMapCopybackCache to set the appropriate caching.<br>
	kIOMapReadOnly to allow only read only accesses to the memory - writes will cause and access fault.<br>
	kIOMapReference will only succeed if the mapping already exists, and the IOMemoryMap object is just an extra reference, ie. no new mapping will be created.<br>
	kIOMapUnique allows a special kind of mapping to be created that may be used with the IOMemoryMap::redirect() API. These mappings will not be shared as is the default - there will always be a unique mapping created for the caller, not an existing mapping with an extra reference.<br>
	kIOMapPrefault will try to prefault the pages corresponding to the mapping. This must not be done on the kernel task, and the memory must have been wired via prepare(). Otherwise, the function will fail.<br>
    @param offset Is a beginning offset into the IOMemoryDescriptor's memory where the mapping starts. Zero is the default to map all the memory.
    @param length Is the length of the mapping requested for a subset of the IOMemoryDescriptor. Zero is the default to map all the memory.
    @result A reference to an IOMemoryMap object representing the mapping, which can supply the virtual address of the mapping and other information. The mapping may be shared with multiple callers - multiple maps are avoided if a compatible one exists. The IOMemoryMap object returned should be released only when the caller has finished accessing the mapping, as freeing the object destroys the mapping. The IOMemoryMap instance also retains the IOMemoryDescriptor it maps while it exists. */

    IOMemoryMap * 	createMappingInTask(
	task_t			intoTask,
	mach_vm_address_t	atAddress,
	IOOptionBits		options,
	mach_vm_size_t		offset = 0,
	mach_vm_size_t		length = 0 );

#ifndef __LP64__
    virtual IOMemoryMap * 	map(
	task_t		intoTask,
	IOVirtualAddress	atAddress,
	IOOptionBits		options,
	IOByteCount		offset = 0,
	IOByteCount		length = 0 ) APPLE_KEXT_DEPRECATED; /* use createMappingInTask() instead */
#endif /* !__LP64__ */

/*! @function map
    @abstract Maps a IOMemoryDescriptor into the kernel map.
    @discussion This is a shortcut method to map all the memory described by a memory descriptor into the kernel map at any available address. See the full version of the createMappingInTask method for further details.
    @param options Mapping options as in the full version of the createMappingInTask method, with kIOMapAnywhere assumed.
    @result See the full version of the createMappingInTask method. */

    virtual IOMemoryMap * 	map(
	IOOptionBits		options = 0 );

/*! @function setMapping
    @abstract Establishes an already existing mapping.
    @discussion This method tells the IOMemoryDescriptor about a mapping that exists, but was created elsewhere. It allows later callers of the map method to share this externally created mapping. The IOMemoryMap object returned is created to represent it. This method is not commonly needed.
    @param task Address space in which the mapping exists.
    @param mapAddress Virtual address of the mapping.
    @param options Caching and read-only attributes of the mapping.
    @result A IOMemoryMap object created to represent the mapping. */

    virtual IOMemoryMap * 	setMapping(
	task_t		task,
	IOVirtualAddress	mapAddress,
	IOOptionBits		options = 0 );

    // Following methods are private implementation

#ifdef __LP64__
    virtual
#endif /* __LP64__ */
    IOReturn redirect( task_t safeTask, bool redirect );

    IOReturn handleFault(
        void *			_pager,
	mach_vm_size_t		sourceOffset,
	mach_vm_size_t		length);

    IOReturn populateDevicePager(
        void *			pager,
	vm_map_t		addressMap,
	mach_vm_address_t	address,
	mach_vm_size_t		sourceOffset,
	mach_vm_size_t		length,
        IOOptionBits		options );

    virtual IOMemoryMap * 	makeMapping(
	IOMemoryDescriptor *	owner,
	task_t			intoTask,
	IOVirtualAddress	atAddress,
	IOOptionBits		options,
	IOByteCount		offset,
	IOByteCount		length );

protected:
    virtual void 		addMapping(
	IOMemoryMap *		mapping );

    virtual void 		removeMapping(
	IOMemoryMap *		mapping );

    virtual IOReturn doMap(
	vm_map_t		addressMap,
	IOVirtualAddress *	atAddress,
	IOOptionBits		options,
	IOByteCount		sourceOffset = 0,
	IOByteCount		length = 0 );

    virtual IOReturn doUnmap(
	vm_map_t		addressMap,
	IOVirtualAddress	logical,
	IOByteCount		length );
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*! @class IOMemoryMap : public OSObject
    @abstract A class defining common methods for describing a memory mapping.
    @discussion The IOMemoryMap object represents a mapped range of memory, described by a IOMemoryDescriptor. The mapping may be in the kernel or a non-kernel task and has processor cache mode attributes. IOMemoryMap instances are created by IOMemoryDescriptor when it creates mappings in its map method, and returned to the caller. */

class IOMemoryMap : public OSObject
{
    OSDeclareDefaultStructors(IOMemoryMap)
#ifdef XNU_KERNEL_PRIVATE
public:
    IOMemoryDescriptor * fMemory;
    IOMemoryMap *	 fSuperMap;
    mach_vm_size_t	 fOffset;
    mach_vm_address_t	 fAddress;
    mach_vm_size_t	 fLength;
    task_t		 fAddressTask;
    vm_map_t		 fAddressMap;
    IOOptionBits	 fOptions;
    upl_t		 fRedirUPL;
    ipc_port_t		 fRedirEntry;
    IOMemoryDescriptor * fOwner;
    uint8_t		 fUserClientUnmap;
#if IOTRACKING
    IOTracking           fTracking;
#endif
#endif /* XNU_KERNEL_PRIVATE */

protected:
    virtual void taggedRelease(const void *tag = 0) const APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

public:
/*! @function getVirtualAddress
    @abstract Accessor to the virtual address of the first byte in the mapping.
    @discussion This method returns the virtual address of the first byte in the mapping. Since the IOVirtualAddress is only 32bit in 32bit kernels, the getAddress() method should be used for compatibility with 64bit task mappings.
    @result A virtual address. */

    virtual IOVirtualAddress 	getVirtualAddress();

/*! @function getPhysicalSegment
    @abstract Break a mapping into its physically contiguous segments.
    @discussion This method returns the physical address of the byte at the given offset into the mapping, and optionally the length of the physically contiguous segment from that offset. It functions similarly to IOMemoryDescriptor::getPhysicalSegment.
    @param offset A byte offset into the mapping whose physical address to return.
    @param length If non-zero, getPhysicalSegment will store here the length of the physically contiguous segement at the given offset.
    @result A physical address, or zero if the offset is beyond the length of the mapping. */

#ifdef __LP64__
    virtual IOPhysicalAddress 	getPhysicalSegment(IOByteCount offset,
	       					   IOByteCount * length,
	       					   IOOptionBits  options = 0);
#else /* !__LP64__ */
    virtual IOPhysicalAddress 	getPhysicalSegment(IOByteCount offset,
	       					   IOByteCount * length);
#endif /* !__LP64__ */

/*! @function getPhysicalAddress
    @abstract Return the physical address of the first byte in the mapping.
    @discussion This method returns the physical address of the  first byte in the mapping. It is most useful on mappings known to be physically contiguous.
    @result A physical address. */

    IOPhysicalAddress getPhysicalAddress();

/*! @function getLength
    @abstract Accessor to the length of the mapping.
    @discussion This method returns the length of the mapping.
    @result A byte count. */

    virtual IOByteCount 	getLength();

/*! @function getAddressTask
    @abstract Accessor to the task of the mapping.
    @discussion This method returns the mach task the mapping exists in.
    @result A mach task_t. */

    virtual task_t		getAddressTask();

/*! @function getMemoryDescriptor
    @abstract Accessor to the IOMemoryDescriptor the mapping was created from.
    @discussion This method returns the IOMemoryDescriptor the mapping was created from.
    @result An IOMemoryDescriptor reference, which is valid while the IOMemoryMap object is retained. It should not be released by the caller. */

    virtual IOMemoryDescriptor * getMemoryDescriptor();

/*! @function getMapOptions
    @abstract Accessor to the options the mapping was created with.
    @discussion This method returns the options to IOMemoryDescriptor::map the mapping was created with.
    @result Options for the mapping, including cache settings. */

    virtual IOOptionBits 	getMapOptions();

/*! @function unmap
    @abstract Force the IOMemoryMap to unmap, without destroying the object.
    @discussion IOMemoryMap instances will unmap themselves upon free, ie. when the last client with a reference calls release. This method forces the IOMemoryMap to destroy the mapping it represents, regardless of the number of clients. It is not generally used.
    @result An IOReturn code. */

    virtual IOReturn 		unmap();

    virtual void		taskDied();

/*! @function redirect
    @abstract Replace the memory mapped in a process with new backing memory.
    @discussion An IOMemoryMap created with the kIOMapUnique option to IOMemoryDescriptor::map() can remapped to a new IOMemoryDescriptor backing object. If the new IOMemoryDescriptor is specified as NULL, client access to the memory map is blocked until a new backing object has been set. By blocking access and copying data, the caller can create atomic copies of the memory while the client is potentially reading or writing the memory. 
    @param newBackingMemory The IOMemoryDescriptor that represents the physical memory that is to be now mapped in the virtual range the IOMemoryMap represents. If newBackingMemory is NULL, any access to the mapping will hang (in vm_fault()) until access has been restored by a new call to redirect() with non-NULL newBackingMemory argument.
    @param options Mapping options are defined in IOTypes.h, and are documented in IOMemoryDescriptor::map()
    @param offset As with IOMemoryDescriptor::map(), a beginning offset into the IOMemoryDescriptor's memory where the mapping starts. Zero is the default.
    @result An IOReturn code. */

#ifndef __LP64__
// For 32 bit XNU, there is a 32 bit (IOByteCount) and a 64 bit (mach_vm_size_t) interface;
// for 64 bit, these fall together on the 64 bit one.
    virtual IOReturn		redirect(IOMemoryDescriptor * newBackingMemory,
					 IOOptionBits         options,
					 IOByteCount          offset = 0);
#endif
    virtual IOReturn		redirect(IOMemoryDescriptor * newBackingMemory,
					 IOOptionBits         options,
					 mach_vm_size_t       offset = 0);

#ifdef __LP64__
/*! @function getAddress
    @abstract Accessor to the virtual address of the first byte in the mapping.
    @discussion This method returns the virtual address of the first byte in the mapping.
    @result A virtual address. */
/*! @function getSize
    @abstract Accessor to the length of the mapping.
    @discussion This method returns the length of the mapping.
    @result A byte count. */
    inline mach_vm_address_t 	getAddress() __attribute__((always_inline));
    inline mach_vm_size_t 	getSize() __attribute__((always_inline));
#else /* !__LP64__ */
/*! @function getAddress
    @abstract Accessor to the virtual address of the first byte in the mapping.
    @discussion This method returns the virtual address of the first byte in the mapping.
    @result A virtual address. */
/*! @function getSize
    @abstract Accessor to the length of the mapping.
    @discussion This method returns the length of the mapping.
    @result A byte count. */
    virtual mach_vm_address_t 	getAddress();
    virtual mach_vm_size_t 	getSize();
#endif /* !__LP64__ */

#ifdef XNU_KERNEL_PRIVATE
    // for IOMemoryDescriptor use
    IOMemoryMap * copyCompatible( IOMemoryMap * newMapping );

    bool init(
	task_t			intoTask,
	mach_vm_address_t	toAddress,
	IOOptionBits		options,
        mach_vm_size_t		offset,
        mach_vm_size_t		length );

    bool    setMemoryDescriptor(IOMemoryDescriptor * _memory, mach_vm_size_t _offset);

    IOReturn redirect(
	task_t			intoTask, bool redirect );

    IOReturn userClientUnmap();
#endif /* XNU_KERNEL_PRIVATE */

    IOReturn wireRange(
    	uint32_t		options,
        mach_vm_size_t		offset,
        mach_vm_size_t		length);

    OSMetaClassDeclareReservedUnused(IOMemoryMap, 0);
    OSMetaClassDeclareReservedUnused(IOMemoryMap, 1);
    OSMetaClassDeclareReservedUnused(IOMemoryMap, 2);
    OSMetaClassDeclareReservedUnused(IOMemoryMap, 3);
    OSMetaClassDeclareReservedUnused(IOMemoryMap, 4);
    OSMetaClassDeclareReservedUnused(IOMemoryMap, 5);
    OSMetaClassDeclareReservedUnused(IOMemoryMap, 6);
    OSMetaClassDeclareReservedUnused(IOMemoryMap, 7);
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef XNU_KERNEL_PRIVATE
// Also these flags should not overlap with the options to
//	IOMemoryDescriptor::initWithRanges(... IOOptionsBits options);
enum {
    _kIOMemorySourceSegment	= 0x00002000
};
#endif /* XNU_KERNEL_PRIVATE */

// The following classes are private implementation of IOMemoryDescriptor - they
// should not be referenced directly, just through the public API's in the 
// IOMemoryDescriptor class. For example, an IOGeneralMemoryDescriptor instance
// might be created by IOMemoryDescriptor::withAddressRange(), but there should be 
// no need to reference as anything but a generic IOMemoryDescriptor *.

class IOGeneralMemoryDescriptor : public IOMemoryDescriptor
{
    OSDeclareDefaultStructors(IOGeneralMemoryDescriptor);

public:
    union Ranges {
        IOVirtualRange   *v;
        IOAddressRange   *v64;
        IOPhysicalRange  *p;
	void 		 *uio;
    };
protected:
    Ranges		_ranges;
    unsigned		_rangesCount;       /* number of address ranges in list */
#ifndef __LP64__
    bool		_rangesIsAllocated; /* is list allocated by us? */
#endif /* !__LP64__ */

    task_t		_task;               /* task where all ranges are mapped to */

    union {
        IOVirtualRange	v;
        IOPhysicalRange	p;
    }			_singleRange;	   /* storage space for a single range */

    unsigned		_wireCount;        /* number of outstanding wires */

#ifndef __LP64__
    uintptr_t _cachedVirtualAddress;

    IOPhysicalAddress	_cachedPhysicalAddress;
#endif /* !__LP64__ */

    bool		_initialized;      /* has superclass been initialized? */

public:
    virtual void free() APPLE_KEXT_OVERRIDE;

    virtual IOReturn dmaCommandOperation(DMACommandOps op, void *vData, UInt dataSize) const APPLE_KEXT_OVERRIDE;

    virtual uint64_t getPreparationID( void ) APPLE_KEXT_OVERRIDE;

#ifdef XNU_KERNEL_PRIVATE
    // Internal APIs may be made virtual at some time in the future.
    IOReturn wireVirtual(IODirection forDirection);
    IOReturn dmaMap(
	IOMapper                    * mapper,
	IODMACommand                * command,
	const IODMAMapSpecification * mapSpec,
	uint64_t                      offset,
	uint64_t                      length,
	uint64_t                    * mapAddress,
	uint64_t                    * mapLength);
    bool initMemoryEntries(size_t size, IOMapper * mapper);

    IOMemoryReference * memoryReferenceAlloc(uint32_t capacity, 
    					     IOMemoryReference * realloc);
    void memoryReferenceFree(IOMemoryReference * ref);
    void memoryReferenceRelease(IOMemoryReference * ref);

    IOReturn memoryReferenceCreate(
                        IOOptionBits         options,
                        IOMemoryReference ** reference);

    IOReturn memoryReferenceMap(IOMemoryReference * ref,
			 vm_map_t            map,
			 mach_vm_size_t      inoffset,
			 mach_vm_size_t      size,
			 IOOptionBits        options,
			 mach_vm_address_t * inaddr);

    static IOReturn memoryReferenceSetPurgeable(
				IOMemoryReference * ref,
				IOOptionBits newState,
				IOOptionBits * oldState);
    static IOReturn memoryReferenceGetPageCounts(
			       IOMemoryReference * ref,
                               IOByteCount       * residentPageCount,
                               IOByteCount       * dirtyPageCount);
#endif

private:

#ifndef __LP64__
    virtual void setPosition(IOByteCount position);
    virtual void mapIntoKernel(unsigned rangeIndex);
    virtual void unmapFromKernel();
#endif /* !__LP64__ */

    // Internal
    OSData *	    _memoryEntries;
    unsigned int    _pages;
    ppnum_t	    _highestPage;
    uint32_t	    __iomd_reservedA;
    uint32_t	    __iomd_reservedB;

    IOLock *	    _prepareLock;

public:
    /*
     * IOMemoryDescriptor required methods
     */

    // Master initaliser
    virtual bool initWithOptions(void *		buffers,
                                 UInt32		count,
                                 UInt32		offset,
                                 task_t		task,
                                 IOOptionBits	options,
                                 IOMapper *	mapper = kIOMapperSystem) APPLE_KEXT_OVERRIDE;

#ifndef __LP64__
    // Secondary initialisers
    virtual bool initWithAddress(void *		address,
                                 IOByteCount	withLength,
                                 IODirection	withDirection) APPLE_KEXT_OVERRIDE APPLE_KEXT_DEPRECATED;

    virtual bool initWithAddress(IOVirtualAddress address,
                                 IOByteCount    withLength,
                                 IODirection	withDirection,
                                 task_t		withTask) APPLE_KEXT_OVERRIDE APPLE_KEXT_DEPRECATED;

    virtual bool initWithPhysicalAddress(
				 IOPhysicalAddress	address,
				 IOByteCount		withLength,
				 IODirection      	withDirection ) APPLE_KEXT_OVERRIDE APPLE_KEXT_DEPRECATED;

    virtual bool initWithRanges(        IOVirtualRange * ranges,
                                        UInt32           withCount,
                                        IODirection      withDirection,
                                        task_t           withTask,
                                        bool             asReference = false) APPLE_KEXT_OVERRIDE APPLE_KEXT_DEPRECATED;

    virtual bool initWithPhysicalRanges(IOPhysicalRange * ranges,
                                        UInt32           withCount,
                                        IODirection      withDirection,
                                        bool             asReference = false) APPLE_KEXT_OVERRIDE APPLE_KEXT_DEPRECATED;

    virtual addr64_t getPhysicalSegment64( IOByteCount offset,
                                            IOByteCount * length ) APPLE_KEXT_OVERRIDE APPLE_KEXT_DEPRECATED;

    virtual IOPhysicalAddress getPhysicalSegment(IOByteCount offset,
						 IOByteCount * length) APPLE_KEXT_OVERRIDE;

    virtual IOPhysicalAddress getSourceSegment(IOByteCount offset,
                                               IOByteCount * length) APPLE_KEXT_OVERRIDE APPLE_KEXT_DEPRECATED;

    virtual void * getVirtualSegment(IOByteCount offset,
					IOByteCount * length) APPLE_KEXT_OVERRIDE APPLE_KEXT_DEPRECATED;
#endif /* !__LP64__ */

    virtual IOReturn setPurgeable( IOOptionBits newState,
                                    IOOptionBits * oldState ) APPLE_KEXT_OVERRIDE;
    
    virtual addr64_t getPhysicalSegment( IOByteCount   offset,
                                         IOByteCount * length,
#ifdef __LP64__
                                         IOOptionBits  options = 0 ) APPLE_KEXT_OVERRIDE;
#else /* !__LP64__ */
                                         IOOptionBits  options ) APPLE_KEXT_OVERRIDE;
#endif /* !__LP64__ */

    virtual IOReturn prepare(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;

    virtual IOReturn complete(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;

    virtual IOReturn doMap(
	vm_map_t		addressMap,
	IOVirtualAddress *	atAddress,
	IOOptionBits		options,
	IOByteCount		sourceOffset = 0,
	IOByteCount		length = 0 ) APPLE_KEXT_OVERRIDE;

    virtual IOReturn doUnmap(
	vm_map_t		addressMap,
	IOVirtualAddress	logical,
	IOByteCount		length ) APPLE_KEXT_OVERRIDE;

    virtual bool serialize(OSSerialize *s) const APPLE_KEXT_OVERRIDE;

    // Factory method for cloning a persistent IOMD, see IOMemoryDescriptor
    static IOMemoryDescriptor *
	withPersistentMemoryDescriptor(IOGeneralMemoryDescriptor *originalMD);

};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef __LP64__
mach_vm_address_t 	IOMemoryMap::getAddress()
{
    return (getVirtualAddress());
}

mach_vm_size_t 	IOMemoryMap::getSize()
{
    return (getLength());
}
#else /* !__LP64__ */
#include <IOKit/IOSubMemoryDescriptor.h>
#endif /* !__LP64__ */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#endif /* !_IOMEMORYDESCRIPTOR_H */
