/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 *
 *	Copyright (c) 2000 Apple Computer, Inc.  All rights reserved.
 *
 *	HISTORY
 *
 *	2001-01-17	gvdl	Re-implement on IOCommandGate::commandSleep
 *	11/13/2000	CJS	Created IOCommandPool class and implementation
 *
 */

/*!
 * @header IOCommandPool
 * @abstract
 * This header contains the IOCommandPool class definition.
 */

#ifndef _IOKIT_IO_COMMAND_POOL_H_
#define _IOKIT_IO_COMMAND_POOL_H_

/*
 * Kernel
 */

#if defined(KERNEL) && defined(__cplusplus)

#include <kern/kern_types.h>
#include <kern/queue.h>
#include <IOKit/IOCommand.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOService.h>
#include <IOKit/IOWorkLoop.h>

/*!
 * @class IOCommandPool
 * @abstract Manipulates a pool of commands which inherit from IOCommand.
 * @discussion
 * The IOCommandPool class is used to manipulate a pool of commands which
 * inherit from IOCommand. It includes a factory method to create a pool
 * of a certain size. Once the factory method is invoked, the semaphore
 * is set to zero. The caller must then put commands in the pool by creating
 * the command (via the controller's factory method or a memory allocation)
 * and calling the returnCommand method with the newly created command as its
 * argument.
 */

class IOCommandPool : public OSObject
{
	
    OSDeclareDefaultStructors(IOCommandPool)
    
    
protected:

    queue_head_t fQueueHead;	/* head of the queue of elements available */
    UInt32 fSleepers;		/* Count of threads sleeping on this pool */
    IOCommandGate *fSerializer;	/* command gate used for serializing pool access */

/*! @struct ExpansionData
    @discussion This structure will be used to expand the capablilties of the IOEventSource in the future.
    */    
    struct ExpansionData { };

/*! @var reserved
    Reserved for future use.  (Internal use only)  */
    ExpansionData *reserved;

    /*!
     * @const kIOCommandPoolDefaultSize
     * @abstract The default size of any command pool.
     * @discussion
     * kIOCommandPoolDefaultSize is the default size of any command pool.
     * The default size was determined to be the smallest size for which
     * a pool makes sense.
     */

    static const UInt32 kIOCommandPoolDefaultSize = 2;
    
    /*
     * Free all of this object's outstanding resources.
     */

    virtual void free(void) APPLE_KEXT_OVERRIDE;
    
    
public:

    /*!
     * @function initWithWorkLoop
     * @abstract Primary initializer for an IOCommandPool object.
     * @discussion Primary initializer for an IOCommandPool.
     * Should probably use IOCommandPool::withWorkLoop() as it is easier to use.
     * @param inWorkLoop
     * The workloop that this command pool should synchronize with.
     * @result Returns true if command pool was successfully initialized.
     */
    virtual bool initWithWorkLoop(IOWorkLoop *workLoop);

    /*!
     * @function withWorkLoop
     * @abstract Primary factory method for the IOCommandPool class
     * @discussion
     * The withWorkLoop method is what is known as a factory method. It creates
     * a new instance of an IOCommandPool and returns a pointer to that object.
     * @param inWorkLoop
     * The workloop that this command pool should synchronize with.
     * @result
     * Returns a pointer to an instance of IOCommandPool if successful,
     * otherwise NULL.
     */

    static IOCommandPool *withWorkLoop(IOWorkLoop *inWorkLoop);

    /*!
     * @function init
     * @abstract Should never be used, obsolete. See initWithWorkLoop.
     */
    virtual bool init(IOService *inOwner,
                      IOWorkLoop *inWorkLoop,
                      UInt32 inSize = kIOCommandPoolDefaultSize);

    /*!
     * @function withWorkLoop
     * @abstract Should never be used, obsolete. See IOCommandPool::withWorkLoop.
     */
    static IOCommandPool *commandPool(IOService *inOwner,
                                        IOWorkLoop *inWorkLoop,
                                        UInt32 inSize = kIOCommandPoolDefaultSize);
    
    
    /*!
     * @function getCommand
     * @discussion The getCommand method is used to get a pointer to an object of type IOCommand from the pool.
     * @param blockForCommand
     * If the caller would like to have its thread slept until a command is
     * available, it should pass true, else false.
     * @result
     * If the caller passes true in blockForCommand, getCommand guarantees that
     * the result will be a pointer to an IOCommand object from the pool. If
     * the caller passes false, s/he is responsible for checking whether a non-NULL
     * pointer was returned.
     */

    virtual IOCommand *getCommand(bool blockForCommand = true);
    
    /*!
     * @function returnCommand
     * @discussion
     * The returnCommand method is used to place an object of type IOCommand
     * into the pool, whether it be the first time, or the 1000th time.
     * @param commmand
     * The command to place in the pool.
     */
    
    virtual void returnCommand(IOCommand *command);
    
protected:
    
    /*!
     * @function gatedGetCommand
     * @discussion
     * The gatedGetCommand method is used to serialize the extraction of a 
     * command from the pool behind a command gate, runAction-ed by getCommand.
     * @param vCommand
     * A pointer to a pointer to an IOCommand object where the returned
     * command will be stored.
     * @param vBlock
     * A bool that indicates whether to block the request until a command
     * becomes available.
     * @result
     * Returns kIOReturnNoResources if no command is available and the client
     * doesn't wish to block until one does become available.
     * kIOReturnSuccess if the vCommand argument is valid.
     */
    virtual IOReturn gatedGetCommand(IOCommand **command, bool blockForCommand);
    
    /*!
     * @function gatedReturnCommand
     * @discussion
     * The gatedReturnCommand method is used to serialize the return of a 
     * command to the pool behind a command gate, runAction-ed by returnCommand.
     * @param vCommand
     * A pointer to the IOCommand object to be returned to the pool.
     * @result
     * Always returns kIOReturnSuccess if the vCommand argument is valid.
     */
    virtual IOReturn gatedReturnCommand(IOCommand *command);

private:
    OSMetaClassDeclareReservedUnused(IOCommandPool, 0);
    OSMetaClassDeclareReservedUnused(IOCommandPool, 1);
    OSMetaClassDeclareReservedUnused(IOCommandPool, 2);
    OSMetaClassDeclareReservedUnused(IOCommandPool, 3);
    OSMetaClassDeclareReservedUnused(IOCommandPool, 4);
    OSMetaClassDeclareReservedUnused(IOCommandPool, 5);
    OSMetaClassDeclareReservedUnused(IOCommandPool, 6);
    OSMetaClassDeclareReservedUnused(IOCommandPool, 7);
};

#endif	/* defined(KERNEL) && defined(__cplusplus) */

#endif	/* _IOKIT_IO_COMMAND_POOL_H_ */
