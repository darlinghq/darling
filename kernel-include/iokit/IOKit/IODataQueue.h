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

#ifndef _IOKIT_IODATAQUEUE_H
#define _IOKIT_IODATAQUEUE_H

#ifndef DISABLE_DATAQUEUE_WARNING
#warning "IODataQueue is deprecated due to security issues in its interfaces, please use IOSharedDataQueue instead"
#endif


#include <libkern/c++/OSObject.h>
#include <libkern/OSTypes.h>
#include <mach/port.h>
#include <mach/message.h>

typedef struct _IODataQueueMemory IODataQueueMemory;
class IOMemoryDescriptor;

struct _notifyMsg {
    mach_msg_header_t h;
};

/*!
 * @class IODataQueue : public OSObject
 * @abstract A generic queue designed to pass data from the kernel to a user process.
 * @discussion The IODataQueue class is designed to allow kernel code to queue data to a user process.  IODataQueue objects are designed to be used in a single producer / single consumer situation.  As such, there are no locks on the data itself.  Because the kernel enqueue and user-space dequeue methods follow a strict set of guidelines, no locks are necessary to maintain the integrity of the data struct.
 *
 * <br>Each data entry can be variable sized, but the entire size of the queue data region (including overhead for each entry) must be specified up front.
 *
 * <br>In order for the IODataQueue instance to notify the user process that data is available, a notification mach port must be set.  When the queue is empty and a new entry is added, a message is sent to the specified port.
 *
 * <br>User client code exists in the IOKit framework that facilitates the creation of the receive notification port as well as the listen process for new data available notifications.
 *
 * <br>In order to make the data queue memory available to a user process, the method getMemoryDescriptor() must be used to get an IOMemoryDescriptor instance that can be mapped into a user process.  Typically, the clientMemoryForType() method on an IOUserClient instance will be used to request the IOMemoryDescriptor and then return it to be mapped into the user process.
 */
#ifndef DISABLE_DATAQUEUE_WARNING 
class __attribute__((deprecated)) IODataQueue : public OSObject
#else 
class IODataQueue : public OSObject
#endif
{
    OSDeclareDefaultStructors(IODataQueue)

protected:
    IODataQueueMemory *	dataQueue;

    void *		notifyMsg;

    virtual void free() APPLE_KEXT_OVERRIDE;

    /*!
     * @function sendDataAvailableNotification
     * @abstract Sends a dataAvailableNotification message to the specified mach port.
     * @discussion This method sends a message to the mach port passed to setNotificationPort().  It is used to indicate that data is available in the queue.
     */
    virtual void sendDataAvailableNotification();

public:
    /*!
     * @function withCapacity
     * @abstract Static method that creates a new IODataQueue instance with the capacity specified in the size parameter.
     * @discussion The actual size of the entire data queue memory region (to be shared into a user process) is equal to the capacity plus the IODataQueueMemory overhead.  This overhead value can be determined from the DATA_QUEUE_MEMORY_HEADER_SIZE macro in <IOKit/IODataQueueShared.h>.  The size of the data queue memory region must include space for the overhead of each IODataQueueEntry.  This entry overhead can be determined from the DATA_QUEUE_ENTRY_HEADER_SIZE macro in <IOKit/IODataQueueShared.h>.<br>  This method allocates a new IODataQueue instance and then calls initWithCapacity() with the given size parameter.  If the initWithCapacity() fails, the new instance is released and zero is returned.
     * @param size The size of the data queue memory region.
     * @result Returns the newly allocated IODataQueue instance.  Zero is returned on failure.
     */
    static IODataQueue *withCapacity(UInt32 size);

    /*!
     * @function withEntries
     * @abstract Static method that creates a new IODataQueue instance with the specified number of entries of the given size.
     * @discussion This method will create a new IODataQueue instance with enough capacity for numEntries of entrySize.  It does account for the IODataQueueEntry overhead for each entry.  Note that the numEntries and entrySize are simply used to determine the data region size.  They do not actually restrict the size of number of entries that can be added to the queue.<br>  This method allocates a new IODataQueue instance and then calls initWithEntries() with the given numEntries and entrySize parameters.  If the initWithEntries() fails, the new instance is released and zero is returned.
     * @param numEntries Number of entries to allocate space for.
     * @param entrySize Size of each entry.
     * @result Reeturns the newly allocated IODataQueue instance.  Zero is returned on failure.
     */
    static IODataQueue *withEntries(UInt32 numEntries, UInt32 entrySize);

    /*!
     * @function initWithCapacity
     * @abstract Initializes an IODataQueue instance with the capacity specified in the size parameter.
     * @discussion The actual size of the entire data queue memory region (to be shared into a user process) is equal to the capacity plus the IODataQueueMemory overhead.  This overhead value can be determined from the DATA_QUEUE_MEMORY_HEADER_SIZE macro in <IOKit/IODataQueueShared.h>.  The size of the data queue memory region must include space for the overhead of each IODataQueueEntry.  This entry overhead can be determined from the DATA_QUEUE_ENTRY_HEADER_SIZE macro in <IOKit/IODataQueueShared.h>.
     * @param size The size of the data queue memory region.
     * @result Returns true on success and false on failure.
     */
    virtual Boolean initWithCapacity(UInt32 size);

    /*!
     * @function initWithEntries
     * @abstract Initializes an IODataQueue instance with the specified number of entries of the given size.
     * @discussion This method will initialize an IODataQueue instance with enough capacity for numEntries of entrySize.  It does account for the IODataQueueEntry overhead for each entry.  Note that the numEntries and entrySize are simply used to determine the data region size.  They do not actually restrict the size of number of entries that can be added to the queue.<br>  This method allocates a new IODataQueue instance and then calls initWithEntries() with the given numEntries and entrySize parameters.
     * @param numEntries Number of entries to allocate space for.
     * @param entrySize Size of each entry.
     * @result Reeturns true on success and false on failure.
     */
    virtual Boolean initWithEntries(UInt32 numEntries, UInt32 entrySize);

    /*!
     * @function enqueue
     * @abstract Enqueues a new entry on the queue.
     * @discussion This method adds a new data entry of dataSize to the queue.  It sets the size parameter of the entry pointed to by the tail value and copies the memory pointed to by the data parameter in place in the queue.  Once that is done, it moves the tail to the next available location.  When attempting to add a new entry towards the end of the queue and there isn't enough space at the end, it wraps back to the beginning.<br>  If the queue is empty when a new entry is added, sendDataAvailableNotification() is called to send a message to the user process that data is now available.
     * @param data Pointer to the data to be added to the queue.
     * @param dataSize Size of the data pointed to by data.
     * @result Returns true on success and false on failure.  Typically failure means that the queue is full.
     */
    virtual Boolean enqueue(void *data, UInt32 dataSize);

    /*!
     * @function setNotificationPort
     * @abstract Creates a simple mach message targeting the mach port specified in port.
     * @discussion This message is sent when data is added to an empty queue.  It is to notify a user process that new data has become available.
     * @param port The mach port to target with the notification message.
     */
    virtual void setNotificationPort(mach_port_t port);

    /*!
     * @function getMemoryDescriptor
     * @abstract Returns a memory descriptor covering the IODataQueueMemory region.
     * @discussion The IOMemoryDescriptor instance returned by this method is intended to be mapped into a user process.  This is the memory region that the IODataQueueClient code operates on.
     * @result Returns a newly allocated IOMemoryDescriptor for the IODataQueueMemory region.  Returns zero on failure.
     */
    virtual IOMemoryDescriptor *getMemoryDescriptor();
};

#endif /* _IOKIT_IODATAQUEUE_H */
