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
 *  Copyright (c) 1998 Apple Computer, Inc.	 All rights reserved.
 *
 *  HISTORY
 *   1998-7-13	Godfrey van der Linden(gvdl)
 *       Created.
 *   1998-10-30	Godfrey van der Linden(gvdl)
 *       Converted to C++
 */

#ifndef _IOKIT_IOINTERRUPTEVENTSOURCE_H
#define _IOKIT_IOINTERRUPTEVENTSOURCE_H

#include <IOKit/IOEventSource.h>

class IOService;

struct IOInterruptAccountingData;

/*! @class IOInterruptEventSource : public IOEventSource
 *   @abstract Event source for interrupt delivery to work-loop based drivers.
 *   @discussion The IOInterruptEventSource is a generic object that delivers calls interrupt routines in it's client in a guaranteed single-threaded manner.  IOInterruptEventSource is part of the IOKit $link IOWorkLoop infrastructure where the semantic that one and only one action method is executing within a work-loops event chain.
 *  <br><br>
 *  When the action method is called in the client member function will receive 2 arguments, (IOEventSource *) sender and (int) count, See $link IOInterruptEventSource::Action.	Where sender will be reference to the interrupt that occurred and the count will be computed by the difference between the $link producerCount and $link consumerCount.  This number may not be reliable as no attempt is made to adjust for around the world type problems but is provided for general information and statistic gathering.
 *  <br><br>
 *  In general a client will use the factory member function to create and initialise the event source and then add it to their work-loop.  It is the work loop's responsiblity to maintain the new event source in it's event chain.  See $link IOWorkLoop.
 *  <br><br>
 *  An interrupt event source attaches itself to the given provider's interrupt source at initialisation time.  At this time it determines if it is connected to a level or edge triggered interrupt.  If the interrupt is an level triggered interrupt the event source automatically disables the interrupt source at primary interrupt time and after it call's the client it automatically reenables the interrupt.  This action is fairly expensive but it is 100% safe and defaults sensibly so that the driver writer does not have to implement type dependant interrupt routines.  So to repeat, the driver writer does not have to be concerned by the actual underlying interrupt mechanism as the event source hides the complexity.
 *  <br><br>
 *  Saying this if the hardware is a multi-device card, for instance a 4 port NIC, where all of the devices are sharing one level triggered interrupt AND it is possible to determine each port's interrupt state non-destructively then the $link IOFilterInterruptEventSource would be a better choice.
 *  <br><br>
 *  Warning:  All IOInterruptEventSources are created in the disabled state.  If you want to actually schedule interrupt delivery do not forget to enable the source.
 */
class IOInterruptEventSource : public IOEventSource
{
	OSDeclareDefaultStructors(IOInterruptEventSource);

public:
/*! @typedef Action
 *   @discussion 'C' pointer prototype of functions that are called in a single threaded context when an interrupt occurs.
 *   @param owner Pointer to client instance.
 *   @param sender Pointer to generation interrupt event source.
 *   @param count Number of interrupts seen before delivery. */
	typedef void (*Action)(OSObject *owner, IOInterruptEventSource *sender, int count);

#ifdef __BLOCKS__
	typedef void (^ActionBlock)(IOInterruptEventSource *sender, int count);
#endif /* __BLOCKS__ */

/*! @defined IOInterruptEventAction
 *   @discussion Backward compatibilty define for the old non-class scoped type definition.  See $link IOInterruptEventSource::Action */
#define IOInterruptEventAction IOInterruptEventSource::Action

protected:
/*! @var provider IOService that provides interrupts for delivery. */
	IOService *provider;

/*! @var intIndex */
	int intIndex;

/*! @var producerCount
 *   Current count of produced interrupts that have been received. */
	volatile unsigned int producerCount;

/*! @var consumerCount
 *   Current count of produced interrupts that the owner has been informed of. */
	unsigned int consumerCount;

/*! @var autoDisable Do we need to automatically disable the interrupt source when we take an interrupt, i.e. we are level triggered. */
	bool autoDisable;

/*! @var explicitDisable Has the user expicitly disabled this event source, if so then do not overide their request when returning from the callout */
	bool explicitDisable;

/*! @struct ExpansionData
 *   @discussion This structure will be used to expand the capablilties of the IOWorkLoop in the future.
 */
	struct ExpansionData {
		IOInterruptAccountingData * statistics;
	};

/*! @var reserved
 *   Reserved for future use.  (Internal use only)  */
	APPLE_KEXT_WSHADOW_PUSH;
	ExpansionData *reserved;
	APPLE_KEXT_WSHADOW_POP;

/*! @function free
 *   @abstract Sub-class implementation of free method, disconnects from the interrupt source. */
	virtual void free() APPLE_KEXT_OVERRIDE;

/*! @function checkForWork
 *   @abstract Pure Virtual member function used by IOWorkLoop for issueing a client calls.
 *   @discussion This function called when the work-loop is ready to check for any work to do and then to call out the owner/action.
 *   @result Return true if this function needs to be called again before all its outstanding events have been processed. */
	virtual bool checkForWork() APPLE_KEXT_OVERRIDE;

/*! @function setWorkLoop
 *   @abstract Sub-class implementation of setWorkLoop method. */
	virtual void setWorkLoop(IOWorkLoop *inWorkLoop) APPLE_KEXT_OVERRIDE;

public:

/*! @function interruptEventSource
 *   @abstract Factory function for IOInterruptEventSources creation and initialisation.
 *   @param owner Owning client of the new event source.
 *   @param action 'C' Function to call when something happens.
 *   @param provider IOService that represents the interrupt source.  Defaults to 0.  When no provider is defined the event source assumes that the client will in some manner call the interruptOccured method explicitly.  This will start the ball rolling for safe delivery of asynchronous event's into the driver.
 *   @param intIndex The index of the interrupt within the provider's interrupt sources.  Defaults to 0, i.e. the first interrupt in the provider.
 *   @result A new interrupt event source if successfully created and initialised, 0 otherwise.  */
	static IOInterruptEventSource *
	interruptEventSource(OSObject *owner,
	    Action action,
	    IOService *provider = NULL,
	    int intIndex = 0);


#ifdef __BLOCKS__
/*! @function interruptEventSource
 *   @abstract Factory function for IOInterruptEventSources creation and initialisation.
 *   @param owner Owning client of the new event source.
 *   @param provider IOService that represents the interrupt source.  When no provider is defined the event source assumes that the client will in some manner call the interruptOccured method explicitly.  This will start the ball rolling for safe delivery of asynchronous event's into the driver.
 *   @param intIndex The index of the interrupt within the provider's interrupt sources.
 *   @param action Block for the callout routine of this event source..
 *   @result A new interrupt event source if successfully created and initialised, 0 otherwise.  */
	static IOInterruptEventSource *
	interruptEventSource(OSObject *owner,
	    IOService *provider,
	    int intIndex,
	    ActionBlock action);
#endif /* __BLOCKS__ */

#if XNU_KERNEL_PRIVATE
	static void actionToBlock(OSObject *owner, IOInterruptEventSource *sender, int count);
#endif /* XNU_KERNEL_PRIVATE */

/*! @function init
 *   @abstract Primary initialiser for the IOInterruptEventSource class.
 *   @param owner Owning client of the new event source.
 *   @param action 'C' Function to call when something happens.
 *   @param provider IOService that represents the interrupt source.  Defaults to 0.  When no provider is defined the event source assumes that the client will in some manner call the interruptOccured method explicitly.  This will start the ball rolling for safe delivery of asynchronous event's into the driver.
 *   @param intIndex The index of the interrupt within the provider's interrupt sources.  Defaults to 0, i.e. the first interrupt in the provider.
 *   @result true if the inherited classes and this instance initialise
 *  successfully.  */
	virtual bool init(OSObject *owner,
	    Action action,
	    IOService *provider = NULL,
	    int intIndex = 0);

/*! @function enable
 *   @abstract Enable event source.
 *   @discussion A subclass implementation is expected to respect the enabled
 *  state when checkForWork is called.  Calling this function will cause the
 *  work-loop to be signalled so that a checkForWork is performed. */
	virtual void enable() APPLE_KEXT_OVERRIDE;

/*! @function disable
 *   @abstract Disable event source.
 *   @discussion A subclass implementation is expected to respect the enabled
 *  state when checkForWork is called. */
	virtual void disable() APPLE_KEXT_OVERRIDE;

/*! @function getProvider
 *   @abstract Get'ter for $link provider variable.
 *   @result value of provider. */
	virtual const IOService *getProvider() const;

/*! @function getIntIndex
 *   @abstract Get'ter for $link intIndex interrupt index variable.
 *   @result value of intIndex. */
	virtual int getIntIndex() const;

/*! @function getAutoDisable
 *   @abstract Get'ter for $link autoDisable variable.
 *   @result value of autoDisable. */
	virtual bool getAutoDisable() const;

/*! @function interruptOccurred
 *   @abstract Functions that get called by the interrupt controller. See $link IOService::registerInterrupt
 *   @param nub Where did the interrupt originate from
 *   @param ind What is this interrupts index within 'nub'. */
	virtual void interruptOccurred(void *, IOService *nub, int ind);

/*! @function normalInterruptOccurred
 *   @abstract Functions that get called by the interrupt controller.See $link IOService::registerInterrupt
 *   @param nub Where did the interrupt originate from
 *   @param ind What is this interrupts index within 'nub'. */
	virtual void normalInterruptOccurred(void *, IOService *nub, int ind);

/*! @function disableInterruptOccurred
 *   @abstract Functions that get called by the interrupt controller.See $link IOService::registerInterrupt
 *   @param nub Where did the interrupt originate from
 *   @param ind What is this interrupts index within 'nub'. */
	virtual void disableInterruptOccurred(void *, IOService *nub, int ind);

/*! @function warmCPU
 *   @abstract Tries to reduce latency for an interrupt which will be received near a specified time.
 *   @discussion Warms up a CPU in advance of an interrupt so that the interrupt may be serviced with predictable latency.
 *   The warm-up is not periodic; callers should call warmCPU once in advance of each interrupt.  It is recommended that
 *   requests be issues in serial (i.e. each after the target for the previous call has elapsed), as there is a systemwide
 *   cap on the number of outstanding requests.  This routine may be disruptive to the system if used with very small intervals
 *   between requests; it should be used only in cases where interrupt latency is absolutely critical, and tens or hundreds of
 *   milliseconds between targets is the expected time scale.  NOTE: it is not safe to call this method with interrupts disabled.
 *   @param abstime Time at which interrupt is expected. */
	IOReturn warmCPU(uint64_t abstime);

/*! @function enablePrimaryInterruptTimestamp
 *   @abstract Enables collection of mach_absolute_time at primary interrupt.
 *   @discussion Enables collection of mach_absolute_time at primary interrupt.
 *   @param enable True to enable timestamp. */

	void enablePrimaryInterruptTimestamp(bool enable);

/*! @function getPimaryInterruptTimestamp
 *   @abstract Returns mach_absolute_time timestamp of primary interrupt.
 *   @discussion Returns mach_absolute_time timestamp of primary interrupt.
 *   @result Value of the timestamp. Zero if never interrupted, or -1ULL if timestamp collection has not been enabled. */

	uint64_t getPimaryInterruptTimestamp();

private:
	IOReturn registerInterruptHandler(IOService *inProvider, int inIntIndex);
	void unregisterInterruptHandler(IOService *inProvider, int inIntIndex);

private:
	OSMetaClassDeclareReservedUnused(IOInterruptEventSource, 0);
	OSMetaClassDeclareReservedUnused(IOInterruptEventSource, 1);
	OSMetaClassDeclareReservedUnused(IOInterruptEventSource, 2);
	OSMetaClassDeclareReservedUnused(IOInterruptEventSource, 3);
	OSMetaClassDeclareReservedUnused(IOInterruptEventSource, 4);
	OSMetaClassDeclareReservedUnused(IOInterruptEventSource, 5);
	OSMetaClassDeclareReservedUnused(IOInterruptEventSource, 6);
	OSMetaClassDeclareReservedUnused(IOInterruptEventSource, 7);
};

#endif /* !_IOKIT_IOINTERRUPTEVENTSOURCE_H */
