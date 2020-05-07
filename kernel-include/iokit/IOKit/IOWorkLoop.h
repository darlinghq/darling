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

#ifndef __IOKIT_IOWORKLOOP_H
#define __IOKIT_IOWORKLOOP_H

#include <libkern/c++/OSObject.h>
#include <IOKit/IOReturn.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOLocks.h>

#include <IOKit/system.h>

#if IOKITSTATS
#include <IOKit/IOStatisticsPrivate.h>
#endif

class IOEventSource;
class IOTimerEventSource;
class IOCommandGate;

/*! @class IOWorkLoop
 *   @discussion An IOWorkLoop is a thread of control that is intended to be used to provide single threaded access to hardware.  This class has no knowledge of the nature and type of the events that it marshals and forwards.  When a device driver successfully starts (see IOService::start), it is expected to create the event sources it will need to receive events.  Then a work loop is initialized and the events are added to the work loop for monitoring.  In general this set up will be automated by the family superclass of the specific device.
 *  <br><br>
 *       The thread main method walks the event source linked list and messages each one requesting a work check.  At this point each event source is expected to notify its registered owner that the event has occurred.  After each event has been walked and each indicates that another loop isn't required (by setting the 'more' flag to false) the thread will go to sleep on a signaling semaphore.
 *  <br><br>
 *       When an event source is registered with a work loop it is informed of the semaphore to use to wake up the loop.
 */
class IOWorkLoop : public OSObject
{
	OSDeclareDefaultStructors(IOWorkLoop);

public:
/*!
 *   @typedef Action
 *   @discussion Type and arguments of callout C function that is used when
 *  a runCommand is executed by a client.  Cast to this type when you want a C++
 *  member function to be used.  Note the arg1 - arg3 parameters are straight pass
 *  through from the runCommand to the action callout.
 *   @param target
 *       Target of the function, can be used as a refcon.  Note if a C++ function
 *  was specified, this parameter is implicitly the first parameter in the target
 *  member function's parameter list.
 *   @param arg0 Argument to action from run operation.
 *   @param arg1 Argument to action from run operation.
 *   @param arg2 Argument to action from run operation.
 *   @param arg3 Argument to action from run operation.
 */
	typedef IOReturn (*Action)(OSObject *target,
	    void *arg0, void *arg1,
	    void *arg2, void *arg3);

#ifdef __BLOCKS__
	typedef IOReturn (^ActionBlock)();
#endif /* __BLOCKS__ */

	enum {
		kPreciousStack  = 0x00000001,
		kTimeLockPanics = 0x00000002,
	};

private:
/*! @function threadMainContinuation
 *   @abstract Static function that calls the threadMain function.
 */
	static void threadMainContinuation(IOWorkLoop *self);

/*! @function eventSourcePerformsWork
 *       @abstract Checks if the event source passed in overrides checkForWork() to perform any work.
 *  IOWorkLoop uses this to determine if the event source should be polled in runEventSources() or not.
 *       @param inEventSource The event source to check.
 */
	bool eventSourcePerformsWork(IOEventSource *inEventSource);

protected:

/*! @typedef maintCommandEnum
 *   @discussion Enumeration of commands that _maintCommand can deal with.
 *   @constant mAddEvent Used to tag a Remove event source command.
 *   @constant mRemoveEvent Used to tag a Remove event source command.
 */
	typedef enum { mAddEvent, mRemoveEvent } maintCommandEnum;

/*! @var gateLock
 *   Mutual exclusion lock that is used by close and open Gate functions.
 *   This is a recursive lock, which allows multiple layers of code to share a single IOWorkLoop without deadlock.  This is common in IOKit since threads of execution tend to follow the service plane in the IORegistry, and multiple objects along the call path may acquire the gate for the same (shared) workloop.
 */
	IORecursiveLock *gateLock;

/*! @var eventChain
 *   Pointer to first event source in linked list.
 */
	IOEventSource *eventChain;

/*! @var controlG
 *   Internal control gate to maintain event system.
 */
	IOCommandGate *controlG;

/*! @var workToDoLock
 *   The spin lock that is used to guard the 'workToDo' variable.
 */
	IOSimpleLock *workToDoLock;

/*! @var workThread
 *   Work loop thread.
 */
	IOThread workThread;

/*! @var workToDo
 *   Used to to indicate that an interrupt has fired and needs to be processed.
 */
	volatile bool workToDo;

/*! @var loopRestart
 *   Set if an event chain has been changed and the system has to be rechecked from start.  (Internal use only)
 */
	bool loopRestart;

/*! @struct ExpansionData
 *   @discussion This structure will be used to expand the capablilties of the IOWorkLoop in the future.
 */
	struct ExpansionData {
		IOOptionBits options;
		IOEventSource *passiveEventChain;
#if IOKITSTATS
		struct IOWorkLoopCounter *counter;
#else
		void *iokitstatsReserved;
#endif
		uint64_t lockInterval;
		uint64_t lockTime;
	};

/*! @var reserved
 *   Reserved for future use.  (Internal use only)
 */
	ExpansionData *reserved;

/*! @function _maintRequest
 *   @abstract Synchronous implementation of addEventSource and removeEventSource functions.
 *   @discussion This function implements the commands as defined in the maintCommandEnum.  It can be subclassed but it isn't an external API in the usual sense.  A subclass implementation of _maintRequest would be called synchronously with respect to the work loop and it should be implemented in the usual way that an ioctl would be.
 *   @return kIOReturnUnsupported if the command given is not implemented, kIOReturnSuccess otherwise.
 */
	virtual IOReturn _maintRequest(void *command, void *data, void *, void *);

/*! @function free
 *   @discussion Mandatory free of the object independent of the current retain count.  If the work loop is running, this method will not return until the thread has successfully terminated.  Each event source in the chain will be released and the working semaphore will be destroyed.
 *  <br><br>
 *       If the client has some outstanding requests on an event they will never be informed of completion.  If an external thread is blocked on any of the event sources they will be awakened with a KERN_INTERUPTED status.
 */
	virtual void free() APPLE_KEXT_OVERRIDE;

/*! @function threadMain
 *   @discussion Work loop threads main function.  This function consists of 3
 *   loops: the outermost loop is the semaphore clear and wait loop, the middle
 *   loop terminates when there is no more work, and the inside loop walks the
 *   event list calling the checkForWork method in each event source.  If an
 *   event source has more work to do, it can set the more flag and the middle
 *   loop will repeat.  When no more work is outstanding the outermost will
 *   sleep until an event is signalled.
 */
	virtual void threadMain();

public:

/*! @function workLoop
 *   @abstract Factory member function to construct and intialize a work loop.
 *   @result Returns a workLoop instance if constructed successfully, 0 otherwise.
 */
	static IOWorkLoop *workLoop();

/*! @function workLoopWithOptions(IOOptionBits options)
 *   @abstract Factory member function to constuct and intialize a work loop.
 *   @param options Options - kPreciousStack to avoid stack deallocation on paging path.
 *   @result Returns a workLoop instance if constructed successfully, 0 otherwise.
 */
	static IOWorkLoop *workLoopWithOptions(IOOptionBits options);

/*! @function init
 *   @discussion Initializes an instance of the workloop.  This method creates and initializes the signaling semaphore, the controller gate lock, and spawns the thread that will continue executing.
 *   @result Returns true if initialized successfully, false otherwise.
 */
	virtual bool init() APPLE_KEXT_OVERRIDE;

/*! @function getThread
 *   @abstract Gets the workThread.
 *   @result Returns workThread.
 */
	virtual IOThread getThread() const;

/*! @function onThread
 *   @abstract Is the current execution context on the work thread?
 *   @result Returns true if IOThreadSelf() == workThread.
 */
	virtual bool onThread() const;

/*! @function inGate
 *   @abstract Is the current execution context holding the work-loop's gate?
 *   @result Returns true if IOThreadSelf() is gate holder.
 */
	virtual bool inGate() const;

/*! @function addEventSource
 *   @discussion Add an event source to be monitored by the work loop.  This function does not return until the work loop has acknowledged the arrival of the new event source.  When a new event has been added the threadMain will always restart its loop and check all outstanding events.  The event source is retained by the work loop.
 *   @param newEvent Pointer to IOEventSource subclass to add.
 *   @result Always returns kIOReturnSuccess.
 */
	virtual IOReturn addEventSource(IOEventSource *newEvent);

/*! @function removeEventSource
 *   @discussion Remove an event source from the work loop.  This function does not return until the work loop has acknowledged the removal of the event source.  When an event has been removed the threadMain will always restart its loop and check all outstanding events.  The event source will be released before return.
 *   @param toRemove Pointer to IOEventSource subclass to remove.
 *   @result Returns kIOReturnSuccess if successful, kIOReturnBadArgument if toRemove couldn't be found.
 */
	virtual IOReturn removeEventSource(IOEventSource *toRemove);

/*! @function enableAllEventSources
 *   @abstract Calls enable() in all event sources.
 *   @discussion For all event sources in eventChain, call enable() function.  See IOEventSource::enable().
 */
	virtual void enableAllEventSources() const;

/*! @function disableAllEventSources
 *   @abstract Calls disable() in all event sources.
 *   @discussion For all event sources in eventChain, call disable() function.  See IOEventSource::disable().
 */
	virtual void disableAllEventSources() const;

/*! @function enableAllInterrupts
 *   @abstract Calls enable() in all interrupt event sources.
 *   @discussion For all event sources (ES) for which OSDynamicCast(IOInterruptEventSource, ES) is valid, in eventChain call enable() function.  See IOEventSource::enable().
 */
	virtual void enableAllInterrupts() const;

/*! @function disableAllInterrupts
 *   @abstract Calls disable() in all interrupt event sources.
 *   @discussion For all event sources (ES) for which OSDynamicCast(IOInterruptEventSource, ES) is valid,  in eventChain call disable() function.  See IOEventSource::disable().
 */
	virtual void disableAllInterrupts() const;


protected:
// Internal APIs used by event sources to control the thread
	friend class IOEventSource;
	friend class IOTimerEventSource;
	friend class IOCommandGate;
#if IOKITSTATS
	friend class IOStatistics;
#endif
	virtual void signalWorkAvailable();
	virtual void openGate();
	virtual void closeGate();
	virtual bool tryCloseGate();
	virtual int  sleepGate(void *event, UInt32 interuptibleType);
	virtual void wakeupGate(void *event, bool oneThread);

public:
/* methods available in Mac OS X 10.1 or later */

/*! @function runAction
 *   @abstract Single thread a call to an action with the work-loop.
 *   @discussion Client function that causes the given action to be called in a single threaded manner.  Beware: the work-loop's gate is recursive and runAction can cause direct or indirect re-entrancy.  When executing on a client's thread, runAction will sleep until the work-loop's gate opens for execution of client actions, the action is single threaded against all other work-loop event sources.
 *   @param action Pointer to function to be executed in work-loop context.
 *   @param arg0 Parameter for action parameter, defaults to 0.
 *   @param arg1 Parameter for action parameter, defaults to 0.
 *   @param arg2 Parameter for action parameter, defaults to 0.
 *   @param arg3 Parameter for action parameter, defaults to 0.
 *   @result Returns the value of the Action callout.
 */
	virtual IOReturn runAction(Action action, OSObject *target,
	    void *arg0 = NULL, void *arg1 = NULL,
	    void *arg2 = NULL, void *arg3 = NULL);

#ifdef __BLOCKS__
/*! @function runAction
 *   @abstract Single thread a call to an action with the work-loop.
 *   @discussion Client function that causes the given action to be called in a single threaded manner.  Beware: the work-loop's gate is recursive and runAction can cause direct or indirect re-entrancy.  When executing on a client's thread, runAction will sleep until the work-loop's gate opens for execution of client actions, the action is single threaded against all other work-loop event sources.
 *   @param action Block to be executed in work-loop context.
 *   @result Returns the result of the action block.
 */
	IOReturn runActionBlock(ActionBlock action);
#endif /* __BLOCKS__ */

/*! @function runEventSources
 *   @discussion Consists of the inner 2 loops of the threadMain function(qv).
 *   The outer loop terminates when there is no more work, and the inside loop
 *   walks the event list calling the checkForWork method in each event source.
 *   If an event source has more work to do, it can set the more flag and the
 *   outer loop will repeat.
 *  <br><br>
 *   This function can be used to clear a priority inversion between the normal
 *   workloop thread and multimedia's real time threads.  The problem is that
 *   the interrupt action routine is often held off by high priority threads.
 *   So if they want to get their data now they will have to call us and ask if
 *   any data is available.  The multi-media user client will arrange for this
 *   function to be called, which causes any pending interrupts to be processed
 *   and the completion routines called.  By the time the function returns all
 *   outstanding work will have been completed at the real time threads
 *   priority.
 *
 *   @result Return false if the work loop is shutting down, true otherwise.
 */
	virtual bool runEventSources();

/*! @function setMaximumLockTime
 *   @discussion For diagnostics use in DEVELOPMENT kernels, set a time interval which if the work loop lock is held for this time or greater, IOWorkLoop will panic or log a backtrace.
 *   @param interval An absolute time interval, eg. created with clock_interval_to_absolutetime_interval().
 *   @param options Pass IOWorkLoop::kTimeLockPanics to panic when the time is exceeded, otherwise a log will be generated with OSReportWithBacktrace().
 */
	void setMaximumLockTime(uint64_t interval, uint32_t options);

protected:
// Internal APIs used by event sources to control the thread
	virtual int sleepGate(void *event, AbsoluteTime deadline, UInt32 interuptibleType);

#if XNU_KERNEL_PRIVATE
	void lockTime(void);
#endif /* XNU_KERNEL_PRIVATE */

protected:
#if __LP64__
	OSMetaClassDeclareReservedUnused(IOWorkLoop, 0);
	OSMetaClassDeclareReservedUnused(IOWorkLoop, 1);
	OSMetaClassDeclareReservedUnused(IOWorkLoop, 2);
#else
	OSMetaClassDeclareReservedUsed(IOWorkLoop, 0);
	OSMetaClassDeclareReservedUsed(IOWorkLoop, 1);
	OSMetaClassDeclareReservedUsed(IOWorkLoop, 2);
#endif
	OSMetaClassDeclareReservedUnused(IOWorkLoop, 3);
	OSMetaClassDeclareReservedUnused(IOWorkLoop, 4);
	OSMetaClassDeclareReservedUnused(IOWorkLoop, 5);
	OSMetaClassDeclareReservedUnused(IOWorkLoop, 6);
	OSMetaClassDeclareReservedUnused(IOWorkLoop, 7);
};

#endif /* !__IOKIT_IOWORKLOOP_H */
