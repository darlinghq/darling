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

#ifndef _IOKIT_IOSHAREDDATAQUEUE_H
#define _IOKIT_IOSHAREDDATAQUEUE_H

#ifdef dequeue
#undef dequeue
#endif

#define DISABLE_DATAQUEUE_WARNING /* IODataQueue is deprecated, please use IOSharedDataQueue instead */

#include <IOKit/IODataQueue.h>

#undef DISABLE_DATAQUEUE_WARNING

typedef struct _IODataQueueEntry IODataQueueEntry;

/*!
 * @class IOSharedDataQueue : public IODataQueue
 * @abstract A generic queue designed to pass data both from the kernel to a user process and from a user process to the kernel.
 * @discussion The IOSharedDataQueue class is designed to also allow a user process to queue data to kernel code.  IOSharedDataQueue objects are designed to be used in a single producer / single consumer situation.  As such, there are no locks on the data itself.  Because the kernel enqueue and user-space dequeue methods follow a strict set of guidelines, no locks are necessary to maintain the integrity of the data struct.
 *
 * <br>Each data entry can be variable sized, but the entire size of the queue data region (including overhead for each entry) must be specified up front.
 *
 * <br>In order for the IODataQueue instance to notify the user process that data is available, a notification mach port must be set.  When the queue is empty and a new entry is added, a message is sent to the specified port.
 *
 * <br>In order to make the data queue memory available to a user process, the method getMemoryDescriptor() must be used to get an IOMemoryDescriptor instance that can be mapped into a user process.  Typically, the clientMemoryForType() method on an IOUserClient instance will be used to request the IOMemoryDescriptor and then return it to be mapped into the user process.
 */
class IOSharedDataQueue : public IODataQueue
{
    OSDeclareDefaultStructors(IOSharedDataQueue)

    struct ExpansionData { 
        UInt32 queueSize;
    };
    /*! @var reserved
        Reserved for future use.  (Internal use only)  */
    ExpansionData * _reserved;

protected:
    virtual void free() APPLE_KEXT_OVERRIDE;

    /*!
     * @function getQueueSize
     * @abstract Returns the size of the data queue.
     * @discussion Use this method to access the size of the data queue. Do not access the value of size directly, as it can get modified from userspace and is not reliable. 
     * @result Returns the size of the data queue, or zero in case of failure.
     */
    UInt32 getQueueSize();

    /*!
     * @function setQueueSize
     * @abstract Stores the value of the size of the data queue.
     * @discussion Use this method to store the value of the size of the data queue. Do not access the value of size directly, as it can get modified from userspace and is not reliable.
     * @param size The size of the data queue.  
     * @result Returns true in case of success, false otherwise.
     */
    Boolean setQueueSize(UInt32 size);

public:
    /*!
     * @function withCapacity
     * @abstract Static method that creates a new IOSharedDataQueue instance with the capacity specified in the size parameter.
     * @discussion The actual size of the entire data queue memory region (to be shared into a user process) is equal to the capacity plus the IODataQueueMemory overhead.  This overhead value can be determined from the DATA_QUEUE_MEMORY_HEADER_SIZE macro in <IOKit/IODataQueueShared.h>.  The size of the data queue memory region must include space for the overhead of each IODataQueueEntry.  This entry overhead can be determined from the DATA_QUEUE_ENTRY_HEADER_SIZE macro in <IOKit/IODataQueueShared.h>.<br>  This method allocates a new IODataQueue instance and then calls initWithCapacity() with the given size parameter.  If the initWithCapacity() fails, the new instance is released and zero is returned.
     * @param size The size of the data queue memory region.
     * @result Returns the newly allocated IOSharedDataQueue instance.  Zero is returned on failure.
     */
    static IOSharedDataQueue *withCapacity(UInt32 size);

    /*!
     * @function withEntries
     * @abstract Static method that creates a new IOSharedDataQueue instance with the specified number of entries of the given size.
     * @discussion This method will create a new IOSharedDataQueue instance with enough capacity for numEntries of entrySize.  It does account for the IODataQueueEntry overhead for each entry.  Note that the numEntries and entrySize are simply used to determine the data region size.  They do not actually restrict the size of number of entries that can be added to the queue.<br>  This method allocates a new IODataQueue instance and then calls initWithEntries() with the given numEntries and entrySize parameters.  If the initWithEntries() fails, the new instance is released and zero is returned.
     * @param numEntries Number of entries to allocate space for.
     * @param entrySize Size of each entry.
     * @result Reeturns the newly allocated IOSharedDataQueue instance.  Zero is returned on failure.
     */
    static IOSharedDataQueue *withEntries(UInt32 numEntries, UInt32 entrySize);

    /*!
     * @function initWithCapacity
     * @abstract Initializes an IOSharedDataQueue instance with the capacity specified in the size parameter.
     * @discussion The actual size of the entire data queue memory region (to be shared into a user process) is equal to the capacity plus the IODataQueueMemory overhead.  This overhead value can be determined from the DATA_QUEUE_MEMORY_HEADER_SIZE and DATA_QUEUE_MEMORY_APPENDIX_SIZE macro in <IOKit/IODataQueueShared.h>.  The size of the data queue memory region must include space for the overhead of each IODataQueueEntry.  This entry overhead can be determined from the DATA_QUEUE_ENTRY_HEADER_SIZE macro in <IOKit/IODataQueueShared.h>.
     * @param size The size of the data queue memory region.
     * @result Returns true on success and false on failure.
     */
    virtual Boolean initWithCapacity(UInt32 size) APPLE_KEXT_OVERRIDE;

    /*!
     * @function getMemoryDescriptor
     * @abstract Returns a memory descriptor covering the IODataQueueMemory region.
     * @discussion The IOMemoryDescriptor instance returned by this method is intended to be mapped into a user process.  This is the memory region that the IODataQueueClient code operates on.
     * @result Returns a newly allocated IOMemoryDescriptor for the IODataQueueMemory region.  Returns zero on failure.
     */
    virtual IOMemoryDescriptor *getMemoryDescriptor() APPLE_KEXT_OVERRIDE;

    /*!
     * @function peek
     * @abstract Used to peek at the next entry on the queue.
     * @discussion This function can be used to look at the next entry which allows the entry to be received without having to copy it with dequeue.  In order to do this, call peek to get the entry.  Then call dequeue with a NULL data pointer.  That will cause the head to be moved to the next entry, but no memory to be copied.
     * @result Returns a pointer to the next IODataQueueEntry if one is available.  0 (NULL) is returned if the queue is empty.
     */
    virtual IODataQueueEntry * peek();

    /*!
     * @function dequeue
     * @abstract Dequeues the next available entry on the queue and copies it into the given data pointer.
     * @discussion This function will dequeue the next available entry on the queue.  If a data pointer is provided, it will copy the data into the memory region if there is enough space available as specified in the dataSize parameter.  If no data pointer is provided, it will simply move the head value past the current entry.
     * @param data A pointer to the data memory region in which to copy the next entry data on the queue.  If this parameter is 0 (NULL), it will simply move to the next entry.
     * @param dataSize A pointer to the size of the data parameter.  On return, this contains the size of the actual entry data - even if the original size was not large enough.
     * @result Returns true on success and false on failure.  Typically failure means that the queue is empty.
     */
    virtual Boolean dequeue(void *data, UInt32 *dataSize);

    /*!
     * @function enqueue
     * @abstract Enqueues a new entry on the queue.
     * @discussion This method adds a new data entry of dataSize to the queue.  It sets the size parameter of the entry pointed to by the tail value and copies the memory pointed to by the data parameter in place in the queue.  Once that is done, it moves the tail to the next available location.  When attempting to add a new entry towards the end of the queue and there isn't enough space at the end, it wraps back to the beginning.<br>  If the queue is empty when a new entry is added, sendDataAvailableNotification() is called to send a message to the user process that data is now available.
     * @param data Pointer to the data to be added to the queue.
     * @param dataSize Size of the data pointed to by data.
     * @result Returns true on success and false on failure.  Typically failure means that the queue is full.
     */
    virtual Boolean enqueue(void *data, UInt32 dataSize) APPLE_KEXT_OVERRIDE;

    OSMetaClassDeclareReservedUnused(IOSharedDataQueue, 0);
    OSMetaClassDeclareReservedUnused(IOSharedDataQueue, 1);
    OSMetaClassDeclareReservedUnused(IOSharedDataQueue, 2);
    OSMetaClassDeclareReservedUnused(IOSharedDataQueue, 3);
    OSMetaClassDeclareReservedUnused(IOSharedDataQueue, 4);
    OSMetaClassDeclareReservedUnused(IOSharedDataQueue, 5);
    OSMetaClassDeclareReservedUnused(IOSharedDataQueue, 6);
    OSMetaClassDeclareReservedUnused(IOSharedDataQueue, 7);
};

#endif /* _IOKIT_IOSHAREDDATAQUEUE_H */
