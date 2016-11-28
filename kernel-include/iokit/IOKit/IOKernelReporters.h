/*
 * Copyright (c) 2012-2014 Apple Computer, Inc.  All Rights Reserved.
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
 * FILE: IOReporter.h
 * AUTH: Cyril & Soren (Core OS)
 * DATE: 2012-2013 (Copyright Apple Inc.)
 * DESC: IOReporting interfaces for I/O Kit drivers
 *
 */

#ifndef _IOKERNEL_REPORTERS_H_
#define _IOKERNEL_REPORTERS_H_

#include <machine/limits.h>

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOBufferMemoryDescriptor.h>

#include <IOKit/IOReportTypes.h>
#include <IOKit/IOKernelReportStructs.h>

typedef OSDictionary IOReportLegendEntry;

/*******************************
  TOC: this file contains
    1. Introduction
    2a. IOReporter class declaration (public & non-public members)
    2b. static IOReporter methods unrelated to the class
    3. IOReporter subclass declarations (public & non-public members)
    4. IOReportLegend class declaration
*******************************/

/*!
   1. Introduction

   IOReporting is a mechanism for I/O Kit drivers to gather statistics
   (or other information) and make it available to various "observers,"
   which are generally in user space. Requests for information come
   through two new IOService methods: ::configureReport(...) and
   ::updateReport(...). While not required (see IOReportTypes.h), drivers
   will generally use IOReporter subclass instances to track the requested
   information and respond to IOReporting requests. Drivers can use these
   classes to track information, either all the time or between "enable"
   and "disable" calls to IOService::configureReport().

   Available information is organized into "channels." A channel is
   uniquely identified by both driver (registry) ID and a 64-bit channel
   ID. One way drivers can advertise their channels is by publishing
   "legends" in the I/O Kit registry. In addition to collecting
   information and responding to queries, IOReporter objects can produce
   legend entries describing their channels. The IOReportLegend class
   helps manage legend entries from multiple reporter objects as well
   as with grouping channels logically for observers.

   An important basic constraint of the current implementation is that
   all channels reported by a particular reporter instance must share all
   traits except channel ID and name.  Specifically, the channel type
   (including report class, categories, & size) and units.  Additionally,
   IOHistogramReporter currently only supports one channel at a time.

   Currently, ::{configure/update}Report() can be called any time between
   when a driver calls registerService() and when free() is called on
   your driver. 12960947 tracks improvements / recommendations for
   correctly handling these calls during termination.

   * Locking
   IOReporting only imposes concurrent access constraints when multiple
   threads are accessing the same object.  Three levels of constraint apply
   depending on a method's purpose:
   1. Allocation/Teardown - same-instance concurrency UNSAFE, MAY BLOCK
   2. Configuration - same-instance concurrency SAFE, MAY BLOCK
   3. Update - same-instance concurrency SAFE, WILL NOT BLOCK
   
   Configuration requires memory management which can block and must
   be invoked with interrupts ENABLED (for example, NOT in the interrupt
   context NOR with a spin lock -- like IOSimpleLock -- held).
   
   Updates can be performed with interrupts disabled, but clients should
   take into account that IOReporters' non-blocking currenency is achieved
   with IOSimpleLockLockDisable/UnlockEnableInterrupts(): that is, by
   disabling interrupts and taking a spin lock.  While IOReporting will
   never hold a lock beyond a call into it, some time may be spent within
   the call spin-waiting for the lock.  Clients holding their own
   spin locks should carefully consider the impact of IOReporting's
   (small) additional latency before calling it while holding a spin lock.
 
   The documentation for each method indicates any concurrency guarantees.
 */


/*********************************/
/*** 2a. IOReporter Base Class ***/
/*********************************/

class IOReporter : public OSObject
{
    OSDeclareDefaultStructors(IOReporter);

protected:
/*! @function   IOReporter::init
    @abstract   base init() method, called by subclass initWith() methods

    @param  reportingService - IOService associated with all channels
    @param  channelType - type info for all channels (element_idx = 0)
    @param  unit - description applied for all channels
    @result     true on success, false otherwise

    @discussion
        init() establishes the parameters of all channels for this reporter
        instance. Any channels added via addChannel() will be of this type
        and have this unit.

        IOReporter clients should use the static <subclass>::with() methods
        below to obtain fully-initialized reporter instances.  ::free()
        expects ::init() to have completed successfully.  On failure, any
        allocations are cleaned up.

    Locking: same-instance concurrency UNSAFE
*/
    virtual bool init(IOService *reportingService,
                      IOReportChannelType channelType,
                      IOReportUnits unit);

public:

/*! @function   IOReporter::addChannel
    @abstract   add an additional, similar channel to the reporter

    @param  channelID - identifier for the channel to be added
    @param  channelName - an optional human-readble name for the channel
    @result     appropriate IOReturn code

    @discussion
        The reporter will allocate memory to track a new channel with the
        provided ID and name (if any).  Its other traits (type, etc) will
        be those provided when the reporter was initialized.  If no channel
        name is provided and the channelID consists solely of ASCII bytes,
        those bytes (ignoring any NUL bytes) will be used as the
        human-readable channel name in user space.  The IOREPORT_MAKEID()
        macro in IOReportTypes.h can be used to create ASCII channel IDs.

    Locking: same-instance concurrency SAFE, MAY BLOCK
*/
    IOReturn addChannel(uint64_t channelID, const char *channelName = NULL);
    
/*! @function   IOReporter::createLegend
    @abstract   create a legend entry represending this reporter's channels
    @result     An IOReportLegendEntry object or NULL on failure.
    @discussion
        All channels added to the reporter will be represented
        in the resulting legend entry.

        Legends must be published togethar as an array under the
        kIOReportLegendKey in the I/O Kit registry.  The IOReportLegend
        class can be used to properly combine legend entries from multiple
        reporters as well as to put channels into groups of interest to
        observers.  When published, individual legend entries share
        characteristics such as group and sub-group.  Multiple IOReporter
        instances are required to produce independent legend entries which
        can then be published with different characteristics.

        Drivers wishing to publish legends should do so as part of their
        ::start() routine.  As superclasses *may* have installed legend
        entries, any existing existing legend should be retrieved and
        IOReportLegend used to merge it with the new entries. 
 
        Recommendations for best practices are forthcoming.
 
        Instead of calling createLegend on your reporter object and then 
        appending it manually to IOReportLegend, one may prefer to call
        IOReportLegend::appendReporterLegend which creates and appends a
        reporter's IOReportLegendEntry in a single call. 

    Locking: same-instance concurrency SAFE, MAY BLOCK
*/
    IOReportLegendEntry* createLegend(void);
        
/*! @function   IOReporter::configureReport
    @abstract   track IOService::configureReport(), provide sizing info

    @param  channelList - channels to configure
    @param  action - enable/disable/size, etc (see IOReportTypes.h)
    @param  result - *incremented* for kIOReportGetDimensions
    @param  destination - action-specific default destination
    @result     appropriate IOReturn code

    @discussion
        Any time a reporting driver's ::configureReport method is invoked,
        this method should be invoked on each IOReporter that is being
        used by that driver to report channels in channelList.

        Any channels in channelList which are not tracked by this reporter
        are ignored.  ::configureReport(kIOReportGetDimensions) expects
        the full size of all channels, including any reported by
        superclasses.  It is valid to call this routine on multiple
        reporter objects in succession and they will increment 'result'
        to provide the correct total.

        In the initial release, this routine is only required to calculate
        the response to kIOReportGetDimensions, but in the future it will
        will enable functionality like "triggered polling" via
        kIOReportNotifyHubOnChange.  Internally, it is already keeping
        track of the number of times each channel has been enabled and
        disabled.  13073064 tracks adding a method to see whether any
        channels are currently being observed.

        The static IOReporter::configureAllReports() will call this method
        on multiple reporters grouped in an OSSet.

    Locking: same-instance concurrency SAFE, MAY BLOCK
*/
    IOReturn configureReport(IOReportChannelList *channelList,
                             IOReportConfigureAction action,
                             void *result,
                             void *destination);
    
/*! @function   IOReporter::updateReport
    @abstract   Produce standard reply to IOService::updateReport()

    @param  channelList - channels to update
    @param  action - copy/trace data (see IOReportTypes.h)
    @param  result - action-specific return value (e.g. size of data)
    @param  destination - destination for this update (action-specific)
    @result     appropriate IOReturn code

    @discussion
        This method searches channelList for channels tracked by this
        reporter, writes the corresponding data into 'destination', and 
        updates 'result'.  It should be possible to pass a given set of
        IOService::updateReport() arguments to any and all reporters as
        well as to super::updateReport() and get the right result.

        The static IOReporter::updateAllReports() will call this method
        on an OSSet of reporters.

    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    IOReturn updateReport(IOReportChannelList *channelList,
                          IOReportConfigureAction action,
                          void *result,
                          void *destination);

/*! @function   IOReporter::free
    @abstract   Releases the object and all its resources.

    @discussion
        ::free() [called on last ->release()] assumes that init() [called
        by static ::with() methods] has completed successfully.

    Locking: same-instance concurrency UNSAFE
*/
    virtual void free(void) APPLE_KEXT_OVERRIDE;
    

/*********************************/
/*** 2b. Useful Static Methods ***/
/*********************************/

/* The following static functions are intended to simplify the management
 * of multiple reporters.  They may be superseded in the future by an
 * IOReportManager class.
 */

/*! @function   IOReporter::configureAllReports
    @abstract   call configureReport() on multiple IOReporter objects

    @param  reporters - OSSet of IOReporter objects
    @param  channelList - full list of channels to configure
    @param  action - enable/disable/size, etc
    @param  result - action-specific returned value
    @param  destination - action-specific default destination
    @result     success if all objects successfully complete
                IOReporter::configureReport()

    @discussion
        The OSSet must only contain IOReporter instances.  The presence
        of non-IOReporter instances will cause this function to return
        kIOReturnBadArgument.  If any reporter returns an error, the
        function will immediately return that error.

        Per the IOReporter::configureReport() documentation, each
        reporter will search channelList for channels it is reporting
        and provide a partial response.
*/
    static IOReturn configureAllReports(OSSet *reporters,
                                        IOReportChannelList *channelList,
                                        IOReportConfigureAction action,
                                        void *result,
                                        void *destination);
// FIXME: just put the function (inline-ish) here?
    
/*! @function   IOReporter::updateAllReports
    @abstract   call updateReport() on multiple IOReporter objects

    @param  reporters - OSSet of IOReporter objects
    @param  channels - full list of channels to update
    @param  action - type/style of update
    @param  result - returned details about what was updated
    @param  destination - destination for this update (action-specific)
    @result     IOReturn code
    @discussion
        The OSSet must only contain IOReporter instances.  The presence
        of non-IOReporter instances will cause this function to return
        kIOReturnBadArgument.  If any reporter returns an error, the
        function will immediately return that error.

        Per the IOReporter::configureReport() documentation, each
        reporter will search channelList for channels it is reporting
        and provide a partial response.
*/
    static IOReturn updateAllReports(OSSet *reporters,
                                     IOReportChannelList *channelList,
                                     IOReportConfigureAction action,
                                     void *result,
                                     void *destination);
// FIXME: just put the function (inline-ish) here?


    /*  Protected (subclass-only) Methods

        General subclassing is not encouraged as we intend to improve
        internal interfaces.  If you need something that might require
        a subclass, please file a bug against IOReporting/X and we will
        help you.

        One important concept for sub-classes (not clients) is that report
        data is stored in IOReportElement structures (see IOReportTypes.h).
     */
protected:

/*! @function   IOReporter::lockReporterConfig
    @function   IOReporter::unlockReporterConfig
    @abstract   prevent concurrent reconfiguration of a reporter

    @discussion
        lockReporterConfig() takes a mutex-based lock intended to prevent
        concurrent access to the reporter's configuration.  It is not
        intended to prevent updates to the reporter's data.  As long as
        all other requirements are met, it is safe to simultaneously hold
        both the configuration and data locks on a single reporter.

        lockReporterConfig() is used by routines such as addChannel().
        See also lockReporter() and ::handle*Swap*() below.
*/
    void lockReporterConfig(void);
    void unlockReporterConfig(void);
    
/*! @function   IOReporter::lockReporter
    @function   IOReporter::unlockReporter
    @abstract   prevent concurrent access to a reporter's data
     
    @discussion
        This method grabs a lock intended to control access the reporter's
        reporting data.  Sub-classes maninupating internal report values
        must make sure the reporter is locked (usually by the most generic
        public interface) before calling getElementValues(),
        copyElementValues(), or setElementValues().

        Subclasses should ensure that this lock is taken exactly once
        before directly accessing reporter data.  For example,
        [virtual] IOFooReporter::handleSetFoo(.) {
            // assert(lock_held)
            getElementValues(1..)
            getElementValues(3..)
            getElementValues(5..)
            [calculate]
            setElementValues(6..)
        }
        IOFooReporter::setFoo(.) {      // not virtual
            lockReporter()
            handleSetFoo(.)
            unlockReporter()
        }

        IOReporter::handle*() use lockReporter() similarly.  For example,
        the lock is taken by IOReporter::updateReport() and is already
        held by the time any ::updateChannelValues() methods are called.

        Subclasses cannot call this routine if the lock is already held.
        That's why IOReporting generally only calls it from non-virtual
        public methods.  In particular, this method should not be called
        it from ::handle*() methods which exist to allow override after
        the lock is taken.

        Because lockReporter() uses a spin lock, it is SAFE to use in the
        interrupt context.  For the same reason, however, it is UNSAFE
        to perform any blocking blocking operations (including memory
        allocations) while holding this lock.
*/
    void lockReporter(void);
    void unlockReporter(void);

/*!
    @discussion
        The ::handle*Swap* functions allow subclasses to safely reconfigure
        their internal state.  A non-virtual function handles locking
        and invokes the functions in order:
        - lockReporterConfig()  // protecting instance vars but not content
        - prepare / allocate buffers of the new size
        - if error, bail (unlocking, of course)

        - lockReporter()        // protecting data / blocking updates
        - swap: preserve continuing data / install new buffers
        - unlockReporter()

        - deallocate now-unused buffers
        - unlockReporterConfig()
*/
/*! @function   IOReporter::handleSwapPrepare
    @abstract   allocate memory in preparation for an instance variable swap

    @param newNChannels   target number of channels
    @result     IOReturn code

    @discussion
        ::handleSwapPrepare() is responsible for allocating appropriately-
        sized buffers (based on the new number of channels) and storing
        them in _swap* instance variables.  If returning and error, it
        must deallocate any buffers and set to NULL any _swap* variables.

    Locking: The caller must ensure that the *config* lock is HELD but
             that the reporter (data) lock is *NOT HELD*.
*/
    virtual IOReturn handleSwapPrepare(int newNChannels);

/*! @function   IOReporter::handleAddChannelSwap
    @abstract   update primary instance variables with new buffers

    @param channelID   ID of channel being added
    @param channelName   optional channel name, in an allocated object
    @result     IOReturn code

    @discussion
        handlAddChannelSwap() replaces the primary instance variables
        with buffers allocated in handlePrepareSwap().  It copies the the
        existing data into the appropriate portion of the new buffers.
        Because it is specific to adding one channel, it assumes that the
        target number of channels is one greater than the current value
        of _nChannels.

        IOReporter::handleAddChannelSwap() increments _nElements and
        _nChannels.  To ensure that these variables describe the current
        buffers throughout ::handle*Swap(), subclasses overriding this
        method should call super::handleAddChannelSwap() after swapping
        their own instance variables.

        If returning an error, all implementations should leave their
        instance variables as they found them (*unswapped*).  That ensures
        handleSwapCleanup() cleans up the unused buffers regardless of
        whether the swap was complete.

        Pseudo-code incorporating these suggestions:
        res = <err>; swapComplete = false;
        if (<unexpected>)       goto finish
        tmpBuf = _primaryBuf; _primaryBuf = _swapBuf; _swapBuf = _primaryBuf;
        ...
        swapComplete = true;
        res = super::handle*Swap()
        ...
        finish:
        if (res && swapComplete)    // unswap

    Locking: The caller must ensure that BOTH the configuration and
             reporter (data) locks are HELD.
*/
    virtual IOReturn handleAddChannelSwap(uint64_t channel_id,
                                          const OSSymbol *symChannelName);

/*! @function   IOReporter::handleSwapCleanup
    @abstract   release and forget unused buffers

    @param swapNChannels   channel-relative size of the _swap buffers

    @discussion
        ::handleSwapCleanup() is responsible for deallocating the buffers
        no longer used after a swap.  It must always be called if
        SwapPrepare() completes successfully.  Because bufers may be
        swapped in and out of existance, the _swap* variables may be
        NULL and should be set to NULL when complete.

    Locking: The caller must ensure that the *config* lock is HELD but
             that the reporter (data) lock is *NOT HELD*.
*/
    virtual void handleSwapCleanup(int swapNChannels);

/*! @function   IOReporter::handleConfigureReport
    @abstract   override vector for IOReporter::configureReport()
                [parameters and result should exactly match]
     
    @discussion
        The public base class method takes the reporter lock, calls this
        function, and then drops the lock.  Subclasses should not call
        this function directly.
*/
    virtual IOReturn handleConfigureReport(IOReportChannelList *channelList,
                                           IOReportConfigureAction action,
                                           void *result,
                                           void *destination);

/*! @function   IOReporter::handleUpdateReport
    @abstract   override vector for IOReporter::updateReport()
                [parameters and result should exactly match]
     
    @discussion
        The public base class method takes the reporter lock, calls this
        function, and then drops the lock.  Subclasses should not call
        this function directly.
 
        This function may be overriden but the common case should be to 
        simply update reporter's specific values by overriding 
        IOReporter::updateChannelValues().
*/
    virtual IOReturn handleUpdateReport(IOReportChannelList *channelList,
                                        IOReportConfigureAction action,
                                        void *result,
                                        void *destination);

/*  @function   IOReporter::handleCreateLegend
    @abstract   override vector for IOReporter::createLegend()
                [parameters and result should exactly match]
     
    @discussion
        The public base class method takes the reporter lock, calls this
        function, and then drops the lock.  Subclasses should not call
        this function directly.
*/
    virtual IOReportLegendEntry* handleCreateLegend(void);

/*! @function   IOReporter::updateChannelValues
    @abstract   update channel values for IOReporter::updateReport()
     
    @param  channel_index - logical (internal) index of the channel
    @result     appropriate IOReturn code
     
    @discussion
        Internal reporter method to allow a subclass to update channel
        data when updateReport() is called.  This routine handles the
        common case of a subclass needing to refresh state in response
        to IOReporter::updateReport().  It saves the complexity of
        parsing the full parameters to IOReporter::updateReport().
 
        The IOReporter base class implementation does not do anything
        except return success.

    Locking: IOReporter::updateReport() takes the reporter lock,
             determines the indices involved, calls this function, and
             then proceeds to provide values to the caller.  If subclasses
             need to call this routine directly, they must ensure that
             the reporter (data) lock is held: see
             IOReporter::lockReporter().
*/
    virtual IOReturn updateChannelValues(int channel_index);

    
/*! @function   IOReporter::updateReportChannel
    @abstract   Internal method to extract channel data to a destination

    @param  channel_index - offset into internal elements array
    @param  nElements - incremented by the number of IOReportElements added
    @param  destination - pointer to the destination buffer
    @result     IOReturn code

    @discussion
        updateReportChannel() is used to extract a single channel's
        data to the updateReport() destination.

    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    IOReturn updateReportChannel(int channel_index,
                                 int *nElements,
                                 IOBufferMemoryDescriptor *destination);
    
    
/*! @function   IOReporter::setElementValues
    @abstract   Atomically update a specific member of _elements[].

    @param  element_index - index of the _element in internal array
    @param  values - IORepoterElementValues to replace those at _elements[idx]
    @param  record_time - optional mach_absolute_time to be used for metadata 
    @result     IOReturn code 

    @discussion
        element_index can be obtained from getFirstElementIndex().  If
        record_time is not provided, IOReporter::setElementValues() will
        fetch the current mach_absolute_time.  If the current time is
        already known, it is more efficient to pass it along.

    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    virtual IOReturn setElementValues(int element_index,
                                      IOReportElementValues *values,
                                      uint64_t record_time = 0);

/*! @function   IOReporter::getElementValues
    @abstract   Internal method to directly access the values of an element
     
    @param  element_index - index of the _element in internal array
    @result     A pointer to the element values requested or NULL on failure
     
    @discussion
 
    Locking: Caller must ensure that the reporter (data) lock is held.
    The returned pointer is only valid until unlockReporter() is called.
*/
    virtual const IOReportElementValues* getElementValues(int element_index);
    
/*! @function   IOReporter::getFirstElementIndex
    @abstract   Returns the first element index for a channel

    @param  channel_id - ID of the channel
    @param  element_index - pointer to the returned element_index
    @result     appropriate IOReturn code

    @discussion
        For efficiently and thread-safely reading _elements
 
    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    virtual IOReturn getFirstElementIndex(uint64_t channel_id,
                                          int *element_index);
    
/*! @function   IOReporter::getChannelIndex
    @abstract   Returns the index of a channel from internal data structures

    @param  channel_id - ID of the channel
    @param  element_index - pointer to the returned element_index
    @result     appropriate IOReturn code
    
    @discussion
        For efficiently and thread-safely reading channels

    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    virtual IOReturn getChannelIndex(uint64_t channel_id,
                                     int *channel_index);
    
/*! @function   IOReporter::getChannelIndices
    @abstract   Returns the index of a channel and its corresponding
                first element index from internal data structure
     
     @param  channel_id - ID of the channel
     @param  channel_index - pointer to the returned channel_index
     @param  element_index - pointer to the returned element_index
     @result     appropriate IOReturn code
     
     @discussion
        For efficiently and thread-safely reading channel elements.
        It is commonly useful to get access to both channel and element
        indices togther.  This convenience method allows sub-classes to
        get both indices simultaneously.
     
    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    virtual IOReturn getChannelIndices(uint64_t channel_id,
                                       int *channel_index,
                                       int *element_index);

/*! @function   IOReporter::copyElementValues
    @abstract   Copies the values of an internal element to *elementValues

    @param  element_index - Index of the element to return values from
    @param  elementValues - For returning the content of element values
    @result     Returns the content of an element

    @discussion 
        For efficiently and thread-safely reading _elements.
        May need to find the index of the element first.

    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    virtual IOReturn copyElementValues(int element_index,
                                       IOReportElementValues *elementValues);
    
// private methods
private:
/*! @function   IOReporter::copyChannelIDs
    @abstract   return an an OSArray of the reporter's 
                channel IDs
     
    @param  none
    @result     An OSArray of the repoter's channel ID's as OSNumbers
 
    @discussion
        This method is an internal helper function used to prepare a
        legend entry.  It encapsulates the channel IDs in OSNumbers and
        aggregates them in an OSArray used when building the IOReportLegend
     
    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    OSArray* copyChannelIDs(void);

/*! @function   IOReporter::legendWith
    @abstract   Internal method to help create legend entries

    @param  channelIDs - OSArray of OSNumber(uint64_t) channels IDs.
    @param  channelNames - parrallel OSArray of OSSymbol(rich names)
    @param  channelType - the type of all channels in this legend
    @param  unit - The unit for the quantity recorded by this reporter object

    @result     An IOReportLegendEntry object or NULL on failure 
 
    @discussion
        This static method is the main legend creation function. It is called by 
        IOReporter sub-classes and is responsible for building an 
        IOReportLegendEntry corresponding to this reporter object.
        This legend entry may be extended by the sub-class of IOReporter if 
        required.
 
    Locking: SAFE to call concurrently (no static globals), MAY BLOCK
*/
    static IOReportLegendEntry* legendWith(OSArray *channelIDs,
                                           OSArray *channelNames,
                                           IOReportChannelType channelType,
                                           IOReportUnits unit);

// protected instance variables (want to get rid of these)
protected:
    IOReportChannelType _channelType;
    uint64_t            _driver_id;         // driver reporting data
    
    // IOHistogramReporter accesses these; need to re-do its instantiation
    IOReportElement    *_elements;
    int                *_enableCounts;      // refcount kIOReportEnable/Disable
    uint16_t            _channelDimension;  // Max channel size
    int                 _nElements;
    int                 _nChannels;         // Total Channels in this reporter
    OSArray            *_channelNames;
    
    // MUST be protected because check is a macro!
    bool                _reporterIsLocked;
    bool                _reporterConfigIsLocked;
    
    // Required for swapping inside addChannel
    IOReportElement    *_swapElements;
    int                *_swapEnableCounts;

// private instance variables
private:
    IOReportUnits       _unit;

    int                 _enabled;   // 'enabled' if _enabled > 0

    IOLock             *_configLock;
    IOInterruptState    _interruptState;
    IOSimpleLock       *_reporterLock;
 
};


/************************************/
/***** 3. IOReporter Subclasses *****/
/************************************/

/*!
    @class      IOSimpleReporter
    @abstract   Report simple integers
    @discussion
        Each IOSimpleReporter can have an arbitrary number of channels,
        each publishing a single integer value at any given time.
*/

class IOSimpleReporter : public IOReporter
{
    OSDeclareDefaultStructors(IOSimpleReporter);
    
public:

/*! @function   IOSimpleReporter::with
    @abstract   create an initialized simple reporter

    @param  reportingService - IOService associated with all channels
    @param  categories - The category in which the report should be classified
    @param  unit - The unit for the quantity recorded by the reporter object
    @result     On success, an instance of IOSimpleReporter, else NULL
    
    @discussion 
        Creates an instance of IOSimpleReporter object
 
    Locking: SAFE to call concurrently (no static globals), MAY BLOCK.
*/
    static IOSimpleReporter* with(IOService *reportingService,
                                  IOReportCategories categories,
                                  IOReportUnits unit);
    
/*! @function   IOSimpleReporter::setValue
    @abstract   Thread safely set a channel's value

    @param  channel_id - ID of the channel for which the value needs to be set
    @param  value - New channel value
    @result     Appropriate IOReturn code

    @discussion 
        Updates the value of a channel to the provided value.

    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    IOReturn setValue(uint64_t channel_id,
                      int64_t value);
    
/*! @function   IOSimpleReporter::incrementValue
    @abstract   Thread safely increment a channel's value by a given amount

    @param  channel_id - ID of the channel for which the value needs to be incremented
    @param  increment - Amount to be added to the current channel value
    @result     Appropriate IOReturn code
    @discussion 
        Increments the value of the channel ID by the provided amount.

    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    IOReturn incrementValue(uint64_t channel_id,
                            int64_t increment);
    
/*! @function   IOSimpleReporter::getValue
    @abstract   Thread safely access a channel value

    @param  channel_id - ID of the channel to get a value from
    @result     Returns the current value stored in the channel 
    @discussion 
        Accessor method to a channel's current stored value

    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    int64_t getValue(uint64_t channel_id);
    
protected:
    
/*! @function   IOSimpleReporter::initWith
    @abstract   instance method implementation called by IOSimpleReporter::with

    @discussion 
        See description of parameters above

    Locking: same-instance concurrency UNSAFE
*/
    virtual bool initWith(IOService *reportingService,
                          IOReportCategories categories,
                          IOReportUnits unit);
    
private:
    
};



/*! 
    @class      IOStateReporter
    @abstract   Report state machine data
    @discussion
        Each IOStateReporter can report information for an arbitrary number
        of similar state machines.  All must have the same number of states.
*/
class IOStateReporter : public IOReporter
{
    OSDeclareDefaultStructors(IOStateReporter);
    
public:
    
/*! @function   IOStateReporter::with
    @abstract   State reporter static creation method

    @param  reportingService - The I/O Kit service for this reporter's channels
    @param  categories - The categories for this reporter's channels
    @param  nstates - Maximum number of states for this reporter's channels
    @param  unit - optional parameter if using override/increment...()
    @result     on success, an IOStateReporter instance, else NULL
    
    @discussion 
        Creates an instance of IOStateReporter.  The default time scale
        is the current system's notion of mach_absolute_time().  Using a
        non-default time scale requires the use of
        override/incrementChannelState() instead of setState().
        setState() always updates using mach_absolute_time().

    Locking: SAFE to call concurrently (no static globals), MAY BLOCK
*/
    static IOStateReporter* with(IOService *reportingService,
                                 IOReportCategories categories,
                                 int nstates,
                                 IOReportUnits unit = kIOReportUnitHWTicks);
        
/*! @function   IOStateReporter::setStateID
    @abstract   Assign a non-default ID to a state

    @param  channel_id - ID of channel containing the state in question
    @param  state_index - index of state to give an ID: [0..(nstates-1)]
    @param  state_id - 64-bit state ID, for ASCII, use IOREPORT_MAKEID
 
    @result     Appropriate IOReturn code
 
    @discussion 
        By default, IOStateReporter identifies its channel states by
        numbering them from 0 to <nstates - 1>.  If setStateID is not
        called to customize the state IDs, the numbered states will be
        kept throughout the life of the object and it is safe to reference
        those states by their indices.  Otherwise, after setStateID() has
        been called, the ordering of states is no longer guaranteed and
        the client must reference states by their assigned state ID.
 
    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    IOReturn setStateID(uint64_t channel_id,
                        int state_index,
                        uint64_t state_id);

/*! @function   IOStateReporter::setChannelState
    @abstract   Updates the current state of a channel to a new state

    @param  channel_id - ID of the channel which is updated to a new state
    @param  new_state_id - ID of the target state for this channel
    @param  last_intransition - deprecated: time of most recent entry
    @param  prev_state_residency - deprecated: time spent in previous state 
    @result     Appropriate IOReturn code

    @discussion 
        setChannelState() updates the amount of time spent in the previous
        state (if any) and increments the number of transitions into the
        new state.  It also sets the target state's last transition time to
        the current time and enables internal time-keeping for the channel.
        In this mode, calls like getStateResidencyTime() and updateReport()
        automatically update a channel's time in state.

        new_state_id identifies the target state as initialized
        (0..<nstates-1>) or as configured by setStateID().

        Drivers wishing to compute and report their own time in state
        should use incrementChannelState() or overrideChannelState().  It
        is not currently possible for a driver to synchronize with the
        automatic time-keeping enabled by setChannelState().  The
        4-argument version of setChannelState() is thus impossible to
        use correctly.  In the future, there may be a setChannelState()
        which accepts a last_intransition parameter and uses it to
        automatically calculate time in state (ERs -> IOReporting / X).
 
    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    IOReturn setChannelState(uint64_t channel_id,
                             uint64_t new_state_id);

    IOReturn setChannelState(uint64_t channel_id,
                             uint64_t new_state_id,
                             uint64_t last_intransition,
                             uint64_t prev_state_residency) __deprecated;

/*! @function   IOStateReporter::setState
    @abstract   Updates state for single channel reporters

    @param  new_state_id - New state for the channel
    @param  last_intransition - deprecated: time of most recent entry
    @param  prev_state_residency - deprecated: spent in previous state 
    @result     Appropriate IOReturn code.

    @discussion 
        setState() is a convenience method for single-channel state
        reporter instances.  An error will be returned if the reporter
        in question has more than one channel.

        See further discussion at setChannelState().

    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    IOReturn setState(uint64_t new_state_id);
    
    IOReturn setState(uint64_t new_state_id,
                      uint64_t last_intransition,
                      uint64_t prev_state_residency) __deprecated;

/*! @function   IOStateReporter::overrideChannelState
    @abstract   Overrides state data for a channel with passed arguments
     
    @param  channel_id - ID of the channel which state is to be updated
    @param  state_id - state id for the channel
    @param  time_in_state - time used as new total time in state
    @param  intransitions - total number of transitions into state
    @param  last_intransition - mach_absolute_time of most recent entry (opt)
    @result     Appropriate IOReturn code
     
    @discussion
        overrideChannelState() sets a particular state's time in state
        and transition count to the values provided.  The optional
        last_intransition records the last time the channel transitioned
        into the given state.  Passing 0 for time_in_state and
        intransitions will force the current values to 0.  Passing 0
        for last_intransition for all states will disable the notion
        of a channel's "current state."

        The most recent last_intransition (amongst all states in a channel)
        logically determines the current state.  If last_intransition is
        not provided for any state, the channel will not report a current
        For consistent results, it is important to either never specify
        last_intransition or to always specify it.

        There is currently a bug in determining current state (13423273).
        The IOReportMacros.h macros only update the state's metadata
        timestamp and libIOReport only looks at the metadata timestamps
        to determine the current state.  Until that bug is fixed, whichever
        state is updated most recently will be considered the "current"
        state by libIOReport.

        ::setState()'s automatic "time in state" updates are not supported
        when using overrideChannelState().  Clients must not use
        overrideChannelState() on any channel that has ::setState() called
        on it.  Unlike with ::setState(), clients using
        overrideChannelState() are responsible for ensuring that data is
        up to date for updateReport() calls.  The correct way to do this
        is for a driver's ::updateReport() method to push the most up to
        date values into the reporters before calling
        super::updateReport().
     
    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/    
    IOReturn overrideChannelState(uint64_t channel_id,
                                  uint64_t state_id,
                                  uint64_t time_in_state,
                                  uint64_t intransitions,
                                  uint64_t last_intransition = 0);

/*! @function   IOStateReporter::incrementChannelState
    @abstract   Updates state data for a channel with passed arguments
     
    @param  channel_id - ID of the channel which state is to be updated
    @param  state_id - state id for the channel
    @param  time_in_state - time to be accumulated for time in state
    @param  intransitions - number of transitions into state to be added
    @param  last_intransition - mach_absolute_time of most recent entry (opt)
    @result     Appropriate IOReturn code
     
    @discussion
        incrementChannelState() adds time_in_state and intransitions
        to the current values stored for a particular state.  If provided,
        last_intransition overwrites the time the state was most recently
        entered.  Passing 0 for time_in_state and intransitions will have
        no effect.  Passing 0 for last_intransition for all states will
        disable the notion of a channel's "current state."

        The most recent last_intransition (amongst all states in a channel)
        logically determines the current state.  If last_intransition is
        not provided for any state, the channel will not report a current
        For consistent results, it is important to either never specify
        last_intransition or to always specify it.

        There is currently a bug in determining current state (13423273).
        The IOReportMacros.h macros only update the state's metadata
        timestamp and libIOReport only looks at the metadata timestamps
        to determine the current state.  Until that bug is fixed, whichever
        state is updated most recently will be considered the "current"
        state by libIOReport.

        ::setState()'s automatic "time in state" updates are not supported
        when using incrementChannelState().  Clients must not use
        incrementChannelState() on any channel that has ::setState()
        called on it.  Unlike with ::setState(), clients using
        incrementChannelState() are responsible for ensuring that data
        is up to date for updateReport() calls.  The correct way to do
        this is for a driver's ::updateReport() method to push the most
        up to date values into the reporters before calling
        super::updateReport().
 
    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/    
    IOReturn incrementChannelState(uint64_t channel_id,
                                   uint64_t state_id,
                                   uint64_t time_in_state,
                                   uint64_t intransitions,
                                   uint64_t last_intransition = 0);
    
/*! @function   IOStateReporter::setStateByIndices
    @abstract   update a channel state without validating channel_id
 
    @param  channel_index - 0..<nChannels>, available from getChannelIndex()
    @param  new_state - New state (by index) for the channel
    @param  last_intransition - deprecated: time of most recent entry
    @param  prev_state_residency - deprecated: time spent in previous state 
    @result     Appropriate IOReturn code
 
    @discussion
        Similar to setState(), setStateByIndices() sets a channel's state
        without searching for the channel or state IDs.  It will perform
        bounds checking, but relies on the caller to properly indicate
        the indices of the channel and state.  Clients can rely on channels
        being added to IOStateReporter in order: the first channel will
        have index 0, the second index 1, etc.  Like ::setState(),
        "time in state" calculations are handled automatically.

        setStateByIndices() is faster than than setChannelState(), but
        it should only be used where the latter's performance overhead
        might be a problem.  For example, many channels in a single
        reporter and high-frequency state changes.

        Drivers wishing to compute and report their own time in state
        should use incrementChannelState() or overrideChannelState().  It
        is not currently possible for a driver to synchronize with the
        automatic time-keeping enabled by setStateByIndices().  The
        4-argument version of setChannelState() is thus impossible to
        use correctly.  In the future, there may be a setChannelState()
        which accepts a last_intransition parameter and uses it to
        automatically calculate time in state (ERs -> IOReporting / X).
 
    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    IOReturn setStateByIndices(int channel_index,
                               int new_state_index);
    
    IOReturn setStateByIndices(int channel_index,
                               int new_state_index,
                               uint64_t last_intransition,
                               uint64_t prev_state_residency) __deprecated;
    
/*! @function   IOStateReporter::getStateInTransitions
    @abstract   Accessor method for count of transitions into state
     
    @param  channel_id - ID of the channel
    @param  channel_state - State of the channel
    @result     Count of transitions into the requested state.
     
    @discussion
        Some clients may need to consume internally the data aggregated by the
        reporter object. This method allows a client to retrieve the count of
        transitions into the requested state for the channel_id.
 
    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    uint64_t getStateInTransitions(uint64_t channel_id,
                                   uint64_t state_id);

/*! @function   IOStateReporter::getStateResidencyTime
    @abstract   Accessor method for time spent in a given state
     
    @param  channel_id - ID of the channel
    @param  channel_state - State of the channel
    @result     Absolute time spent in specified state
     
    @discussion
        Some clients may need to consume internally the data aggregated
        by the by the reporter object.  This method allows a client to
        retrieve the absolute time a particular channel recorded as spent
        in a specified state.
     
    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    uint64_t getStateResidencyTime(uint64_t channel_id,
                                   uint64_t state_id);
    
/*! @function   IOStateReporter::getStateLastTransitionTime
    @abstract   Accessor method for last time a transition occured
     
    @param  channel_id - ID of the channel
    @param  channel_state - State of the channel
    @result     Absolute time for when the last transition occured
     
    @discussion
        Some clients may need to consume internally the data aggregated
        by the by the reporter object.  This method allows a client to
        retrieve the absolute time stamp for when the last transition into
        a specific state was recorded.
     
    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    uint64_t getStateLastTransitionTime(uint64_t channel_id, uint64_t state_id);
    
/*! @function   [DEPRECATED] IOStateReporter::getStateLastChannelUpdateTime
    @abstract   Deprecated accessor for last time a channel was auto-updated
     
    @param  channel_id - ID of the channel
    @result     Absolute time for last time the channel was updated
     
    @discussion
        If a channel has had ::setState() called on it, calls such as
        getStateResidencyTime() or updateReport() will update time in the
        current state and update an internal "last channel update time."
        Because clients have no way to interlock with those methods, there
        is no sensible way to use this method and it will be removed in
        a future release.
 
    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    uint64_t getStateLastChannelUpdateTime(uint64_t channel_id) __deprecated;
    
/*! @function   IOStateReporter::free
    @abstract   Releases the object and all its resources.
     
    @discussion
        ::free() assumes that init() has completed.  Clients should use
        the static ::with() methods to obtain fully-initialized reporter
        instances.
     
    Locking: same-instance concurrency UNSAFE
*/
    virtual void free(void) APPLE_KEXT_OVERRIDE;
    
protected:

/*! @function   IOStateReporter::initWith
    @abstract   Instance method implementation called by ::with
    
    @discussion 
        See description of parameters above
*/
    virtual bool initWith(IOService *reportingService,
                          IOReportCategories categories,
                          int16_t nstates, IOReportUnits unit);

    
/*! @function   IOStateReporter::handleSwapPrepare
    @abstract   _swap* = <IOStateReporter-specific per-channel buffers>

    @function   IOStateReporter::handleAddChannelSwap
    @abstract   swap in IOStateReporter's variables

    @function   IOStateReporter::handleSwapCleanup
    @abstract   clean up unused buffers in _swap* 
     
    [see IOReporter::handle*Swap* for more info]
*/
    virtual IOReturn handleSwapPrepare(int newNChannels) APPLE_KEXT_OVERRIDE;
    virtual IOReturn handleAddChannelSwap(uint64_t channel_id,
                                          const OSSymbol *symChannelName) APPLE_KEXT_OVERRIDE;
    virtual void handleSwapCleanup(int swapNChannels) APPLE_KEXT_OVERRIDE;
    
/*! @function   IOStateReporter::updateChannelValues
    @abstract   Update accounting of time spent in current state

    @param  channel_index - internal index of the channel
    @result     appropriate IOReturn code
    
    @discussion
        Internal State reporter method to account for the time spent in
        the current state when updateReport() is called on the reporter's
        channels.

    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    virtual IOReturn updateChannelValues(int channel_index) APPLE_KEXT_OVERRIDE;

/*! @function   IOStateReporter::setStateByIndices
    @abstract   update a channel state without validating channel_id
     
    @param  channel_index - 0..<nChannels>, available from getChannelIndex()
    @param  new_state - New state for the channel
    @param  last_intransition - to remove: time of most recent entry
    @param  prev_state_residency - to remove: time spent in previous state 
    @result     Appropriate IOReturn code
     
    @discussion
        Locked version of IOReporter::setStateByIndices().  This method may be
        overriden by sub-classes.
 
    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    virtual IOReturn handleSetStateByIndices(int channel_index,
                                             int new_state_index,
                                             uint64_t last_intransition,
                                             uint64_t prev_state_residency);
    
/*! @function   IOStateReporter::setStateID
    @abstract   Assign a non-default ID to a state
     
    @param  channel_id - ID of channel containing the state in question
    @param  state_index - index of state to give an ID: [0..(nstates-1)]
    @param  state_id - 64-bit state ID, for ASCII, use IOREPORT_MAKEID
     
    @result     Appropriate IOReturn code
     
    @discussion
        Locked version of IOReporter::setStateID(). This method may be
        overriden by sub-classes
 
    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    virtual IOReturn handleSetStateID(uint64_t channel_id,
                                      int state_index,
                                      uint64_t state_id);
    
/*! @function   IOStateReporter::handleOverrideChannelStateByIndices
    @abstract   Overrides state data for a channel with passed arguments
     
    @param  channel_index - index of the channel which state is to be updated
    @param  state_index - index of the state id for the channel
    @param  time_in_state - time used as new total time in state
    @param  intransitions - total number of transitions into state
    @param  last_intransition - mach_absolute_time of most recent entry (opt)
    @result     Appropriate IOReturn code
     
    @discussion
        Locked version of IOReporter::overrideChannelState().  This method
        may be overriden by sub-classes.
     
    Locking: Caller must ensure that the reporter (data) lock is held.
*/
    virtual IOReturn handleOverrideChannelStateByIndices(int channel_index,
                                             int state_index,
                                             uint64_t time_in_state,
                                             uint64_t intransitions,
                                             uint64_t last_intransition = 0);

/*! @function   IOStateReporter::handleIncrementChannelStateByIndices
    @abstract   Updates state data for a channel with passed arguments
     
    @param  channel_index - index of the channel which state is to be updated
    @param  state_index - index of the state id for the channel
    @param  time_in_state - time used as new total time in state
    @param  intransitions - total number of transitions into state
    @param  last_intransition - mach_absolute_time of most recent entry (opt)
    @result     Appropriate IOReturn code
 
    @discussion
        Locked version of IOReporter::incrementChannelState(). This method
        may be overriden by sub-classes.
     
    Locking: Caller must ensure that the reporter (data) lock is held.
*/    
    virtual IOReturn handleIncrementChannelStateByIndices(int channel_index,
                                              int state_index,
                                              uint64_t time_in_state,
                                              uint64_t intransitions,
                                              uint64_t last_intransition = 0);
private:

    int            *_currentStates;         // current states (per chonnel)
    uint64_t       *_lastUpdateTimes;       // most recent auto-update
    
    // Required for swapping inside addChannel
    int            *_swapCurrentStates;
    uint64_t       *_swapLastUpdateTimes;

enum valueSelector {
    kInTransitions,
    kResidencyTime,
    kLastTransitionTime
};
    uint64_t _getStateValue(uint64_t channel_id,
                            uint64_t state_id,
                            enum valueSelector value);

    IOReturn _getStateIndices(uint64_t channel_id,
                              uint64_t state_id,
                              int *channel_index,
                              int *state_index);

};


/*!
    @class      IOHistogramReporter
    @abstract   Report histograms of values
    @discussion
        Each IOHistogramReporter can report one histogram representing
        how a given value has changed over time.
*/
class IOHistogramReporter : public IOReporter
{
    OSDeclareDefaultStructors(IOHistogramReporter);
    
public:
/*! @function   IOHistogramReporter::with
    @abstract   Initializes the IOHistogramReporter instance variables and data structures

    @param  reportingService - The I/O Kit service for this reporter's channels
    @param  categories - The categories in which the report should be classified
    @param  channelID - uint64_t channel identifier
    @param  channelName - rich channel name as char*
    @param  unit - The unit for the quantity recorded by the reporter object
    @param  nSegments - Number of segments to be extracted from the config data structure
    @param  config - Histograms require the caller to pass a configuration by segments
    @result     an instance of the IOSimpleReporter object or NULL on error
    
    @discussion
        Creates an instance of histogram reporter object.

FIXME: need more explanation of the config

        IOHistogramReporter currently only supports a single channel.

 
 */
    static IOHistogramReporter* with(IOService *reportingService,
                                     IOReportCategories categories,
                                     uint64_t channelID,
                                     const char *channelName,
                                     IOReportUnits unit,
                                     int nSegments,
                                     IOHistogramSegmentConfig *config);

/*! @function   IOHistogramReporter::addChannel
    @abstract   Override IOReporter::addChannel(*) to return an error

    @result     kIOReturnUnsupported - doesn't support adding channels
*/
    IOReturn addChannel(uint64_t channelID, const char *channelName = NULL) {
            return kIOReturnUnsupported;
    }
        
/*! @function   IOHistogramReporter::tallyValue
    @abstract   Add a new value to the histogram

    @param  value - new value to add to the histogram
    @result     the index of the affected bucket, or -1 on error
 
    @discussion 
        The histogram reporter determines in which bucket the value
        falls and increments it.  The lowest and highest buckets
        extend to negative and positive infinity, respectively.

    Locking: same-instance concurrency SAFE, WILL NOT BLOCK
*/
    int tallyValue(int64_t value);

/*! @function   IOHistogramReporter::free
    @abstract   Releases the object and all its resources.
     
    @discussion
       ::free() assumes that init() has completed.  Clients should use
       the static ::with() methods to obtain fully-initialized reporter
       instances.
     
    Locking: same-instance concurrency UNSAFE
*/
    virtual void free(void) APPLE_KEXT_OVERRIDE;

protected:

/*! @function   IOHistogramReporter::initWith
    @abstract   instance method implementation called by ::with

    @discussion
        See description of parameters above
*/
    virtual bool initWith(IOService *reportingService,
                          IOReportCategories categories,
                          uint64_t channelID,
                          const OSSymbol *channelName,
                          IOReportUnits unit,
                          int nSegments,
                          IOHistogramSegmentConfig  *config);
    
/*! @function   IOHistogramReporter::handleCreateLegend
    @abstract   Builds an IOReporting legend entry representing the channels of this reporter.
     
    @result     An IOReportLegendEntry or NULL on failure
     
    @discussion
        The returned legend entry may be appended to kIOReportLegendKey
        to be published by the caller in the IORegistry.  See the
        IOReportLegend class for more details.
     
    Locking: same-instance concurrency SAFE, MAY BLOCK
*/
    IOReportLegendEntry* handleCreateLegend(void) APPLE_KEXT_OVERRIDE;
    
    
private:
    
    int                         _segmentCount;
    int64_t                    *_bucketBounds;
    int                         _bucketCount;
    IOHistogramSegmentConfig   *_histogramSegmentsConfig;
};


/***********************************/
/***** 4. IOReportLegend Class *****/
/***********************************/

/*!
    @class      IOReportLegend
    @abstract   combine legend entries into a complete legend
    @discussion
        IOReportLegend adds metadata to legend entries and combines them
        into a single OSArray that can be published under the
        kIOReportLegendKey property in the I/O Kit registry.
*/
class IOReportLegend : public OSObject
{
    OSDeclareDefaultStructors(IOReportLegend);
    
public:
/*! @function   IOReportLegend::with
    @abstract   Create an instance of IOReportLegend

    @param  legend - OSArray of the legend possibly already present in registry
    @result     an instance of IOReportLegend, or NULL on failure
 
    @discussion 
        An IOReporting legend (an OSArray of legend entries) may be already
        present in the IORegistry.  Thus the recommended way to publish
        new entries is to append to any existing array as follows:
        1. call getProperty(kIOReportLegendKey) to get an existing legend.

        2a. If it exists
        - OSDynamicCast to OSArray
        - and pass it to ::with()
        IOReportLegend *legendMaker = IOReportLegend::with(legend);
        The provided array is retained by IOReportLegend.

        2b. If no legend already exists in the registry, pass NULL
        IOReportLegend *legend = IOReportLegend::with(NULL);
        This latter invocation will cause IOReportLegend to create a new
        array internally (also holding one reference).

        At the cost of some registry churn, the static
        IOReportLegend::addReporterLegend() will handle the above, removing
        the need for any direct use of the IOReportLegend class.
*/
    static IOReportLegend* with(OSArray *legend);
    
/*! @function   IOReportLegend::addLegendEntry
    @abstract   Add a new legend entry

    @param  legendEntry - entry to be added to the internal legend array
    @param  groupName - primary group name for this entry
    @param  subGroupName - secondary group name for this entry
    @result     appropriate IOReturn code
 
    @discussion 
        The entry will be retained as an element of the internal array.
        Legend entries are available from reporter objects.  Entries
        represent some number of channels with similar properties (such
        as group and sub-group).  Multiple legend entries with the same
        group names will be aggregated in user space.

        Drivers that instantiate their reporter objects in response to
        IOService::configureReport(kIOReportDisable) will need to create
        temporary reporter objects for the purpose of creating their
        legend entries.  User-space legends are tracked by 12836893.
*/
    IOReturn addLegendEntry(IOReportLegendEntry *legendEntry,
                            const char *groupName,
                            const char *subGroupName);
                            
/*! @function   IOReportLegend::addReporterLegend
    @abstract   Add a legend entry from a reporter object

    @param  reportingService - IOService data provider into the reporter object
    @param  reporter - IOReporter to use to extract and append the legend
    @param  groupName - primary group name for this entry
    @param  subGroupName - secondary group name for this entry
    @result     appropriate IOReturn code
     
    @discussion
        An IOReportLegendEntry will be created internally to this method from 
        the IOReporter object passed in argument. The entry will be released
        internally after being appended to the IOReportLegend object.
        Legend entries are available from reporter objects.  Entries
        represent some number of channels with similar properties (such
        as group and sub-group).  Multiple legend entries with the same
        group names will be aggregated in user space.
     
        Drivers that instantiate their reporter objects in response to
        IOService::configureReport(kIOReportDisable) will need to create
        temporary reporter objects for the purpose of creating their
        legend entries.  User-space legends are tracked by 12836893.

        The static version of addReporterLegend adds the reporter's legend
        directly to reportingService's kIOReportLegendKey.  It is not
        possible to safely update kIOReportLegendKey from multiple threads.

        Locking: same-reportingService and same-IORLegend concurrency UNSAFE
*/
    IOReturn addReporterLegend(IOReporter *reporter,
                               const char *groupName,
                               const char *subGroupName);
    
    static  IOReturn addReporterLegend(IOService *reportingService,
                                       IOReporter *reporter,
                                       const char *groupName,
                                       const char *subGroupName);

/*! @function   IOReportLegend::getLegend
    @abstract   Accessor method to get the legend array

    @result     Returns the OSObject holding the legend to be published by the driver
    @discussion 
        This array will include all legend entries added to the object.
*/
    OSArray* getLegend(void);

/*! @function   IOReportLegend::free
    @abstract   Frees the IOReportLegend object

    @discussion 
        ::free() cleans up the reporter and anything it allocated.

        ::free() releases the internal array (which was either passed
        to ::with() or created as a result of ::with(NULL)).  Assuming
        the caller extracted the array with getLegend() and published it
        in the I/O Kit registry, its ownership will now be with the
        registry.
*/
    void free(void) APPLE_KEXT_OVERRIDE;
    

    
protected:
    
private:
    
    OSArray     *_reportLegend;
    
    IOReturn initWith(OSArray *legend);
    
/*! @function   IOReportLegend::organizeLegend
    @abstract   Sets up the legend entry, organizing it with group and sub-group names

    @param  groupName - Primary group name
    @param  subGroupName - Secondary group name
    @result     IOReturn code
    
    @discussion
*/
    IOReturn organizeLegend(IOReportLegendEntry *legendEntry,
                            const OSSymbol *groupName,
                            const OSSymbol *subGroupName);

// FUTURE POSSIBILITY (NOT IMPLEMENTED!)
/*! @function   IOReportLegend::createReporters
    @abstract   Creates as many IOReporter objects as the legend contains

    @param  legend - OSArray legend object containing the description of all reporters
            the driver is able to address
    @param  reporter - OSSet of reporter objects created by this call
    @result     IOReturn code kIOReturnSuccess if successful

    @discussion
        NOT SUPPORTED at the time of writing
        Convenience method to create all the driver's reporter objects from a legend.
        Can be used when a legend is made public through the IORegistry but IOReporter
        objects have not yet been created to save memory, waiting for observers.
        Upon a call to configureReport via the IOService method, a driver could
        create all reporter objects on the fly using this function.
*/
    // For Future IOReporterManager...
    // static IOReturn createReporters(requestedChannels, legend);
};

#endif  /* ! _IOKERNEL_REPORTERS_H_ */
