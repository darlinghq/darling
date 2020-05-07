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
/*[
 *   1999-8-10	Godfrey van der Linden(gvdl)
 *       Created.
 *  ]*/
/*! @language embedded-c++ */

#ifndef _IOKIT_IOCOMMANDGATE_H
#define _IOKIT_IOCOMMANDGATE_H

#include <IOKit/IOEventSource.h>

/*!
 *   @class IOCommandGate : public IOEventSource
 *   @abstract Single-threaded work loop client request mechanism.
 *   @discussion An IOCommandGate instance is an extremely lightweight mechanism
 *  that executes an action on the driver's work loop.  Although the code does not
 *  technically execute on the work loop itself, a single-threaded work loop semantic
 *  is maintained for this event source using the work loop gate.  The command gate
 *  tests for a potential self dead lock by checking if the runCommand request is
 *  made from the work loop's thread, it doesn't check for a mutual dead lock though
 *  where a pair of work loop's dead lock each other.
 *  <br><br>
 *       The IOCommandGate is a lighter weight version of the IOCommandQueue and
 *  should be used in preference.  Generally use a command queue whenever you need a
 *  client to submit a request to a work loop.  A typical command gate action would
 *  check if the hardware is active, if so it will add the request to a pending
 *  queue internal to the device or the device's family.  Otherwise if the hardware
 *  is inactive then this request can be acted upon immediately.
 *  <br><br>
 *   CAUTION: The runAction, runCommand, and attemptCommand functions cannot be called from an interrupt context.
 *
 */
class IOCommandGate : public IOEventSource
{
	OSDeclareDefaultStructors(IOCommandGate);

public:
/*!
 *   @typedef Action
 *   @discussion Type and arguments of callout C function that is used when
 *  a runCommand is executed by a client.  Cast to this type when you want a C++
 *  member function to be used.  Note the arg1 - arg3 parameters are straight pass
 *  through from the runCommand to the action callout.
 *   @param owner
 *       Target of the function, can be used as a refcon.  The owner is set
 *  during initialisation of the IOCommandGate instance.	 Note if a C++ function
 *  was specified this parameter is implicitly the first paramter in the target
 *  member function's parameter list.
 *   @param arg0 Argument to action from run operation.
 *   @param arg1 Argument to action from run operation.
 *   @param arg2 Argument to action from run operation.
 *   @param arg3 Argument to action from run operation.
 */
	typedef IOReturn (*Action)(OSObject *owner,
	    void *arg0, void *arg1,
	    void *arg2, void *arg3);

protected:

/*! @struct ExpansionData
 *   @discussion This structure will be used to expand the capablilties of the IOWorkLoop in the future.
 */
	struct ExpansionData { };

/*! @var reserved
 *   Reserved for future use.  (Internal use only)  */
	APPLE_KEXT_WSHADOW_PUSH;
	ExpansionData *reserved;
	APPLE_KEXT_WSHADOW_POP;

public:
/*! @function commandGate
 *   @abstract Factory method to create and initialise an IOCommandGate, See $link init.
 *   @result Returns a pointer to the new command gate if sucessful, 0 otherwise. */
	static IOCommandGate *commandGate(OSObject *owner, Action action = NULL);

/*! @function init
 *   @abstract Class initialiser.
 *   @discussion Initialiser for IOCommandGate operates only on newly 'newed'
 *  objects.  Shouldn't be used to re-init an existing instance.
 *   @param owner       Owner of this, newly created, instance of the IOCommandGate.  This argument will be used as the first parameter in the action callout.
 *   @param action
 *       Pointer to a C function that is called whenever a client of the
 *  IOCommandGate calls runCommand.	 NB Can be a C++ member function but caller
 *  must cast the member function to $link IOCommandGate::Action and they will get a
 *  compiler warning.  Defaults to zero, see $link IOEventSource::setAction.
 *   @result True if inherited classes initialise successfully. */
	virtual bool init(OSObject *owner, Action action = NULL);

// Superclass overrides
	virtual void free() APPLE_KEXT_OVERRIDE;
	virtual void setWorkLoop(IOWorkLoop *inWorkLoop) APPLE_KEXT_OVERRIDE;

/*! @function runCommand
 *   @abstract Single thread a command with the target work loop.
 *   @discussion Client function that causes the current action to be called in
 *  a single threaded manner.  Beware the work loop's gate is recursive and command
 *  gates can cause direct or indirect re-entrancy.	 When the executing on a
 *  client's thread runCommand will sleep until the work loop's gate opens for
 *  execution of client actions, the action is single threaded against all other
 *  work loop event sources.  If the command is disabled the attempt to run a command will be stalled until enable is called.
 *   @param arg0 Parameter for action of command gate, defaults to 0.
 *   @param arg1 Parameter for action of command gate, defaults to 0.
 *   @param arg2 Parameter for action of command gate, defaults to 0.
 *   @param arg3 Parameter for action of command gate, defaults to 0.
 *   @result kIOReturnSuccess if successful. kIOReturnAborted if a disabled command gate is free()ed before being reenabled, kIOReturnNoResources if no action available.
 */
	virtual IOReturn runCommand(void *arg0 = NULL, void *arg1 = NULL,
	    void *arg2 = NULL, void *arg3 = NULL);

/*! @function runAction
 *   @abstract Single thread a call to an action with the target work loop.
 *   @discussion Client function that causes the given action to be called in
 *  a single threaded manner.  Beware the work loop's gate is recursive and command
 *  gates can cause direct or indirect re-entrancy.	 When the executing on a
 *  client's thread runAction will sleep until the work loop's gate opens for
 *  execution of client actions, the action is single threaded against all other
 *  work loop event sources.  If the command is disabled the attempt to run a command will be stalled until enable is called.
 *   @param action Pointer to function to be executed in the context of the work loop.
 *   @param arg0 Parameter for action parameter, defaults to 0.
 *   @param arg1 Parameter for action parameter, defaults to 0.
 *   @param arg2 Parameter for action parameter, defaults to 0.
 *   @param arg3 Parameter for action parameter, defaults to 0.
 *   @result The return value of action if it was called, kIOReturnBadArgument if action is not defined, kIOReturnAborted if a disabled command gate is free()ed before being reenabled.
 */
	virtual IOReturn runAction(Action action,
	    void *arg0 = NULL, void *arg1 = NULL,
	    void *arg2 = NULL, void *arg3 = NULL);

#ifdef __BLOCKS__
/*! @function runActionBlock
 *   @abstract Single thread a call to an action with the target work loop.
 *   @discussion Client function that causes the given action to be called in
 *  a single threaded manner.  Beware the work loop's gate is recursive and command
 *  gates can cause direct or indirect re-entrancy.	 When the executing on a
 *  client's thread runAction will sleep until the work loop's gate opens for
 *  execution of client actions, the action is single threaded against all other
 *  work loop event sources.  If the command is disabled the attempt to run a command will be stalled until enable is called.
 *   @param action Block to be executed in the context of the work loop.
 *   @result The return value of action if it was called, kIOReturnBadArgument if action is not defined, kIOReturnAborted if a disabled command gate is free()ed before being reenabled.
 */
	IOReturn runActionBlock(ActionBlock action);
#endif /* __BLOCKS__ */

/*! @function attemptCommand
 *   @abstract Single thread a command with the target work loop.
 *   @discussion Client function that causes the current action to be called in
 *  a single threaded manner.  When the executing on a client's thread attemptCommand will fail if the work loop's gate is closed.
 *   @param arg0 Parameter for action of command gate, defaults to 0.
 *   @param arg1 Parameter for action of command gate, defaults to 0.
 *   @param arg2 Parameter for action of command gate, defaults to 0.
 *   @param arg3 Parameter for action of command gate, defaults to 0.
 *   @result kIOReturnSuccess if successful. kIOReturnNotPermitted if this event source is currently disabled, kIOReturnNoResources if no action available, kIOReturnCannotLock if lock attempt fails.
 */
	virtual IOReturn attemptCommand(void *arg0 = NULL, void *arg1 = NULL,
	    void *arg2 = NULL, void *arg3 = NULL);

/*! @function attemptAction
 *   @abstract Single thread a call to an action with the target work loop.
 *   @discussion Client function that causes the given action to be called in
 *  a single threaded manner.  Beware the work loop's gate is recursive and command
 *  gates can cause direct or indirect re-entrancy.	 When the executing on a
 *  client's thread attemptCommand will fail if the work loop's gate is closed.
 *   @param action Pointer to function to be executed in context of the work loop.
 *   @param arg0 Parameter for action parameter, defaults to 0.
 *   @param arg1 Parameter for action parameter, defaults to 0.
 *   @param arg2 Parameter for action parameter, defaults to 0.
 *   @param arg3 Parameter for action parameter, defaults to 0.
 *   @result kIOReturnSuccess if successful. kIOReturnBadArgument if action is not defined, kIOReturnNotPermitted if this event source is currently disabled, kIOReturnCannotLock if lock attempt fails.
 *
 */
	virtual IOReturn attemptAction(Action action,
	    void *arg0 = NULL, void *arg1 = NULL,
	    void *arg2 = NULL, void *arg3 = NULL);

/*! @function commandSleep
 *   @abstract Put a thread that is currently holding the command gate to sleep.
 *   @discussion Put a thread to sleep waiting for an event but release the gate first.  If the event occurs then the commandGate is closed before the function returns. If the thread does not hold the gate, panic.
 *   @param event Pointer to an address.
 *   @param interruptible THREAD_UNINT, THREAD_INTERRUPTIBLE or THREAD_ABORTSAFE.  THREAD_UNINT specifies that the sleep cannot be interrupted by a signal.  THREAD_INTERRUPTIBLE specifies that the sleep may be interrupted by a "kill -9" signal.  THREAD_ABORTSAFE (the default value) specifies that the sleep may be interrupted by any user signal.
 *   @result THREAD_AWAKENED - normal wakeup, THREAD_TIMED_OUT - timeout expired, THREAD_INTERRUPTED - interrupted, THREAD_RESTART - restart operation entirely. */
	virtual IOReturn commandSleep(void *event,
	    UInt32 interruptible = THREAD_ABORTSAFE);

/*! @function commandWakeup
 *   @abstract Wakeup one or more threads that are asleep on an event.
 *   @param event Pointer to an address.
 *   @param oneThread true to only wake up at most one thread, false otherwise. */
	virtual void commandWakeup(void *event, bool oneThread = false);

/*! @function disable
 *   @abstract Disable the command gate
 *   @discussion When a command gate is disabled all future calls to runAction and runCommand will stall until the gate is enable()d later.  This can be used to block client threads when a system sleep is requested.  The IOWorkLoop thread itself will never stall, even when making runAction/runCommand calls.  This call must be made from a gated context, to clear potential race conditions.  */
	virtual void disable() APPLE_KEXT_OVERRIDE;

/*! @function enable
 *   @abstract Enable command gate, this will unblock any blocked Commands and Actions.
 *   @discussion  Enable the command gate.  The attemptAction/attemptCommand calls will now be enabled and can succeeed.  Stalled runCommand/runAction calls will be woken up. */
	virtual void enable() APPLE_KEXT_OVERRIDE;

/*! @function commandSleep
 *   @abstract Put a thread that is currently holding the command gate to sleep.
 *   @discussion Put a thread to sleep waiting for an event but release the gate first.  If the event occurs or timeout occurs then the commandGate is closed before the function returns.  If the thread does not hold the gate, panic.
 *   @param event Pointer to an address.
 *       @param deadline Clock deadline to timeout the sleep.
 *   @param interruptible THREAD_UNINT, THREAD_INTERRUPTIBLE or THREAD_ABORTSAFE.  THREAD_UNINT specifies that the sleep cannot be interrupted by a signal.  THREAD_INTERRUPTIBLE specifies that the sleep may be interrupted by a "kill -9" signal.  THREAD_ABORTSAFE specifies that the sleep may be interrupted by any user signal.
 *   @result THREAD_AWAKENED - normal wakeup, THREAD_TIMED_OUT - timeout expired, THREAD_INTERRUPTED - interrupted, THREAD_RESTART - restart operation entirely. */
	virtual IOReturn commandSleep(void *event,
	    AbsoluteTime deadline,
	    UInt32 interruptible);

private:
#if __LP64__
	OSMetaClassDeclareReservedUnused(IOCommandGate, 0);
#else
	OSMetaClassDeclareReservedUsed(IOCommandGate, 0);
#endif
	OSMetaClassDeclareReservedUnused(IOCommandGate, 1);
	OSMetaClassDeclareReservedUnused(IOCommandGate, 2);
	OSMetaClassDeclareReservedUnused(IOCommandGate, 3);
	OSMetaClassDeclareReservedUnused(IOCommandGate, 4);
	OSMetaClassDeclareReservedUnused(IOCommandGate, 5);
	OSMetaClassDeclareReservedUnused(IOCommandGate, 6);
	OSMetaClassDeclareReservedUnused(IOCommandGate, 7);
};

#endif /* !_IOKIT_IOCOMMANDGATE_H */
