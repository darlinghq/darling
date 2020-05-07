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
 * Copyright (c) 1999 Apple Computer, Inc.  All rights reserved.
 *
 * IOTimerEventSource.h
 *
 * HISTORY
 * 2-Feb-1999		Joe Liu (jliu) created.
 *
 */

#ifndef _IOTIMEREVENTSOURCE
#define _IOTIMEREVENTSOURCE

#include <sys/cdefs.h>

__BEGIN_DECLS
#include <kern/clock.h>
__END_DECLS

#include <IOKit/IOEventSource.h>
#include <IOKit/IOTypes.h>

/*!
 *       @enum IOTimerEventSource constructor options
 *       @abstract Constants defining behavior of the IOTimerEventSource.
 *       @constant kIOTimerEventSourceOptionsPriorityHigh Importance above everything but realtime.
 *       Thread calls allocated with this priority execute at extremely high priority,
 *       above everything but realtime threads.  They are generally executed in serial.
 *       Though they may execute concurrently under some circumstances, no fan-out is implied.
 *       These work items should do very small amounts of work or risk disrupting system
 *       responsiveness.
 *       @constant kIOTimerEventSourceOptionsPriorityKernelHigh Importance higher than most kernel
 *       threads.
 *       @constant kIOTimerEventSourceOptionsPriorityKernel Importance similar to that of normal kernel
 *       threads.
 *       @constant kIOTimerEventSourceOptionsPriorityUser Importance similar to that of normal user threads.
 *       @constant kIOTimerEventSourceOptionsPriorityLow Very low importance.
 *       @constant kIOTimerEventSourceOptionsPriorityWorkLoop Run the callout on the thread of the IOWorkLoop
 *       the event source has been added to.
 *       @constant kIOTimerEventSourceOptionsAllowReenter Allow the callout to be rescheduled and potentially
 *       re-entered, if the IOWorkLoop lock has been released (eg. with commandSleep) during its invocation.
 *       @constant kIOTimerEventSourceOptionsDefault Recommended default options.
 */
enum{
	kIOTimerEventSourceOptionsPriorityMask       = 0x000000ff,
	kIOTimerEventSourceOptionsPriorityHigh       = 0x00000000,
	kIOTimerEventSourceOptionsPriorityKernelHigh = 0x00000001,
	kIOTimerEventSourceOptionsPriorityKernel     = 0x00000002,
	kIOTimerEventSourceOptionsPriorityUser       = 0x00000003,
	kIOTimerEventSourceOptionsPriorityLow        = 0x00000004,
	kIOTimerEventSourceOptionsPriorityWorkLoop   = 0x000000ff,

	kIOTimerEventSourceOptionsAllowReenter       = 0x00000100,

	kIOTimerEventSourceOptionsDefault            = kIOTimerEventSourceOptionsPriorityKernelHigh
};

#define IOTIMEREVENTSOURCEOPTIONS_DEFINED       1

/*!
 *       @enum IOTimerEventSource setTimeout/wakeAtTime options
 *       @abstract Constants defining behavior of a scheduled call from IOTimerEventSource.
 *       @constant kIOTimeOptionsWithLeeway Use the leeway parameter to the call.
 *       @constant kIOTimeOptionsContinuous Use mach_continuous_time() to generate the callback.
 */
enum{
	kIOTimeOptionsWithLeeway = 0x00000020,
	kIOTimeOptionsContinuous = 0x00000100,
};

/*!
 *   @class IOTimerEventSource : public IOEventSource
 *   @abstract Time based event source mechanism.
 *   @discussion An event source that implements a simple timer.	 A timeout handler is called once the timeout period expires.  This timeout handler will be called by the work-loop that this event source is attached to.
 *  <br><br>
 *       Usually a timer event source will be used to implement a timeout.  In general when a driver makes a request it will need to setup a call to keep track of when the I/O doesn't complete.  This class is designed to make that somewhat easier.
 *  <br><br>
 *       Remember the system doesn't guarantee the accuracy of the callout.	It is possible that a higher priority thread is running which will delay the execution of the action routine.  In fact the thread will be made runable at the exact requested time, within the accuracy of the CPU's decrementer based interrupt, but the scheduler will then control execution.
 */

class IOTimerEventSource : public IOEventSource
{
	OSDeclareDefaultStructors(IOTimerEventSource);

protected:
/*! @var calloutEntry thread_call entry for preregistered thread callouts */
	void *calloutEntry;

/*! @var abstime time to wake up next, see enable. */
	AbsoluteTime abstime;

/*! @struct ExpansionData
 *   @discussion This structure is private to the IOTimerEventSource implementation.
 */
	struct ExpansionData {
		SInt32       calloutGeneration;
		SInt32       calloutGenerationSignaled;
		IOWorkLoop * workLoop;
	};

/*! @var reserved
 *   Reserved for future use.  (Internal use only)  */
	APPLE_KEXT_WSHADOW_PUSH;
	ExpansionData *reserved;
	APPLE_KEXT_WSHADOW_POP;

/*! @function timeout
 *   @abstract Function that routes the call from the OS' timeout mechanism into a work-loop context.
 *   @discussion timeout will normally not be called nor overridden by a subclass.  If the event source is enabled then close the work-loop's gate and call the action routine.
 *   @param self This argument will be cast to an IOTimerEventSource. */
	static void timeout(void *self);

/*! @function setTimeoutFunc
 *   @abstract Set's timeout as the function of calloutEntry.
 *   @discussion IOTimerEventSource is based upon the kern/thread_call.h APIs currently.	 This function allocates the calloutEntry member variable by using thread_call_allocate(timeout, this).	 If you need to write your own subclass of IOTimerEventSource you probably should override this method to allocate an entry that points to your own timeout routine. */
	virtual void setTimeoutFunc();

/*! @function free
 *   @abstract Sub-class implementation of free method, frees calloutEntry */
	virtual void free() APPLE_KEXT_OVERRIDE;

	virtual void setWorkLoop(IOWorkLoop *workLoop) APPLE_KEXT_OVERRIDE;

public:

/*! @typedef Action
 *   @discussion 'C' Function pointer defining the callout routine of this event source.
 *   @param owner Owning target object.	Note by a startling coincidence the first parameter in a C callout is currently used to define the target of a C++ member function.
 *   @param sender The object that timed out. */
	typedef void (*Action)(OSObject *owner, IOTimerEventSource *sender);

#ifdef __BLOCKS__
	typedef void (^ActionBlock)(IOTimerEventSource *sender);
#endif /* __BLOCKS__ */

	static IOTimerEventSource *
	timerEventSource(OSObject *owner, Action action = NULL);

/*! @function timerEventSource
 *   @abstract Allocates and returns an initialized timer instance.
 *   @param options Mask of kIOTimerEventSourceOptions* options.
 *   @param owner The object that that will be passed to the Action callback.
 *   @param action 'C' Function pointer for the callout routine of this event source.
 */
	static IOTimerEventSource *
	timerEventSource(uint32_t options, OSObject *owner, Action action = NULL);

#ifdef __BLOCKS__
/*! @function timerEventSource
 *   @abstract Allocates and returns an initialized timer instance.
 *   @param options Mask of kIOTimerEventSourceOptions* options.
 *   @param inOwner The object that that will be passed to the Action callback.
 *   @param action Block for the callout routine of this event source.
 */
	static IOTimerEventSource *
	timerEventSource(uint32_t options, OSObject *inOwner, ActionBlock action);
#endif /* __BLOCKS__ */

#if XNU_KERNEL_PRIVATE
	__inline__ void invokeAction(IOTimerEventSource::Action action, IOTimerEventSource * ts,
	    OSObject * owner, IOWorkLoop * workLoop);
#endif /* XNU_KERNEL_PRIVATE */

/*! @function init
 *   @abstract Initializes the timer with an owner, and a handler to call when the timeout expires.
 */
	virtual bool init(OSObject *owner, Action action = NULL);

/*! @function enable
 *   @abstract Enables a call to the action.
 *   @discussion Allows the action function to be called.  If the timer event source was disabled while a call was outstanding and the call wasn't cancelled then it will be rescheduled.  So a disable/enable pair will disable calls from this event source. */
	virtual void enable() APPLE_KEXT_OVERRIDE;

/*! @function disable
 *   @abstract Disable a timed callout.
 *   @discussion When disable returns the action will not be called until the next time enable(qv) is called. */
	virtual void disable() APPLE_KEXT_OVERRIDE;

/*! @function checkForWork
 *   @abstract Pure Virtual member function used by IOWorkLoop for issuing a client calls.
 *   @discussion This function called when the work-loop is ready to check for any work to do and then to call out the owner/action.
 *   @result Return true if this function needs to be called again before all its outstanding events have been processed. */
	virtual bool checkForWork() APPLE_KEXT_OVERRIDE;

/*! @function setTimeoutTicks
 *   @abstract Setup a callback at after the delay in scheduler ticks.  See wakeAtTime(AbsoluteTime).
 *   @param ticks Delay from now to wake up, in scheduler ticks, whatever that may be.
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared. */
	virtual IOReturn setTimeoutTicks(UInt32 ticks);

/*! @function setTimeoutMS
 *   @abstract Setup a callback at after the delay in milliseconds.  See wakeAtTime(AbsoluteTime).
 *   @param ms Delay from now to wake up, time in milliseconds.
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared. */
	virtual IOReturn setTimeoutMS(UInt32 ms);

/*! @function setTimeoutUS
 *       @abstract Setup a callback at after the delay in microseconds.	 See wakeAtTime(AbsoluteTime).
 *   @param us Delay from now to wake up, time in microseconds.
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared. */
	virtual IOReturn setTimeoutUS(UInt32 us);

/*! @function setTimeout
 *   @abstract Setup a callback at after the delay in some unit.	 See wakeAtTime(AbsoluteTime).
 *   @param interval Delay from now to wake up in some defined unit.
 *   @param scale_factor Define the unit of interval, default to nanoseconds.
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared. */
	virtual IOReturn setTimeout(UInt32 interval,
	    UInt32 scale_factor = kNanosecondScale);

#if !defined(__LP64__)
	virtual IOReturn setTimeout(mach_timespec_t interval)
	APPLE_KEXT_DEPRECATED;
#endif

/*! @function setTimeout
 *   @abstract Setup a callback at after the delay in decrementer ticks.	 See wakeAtTime(AbsoluteTime).
 *   @param interval Delay from now to wake up in decrementer ticks.
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared. */
	virtual IOReturn setTimeout(AbsoluteTime interval);

/*! @function wakeAtTimeTicks
 *   @abstract Setup a callback at this absolute time.  See wakeAtTime(AbsoluteTime).
 *   @param ticks Time to wake up in scheduler quantums, whatever that is?
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared. */
	virtual IOReturn wakeAtTimeTicks(UInt32 ticks);

/*! @function wakeAtTimeMS
 *   @abstract Setup a callback at this absolute time.  See wakeAtTime(AbsoluteTime).
 *   @param ms Time to wake up in milliseconds.
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared. */
	virtual IOReturn wakeAtTimeMS(UInt32 ms);

/*! @function wakeAtTimeUS
 *   @abstract Setup a callback at this absolute time.  See wakeAtTime(AbsoluteTime).
 *   @param us Time to wake up in microseconds.
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared. */
	virtual IOReturn wakeAtTimeUS(UInt32 us);

/*! @function wakeAtTime
 *   @abstract Setup a callback at this absolute time.  See wakeAtTime(AbsoluteTime).
 *   @param abstime Time to wake up in some unit.
 *   @param scale_factor Define the unit of abstime, default to nanoseconds.
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared. */
	virtual IOReturn wakeAtTime(UInt32 abstime,
	    UInt32 scale_factor = kNanosecondScale);

#if !defined(__LP64__)
	virtual IOReturn wakeAtTime(mach_timespec_t abstime)
	APPLE_KEXT_DEPRECATED;
#endif

/*! @function wakeAtTime
 *   @abstract Setup a callback at this absolute time.
 *   @discussion Starts the timer, which will expire at abstime. After it expires, the timer will call the 'action' registered in the init() function. This timer is not periodic, a further call is needed to reset and restart the timer after it expires.
 *   @param abstime Absolute Time when to wake up, counted in 'decrementer' units and starts at zero when system boots.
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared by init or IOEventSource::setAction (qqv). */
	virtual IOReturn wakeAtTime(AbsoluteTime abstime);

/*! @function cancelTimeout
 *   @abstract Disable any outstanding calls to this event source.
 *   @discussion Clear down any oustanding calls.  By the time this function completes it is guaranteed that the action will not be called again. */
	virtual void cancelTimeout();

/*! @function init
 *   @abstract Initializes the timer with an owner, and a handler to call when the timeout expires.
 */
	virtual bool init(uint32_t options, OSObject *inOwner, Action inAction);

/*! @function setTimeout
 *   @abstract Setup a callback at after the delay in decrementer ticks.	 See wakeAtTime(AbsoluteTime).
 *   @param options see kIOTimeOptionsWithLeeway and kIOTimeOptionsContinuous
 *   @param interval Delay from now to wake up in decrementer ticks.
 *   @param leeway Allowable leeway to wake time, if the kIOTimeOptionsWithLeeway option is set
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared. */
	virtual IOReturn setTimeout(uint32_t options, AbsoluteTime interval, AbsoluteTime leeway);

/*! @function wakeAtTime
 *   @abstract Setup a callback at this absolute time.
 *   @discussion Starts the timer, which will expire at abstime. After it expires, the timer will call the 'action' registered in the init() function. This timer is not periodic, a further call is needed to reset and restart the timer after it expires.
 *   @param options see kIOTimeOptionsWithLeeway and kIOTimeOptionsContinuous
 *   @param abstime Absolute Time when to wake up, counted in 'decrementer' units and starts at zero when system boots.
 *   @param leeway Allowable leeway to wake time, if the kIOTimeOptionsWithLeeway option is set
 *   @result kIOReturnSuccess if everything is fine, kIOReturnNoResources if action hasn't been declared by init or IOEventSource::setAction (qqv). */
	virtual IOReturn wakeAtTime(uint32_t options, AbsoluteTime abstime, AbsoluteTime leeway);

private:
	static void timeoutAndRelease(void *self, void *c);
	static void timeoutSignaled(void *self, void *c);

private:
	OSMetaClassDeclareReservedUsed(IOTimerEventSource, 0);
	OSMetaClassDeclareReservedUsed(IOTimerEventSource, 1);
	OSMetaClassDeclareReservedUsed(IOTimerEventSource, 2);
	OSMetaClassDeclareReservedUnused(IOTimerEventSource, 3);
	OSMetaClassDeclareReservedUnused(IOTimerEventSource, 4);
	OSMetaClassDeclareReservedUnused(IOTimerEventSource, 5);
	OSMetaClassDeclareReservedUnused(IOTimerEventSource, 6);
	OSMetaClassDeclareReservedUnused(IOTimerEventSource, 7);
};

#endif /* !_IOTIMEREVENTSOURCE */
