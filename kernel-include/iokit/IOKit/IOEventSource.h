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
 *  HISTORY
 *   1998-7-13	Godfrey van der Linden(gvdl)
 *       Created.
 *   1998-10-30	Godfrey van der Linden(gvdl)
 *       Converted to C++
 */
#ifndef _IOKIT_IOEVENTSOURCE_H
#define _IOKIT_IOEVENTSOURCE_H

#include <sys/cdefs.h>

#include <libkern/c++/OSObject.h>

#include <IOKit/IOLib.h>
#include <IOKit/system.h>
#include <IOKit/IOWorkLoop.h>

#if IOKITSTATS
#include <IOKit/IOStatisticsPrivate.h>
#endif

__BEGIN_DECLS
#include <mach/clock_types.h>
#include <kern/clock.h>
__END_DECLS

/*!
 *   @class IOEventSource : public OSObject
 *   @abstract Abstract class for all work-loop event sources.
 *   @discussion The IOEventSource declares the abstract super class that all
 *  event sources must inherit from if an IOWorkLoop is to receive events from them.
 *  <br><br>
 *       An event source can represent any event that should cause the work-loop of a
 *  device to wake up and perform work.  Two examples of event sources are the
 *  IOInterruptEventSource which delivers interrupt notifications and IOCommandGate
 *  which delivers command requests.
 *  <br><br>
 *       A kernel module can always use the work-loop model for serialising access to
 *  anything at all.  The IOEventSource is used for communicating events to the
 *  work-loop, and the chain of event sources should be used to walk the possible
 *  event sources and demultipex them.  Note a particular instance of an event
 *  source may only be a member of 1 linked list chain.  If you need to move it
 *  between chains than make sure it is removed from the original chain before
 *  attempting to move it.
 *  <br><br>
 *       The IOEventSource makes no attempt to maintain the consistency of its internal data across multi-threading.  It is assumed that the user of these basic tools will protect the data that these objects represent in some sort of device wide instance lock.	For example the IOWorkLoop maintains the event chain by using an IOCommandGate and thus single threading access to its state.
 *  <br><br>
 *       All subclasses of IOEventSource that wish to perform work on the work-loop thread are expected to implement the checkForWork() member function. As of Mac OS X, 10.7 (Darwin 11), checkForWork is no longer pure virtual, and should not be overridden if there is no work to be done.
 *
 *  <br><br>
 *       checkForWork() is the key method in this class.	 It is called by some work-loop when convienient and is expected to evaluate its internal state and determine if an event has occurred since the last call.  In the case of an event having occurred then the instance defined target(owner)/action will be called.	 The action is stored as an ordinary C function pointer but the first parameter is always the owner.  This means that a C++ member function can be used as an action function though this depends on the ABI.
 *  <br><br>
 *       Although the eventChainNext variable contains a reference to the next event source in the chain this reference is not retained.  The list 'owner' i.e. the client that creates the event, not the work-loop, is expected to retain the source.
 */
class IOEventSource : public OSObject
{
	OSDeclareAbstractStructors(IOEventSource);
	friend class IOWorkLoop;
#if IOKITSTATS
	friend class IOStatistics;
#endif

public:
/*!
 *   @typedef Action
 *   @discussion Placeholder type for C++ function overloading discrimination.
 *  As the all event sources require an action and it has to be stored somewhere
 *  and be of some type, this is that type.
 *   @param owner
 *       Target of the function, can be used as a refcon.  The owner is set
 *  during initialisation.	 Note if a C++ function was specified this parameter
 *  is implicitly the first paramter in the target member function's parameter list.
 */
	typedef void (*Action)(OSObject *owner, ...);

/*! @defined IOEventSourceAction
 *   @discussion Backward compatibilty define for the old non-class scoped type definition.  See $link IOEventSource::Action */
 #define IOEventSourceAction IOEventSource::Action

#ifdef __BLOCKS__
	typedef IOReturn (^ActionBlock)();
#endif /* __BLOCKS__ */

protected:
/*! @var eventChainNext
 *       The next event source in the event chain. nil at end of chain. */
	IOEventSource *eventChainNext;

/*! @var owner The owner object called when an event has been delivered. */
	OSObject *owner;

/*! @var action
 *       The action method called when an event has been delivered */

#if XNU_KERNEL_PRIVATE
	union { Action action; ActionBlock actionBlock; };
#else /* XNU_KERNEL_PRIVATE */
	Action action;
#endif /* !XNU_KERNEL_PRIVATE */

/*! @var enabled
 *       Is this event source enabled to deliver requests to the work-loop. */
	bool enabled;

#if XNU_KERNEL_PRIVATE
	enum{
		kPassive         = 0x0001,
		kActive          = 0x0002,
		kActionBlock     = 0x0004,
		kSubClass0       = 0x0008,
	};
	uint8_t  eventSourceReserved1[1];
	uint16_t flags;
#if __LP64__
	uint8_t eventSourceReserved2[4];
#endif /* __LP64__ */

#endif /* XNU_KERNEL_PRIVATE */

/*! @var workLoop What is the work-loop for this event source. */
	IOWorkLoop *workLoop;

/*! @var refcon What ever the client wants to do, see $link setRefcon. */
	void *refcon;

/*! @struct ExpansionData
 *   @discussion This structure will be used to expand the capablilties of the IOEventSource in the future.
 */
	struct ExpansionData {
#if IOKITSTATS
		struct IOEventSourceCounter *counter;
#else
		void *iokitstatsReserved;
#endif
	};

/*! @var reserved
 *   Reserved for future use.  (Internal use only)  */
	ExpansionData *reserved;

/*! @function init
 *   @abstract Primary initialiser for the IOEventSource class.
 *   @param owner
 *       Owner of this instance of an event source.  Used as the first parameter
 *  of the action callout.	Owner must be an OSObject.
 *   @param action
 *       Pointer to C call out function.	 Action is a pointer to a C function
 *  that gets called when this event source has outstanding work.  It will usually
 *  be called by the checkForWork member function.	The first parameter of the
 *  action call out will always be the owner, this allows C++ member functions to
 *  be used as actions.  Defaults to 0.
 *   @result true if the inherited classes and this instance initialise
 *  successfully.
 */
	virtual bool init(OSObject *owner, IOEventSource::Action action = NULL);

	virtual void free( void ) APPLE_KEXT_OVERRIDE;

/*! @function checkForWork
 *   @abstract Virtual member function used by IOWorkLoop for work
 *  scheduling.
 *   @discussion This function will be called to request a subclass to check
 *  its internal state for any work to do and then to call out the owner/action.
 *  If this event source never performs any work (e.g. IOCommandGate), this
 *  method should not be overridden. NOTE: This method is no longer declared pure
 *  virtual. A default implementation is provided in IOEventSource.
 *   @result Return true if this function needs to be called again before all its outstanding events have been processed.
 */
	virtual bool checkForWork();

/*! @function setWorkLoop
 *   @abstract Set'ter for $link workLoop variable.
 *   @param workLoop
 *       Target work-loop of this event source instance.	 A subclass of
 *  IOWorkLoop that at least reacts to signalWorkAvailable() and onThread functions.
 */
	virtual void setWorkLoop(IOWorkLoop *workLoop);

/*! @function setNext
 *   @abstract Set'ter for $link eventChainNext variable.
 *   @param next
 *       Pointer to another IOEventSource instance.
 */
	virtual void setNext(IOEventSource *next);

/*! @function getNext
 *   @abstract Get'ter for $link eventChainNext variable.
 *   @result value of eventChainNext.
 */
	virtual IOEventSource *getNext() const;


protected:
// Methods to access the IOWorkLoop exported fields
	void signalWorkAvailable();
	void openGate();
	void closeGate();
	bool tryCloseGate();
	int sleepGate(void *event, UInt32 type);
	int sleepGate(void *event, AbsoluteTime deadline, UInt32 type);
	void wakeupGate(void *event, bool oneThread);

public:
/*! @function setAction
 *   @abstract Set'ter for $link action variable.
 *   @param action Pointer to a C function of type IOEventSource::Action. */
	virtual void setAction(IOEventSource::Action action);

/*! @function getAction
 *   @abstract Get'ter for $link action variable.
 *   @result value of action. */
	virtual IOEventSource::Action getAction() const;

#ifdef __BLOCKS__
/*! @function setActionBlock
 *   @abstract Setter for action ivar. The current block is released, & the new block is retained.
 *   @param block Block pointer of type IOEventSource::ActionBlock. */
	void setActionBlock(ActionBlock block);
/*! @function getActionBlock
 *   @abstract Getter for action ivar.
 *   @result Block pointer of type IOEventSource::ActionBlock, if set, or NULL. */
	ActionBlock getActionBlock(ActionBlock) const;
#endif /* __BLOCKS__ */

/*! @function setRefcon
 *   @abstract Setter for refcon ivar. This function will assert if a block action has been set.
 *   @param refcon Refcon. */
	void setRefcon(void *refcon);
/*! @function getRefcon
 *   @abstract Getter for refcon ivar.
 *   @result The refcon. This function will assert if a block action has been set. */
	void * getRefcon() const;

/*! @function enable
 *   @abstract Enable event source.
 *   @discussion A subclass implementation is expected to respect the enabled
 *  state when checkForWork is called.  Calling this function will cause the
 *  work-loop to be signalled so that a checkForWork is performed. */
	virtual void enable();

/*! @function disable
 *   @abstract Disable event source.
 *   @discussion A subclass implementation is expected to respect the enabled
 *  state when checkForWork is called. */
	virtual void disable();

/*! @function isEnabled
 *   @abstract Get'ter for $link enable variable.
 *   @result true if enabled. */
	virtual bool isEnabled() const;

/*! @function getWorkLoop
 *   @abstract Get'ter for $link workLoop variable.
 *   @result value of workLoop. */
	virtual IOWorkLoop *getWorkLoop() const;

/*! @function onThread
 *   @abstract Convenience function for workLoop->onThread.
 *   @result true if called on the work-loop thread.
 */
	virtual bool onThread() const;

private:
	OSMetaClassDeclareReservedUnused(IOEventSource, 0);
	OSMetaClassDeclareReservedUnused(IOEventSource, 1);
	OSMetaClassDeclareReservedUnused(IOEventSource, 2);
	OSMetaClassDeclareReservedUnused(IOEventSource, 3);
	OSMetaClassDeclareReservedUnused(IOEventSource, 4);
	OSMetaClassDeclareReservedUnused(IOEventSource, 5);
	OSMetaClassDeclareReservedUnused(IOEventSource, 6);
	OSMetaClassDeclareReservedUnused(IOEventSource, 7);
};

#endif /* !_IOKIT_IOEVENTSOURCE_H */
