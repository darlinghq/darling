/*
 * Copyright (c) 2005-2016 Apple Inc. All rights reserved.
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
#ifndef _IODMACOMMAND_H
#define _IODMACOMMAND_H

#include <IOKit/IOCommand.h>
#include <IOKit/IOMemoryDescriptor.h>
class IOMapper;
class IOBufferMemoryDescriptor;

enum{
	kIODMAMapOptionMapped       = 0x00000000,
	kIODMAMapOptionBypassed     = 0x00000001,
	kIODMAMapOptionNonCoherent  = 0x00000002,
	kIODMAMapOptionUnmapped     = 0x00000003,
	kIODMAMapOptionTypeMask     = 0x0000000f,

	kIODMAMapOptionNoCacheStore = 0x00000010, // Memory in descriptor
	kIODMAMapOptionOnChip       = 0x00000020,// Indicates DMA is on South Bridge
	kIODMAMapOptionIterateOnly  = 0x00000040// DMACommand will be used as a cursor only
};

/**************************** class IODMACommand ***************************/

/*!
 *   @class IODMACommand
 *   @abstract A mechanism to convert memory references to I/O bus addresses.
 *   @discussion The IODMACommand is supersedes the IOMemoryCursor and greatly enhances the functionality and power of it.  The command can be specified to output 64 bit physical addresses and also allows driver writers bypass mapping hardware or get addresses suitable for non-snooped DMA.
 *  <br><br>
 *   The command is designed to be very easily subclassable.  Most driver writers need to associate some DMA operations with their memory descriptor and usually use a C structure for that purpose.  This structure is often kept in a linked list.  This IODMACommand has built it <kern/queue.h> linkage and can be derived and 'public:' variables added, giving the developer a structure that can associate a memory descriptor with a particular dma command but will also allow the developer to generate that command and keep the state necessary for tracking it.
 *  <br><br>
 *   It is envisaged that a pool of IODMACommands will be created at driver initialisation and each command will be kept in an IOCommandPool while not in use.  However if developers wishes to maintain their own free lists that is certainly possible.  See the <kern/queue.h> and <xnu/iokit/Kernel/IOCommandPool> for sample code on manipulating the command's doubly linked list entries.
 *  <br><br>
 *   The IODMACommand can be used in a 'weak-linked' manner.  To do this you must avoid using any static member functions.  Use the, much slower but safe, weakWithSpecification function.  On success a dma command instance will be returned.  This instance can then be used to clone as many commands as is needed.  Remember deriving from this class can not be done weakly, that is no weak subclassing!
 */

class IODMACommand : public IOCommand
{
	OSDeclareDefaultStructors(IODMACommand);

	friend class IODMAEventSource;

public:

/*!
 *   @typedef Segment32
 *   @discussion A 32 bit I/O bus address/length pair
 */
	struct Segment32 {
		UInt32 fIOVMAddr, fLength;
	};

/*!
 *   @typedef Segment64
 *   @discussion A 64 bit I/O bus address/length pair
 */
	struct Segment64 {
		UInt64 fIOVMAddr, fLength;
	};

/*! @enum MappingOptions
 *   @abstract Mapping types to indicate the desired mapper type for translating memory descriptors into I/O DMA Bus addresses.
 *   @constant kNonCoherent	Used by drivers for non-coherent transfers, implies unmapped memmory
 *   @constant kMapped		Allow a driver to define addressing size
 *   @constant kBypassed		Allow drivers to bypass any mapper
 *   @constant kMaxMappingOptions	Internal use only
 */
	enum MappingOptions {
		kMapped       = kIODMAMapOptionMapped,
		kBypassed     = kIODMAMapOptionBypassed,
		kNonCoherent  = kIODMAMapOptionNonCoherent,
		kUnmapped     = kIODMAMapOptionUnmapped,
		kTypeMask     = kIODMAMapOptionTypeMask,

		kNoCacheStore = kIODMAMapOptionNoCacheStore, // Memory in descriptor
		kOnChip       = kIODMAMapOptionOnChip,  // Indicates DMA is on South Bridge
		kIterateOnly  = kIODMAMapOptionIterateOnly// DMACommand will be used as a cursor only
	};

	struct SegmentOptions {
		uint8_t  fStructSize;
		uint8_t  fNumAddressBits;
		uint64_t fMaxSegmentSize;
		uint64_t fMaxTransferSize;
		uint32_t fAlignment;
		uint32_t fAlignmentLength;
		uint32_t fAlignmentInternalSegments;
	};

/*! @enum SynchronizeOptions
 *   @abstract Options for the synchronize method.
 *   @constant kForceDoubleBuffer Copy the entire prepared range to a new page aligned buffer.
 */
	enum SynchronizeOptions {
		kForceDoubleBuffer = 0x01000000
	};

/*!
 *   @typedef SegmentFunction
 *   @discussion Pointer to a C function that translates a 64 segment and outputs a single desired segment to the array at the requested index. There are a group of pre-implemented SegmentFunctions that may be usefull to the developer below.
 *   @param segment The 64Bit I/O bus address and length.
 *   @param segments Base of the output vector of DMA address length pairs.
 *   @param segmentIndex Index to output 'segment' in the 'segments' array.
 *   @result Returns true if segment encoding succeeded.  false may be returned if the current segment does not fit in an output segment, i.e. a 38bit address wont fit into a 32 encoding.
 */
	typedef bool (*SegmentFunction)(IODMACommand *target,
	    Segment64 segment,
	    void     *segments,
	    UInt32    segmentIndex);

// -------------- Preimplemented output functions ----------------

/*! @function OutputHost32
 *   @abstract Output host natural Segment32 output segment function.
 */
	static bool OutputHost32(IODMACommand *target,
	    Segment64 seg, void *segs, UInt32 ind);

/*! @defined kIODMACommandOutputHost32
 *   @abstract Output host natural Segment32 output segment function.
 */
#define kIODMACommandOutputHost32   (IODMACommand::OutputHost32)

/*! @function OutputBig32
 *   @abstract Output big-endian Segment32 output segment function.
 */
	static bool OutputBig32(IODMACommand *target,
	    Segment64 seg, void *segs, UInt32 ind);

/*! @defined kIODMACommandOutputBig32
 *   @abstract Output big-endian Segment32 output segment function.
 */
#define kIODMACommandOutputBig32    (IODMACommand::OutputBig32)

/*! @function OutputLittle32
 *   @abstract Output little-endian Segment32 output segment function.
 */
	static bool OutputLittle32(IODMACommand *target,
	    Segment64 seg, void *segs, UInt32 ind);

/*! @defined kIODMACommandOutputLittle32
 *   @abstract Output little-endian Segment32 output segment function.
 */
#define kIODMACommandOutputLittle32 (IODMACommand::OutputLittle32)

/*! @function OutputHost64
 *   @abstract Output host natural Segment64 output segment function.
 */
	static bool OutputHost64(IODMACommand *target,
	    Segment64 seg, void *segs, UInt32 ind);

/*! @defined kIODMACommandOutputHost64
 *   @abstract Output host natural Segment64 output segment function.
 */
#define kIODMACommandOutputHost64   (IODMACommand::OutputHost64)

/*! @function OutputBig64
 *   @abstract Output big-endian Segment64 output segment function.
 */
	static bool OutputBig64(IODMACommand *target,
	    Segment64 seg, void *segs, UInt32 ind);

/*! @defined kIODMACommandOutputBig64
 *   @abstract Output big-endian Segment64 output segment function.
 */
#define kIODMACommandOutputBig64    (IODMACommand::OutputBig64)

/*! @function OutputLittle64
 *   @abstract Output little-endian Segment64 output segment function.
 */
	static bool OutputLittle64(IODMACommand *target,
	    Segment64 seg, void *segs, UInt32 ind);

/*! @defined kIODMACommandOutputLittle64
 *   @abstract Output little-endian Segment64 output segment function.
 */
#define kIODMACommandOutputLittle64 (IODMACommand::OutputLittle64)

/*! @function withSpecification
 *   @abstract Creates and initializes an IODMACommand in one operation.
 *   @discussion Factory function to create and initialize an IODMACommand in one operation.
 *   @param outSegFunc SegmentFunction to call to output one physical segment. A set of nine commonly required segment functions are provided.
 *   @param numAddressBits Number of bits that the hardware uses on its internal address bus. Typically 32 but may be more on modern hardware.  A 0 implies no-restriction other than that implied by the output segment function.
 *   @param maxSegmentSize Maximum allowable size for one segment.  If 0 is passed the maximum segment size is unlimited.
 *   @param mappingOptions is the type of mapping that is required to translate an IOMemoryDescriptor into the desired number of bits.  For instance if your hardware only supports 32 bits but must run on machines with > 4G of RAM some mapping will be required.  Number of bits will be specified in numAddressBits, see below.This parameter can take 3 values:- kNonCoherent - used for non-coherent hardware transfers, Mapped - Validate that all I/O bus generated addresses are within the number of addressing bits specified, Bypassed indicates that bypassed addressing is required, this is used when the hardware transferes are into coherent memory but no mapping is required.  See also prepare() for failure cases.
 *   @param maxTransferSize Maximum size of an entire transfer.	Defaults to 0 indicating no maximum.
 *   @param alignment Alignment restriction, in bytes, on I/O bus addresses.  Defaults to single byte alignment.
 *   @param mapper For mapping types kMapped & kBypassed mapper is used to define the hardware that will perform the mapping, defaults to the system mapper.
 *   @param refCon Reference Constant
 *   @result Returns a new IODMACommand if successfully created and initialized, 0 otherwise.
 */
	static IODMACommand *
	withSpecification(SegmentFunction  outSegFunc,
	    UInt8            numAddressBits,
	    UInt64           maxSegmentSize,
	    MappingOptions   mappingOptions = kMapped,
	    UInt64           maxTransferSize = 0,
	    UInt32           alignment = 1,
	    IOMapper        *mapper = NULL,
	    void            *refCon = NULL);

/*! @function weakWithSpecification
 *   @abstract Creates and initialises an IODMACommand in one operation if this version of the operating system supports it.
 *   @discussion Factory function to create and initialise an IODMACommand in one operation.  The function allows a developer to 'weak' link with IODMACommand.  This function will return kIOReturnUnsupported if the IODMACommand is unavailable.  This function is actually fairly slow so it will be better to call it once then clone the successfully create command using cloneCommand (q.v.).
 *   @param newCommand Output reference variable of the newly created IODMACommand.
 *   @param outSegFunc SegmentFunction to call to output one physical segment. A set of nine commonly required segment functions are provided.
 *   @param numAddressBits Number of bits that the hardware uses on its internal address bus. Typically 32 but may be more on modern hardware.  A 0 implies no-restriction other than that implied by the output segment function.
 *   @param maxSegmentSize Maximum allowable size for one segment. Zero is treated as an unlimited segment size.
 *   @param mapType is the type of mapping that is required to translate an IOMemoryDescriptor into the desired number of bits.  For instance if your hardware only supports 32 bits but must run on machines with > 4G of RAM some mapping will be required.  Number of bits will be specified in numAddressBits, see below.  This parameter can take 3 values:- kNonCoherent - used for non-coherent hardware transfers, Mapped - Validate that all I/O bus generated addresses are within the number of addressing bits specified, Bypassed indicates that bypassed addressing is required, this is used when the hardware transfers are into coherent memory but no mapping is required.  See also prepare() for failure cases.
 *   @param maxTransferSize Maximum size of an entire transfer.  Defaults to 0 indicating no maximum.
 *   @param alignment Alignment restriction, in bytes, on I/O bus addresses.  Defaults to single byte alignment.
 *   @param mapper For mapping types kMapped & kBypassed mapper is used to define the hardware that will perform the mapping, defaults to the system mapper.
 *   @param refCon Reference Constant
 *   @result kIOReturnSuccess if everything is OK, otherwise kIOReturnBadArgument if newCommand is NULL, kIOReturnUnsupported if the kernel doesn't export IODMACommand or IOReturnError if the new command fails to init, q.v. initWithSpecification.
 */
// Note that the function has the attribute always_inline.
// The point of this function is to make a call into the kernel
// without generating an undefined symbol.  If the client could call
// the code as a function then the goal of no undefined symbols
// would be lost thus defeating the purpose.
	static inline IOReturn weakWithSpecification
	(IODMACommand   **newCommand,
	    SegmentFunction outSegFunc,
	    UInt8           numAddressBits,
	    UInt64          maxSegmentSize,
	    MappingOptions  mapType = kMapped,
	    UInt64          maxTransferSize = 0,
	    UInt32          alignment = 1,
	    IOMapper       *mapper = NULL,
	    void           *refCon = NULL) __attribute__((always_inline));

	static IODMACommand *
	withSpecification(SegmentFunction        outSegFunc,
	    const SegmentOptions * segmentOptions,
	    uint32_t               mappingOptions,
	    IOMapper             * mapper,
	    void                 * refCon);


/*! @function withRefCon
 *   @abstract Creates and initializes an unspecified IODMACommand.
 *   @discussion Factory function to create and initialize an unspecified IODMACommand. prepareWithSpecification() must be used to prepare the IODMACommand before use.
 *   @param refCon Reference Constant
 *   @result Returns a new IODMACommand if successfully created and initialized, 0 otherwise.
 */
	static IODMACommand * withRefCon(void * refCon);

/*!
 *   @function cloneCommand
 *   @abstract Creates a new command based on the specification of the current one.
 *   @discussion Factory function to create and initialise an IODMACommand in one operation.  The current command's specification will be duplicated in the new object, but however none of its state will be duplicated.  This means that it is safe to clone a command even if it is currently active and running, however you must be certain that the command to be duplicated does have a valid reference for the duration.
 *   @result Returns a new IODMACommand if successfully created and initialised, 0 otherwise.
 */
	virtual IODMACommand *cloneCommand(void *refCon = NULL);

/*! @function initWithSpecification
 *   @abstract Primary initializer for the IODMACommand class.
 *   @param outSegFunc SegmentFunction to call to output one physical segment. A set of nine commonly required segment functions are provided.
 *   @param numAddressBits Number of bits that the hardware uses on its internal address bus. Typically 32 but may be more on modern hardware.  A 0 implies no-restriction other than that implied by the output segment function.
 *   @param maxSegmentSize Maximum allowable size for one segment.  Defaults to 0 which means any size.
 *   @param mappingOptions is the type of mapping that is required to translate an IOMemoryDescriptor into the desired number of bits.  For instance if your hardware only supports 32 bits but must run on machines with > 4G of RAM some mapping will be required.  Number of bits will be specified in numAddressBits, see below.This parameter can take 3 values:- kNonCoherent - used for non-coherent hardware transfers, Mapped - Validate that all I/O bus generated addresses are within the number of addressing bits specified, Bypassed indicates that bypassed addressing is required, this is used when the hardware transferes are into coherent memory but no mapping is required.  See also prepare() for failure cases.
 *   @param maxTransferSize Maximum size of an entire transfer.	Defaults to 0 indicating no maximum.
 *   @param alignment Alignment restriction, in bytes, on I/O bus addresses.  Defaults to single byte alignment.
 *   @param mapper For mapping types kMapped & kBypassed mapper is used to define the hardware that will perform the mapping, defaults to the system mapper.
 *   @param refCon Reference Constant
 *   @result Can fail if the mapping type is not recognised, if one of the 3 mandatory parameters are set to 0, if a 32 bit output function is selected when more than 32 bits of address is required or, if kBypassed is requested on a machine that doesn't support bypassing.  Returns true otherwise.
 */
	virtual bool initWithSpecification( SegmentFunction  outSegFunc,
	    UInt8     numAddressBits,
	    UInt64    maxSegmentSize,
	    MappingOptions mappingOptions = kMapped,
	    UInt64    maxTransferSize = 0,
	    UInt32    alignment = 1,
	    IOMapper *mapper = NULL,
	    void     *refCon = NULL);

/*! @function setMemoryDescriptor
 *   @abstract Sets and resets the DMACommand's current memory descriptor
 *   @discussion The DMA command will configure itself based on the information that it finds in the memory descriptor.  It looks for things like the direction of the memory descriptor and whether the current memory descriptor is already mapped into some IOMMU.  As a programmer convenience it can also prepare the DMA command immediately.  See prepare().  Note the IODMACommand is designed to used multiple times with a succession of memory descriptors, making the pooling of commands possible.  It is an error though to attempt to reset a currently prepared() DMA command.  Warning: This routine may block so never try to autoprepare an IODMACommand while in a gated context, i.e. one of the WorkLoops action call outs.
 *   @param mem A pointer to the current I/Os memory descriptor.
 *   @param autoPrepare An optional boolean variable that will call the prepare() function automatically after the memory descriptor is processed. Defaults to true.
 *   @result Returns kIOReturnSuccess, kIOReturnBusy if currently prepared, kIOReturnNoSpace if the length(mem) >= Maximum Transfer Size or the error codes returned by prepare() (qv).
 */
	virtual IOReturn setMemoryDescriptor(const IOMemoryDescriptor *mem,
	    bool autoPrepare = true);

/*! @function clearMemoryDescriptor
 *   @abstract Clears the DMACommand's current memory descriptor
 *   @discussion completes and invalidates the cache if the DMA command is currently active, copies all data from bounce buffers if necessary and releases all resources acquired during setMemoryDescriptor.
 *   @param autoComplete An optional boolean variable that will call the complete() function automatically before the memory descriptor is processed. Defaults to true.
 */
	virtual IOReturn clearMemoryDescriptor(bool autoComplete = true);

/*! @function getMemoryDescriptor
 *   @abstract Get the current memory descriptor
 */
	virtual const IOMemoryDescriptor *getMemoryDescriptor() const;

/*! @function getIOMemoryDescriptor
 *   @abstract Get the memory descriptor to be used for DMA
 */
	IOMemoryDescriptor * getIOMemoryDescriptor() const;

/*! @function prepare
 *   @abstract Prepare the memory for an I/O transfer.
 *   @discussion Allocate the mapping resources neccessary for this transfer, specifying a sub range of the IOMemoryDescriptor that will be the target of the I/O.  The complete() method frees these resources.  Data may be copied to buffers for kIODirectionOut memory descriptors, depending on hardware mapping resource availabilty or alignment restrictions.  It should be noted that the this function may block and should only be called on the clients context, i.e never call this routine while gated; also the call itself is not thread safe though this should be an issue as each IODMACommand is independant.
 *   @param offset defines the starting offset in the memory descriptor the DMA command will operate on. genIOVMSegments will produce its results based on the offset and length passed to the prepare method.
 *   @param length defines the ending position in the memory descriptor the DMA command will operate on. genIOVMSegments will produce its results based on the offset and length passed to the prepare method.
 *   @param flushCache Flush the caches for the memory descriptor and make certain that the memory cycles are complete.  Defaults to true for kNonCoherent and is ignored by the other types.
 *   @param synchronize Copy any buffered data back from the target IOMemoryDescriptor.  Defaults to true, if synchronize() is being used to explicitly copy data, passing false may avoid an unneeded copy.
 *   @result An IOReturn code. */

	virtual IOReturn prepare(UInt64 offset = 0, UInt64 length = 0, bool flushCache = true, bool synchronize = true);

/*! @function complete
 *   @abstract Complete processing of DMA mappings after an I/O transfer is finished.
 *   @discussion This method should not be called unless a prepare was previously issued; the prepare() and complete() must occur in pairs, before and after an I/O transfer
 *   @param invalidateCache Invalidate the caches for the memory descriptor.  Defaults to true for kNonCoherent and is ignored by the other types.
 *   @param synchronize Copy any buffered data back to the target IOMemoryDescriptor.  Defaults to true, if synchronize() is being used to explicitly copy data, passing false may avoid an unneeded copy.
 *   @result kIOReturnNotReady if not prepared, kIOReturnSuccess otherwise. */

	virtual IOReturn complete(bool invalidateCache = true, bool synchronize = true);

/*! @function synchronize
 *   @abstract Bring IOMemoryDescriptor and IODMACommand buffers into sync.
 *   @discussion This method should not be called unless a prepare was previously issued. If needed a caller may synchronize any IODMACommand buffers with the original IOMemoryDescriptor buffers.
 *   @param options Specifies the direction of the copy:
 *       kIODirectionOut copy IOMemoryDesciptor memory to any IODMACommand buffers. By default this action takes place automatically at prepare().
 *       kIODirectionIn copy any IODMACommand buffers back to the IOMemoryDescriptor. By default this action takes place automatically at complete().
 *       kForceDoubleBuffer copy the entire prepared range to a new page aligned buffer.
 *   @result kIOReturnNotReady if not prepared, kIOReturnBadArgument if invalid options are passed, kIOReturnSuccess otherwise. */

	virtual IOReturn synchronize(IOOptionBits options);

/*! @function genIOVMSegments
 *   @abstract Generates a physical scatter/gather for the current DMA command
 *   @discussion Generates a list of physical segments from the given memory descriptor, relative to the current position of the descriptor.  The constraints that are set during initialisation will be respected. This function maintains the state across multiple calls for efficiency.  However the state is discarded if the new offset is not the expected one.
 *   @param offset input/output parameter, defines the starting and ending offset in the memory descriptor, relative to any offset passed to the prepare() method.
 *   @param segments Void pointer to base of output physical scatter/gather list.  Always passed directly onto the SegmentFunction.
 *   @param numSegments Input/output parameter Number of segments that can fit in the segment array and returns number of segments generated.
 *   @result kIOReturnSuccess on success, kIOReturnOverrun if the memory descriptor is exhausted, kIOReturnMessageTooLarge if the output segment function's address bits has insufficient resolution for a segment, kIOReturnNotReady if the DMA command has not be prepared, kIOReturnBadArgument if the DMA command doesn't have a memory descriptor yet or some of the parameters are NULL and kIOReturnNotReady if the DMA command is not prepared.
 */
	virtual IOReturn genIOVMSegments(UInt64 *offset,
	    void   *segments,
	    UInt32 *numSegments);

private:
	virtual UInt64 transfer( IOOptionBits transferOp, UInt64 offset, void * buffer, UInt64 length );

public:

/*! @function writeBytes
 *   @abstract Copy data to the IODMACommand's buffer from the specified buffer.
 *   @discussion This method copies data to the IODMACommand's memory at the given offset, from the caller's buffer. The IODMACommand must be prepared, and the offset is relative to the prepared offset.
 *   @param offset A byte offset into the IODMACommand's memory, relative to the prepared offset.
 *   @param bytes The caller supplied buffer to copy the data from.
 *   @param length The length of the data to copy.
 *   @result The number of bytes copied, zero will be returned if the specified offset is beyond the prepared length of the IODMACommand. */

	UInt64 writeBytes(UInt64 offset, const void *bytes, UInt64 length);

/*! @function readBytes
 *   @abstract Copy data from the IODMACommand's buffer to the specified buffer.
 *   @discussion This method copies data from the IODMACommand's memory at the given offset, to the caller's buffer. The IODMACommand must be prepared, and the offset is relative to the prepared offset.
 *   @param offset A byte offset into the IODMACommand's memory, relative to the prepared offset.
 *   @param bytes The caller supplied buffer to copy the data to.
 *   @param length The length of the data to copy.
 *   @result The number of bytes copied, zero will be returned if the specified offset is beyond the prepared length of the IODMACommand. */

	UInt64 readBytes(UInt64 offset, void *bytes, UInt64 length);

/*! @function gen32IOVMSegments
 *   @abstract Helper function for a type checked call to genIOVMSegments(qv), for use with an IODMACommand set up with the output function kIODMACommandOutputHost32, kIODMACommandOutputBig32, or kIODMACommandOutputLittle32. If the output function of the IODMACommand is not a 32 bit function, results will be incorrect.
 */
	inline IOReturn
	gen32IOVMSegments(UInt64   *offset,
	    Segment32 *segments,
	    UInt32     *numSegments)
	{
		return genIOVMSegments(offset, segments, numSegments);
	}

/*! @function gen64IOVMSegments
 *   @abstract Helper function for a type checked call to genIOVMSegments(qv), for use with an IODMACommand set up with the output function kIODMACommandOutputHost64, kIODMACommandOutputBig64, or kIODMACommandOutputLittle64. If the output function of the IODMACommand is not a 64 bit function, results will be incorrect.
 */
	inline IOReturn
	gen64IOVMSegments(UInt64    *offset,
	    Segment64 *segments,
	    UInt32    *numSegments)
	{
		return genIOVMSegments(offset, segments, numSegments);
	}

	IOReturn
	genIOVMSegments(SegmentFunction segmentFunction,
	    UInt64 *offsetP,
	    void   *segmentsP,
	    UInt32 *numSegmentsP);

	virtual void free() APPLE_KEXT_OVERRIDE;

private:
	IOReturn setSpecification(SegmentFunction        outSegFunc,
	    const SegmentOptions * segmentOptions,
	    uint32_t               mappingOptions,
	    IOMapper             * mapper);

	typedef IOReturn (*InternalSegmentFunction)(
		void         *reference,
		IODMACommand *target,
		Segment64     segment,
		void         *segments,
		UInt32        segmentIndex);

	IOReturn genIOVMSegments(uint32_t op,
	    InternalSegmentFunction outSegFunc,
	    void   *reference,
	    UInt64 *offsetP,
	    void   *segmentsP,
	    UInt32 *numSegmentsP);

	static IOReturn clientOutputSegment(
		void *reference, IODMACommand *target,
		Segment64 segment, void *vSegList, UInt32 outSegIndex);

	static IOReturn segmentOp(
		void         *reference,
		IODMACommand *target,
		Segment64     segment,
		void         *segments,
		UInt32        segmentIndex);
	IOReturn walkAll(UInt8 op);

public:

/*! @function prepareWithSpecification
 *   @abstract Prepare the memory for an I/O transfer with a new specification.
 *   @discussion Allocate the mapping resources neccessary for this transfer, specifying a sub range of the IOMemoryDescriptor that will be the target of the I/O.  The complete() method frees these resources.  Data may be copied to buffers for kIODirectionOut memory descriptors, depending on hardware mapping resource availabilty or alignment restrictions.  It should be noted that the this function may block and should only be called on the clients context, i.e never call this routine while gated; also the call itself is not thread safe though this should be an issue as each IODMACommand is independant.
 *   @param outSegFunc SegmentFunction to call to output one physical segment. A set of nine commonly required segment functions are provided.
 *   @param numAddressBits Number of bits that the hardware uses on its internal address bus. Typically 32 but may be more on modern hardware.  A 0 implies no-restriction other than that implied by the output segment function.
 *   @param maxSegmentSize Maximum allowable size for one segment.  Defaults to 0 which means any size.
 *   @param mappingOptions is the type of mapping that is required to translate an IOMemoryDescriptor into the desired number of bits.  For instance if your hardware only supports 32 bits but must run on machines with > 4G of RAM some mapping will be required.  Number of bits will be specified in numAddressBits, see below.This parameter can take 3 values:- kNonCoherent - used for non-coherent hardware transfers, Mapped - Validate that all I/O bus generated addresses are within the number of addressing bits specified, Bypassed indicates that bypassed addressing is required, this is used when the hardware transferes are into coherent memory but no mapping is required.  See also prepare() for failure cases.
 *   @param maxTransferSize Maximum size of an entire transfer.	Defaults to 0 indicating no maximum.
 *   @param alignment Alignment restriction, in bytes, on I/O bus addresses.  Defaults to single byte alignment.
 *   @param mapper For mapping types kMapped & kBypassed mapper is used to define the hardware that will perform the mapping, defaults to the system mapper.
 *   @param offset defines the starting offset in the memory descriptor the DMA command will operate on. genIOVMSegments will produce its results based on the offset and length passed to the prepare method.
 *   @param length defines the ending position in the memory descriptor the DMA command will operate on. genIOVMSegments will produce its results based on the offset and length passed to the prepare method.
 *   @param flushCache Flush the caches for the memory descriptor and make certain that the memory cycles are complete.  Defaults to true for kNonCoherent and is ignored by the other types.
 *   @param synchronize Copy any buffered data back from the target IOMemoryDescriptor.  Defaults to true, if synchronize() is being used to explicitly copy data, passing false may avoid an unneeded copy.
 *   @result An IOReturn code. Can fail if the mapping type is not recognised, if one of the 3 mandatory parameters are set to 0, if a 32 bit output function is selected when more than 32 bits of address is required or, if kBypassed is requested on a machine that doesn't support bypassing.
 */

	virtual IOReturn prepareWithSpecification(SegmentFunction   outSegFunc,
	    UInt8             numAddressBits,
	    UInt64            maxSegmentSize,
	    MappingOptions    mappingOptions = kMapped,
	    UInt64            maxTransferSize = 0,
	    UInt32            alignment = 1,
	    IOMapper          *mapper = NULL,
	    UInt64            offset = 0,
	    UInt64            length = 0,
	    bool              flushCache = true,
	    bool              synchronize = true);

	static IOReturn transferSegment(void         *reference,
	    IODMACommand *target,
	    Segment64     segment,
	    void         *segments,
	    UInt32        segmentIndex);

/*! @function getPreparedOffsetAndLength
 *   @abstract Returns the offset and length into the target IOMemoryDescriptor of a prepared IODDMACommand.
 *   @discussion If successfully prepared, returns the offset and length into the IOMemoryDescriptor. Will fail for an unprepared IODMACommand.
 *   @param offset returns the starting offset in the memory descriptor the DMA command was prepared with. Pass NULL for don't care.
 *   @param length returns the length in the memory descriptor the DMA command was prepared with. Pass NULL for don't care.
 *   @result An IOReturn code. kIOReturnNotReady if the IODMACommand is not prepared. */

	virtual IOReturn getPreparedOffsetAndLength(UInt64 * offset, UInt64 * length);

	UInt8    getNumAddressBits(void);
	UInt32   getAlignment(void);
	uint32_t getAlignmentLength(void);
	uint32_t getAlignmentInternalSegments(void);


/*! @function initWithRefCon
 *   @abstract Secondary initializer for the IODMACommand class.
 *   @param refCon Reference Constant
 *   @result Can fail if super init fails.  Returns true otherwise.
 */

	virtual
	bool initWithRefCon(void * refCon = NULL);

	virtual
	bool initWithSpecification(SegmentFunction        outSegFunc,
	    const SegmentOptions * segmentOptions,
	    uint32_t               mappingOptions,
	    IOMapper             * mapper,
	    void                 * refCon);

	virtual
	IOReturn prepareWithSpecification(SegmentFunction        outSegFunc,
	    const SegmentOptions * segmentOptions,
	    uint32_t               mappingOptions,
	    IOMapper             * mapper,
	    uint64_t               offset,
	    uint64_t               length,
	    bool                   flushCache = true,
	    bool                   synchronize = true);

	virtual
	IOBufferMemoryDescriptor * createCopyBuffer(IODirection direction, UInt64 length);

private:
	OSMetaClassDeclareReservedUsed(IODMACommand, 0);
	OSMetaClassDeclareReservedUsed(IODMACommand, 1);
	OSMetaClassDeclareReservedUsed(IODMACommand, 2);
	OSMetaClassDeclareReservedUsed(IODMACommand, 3);
	OSMetaClassDeclareReservedUsed(IODMACommand, 4);
	OSMetaClassDeclareReservedUsed(IODMACommand, 5);
	OSMetaClassDeclareReservedUsed(IODMACommand, 6);
	OSMetaClassDeclareReservedUnused(IODMACommand, 7);
	OSMetaClassDeclareReservedUnused(IODMACommand, 8);
	OSMetaClassDeclareReservedUnused(IODMACommand, 9);
	OSMetaClassDeclareReservedUnused(IODMACommand, 10);
	OSMetaClassDeclareReservedUnused(IODMACommand, 11);
	OSMetaClassDeclareReservedUnused(IODMACommand, 12);
	OSMetaClassDeclareReservedUnused(IODMACommand, 13);
	OSMetaClassDeclareReservedUnused(IODMACommand, 14);
	OSMetaClassDeclareReservedUnused(IODMACommand, 15);

public:
/*! @var fRefCon Reference Constant, client defined publicly avialable */
	void *fRefCon;

protected:

/*! @var fMaxSegmentSize Maximum size of one segment in a scatter/gather list */
	UInt64  fMaxSegmentSize;

/*! @var fMaxTransferSize
 *   Maximum size of a transfer that this memory cursor is allowed to generate */
	UInt64  fMaxTransferSize;

	UInt32  fAlignMaskLength;
	UInt32  fAlignMaskInternalSegments;

/*! @var fMapper
 *   Client defined mapper. */
	IOMapper *fMapper;

/*! @var fMemory
 *   memory descriptor for current I/O. */
	const IOMemoryDescriptor *fMemory;

/*! @var fOutSeg The action method called when an event has been delivered */
	SegmentFunction fOutSeg;

/*! @var fAlignMask
 *   Alignment restriction mask. */
	UInt32  fAlignMask;

/*! @var fNumAddressBits
 *   Number of bits that the hardware can address */
	UInt32 fNumAddressBits;

/*! @var fNumSegments
 *   Number of contiguous segments required for the current memory descriptor and desired mapping */
	UInt32  fNumSegments;

/*! @var fMappingOptions
 *   What type of I/O virtual address mapping is required for this command */
	uint32_t  fMappingOptions;

/*! @var fActive
 *   fActive indicates that this DMA command is currently prepared and ready to go */
	UInt32 fActive;

/*! @var reserved
 *   Reserved for future use.  (Internal use only)  */
	struct IODMACommandInternal * reserved;
};

IOReturn
IODMACommand::
weakWithSpecification(IODMACommand **newCommand,
    SegmentFunction  outSegFunc,
    UInt8     numAddressBits,
    UInt64    maxSegmentSize,
    MappingOptions mapType,
    UInt64    maxTransferSize,
    UInt32    alignment,
    IOMapper *mapper,
    void     *refCon)
{
	if (!newCommand) {
		return kIOReturnBadArgument;
	}

	IODMACommand *self = (IODMACommand *)
	    OSMetaClass::allocClassWithName("IODMACommand");
	if (!self) {
		return kIOReturnUnsupported;
	}

	IOReturn ret;
	bool inited = self->
	    initWithSpecification(outSegFunc,
	    numAddressBits, maxSegmentSize, mapType,
	    maxTransferSize, alignment, mapper, refCon);
	if (inited) {
		ret =  kIOReturnSuccess;
	} else {
		self->release();
		self = NULL;
		ret = kIOReturnError;
	}

	*newCommand = self;
	return ret;
};
#endif /* !_IODMACOMMAND_H */
