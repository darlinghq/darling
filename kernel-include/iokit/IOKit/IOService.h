/*
 * Copyright (c) 1998-2014 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1998,1999 Apple Computer, Inc.  All rights reserved. 
 *
 * HISTORY
 *
 */
/*!
    @header
    This header contains the definition of the IOService class.  IOService is the sole direct subclass of IORegistryEntry and is the base class of almost all I/O Kit family superclasses.  IOService defines methods that support the life cycle of I/O Kit drivers.  For more information on IOService, see {@linkdoc //apple_ref/doc/uid/TP0000011 I/O Kit Fundamentals}.

    @seealso //apple_ref/doc/header/IORegistryEntry.h IORegistryEntry
*/

#ifndef _IOKIT_IOSERVICE_H
#define _IOKIT_IOSERVICE_H

#include <IOKit/IORegistryEntry.h>
#include <IOKit/IOReturn.h>
#include <IOKit/IODeviceMemory.h>
#include <IOKit/IONotifier.h>
#include <IOKit/IOLocks.h>

#include <IOKit/IOKitDebug.h>
#include <IOKit/IOInterrupts.h>

#include <IOKit/pwr_mgt/IOPMpowerState.h>
#include <IOKit/IOServicePM.h>
#include <IOKit/IOReportTypes.h>

extern "C" {
#include <kern/thread_call.h>
}

#ifndef UINT64_MAX
#define UINT64_MAX        18446744073709551615ULL
#endif


enum {
    kIODefaultProbeScore    = 0
};

// masks for getState()
enum {
    kIOServiceInactiveState = 0x00000001,
    kIOServiceRegisteredState   = 0x00000002,
    kIOServiceMatchedState  = 0x00000004,
    kIOServiceFirstPublishState = 0x00000008,
    kIOServiceFirstMatchState   = 0x00000010
};

enum {
    // options for registerService()
    kIOServiceExclusive     = 0x00000001,

    // options for terminate()
    kIOServiceRequired      = 0x00000001,
    kIOServiceTerminate     = 0x00000004,

    // options for registerService() & terminate()
    kIOServiceSynchronous   = 0x00000002,
    // options for registerService()
    kIOServiceAsynchronous  = 0x00000008
};

// options for open()
enum {
    kIOServiceSeize     = 0x00000001,
    kIOServiceFamilyOpenOptions = 0xffff0000
};

// options for close()
enum {
    kIOServiceFamilyCloseOptions = 0xffff0000
};

typedef void * IONotificationRef;

extern const IORegistryPlane *  gIOServicePlane;
extern const IORegistryPlane *  gIOPowerPlane;

extern const OSSymbol *     gIOResourcesKey;
extern const OSSymbol *     gIOResourceMatchKey;
extern const OSSymbol *     gIOProviderClassKey;
extern const OSSymbol *     gIONameMatchKey;
extern const OSSymbol *     gIONameMatchedKey;
extern const OSSymbol *     gIOPropertyMatchKey;
extern const OSSymbol *     gIOLocationMatchKey;
extern const OSSymbol *     gIOParentMatchKey;
extern const OSSymbol *     gIOPathMatchKey;
extern const OSSymbol *     gIOMatchCategoryKey;
extern const OSSymbol *     gIODefaultMatchCategoryKey;
extern const OSSymbol *     gIOMatchedServiceCountKey;

extern const OSSymbol *     gIOUserClientClassKey;
extern const OSSymbol *     gIOKitDebugKey;
extern const OSSymbol *     gIOServiceKey;

extern const OSSymbol *     gIOCommandPoolSizeKey;

extern const OSSymbol *     gIOPublishNotification;
extern const OSSymbol *     gIOFirstPublishNotification;
extern const OSSymbol *     gIOMatchedNotification;
extern const OSSymbol *     gIOFirstMatchNotification;
extern const OSSymbol *     gIOTerminatedNotification;

extern const OSSymbol *     gIOGeneralInterest;
extern const OSSymbol *     gIOBusyInterest;
extern const OSSymbol *     gIOOpenInterest;
extern const OSSymbol *     gIOAppPowerStateInterest;
extern const OSSymbol *     gIOPriorityPowerStateInterest;
extern const OSSymbol *     gIOConsoleSecurityInterest;

extern const OSSymbol *     gIODeviceMemoryKey;
extern const OSSymbol *     gIOInterruptControllersKey;
extern const OSSymbol *     gIOInterruptSpecifiersKey;

extern SInt32 IOServiceOrdering( const OSMetaClassBase * inObj1, const OSMetaClassBase * inObj2, void * ref );

typedef void (*IOInterruptAction)( OSObject * target, void * refCon,
                   IOService * nub, int source );

/*! @typedef IOServiceNotificationHandler
    @param target Reference supplied when the notification was registered.
    @param refCon Reference constant supplied when the notification was registered.
    @param newService The IOService object the notification is delivering. It is retained for the duration of the handler's invocation and doesn't need to be released by the handler. */

typedef bool (*IOServiceNotificationHandler)( void * target, void * refCon,
                                  IOService * newService );

typedef bool (*IOServiceMatchingNotificationHandler)( void * target, void * refCon,
                                  IOService * newService,
                                  IONotifier * notifier );

/*! @typedef IOServiceInterestHandler
    @param target Reference supplied when the notification was registered.
    @param refCon Reference constant supplied when the notification was registered.
    @param messageType Type of the message - IOKit defined in IOKit/IOMessage.h or family specific.
    @param provider The IOService object who is delivering the notification. It is retained for the duration of the handler's invocation and doesn't need to be released by the handler.
    @param messageArgument An argument for message, dependent on its type.
    @param argSize Non zero if the argument represents a struct of that size, used when delivering messages outside the kernel. */

typedef IOReturn (*IOServiceInterestHandler)( void * target, void * refCon,
                                              UInt32 messageType, IOService * provider,
                                              void * messageArgument, vm_size_t argSize );

typedef void (*IOServiceApplierFunction)(IOService * service, void * context);
typedef void (*OSObjectApplierFunction)(OSObject * object, void * context);

class IOUserClient;
class IOPlatformExpert;

/*! @class IOService
    @abstract The base class for most I/O Kit families, devices, and drivers.
    @discussion The IOService base class defines APIs used to publish services, instantiate other services based on the existance of a providing service (ie. driver stacking), destroy a service and its dependent stack, notify interested parties of service state changes, and general utility functions useful across all families. 

Types of service are specified with a matching dictionary that describes properties of the service. For example, a matching dictionary might describe any IOUSBDevice (or subclass), an IOUSBDevice with a certain class code, or a IOPCIDevice with a set of matching names or device & vendor IDs. Since the matching dictionary is interpreted by the family which created the service, as well as generically by IOService, the list of properties considered for matching depends on the familiy.

Matching dictionaries are associated with IOService classes by the catalogue, as driver property tables, and also supplied by clients of the notification APIs.

IOService provides matching based on C++ class (via OSMetaClass dynamic casting), registry entry name, a registry path to the service (which includes device tree paths), a name assigned by BSD, or by its location (its point of attachment).

<br><br>Driver Instantiation by IOService<br><br>

Drivers are subclasses of IOService, and their availability is managed through the catalogue. They are instantiated based on the publication of an IOService they use (for example, an IOPCIDevice or IOUSBDevice), or when they are added to  the catalogue and the IOService(s) they use are already available.

When an IOService (the "provider") is published with the @link registerService registerService@/link method, the matching and probing process begins, which is always single threaded per provider. A list of matching dictionaries from the catalog and installed publish notification requests, that successfully match the IOService, is constructed, with ordering supplied by <code>kIOProbeScoreKey</code> ("IOProbeScore") property in the dictionary, or supplied with the notification. 

Each entry in the list is then processed in order - for notifications, the notification is delivered, for driver property tables a lot more happens.

The driver class is instantiated and <code>init()</code> called with its property table. The new driver instance is then attached to the provider, and has its @link probe probe@/link method called with the provider as an argument. The default <code>probe</code> method does nothing but return success, but a driver may implement this method to interrogate the provider to make sure it can work with it. It may also modify its probe score at this time. After probe, the driver is detached and the next in the list is considered (ie. attached, probed, and detached).

When the probing phase is complete, the list consists of successfully probed drivers, in order of their probe score (after adjustment during the @link probe probe@/link call). The list is then divided into categories based on the <code>kIOMatchCategoryKey</code> property ("IOMatchCategory"); drivers without a match category are all considered in one default category. Match categories allow multiple clients of a provider to be attached and started, though the provider may also enforce open/close semantics to gain active access to it.

For each category, the highest scoring driver in that category is attached to the provider, and its @link start start@/link method called. If <code>start</code> is successful, the rest of the drivers in the same match category are discarded, otherwise the next highest scoring driver is started, and so on.

The driver should only consider itself in action when the start method is called, meaning it has been selected for use on the provider, and consuming that particular match category. It should also be prepared to be allocated, probed and freed even if the probe was successful.

After the drivers have all synchronously been started, the installed "matched" notifications that match the registered IOService are delivered.

<br><br>Properties used by IOService<br><br>

    <code>kIOClassKey, extern const OSSymbol * gIOClassKey, "IOClass"</code>
<br>
<br>
Class of the driver to instantiate on matching providers.
<br>
<br>
    <code>kIOProviderClassKey, extern const OSSymbol * gIOProviderClassKey, "IOProviderClass"</code>
<br>
<br>
Class of the provider(s) to be considered for matching, checked with OSDynamicCast so subclasses will also match.
<br>
<br>
    <code>kIOProbeScoreKey, extern const OSSymbol * gIOProbeScoreKey, "IOProbeScore"</code>
<br>
<br>
The probe score initially used to order multiple matching drivers.
<br>
<br>
    <code>kIOMatchCategoryKey, extern const OSSymbol * gIOMatchCategoryKey, "IOMatchCategory"</code>
<br>
<br>
A string defining the driver category for matching purposes. All drivers with no <code>IOMatchCategory</code> property are considered to be in the same default category. Only one driver in a category can be started on each provider.
<br>
<br>
    <code>kIONameMatchKey, extern const OSSymbol * gIONameMatchKey, "IONameMatch"</code>
<br>
A string or collection of strings that match the provider's name. The comparison is implemented with the @link //apple_ref/cpp/instm/IORegistryEntry/compareNames/virtualbool/(OSObject*,OSString**) IORegistryEntry::compareNames@/link method, which supports a single string, or any collection (OSArray, OSSet, OSDictionary etc.) of strings. IOService objects with device tree properties (eg. IOPCIDevice) will also be matched based on that standard's "compatible", "name", "device_type" properties. The matching name will be left in the driver's property table in the <code>kIONameMatchedKey</code> property.
<br>
Examples
<pre>
@textblock
    <key>IONameMatch</key>
    <string>pci106b,7</string>
@/textblock
</pre>

For a list of possible matching names, a serialized array of strings should used, eg.
<pre>
@textblock
    <key>IONameMatch</key>
    <array>
        <string>APPL,happy16</string>
        <string>pci106b,7</string>
    </array>
@/textblock
</pre>

<br>
    <code>kIONameMatchedKey, extern const OSSymbol * gIONameMatchedKey, "IONameMatched"</code>
<br>
The name successfully matched name from the <code>kIONameMatchKey</code> property will be left in the driver's property table as the <code>kIONameMatchedKey</code> property.
<br>
<br>
    <code>kIOPropertyMatchKey, extern const OSSymbol * gIOPropertyMatchKey, "IOPropertyMatch"</code>
<br>
A dictionary of properties that each must exist in the matching IOService and compare successfully with the <code>isEqualTo</code> method.

<pre>
@textblock
    <key>IOPropertyMatch</key>
    <dictionary>
        <key>APPL,happy16</key>
        <string>APPL,meek8</string>
    </dictionary>
@/textblock
</pre>

<br>
    <code>kIOUserClientClassKey, extern const OSSymbol * gIOUserClientClassKey, "IOUserClientClass"</code>
<br>
The class name that the service will attempt to allocate when a user client connection is requested.  First the device nub is queried, then the nub's provider is queried by default.
<br>
<br>
    <code>kIOKitDebugKey, extern const OSSymbol * gIOKitDebugKey, "IOKitDebug"</code>
<br>
Set some debug flags for logging the driver loading process. Flags are defined in <code>IOKit/IOKitDebug.h</code>, but <code>65535</code> works well.*/

struct IOInterruptAccountingData;
struct IOInterruptAccountingReporter;

class IOService : public IORegistryEntry
{
    OSDeclareDefaultStructors(IOService)

protected:
/*! @struct ExpansionData
    @discussion This structure will be used to expand the capablilties of this class in the future.
    */    
    struct ExpansionData {
        uint64_t authorizationID;
        /*
         * Variables associated with interrupt accounting.  Consists of an array
         * (that pairs reporters with opaque "statistics" objects), the count for
         * the array, and a lock to guard both of the former variables.  The lock
         * is necessary as IOReporting will not update reports in a manner that is
         * synchonized with the service (i.e, on a workloop).
         */
        IOLock * interruptStatisticsLock;
        IOInterruptAccountingReporter * interruptStatisticsArray;
        int interruptStatisticsArrayCount;
    };

/*! @var reserved
    Reserved for future use.  (Internal use only)  */
    ExpansionData * reserved;

private:
    IOService *     __provider;
    SInt32      __providerGeneration;
    IOService *     __owner;
    IOOptionBits    __state[2];
    uint64_t        __timeBusy;
    uint64_t        __accumBusy;
    IOServicePM *   pwrMgt;

protected:
    // TRUE once PMinit has been called
    bool            initialized;

public:
    // DEPRECATED
    void *          pm_vars;

public:
    /* methods available in Mac OS X 10.1 or later */
/*! @function requestTerminate
    @abstract Passes a termination up the stack.
    @discussion When an IOService is made inactive the default behavior is to also make any of its clients that have it as their only provider also inactive, in this way recursing the termination up the driver stack. This method allows an IOService object to override this behavior. Returning <code>true</code> from this method when passed a just terminated provider will cause the client to also be terminated.
    @param provider The terminated provider of this object.
    @param options Options originally passed to terminate, plus <code>kIOServiceRecursing</code>.
    @result <code>true</code> if this object should be terminated now that its provider has been. */

    virtual bool requestTerminate( IOService * provider, IOOptionBits options );

/*! @function willTerminate
    @abstract Passes a termination up the stack.
    @discussion Notification that a provider has been terminated, sent before recursing up the stack, in root-to-leaf order.
    @param provider The terminated provider of this object.
    @param options Options originally passed to terminate.
    @result <code>true</code>. */

    virtual bool willTerminate( IOService * provider, IOOptionBits options );

/*! @function didTerminate
    @abstract Passes a termination up the stack.
    @discussion Notification that a provider has been terminated, sent after recursing up the stack, in leaf-to-root order.
    @param provider The terminated provider of this object.
    @param options Options originally passed to terminate.
    @param defer If there is pending I/O that requires this object to persist, and the provider is not opened by this object set <code>defer</code> to <code>true</code> and call the <code>IOService::didTerminate()</code> implementation when the I/O completes. Otherwise, leave <code>defer</code> set to its default value of <code>false</code>.
    @result <code>true</code>. */

    virtual bool didTerminate( IOService * provider, IOOptionBits options, bool * defer );

/*! @function nextIdleTimeout
    @availability Mac OS X v10.4 and later
    @abstract Allows subclasses to customize idle power management behavior.
    @discussion Returns the next time that the device should idle into its next lower power state. Subclasses may override for custom idle behavior.
    
    A power managed driver might override this method to provide a more sophisticated idle power off algorithm than the one defined by power management.
    @param currentTime The current time
    @param lastActivity The time of last activity on this device
    @param powerState The device's current power state.
    @result Returns the next time the device should idle off (in seconds, relative to the current time). */

    virtual SInt32 nextIdleTimeout(AbsoluteTime currentTime, 
        AbsoluteTime lastActivity, unsigned int powerState);

/*! @function systemWillShutdown
    @availability Mac OS X v10.5 and later
    @abstract Notifies members of the power plane of system shutdown and restart.
    @discussion This function is called for all members of the power plane in leaf-to-root order. If a subclass needs to wait for a pending I/O, then the call to <code>systemWillShutdown</code> should be postponed until the I/O completes.
    
    Any power managed driver (which has called @link joinPMtree joinPMtree@/link to join the power plane) interested in taking action at system shutdown or restart should override this method.
    @param specifier <code>kIOMessageSystemWillPowerOff</code> or <code>kIOMessageSystemWillRestart</code>. */

    virtual void systemWillShutdown( IOOptionBits specifier );

/*! @function copyClientWithCategory
    @availability Mac OS X v10.6 and later
    @param category An OSSymbol corresponding to an IOMatchCategory matching property.
    @result Returns a reference to the IOService child with the given category. The result should be released by the caller.
*/

    virtual IOService * copyClientWithCategory( const OSSymbol * category );

public:
/*! @function       configureReport
    @abstract       configure IOReporting channels
    @availability   SPI on OS X v10.9 / iOS 7 and later

    @param  channels    - channels to configure
    @param  action      - enable/disable/size, etc
    @param  result      - action-specific returned value
    @param  destination - action-specific default destination
*/
virtual IOReturn configureReport(IOReportChannelList   *channels,
                                 IOReportConfigureAction action,
                                 void                  *result,
                                 void                  *destination);

/*! @function       updateReport
    @abstract       request current data for the specified channels
    @availability   SPI on OS X 10.9 / iOS 7 and later

    @param channels     - channels to be updated
    @param action       - type/style of update
    @param result       - returned details about what was updated
    @param destination  - destination for this update (action-specific)
*/
virtual IOReturn updateReport(IOReportChannelList      *channels,
                              IOReportUpdateAction      action,
                              void                     *result,
                              void                     *destination);

private:
#if __LP64__
    OSMetaClassDeclareReservedUsed(IOService, 0);
    OSMetaClassDeclareReservedUsed(IOService, 1);
    OSMetaClassDeclareReservedUnused(IOService, 2);
    OSMetaClassDeclareReservedUnused(IOService, 3);
    OSMetaClassDeclareReservedUnused(IOService, 4);
    OSMetaClassDeclareReservedUnused(IOService, 5);
    OSMetaClassDeclareReservedUnused(IOService, 6);
    OSMetaClassDeclareReservedUnused(IOService, 7);
#else
    OSMetaClassDeclareReservedUsed(IOService, 0);
    OSMetaClassDeclareReservedUsed(IOService, 1);
    OSMetaClassDeclareReservedUsed(IOService, 2);
    OSMetaClassDeclareReservedUsed(IOService, 3);
    OSMetaClassDeclareReservedUsed(IOService, 4);
    OSMetaClassDeclareReservedUsed(IOService, 5);
    OSMetaClassDeclareReservedUsed(IOService, 6);
    OSMetaClassDeclareReservedUsed(IOService, 7);
#endif

    OSMetaClassDeclareReservedUnused(IOService, 8);
    OSMetaClassDeclareReservedUnused(IOService, 9);
    OSMetaClassDeclareReservedUnused(IOService, 10);
    OSMetaClassDeclareReservedUnused(IOService, 11);
    OSMetaClassDeclareReservedUnused(IOService, 12);
    OSMetaClassDeclareReservedUnused(IOService, 13);
    OSMetaClassDeclareReservedUnused(IOService, 14);
    OSMetaClassDeclareReservedUnused(IOService, 15);
    OSMetaClassDeclareReservedUnused(IOService, 16);
    OSMetaClassDeclareReservedUnused(IOService, 17);
    OSMetaClassDeclareReservedUnused(IOService, 18);
    OSMetaClassDeclareReservedUnused(IOService, 19);
    OSMetaClassDeclareReservedUnused(IOService, 20);
    OSMetaClassDeclareReservedUnused(IOService, 21);
    OSMetaClassDeclareReservedUnused(IOService, 22);
    OSMetaClassDeclareReservedUnused(IOService, 23);
    OSMetaClassDeclareReservedUnused(IOService, 24);
    OSMetaClassDeclareReservedUnused(IOService, 25);
    OSMetaClassDeclareReservedUnused(IOService, 26);
    OSMetaClassDeclareReservedUnused(IOService, 27);
    OSMetaClassDeclareReservedUnused(IOService, 28);
    OSMetaClassDeclareReservedUnused(IOService, 29);
    OSMetaClassDeclareReservedUnused(IOService, 30);
    OSMetaClassDeclareReservedUnused(IOService, 31);
    OSMetaClassDeclareReservedUnused(IOService, 32);
    OSMetaClassDeclareReservedUnused(IOService, 33);
    OSMetaClassDeclareReservedUnused(IOService, 34);
    OSMetaClassDeclareReservedUnused(IOService, 35);
    OSMetaClassDeclareReservedUnused(IOService, 36);
    OSMetaClassDeclareReservedUnused(IOService, 37);
    OSMetaClassDeclareReservedUnused(IOService, 38);
    OSMetaClassDeclareReservedUnused(IOService, 39);
    OSMetaClassDeclareReservedUnused(IOService, 40);
    OSMetaClassDeclareReservedUnused(IOService, 41);
    OSMetaClassDeclareReservedUnused(IOService, 42);
    OSMetaClassDeclareReservedUnused(IOService, 43);
    OSMetaClassDeclareReservedUnused(IOService, 44);
    OSMetaClassDeclareReservedUnused(IOService, 45);
    OSMetaClassDeclareReservedUnused(IOService, 46);
    OSMetaClassDeclareReservedUnused(IOService, 47);

public:
/*! @function getState
    @abstract Accessor for IOService state bits, not normally needed or used outside IOService.
    @result State bits for the IOService, eg. <code>kIOServiceInactiveState</code>, <code>kIOServiceRegisteredState</code>. */

    virtual IOOptionBits getState( void ) const;

/*! @function isInactive
    @abstract Checks if the IOService object has been terminated, and is in the process of being destroyed.
    @discussion When an IOService object is successfully terminated, it is immediately made inactive, which blocks further attach()es, matching or notifications occuring on the object. It remains inactive until the last client closes, and is then finalized and destroyed.
    @result <code>true</code> if the IOService object has been terminated. */

    bool isInactive( void ) const;

    /* Stack creation */

/*! @function registerService
    @abstract Starts the registration process for a newly discovered IOService object.
    @discussion This function allows an IOService subclass to be published and made available to possible clients, by starting the registration process and delivering notifications to registered clients. The object should be completely setup and ready to field requests from clients before <code>registerService</code> is called.
    @param options The default zero options mask is recommended and should be used in most cases. The registration process is usually asynchronous, with possible driver probing and notification occurring some time later. <code>kIOServiceSynchronous</code> may be passed to carry out the matching and notification process for currently registered clients before returning to the caller. */

    virtual void registerService( IOOptionBits options = 0 );

/*! @function probe
    @abstract During an IOService object's instantiation, probes a matched service to see if it can be used.
    @discussion The registration process for an IOService object (the provider) includes instantiating possible driver clients. The <code>probe</code> method is called in the client instance to check the matched service can be used before the driver is considered to be started. Since matching screens many possible providers, in many cases the <code>probe</code> method can be left unimplemented by IOService subclasses. The client is already attached to the provider when <code>probe</code> is called.
    @param provider The registered IOService object that matches a driver personality's matching dictionary.
    @param score Pointer to the current driver's probe score, which is used to order multiple matching drivers in the same match category. It defaults to the value of the <code>IOProbeScore</code> property in the drivers property table, or <code>kIODefaultProbeScore</code> if none is specified. The <code>probe</code> method may alter the score to affect start order.
    @result An IOService instance or zero when the probe is unsuccessful. In almost all cases the value of <code>this</code> is returned on success. If another IOService object is returned, the probed instance is detached and freed, and the returned instance is used in its stead for <code>start</code>. */
    
    virtual IOService * probe(  IOService *     provider,
                                SInt32    *     score );

/*! @function start
    @abstract During an IOService object's instantiation, starts the IOService object that has been selected to run on the provider.
    @discussion The <code>start</code> method of an IOService instance is called by its provider when it has been selected (due to its probe score and match category) as the winning client. The client is already attached to the provider when <code>start</code> is called.<br>Implementations of <code>start</code> must call <code>start</code> on their superclass at an appropriate point. If an implementation of <code>start</code> has already called <code>super::start</code> but subsequently determines that it will fail, it must call <code>super::stop</code> to balance the prior call to <code>super::start</code> and prevent reference leaks.
    @result <code>true</code> if the start was successful; <code>false</code> otherwise (which will cause the instance to be detached and usually freed). */
    
    virtual bool start( IOService * provider );
    
/*! @function stop
    @abstract During an IOService termination, the stop method is called in its clients before they are detached & it is destroyed.
    @discussion The termination process for an IOService (the provider) will call stop in each of its clients, after they have closed the provider if they had it open, or immediately on termination. */

    virtual void stop( IOService * provider );

    /* Open / Close */

/*! @function open
    @abstract Requests active access to a provider.
    @discussion IOService provides generic open and close semantics to track clients of a provider that have established an active datapath. The use of <code>open</code> and @link close close@/link, and rules regarding ownership are family defined, and defined by the @link handleOpen handleOpen@/link and @link handleClose handleClose@/link methods in the provider. Some families will limit access to a provider based on its open state.
    @param forClient Designates the client of the provider requesting the open.
    @param options Options for the open. The provider family may implement options for open; IOService defines only <code>kIOServiceSeize</code> to request the device be withdrawn from its current owner.
    @result <code>true</code> if the open was successful; <code>false</code> otherwise. */

    virtual bool open(   IOService *       forClient,
                         IOOptionBits      options = 0,
                         void *        arg = 0 );

/*! @function close
    @abstract Releases active access to a provider.
    @discussion IOService provides generic open and close semantics to track clients of a provider that have established an active datapath. The use of @link open open@/link and <code>close</code>, and rules regarding ownership are family defined, and defined by the @link handleOpen handleOpen@/link and @link handleClose handleClose@/link methods in the provider.
    @param forClient Designates the client of the provider requesting the close.
    @param options Options available for the close. The provider family may implement options for close; IOService defines none.
    @param arg Family specific arguments which are ignored by IOService. */
    
    virtual void close(  IOService *       forClient,
                         IOOptionBits      options = 0 );
                         
/*! @function isOpen
    @abstract Determines whether a specific, or any, client has an IOService object open.
    @discussion Returns the open state of an IOService object with respect to the specified client, or when it is open by any client.
    @param forClient If non-zero, <codeisOpen</code returns the open state for that client. If zero is passed, <codeisOpen</code returns the open state for all clients.
    @result <codetrue</code if the specific, or any, client has the IOService object open. */

    virtual bool isOpen( const IOService * forClient = 0 ) const;

/*! @function handleOpen
    @abstract Controls the open / close behavior of an IOService object (overrideable by subclasses).
    @discussion IOService calls this method in its subclasses in response to the @link open open@/link method, so the subclass may implement the request. The default implementation provides single owner access to an IOService object via <code>open</code>. The object is locked via @link lockForArbitration lockForArbitration@/link before <code>handleOpen</code> is called.
    @param forClient Designates the client of the provider requesting the open.
    @param options Options for the open, may be interpreted by the implementor of <code>handleOpen</code>.
    @result <code>true</code>if the open was successful; <code>false</code> otherwise. */

    virtual bool handleOpen(    IOService *   forClient,
                                IOOptionBits      options,
                                void *        arg );
                                
/*! @function handleClose
    @abstract Controls the open / close behavior of an IOService object (overrideable by subclasses).
    @discussion IOService calls this method in its subclasses in response to the @link close close@/link method, so the subclass may implement the request. The default implementation provides single owner access to an IOService object via @link open open@/link. The object is locked via @link lockForArbitration lockForArbitration@/link before <code>handleClose</code> is called.
    @param forClient Designates the client of the provider requesting the close.
    @param options Options for the close, may be interpreted by the implementor of @link handleOpen handleOpen@/link. */

    virtual void handleClose(   IOService *       forClient,
                                IOOptionBits      options );
                                
/*! @function handleIsOpen
    @abstract Controls the open / close behavior of an IOService object (overrideable by subclasses).
    @discussion IOService calls this method in its subclasses in response to the @link open open@/link method, so the subclass may implement the request. The default implementation provides single owner access to an IOService object via @link open open@/link. The object is locked via @link lockForArbitration lockForArbitration@/link before <code>handleIsOpen</code> is called.
    @param forClient If non-zero, <code>isOpen</code> returns the open state for that client. If zero is passed, <code>isOpen</code> returns the open state for all clients.
    @result <code>true</code> if the specific, or any, client has the IOService object open. */

    virtual bool handleIsOpen(  const IOService * forClient ) const;

    /* Stacking change */

/*! @function terminate
    @abstract Makes an IOService object inactive and begins its destruction.
    @discussion Registering an IOService object informs possible clients of its existance and instantiates drivers that may be used with it; <code>terminate</code> involves the opposite process of informing clients that an IOService object is no longer able to be used and will be destroyed. By default, if any client has the service open, <code>terminate</code> fails. If the <code>kIOServiceRequired</code> flag is passed however, <code>terminate</code> will be successful though further progress in the destruction of the IOService object will not proceed until the last client has closed it. The service will be made inactive immediately upon successful termination, and all its clients will be notified via their @link message message@/link method with a message of type <code>kIOMessageServiceIsTerminated</code>. Both these actions take place on the caller's thread. After the IOService object is made inactive, further matching or attach calls will fail on it. Each client has its @link stop stop@/link method called upon their close of an inactive IOService object , or on its termination if they do not have it open. After <code>stop</code>, @link detach detach@/link is called in each client. When all clients have been detached, the @link finalize finalize@/link method is called in the inactive service. The termination process is inherently asynchronous because it will be deferred until all clients have chosen to close.
    @param options In most cases no options are needed. <code>kIOServiceSynchronous</code> may be passed to cause <code>terminate</code> to not return until the service is finalized. */

    virtual bool terminate( IOOptionBits options = 0 );

/*! @function finalize
    @abstract Finalizes the destruction of an IOService object.
    @discussion The <code>finalize</code> method is called in an inactive (ie. terminated) IOService object after the last client has detached. IOService's implementation will call @link stop stop@/link, @link close close@/link, and @link detach detach@/link on each provider. When <code>finalize</code> returns, the object's retain count will have no references generated by IOService's registration process.
    @param options The options passed to the @link terminate terminate@/link method of the IOService object are passed on to <code>finalize</code>.
    @result <code>true</code>. */
    
    virtual bool finalize( IOOptionBits options );

/*! @function init
    @abstract Initializes generic IOService data structures (expansion data, etc). */
    virtual bool init( OSDictionary * dictionary = 0 ) APPLE_KEXT_OVERRIDE;

/*! @function init
    @abstract Initializes generic IOService data structures (expansion data, etc). */
    virtual bool init( IORegistryEntry * from,
                       const IORegistryPlane * inPlane ) APPLE_KEXT_OVERRIDE;

/*! @function free
    @abstract Frees data structures that were allocated when power management was initialized on this service. */
    
    virtual void free( void ) APPLE_KEXT_OVERRIDE;

/*! @function lockForArbitration
    @abstract Locks an IOService object against changes in state or ownership.
    @discussion The registration, termination and open / close functions of IOService use <code>lockForArbtration</code> to single-thread access to an IOService object. <code>lockForArbitration</code> grants recursive access to the same thread.
    @param isSuccessRequired If a request for access to an IOService object should be denied if it is terminated, pass <code>false</code>, otherwise pass <code>true</code>. */
    
    virtual bool lockForArbitration( bool isSuccessRequired = true );
    
/*! @function unlockForArbitration
    @abstract Unlocks an IOService obkect after a successful @link lockForArbitration lockForArbitration@/link.
    @discussion A thread granted exclusive access to an IOService object should release it with <code>unlockForArbitration</code>. */
    
    virtual void unlockForArbitration( void );

/*! @function terminateClient
    @abstract Passes a termination up the stack.
    @discussion When an IOService object is made inactive the default behavior is to also make any of its clients that have it as their only provider inactive, in this way recursing the termination up the driver stack. This method allows a terminated  IOService object to override this behavior. Note the client may also override this behavior by overriding its @link terminate terminate@/link method.
    @param client The client of the terminated provider.
    @param options Options originally passed to @link terminate terminate@/link, plus <code>kIOServiceRecursing</code>.
    @result result of the terminate request on the client. */

    virtual bool terminateClient( IOService * client, IOOptionBits options );

    /* Busy state indicates discovery, matching or termination is in progress */

/*! @function getBusyState
    @abstract Returns the <code>busyState</code> of an IOService object.
    @discussion Many activities in IOService are asynchronous. When registration, matching, or termination is in progress on an IOService object, its <code>busyState</code> is increased by one. Change in <code>busyState</code> to or from zero also changes the IOService object's provider's <code>busyState</code> by one, which means that an IOService object is marked busy when any of the above activities is ocurring on it or any of its clients.
    @result The <code>busyState</code> value. */

    virtual UInt32 getBusyState( void );
    
/*! @function adjustBusy
    @abstract Adjusts the <code>busyState</code> of an IOService object.
    @discussion Applies a delta to an IOService object's <code>busyState</code>. A change in the <code>busyState</code> to or from zero will change the IOService object's provider's <code>busyState</code> by one (in the same direction). 
    @param delta The delta to be applied to the IOService object's <code>busyState</code>. */

    virtual void adjustBusy( SInt32 delta );

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
        IOReturn waitQuiet(mach_timespec_t * timeout)
    APPLE_KEXT_DEPRECATED;

/*! @function waitQuiet
    @abstract Waits for an IOService object's <code>busyState</code> to be zero.
    @discussion Blocks the caller until an IOService object is non busy.
    @param timeout The maximum time to wait in nanoseconds. Default is to wait forever.
    @result Returns an error code if Mach synchronization primitives fail, <code>kIOReturnTimeout</code>, or <code>kIOReturnSuccess</code>. */
    
    IOReturn waitQuiet(uint64_t timeout = UINT64_MAX);

    /* Matching */

/*! @function matchPropertyTable
    @abstract Allows a registered IOService object to implement family specific matching.
    @discussion All matching on an IOService object will call this method to allow a family writer to implement matching in addition to the generic methods provided by IOService. The implementer should examine the matching dictionary passed to see if it contains properties the family understands for matching, and use them to match with the IOService object if so. Note that since matching is also carried out by other parts of the I/O Kit, the matching dictionary may contain properties the family does not understand - these should not be considered matching failures.
    @param table The dictionary of properties to be matched against.
    @param score Pointer to the current driver's probe score, which is used to order multiple matching drivers in the same match category. It defaults to the value of the <code>IOProbeScore</code> property in the drivers property table, or <code>kIODefaultProbeScore</code> if none is specified.
    @result <code>false</code> if the family considers the matching dictionary does not match in properties it understands; <code>true</code> otherwise. */

    virtual bool matchPropertyTable( OSDictionary * table,
                                     SInt32       * score );

    virtual bool matchPropertyTable( OSDictionary * table );

/*! @function matchLocation
    @abstract Allows a registered IOService object to direct location matching.
    @discussion By default, a location matching property will be applied to an IOService object's provider. This method allows that behavior to be overridden by families.
    @param client The IOService object at which matching is taking place.
    @result Returns the IOService instance to be used for location matching. */

    virtual IOService * matchLocation( IOService * client );

    /* Resource service */

/*! @function publishResource
    @abstract Uses the resource service to publish a property.
    @discussion The resource service uses IOService's matching and notification to allow objects to be published and found by any I/O Kit client by a global name. <code>publishResource</code> makes an object available to anyone waiting for it or looking for it in the future.
    @param key An OSSymbol key that globally identifies the object.
    @param The object to be published. */

    static void publishResource( const OSSymbol * key, OSObject * value = 0 );

/*! @function publishResource
    @abstract Uses the resource service to publish a property.
    @discussion The resource service uses IOService object's matching and notification to allow objects to be published and found by any I/O Kit client by a global name. <code>publishResource</code> makes an object available to anyone waiting for it or looking for it in the future.
    @param key A C string key that globally identifies the object.
    @param The object to be published. */

    static void publishResource( const char * key, OSObject * value = 0 );
    virtual bool addNeededResource( const char * key );

    /* Notifications */

/*! @function addNotification
    @abstract Deprecated use addMatchingNotification(). Adds a persistant notification handler to be notified of IOService events.
    @discussion IOService will deliver notifications of changes in state of an IOService object to registered clients. The type of notification is specified by a symbol, for example <code>gIOMatchedNotification</code> or <code>gIOTerminatedNotification</code>, and notifications will only include IOService objects that match the supplied matching dictionary. Notifications are ordered by a priority set with <code>addNotification</code>. When the notification is installed, its handler will be called with each of any currently existing IOService objects that are in the correct state (eg. registered) and match the supplied matching dictionary, avoiding races between finding preexisting and new IOService events. The notification request is identified by an instance of an IONotifier object, through which it can be enabled, disabled, or removed. <code>addNotification</code> consumes a retain count on the matching dictionary when the notification is removed.
    @param type An OSSymbol identifying the type of notification and IOService state:
<br>    <code>gIOPublishNotification</code> Delivered when an IOService object is registered.
<br>    <code>gIOFirstPublishNotification</code> Delivered when an IOService object is registered, but only once per IOService instance. Some IOService objects may be reregistered when their state is changed.
<br>    <code>gIOMatchedNotification</code> Delivered when an IOService object has been matched with all client drivers, and they have been probed and started.
<br>    <code>gIOFirstMatchNotification</code> Delivered when an IOService object has been matched with all client drivers, but only once per IOService instance. Some IOService objects may be reregistered when their state is changed.
<br>    <code>gIOTerminatedNotification</code> Delivered after an IOService object has been terminated, during its finalize stage.
    @param matching A matching dictionary to restrict notifications to only matching IOService objects. The dictionary will be released when the notification is removed, consuming the passed-in reference.
    @param handler A C function callback to deliver notifications.
    @param target An instance reference for the callback's use.
    @param ref A reference constant for the callback's use.
    @param priority A constant ordering all notifications of a each type.
    @result An instance of an IONotifier object that can be used to control or destroy the notification request. */

    static IONotifier * addNotification(
                            const OSSymbol * type, OSDictionary * matching,
                            IOServiceNotificationHandler handler,
                            void * target, void * ref = 0,
                            SInt32 priority = 0 )
    APPLE_KEXT_DEPRECATED;

/*! @function addMatchingNotification
    @abstract Adds a persistant notification handler to be notified of IOService events.
    @discussion IOService will deliver notifications of changes in state of an IOService object to registered clients. The type of notification is specified by a symbol, for example <code>gIOMatchedNotification</code> or <code>gIOTerminatedNotification</code>, and notifications will only include IOService objects that match the supplied matching dictionary. Notifications are ordered by a priority set with <code>addNotification</code>. When the notification is installed, its handler will be called with each of any currently existing IOService objects that are in the correct state (eg. registered) and match the supplied matching dictionary, avoiding races between finding preexisting and new IOService events. The notification request is identified by an instance of an IONotifier object, through which it can be enabled, disabled, or removed. <code>addMatchingNotification</code> does not consume a reference on the matching dictionary when the notification is removed, unlike addNotification.
    @param type An OSSymbol identifying the type of notification and IOService state:
<br>    <code>gIOPublishNotification</code> Delivered when an IOService object is registered.
<br>    <code>gIOFirstPublishNotification</code> Delivered when an IOService object is registered, but only once per IOService instance. Some IOService objects may be reregistered when their state is changed.
<br>    <code>gIOMatchedNotification</code> Delivered when an IOService object has been matched with all client drivers, and they have been probed and started.
<br>    <code>gIOFirstMatchNotification</code> Delivered when an IOService object has been matched with all client drivers, but only once per IOService instance. Some IOService objects may be reregistered when their state is changed.
<br>    <code>gIOTerminatedNotification</code> Delivered after an IOService object has been terminated, during its finalize stage.
    @param matching A matching dictionary to restrict notifications to only matching IOService objects. The dictionary is retained while the notification is installed. (Differs from addNotification).
    @param handler A C function callback to deliver notifications.
    @param target An instance reference for the callback's use.
    @param ref A reference constant for the callback's use.
    @param priority A constant ordering all notifications of a each type.
    @result An instance of an IONotifier object that can be used to control or destroy the notification request. */

    static IONotifier * addMatchingNotification(
                            const OSSymbol * type, OSDictionary * matching,
                            IOServiceMatchingNotificationHandler handler,
                            void * target, void * ref = 0,
                            SInt32 priority = 0 );

/*! @function waitForService
    @abstract Deprecated use waitForMatchingService(). Waits for a matching to service to be published.
    @discussion Provides a method of waiting for an IOService object matching the supplied matching dictionary to be registered and fully matched. 
    @param matching The matching dictionary describing the desired IOService object. <code>waitForService</code> consumes one reference of the matching dictionary.
    @param timeout The maximum time to wait.
    @result A published IOService object matching the supplied dictionary. */

    static IOService * waitForService( OSDictionary * matching,
                            mach_timespec_t * timeout = 0);

/*! @function waitForMatchingService
    @abstract Waits for a matching to service to be published.
    @discussion Provides a method of waiting for an IOService object matching the supplied matching dictionary to be registered and fully matched. 
    @param matching The matching dictionary describing the desired IOService object. (Does not consume a reference of the matching dictionary - differs from waitForService() which does consume a reference on the matching dictionary.)
    @param timeout The maximum time to wait in nanoseconds. Default is to wait forever.
    @result A published IOService object matching the supplied dictionary. waitForMatchingService returns a reference to the IOService which should be released by the caller. (Differs from waitForService() which does not retain the returned object.) */

    static IOService * waitForMatchingService( OSDictionary * matching,
                            uint64_t timeout = UINT64_MAX);

/*! @function getMatchingServices
    @abstract Finds the set of current published IOService objects matching a matching dictionary.
    @discussion Provides a method of finding the current set of published IOService objects matching the supplied matching dictionary.   
    @param matching The matching dictionary describing the desired IOService objects.
    @result An instance of an iterator over a set of IOService objects. To be released by the caller. */

    static OSIterator * getMatchingServices( OSDictionary * matching );

/*! @function copyMatchingService
    @abstract Finds one of the current published IOService objects matching a matching dictionary.
    @discussion Provides a method to find one member of the set of published IOService objects matching the supplied matching dictionary.   
    @param matching The matching dictionary describing the desired IOService object.
    @result The IOService object or NULL. To be released by the caller. */

    static IOService * copyMatchingService( OSDictionary * matching );

public:
    /* Helpers to make matching dictionaries for simple cases,
     * they add keys to an existing dictionary, or create one. */

/*! @function serviceMatching
    @abstract Creates a matching dictionary, or adds matching properties to an existing dictionary, that specify an IOService class match.
    @discussion A very common matching criteria for IOService object is based on its class. <code>serviceMatching</code> creates a matching dictionary that specifies any IOService object of a class, or its subclasses. The class is specified by name, and an existing dictionary may be passed in, in which case the matching properties will be added to that dictionary rather than creating a new one.
    @param className The class name, as a const C string. Class matching is successful on IOService objects of this class or any subclass.
    @param table If zero, <code>serviceMatching</code> creates a matching dictionary and returns a reference to it, otherwise the matching properties are added to the specified dictionary.
    @result The matching dictionary created, or passed in, is returned on success, or zero on failure. */

    static OSDictionary * serviceMatching( const char * className,
                                           OSDictionary * table = 0 );

/*! @function serviceMatching
    @abstract Creates a matching dictionary, or adds matching properties to an existing dictionary, that specify an IOService class match.
    @discussion A very common matching criteria for IOService object is based on its class. <code>serviceMatching</code> creates a matching dictionary that specifies any IOService of a class, or its subclasses. The class is specified by name, and an existing dictionary may be passed in, in which case the matching properties will be added to that dictionary rather than creating a new one.
    @param className The class name, as an OSString (which includes OSSymbol). Class matching is successful on IOService objects of this class or any subclass.
    @param table If zero, <code>serviceMatching</code> creates a matching dictionary and returns a reference to it, otherwise the matching properties are added to the specified dictionary.
    @result The matching dictionary created, or passed in, is returned on success, or zero on failure. */

    static OSDictionary * serviceMatching( const OSString * className,
                                           OSDictionary * table = 0 );

/*! @function nameMatching
    @abstract Creates a matching dictionary, or adds matching properties to an existing dictionary, that specify an IOService name match.
    @discussion A very common matching criteria for IOService object is based on its name. <code>nameMatching</code> creates a matching dictionary that specifies any IOService object which responds successfully to the @link //apple_ref/cpp/instm/IORegistryEntry/compareName/virtualbool/(OSString*,OSString**) IORegistryEntry::compareName@/link method. An existing dictionary may be passed in, in which case the matching properties will be added to that dictionary rather than creating a new one.
    @param name The service's name, as a const C string. Name matching is successful on IOService objects that respond successfully to the <code>IORegistryEntry::compareName</code> method.
    @param table If zero, <code>nameMatching</code> creates a matching dictionary and returns a reference to it, otherwise the matching properties are added to the specified dictionary.
    @result The matching dictionary created, or passed in, is returned on success, or zero on failure. */

    static OSDictionary * nameMatching( const char * name,
                                        OSDictionary * table = 0 );

/*! @function nameMatching
    @abstract Creates a matching dictionary, or adds matching properties to an existing dictionary, that specify an IOService name match.
    @discussion A very common matching criteria for IOService object is based on its name. <code>nameMatching</code> creates a matching dictionary that specifies any IOService object which responds successfully to the @link //apple_ref/cpp/instm/IORegistryEntry/compareName/virtualbool/(OSString*,OSString**) IORegistryEntry::compareName@/link method. An existing dictionary may be passed in, in which case the matching properties will be added to that dictionary rather than creating a new one.
    @param name The service's name, as an OSString (which includes OSSymbol). Name matching is successful on IOService objects that respond successfully to the <code>IORegistryEntry::compareName</code> method.
    @param table If zero, <code>nameMatching</code> creates a matching dictionary and returns a reference to it, otherwise the matching properties are added to the specified dictionary.
    @result The matching dictionary created, or passed in, is returned on success, or zero on failure. */

    static OSDictionary * nameMatching( const OSString* name,
                                        OSDictionary * table = 0 );

/*! @function resourceMatching
    @abstract Creates a matching dictionary, or adds matching properties to an existing dictionary, that specify a resource service match.
    @discussion IOService maintains a resource service IOResources that allows objects to be published and found globally in the I/O Kit based on a name, using the standard IOService matching and notification calls.
    @param name The resource name, as a const C string. Resource matching is successful when an object by that name has been published with the <code>publishResource</code> method.
    @param table If zero, <code>resourceMatching</code> creates a matching dictionary and returns a reference to it, otherwise the matching properties are added to the specified dictionary.
    @result The matching dictionary created, or passed in, is returned on success, or zero on failure. */

    static OSDictionary * resourceMatching( const char * name,
                                            OSDictionary * table = 0 );

/*! @function resourceMatching
    @abstract Creates a matching dictionary, or adds matching properties to an existing dictionary, that specify a resource service match.
    @discussion IOService maintains a resource service IOResources that allows objects to be published and found globally in the I/O Kit based on a name, using the standard IOService matching and notification calls.
    @param name The resource name, as an OSString (which includes OSSymbol). Resource matching is successful when an object by that name has been published with the <code>publishResource</code> method.
    @param table If zero, <code>resourceMatching</code> creates a matching dictionary and returns a reference to it, otherwise the matching properties are added to the specified dictionary.
    @result The matching dictionary created, or passed in, is returned on success, or zero on failure. */

    static OSDictionary * resourceMatching( const OSString * name,
                                            OSDictionary * table = 0 );


/*! @function propertyMatching
    @abstract Creates a matching dictionary, or adds matching properties to an existing dictionary, that specify an IOService phandle match.
    @discussion TODO A very common matching criteria for IOService is based on its name. nameMatching will create a matching dictionary that specifies any IOService which respond successfully to the IORegistryEntry method compareName. An existing dictionary may be passed in, in which case the matching properties will be added to that dictionary rather than creating a new one.
    @param key The service's phandle, as a const UInt32. PHandle matching is successful on IOService objects that respond successfully to the IORegistryEntry method compareName.
    @param value The service's phandle, as a const UInt32. PHandle matching is successful on IOService's which respond successfully to the IORegistryEntry method compareName.
    @param table If zero, nameMatching will create a matching dictionary and return a reference to it, otherwise the matching properties are added to the specified dictionary.
    @result The matching dictionary created, or passed in, is returned on success, or zero on failure. */

    static OSDictionary * propertyMatching( const OSSymbol * key, const OSObject * value,
                                            OSDictionary * table = 0 );

/*! @function registryEntryIDMatching
    @abstract Creates a matching dictionary, or adds matching properties to an existing dictionary, that specify a IORegistryEntryID match.
    @discussion <code>registryEntryIDMatching</code> creates a matching dictionary that specifies the IOService object with the assigned registry entry ID (returned by <code>IORegistryEntry::getRegistryEntryID()</code>). An existing dictionary may be passed in, in which case the matching properties will be added to that dictionary rather than creating a new one.
    @param name The service's ID. Matching is successful on the IOService object that return that ID from the <code>IORegistryEntry::getRegistryEntryID()</code> method.
    @param table If zero, <code>registryEntryIDMatching</code> creates a matching dictionary and returns a reference to it, otherwise the matching properties are added to the specified dictionary.
    @result The matching dictionary created, or passed in, is returned on success, or zero on failure. */

    static OSDictionary * registryEntryIDMatching( uint64_t entryID,
                                                   OSDictionary * table = 0 );


/*! @function addLocation
    @abstract Adds a location matching property to an existing dictionary.
    @discussion This function creates matching properties that specify the location of a IOService object, as an embedded matching dictionary. This matching will be successful on an IOService object that attached to an IOService object which matches this location matching dictionary.
    @param table The matching properties are added to the specified dictionary, which must be non-zero.
    @result The location matching dictionary created is returned on success, or zero on failure. */

    static OSDictionary * addLocation( OSDictionary * table );

    /* Helpers for matching dictionaries. */

/*! @function compareProperty
    @abstract Compares a property in a matching dictionary with an IOService object's property table.
    @discussion This is a helper function to aid in implementing @link matchPropertyTable matchPropertyTable@/link. If the property specified by <code>key</code> exists in the matching dictionary, it is compared with a property of the same name in the IOService object's property table. The comparison is performed with the <code>isEqualTo</code> method. If the property does not exist in the matching table, success is returned. If the property exists in the matching dictionary but not the IOService property table, failure is returned.
    @param matching The matching dictionary, which must be non-zero.
    @param key The dictionary key specifying the property to be compared, as a C string.
    @result <code>true</code> if the property does not exist in the matching table. If the property exists in the matching dictionary but not the IOService property table, failure is returned. Otherwise the result of calling the property from the matching dictionary's <code>isEqualTo</code> method with the IOService property as an argument is returned. */

    virtual bool compareProperty(   OSDictionary   * matching,
                                    const char     * key );
/*! @function compareProperty
    @abstract Compares a property in a matching dictionary with an IOService object's property table.
    @discussion This is a helper function to aid in implementing @link matchPropertyTable matchPropertyTable@/link. If the property specified by <code>key</code> exists in the matching dictionary, it is compared with a property of the same name in the IOService object's property table. The comparison is performed with the <code>isEqualTo</code> method. If the property does not exist in the matching table, success is returned. If the property exists in the matching dictionary but not the IOService property table, failure is returned.
    @param matching The matching dictionary, which must be non-zero.
    @param key The dictionary key specifying the property to be compared, as an OSString (which includes OSSymbol).
    @result <code>true</code> if the property does not exist in the matching table. If the property exists in the matching dictionary but not the IOService property table, failure is returned. Otherwise the result of calling the property from the matching dictionary's <code>isEqualTo</code> method with the IOService property as an argument is returned. */

    virtual bool compareProperty(   OSDictionary   * matching,
                                    const OSString * key );

/*! @function compareProperties
    @abstract Compares a set of properties in a matching dictionary with an IOService object's property table.
    @discussion This is a helper function to aid in implementing @link matchPropertyTable matchPropertyTable@/link. A collection of dictionary keys specifies properties in a matching dictionary to be compared, with <code>compareProperty</code>, with an IOService object's property table, if <code>compareProperty</code> returns <code>true</code> for each key, success is returned; otherwise failure.
    @param matching The matching dictionary, which must be non-zero.
    @param keys A collection (eg. OSSet, OSArray, OSDictionary) which should contain OSStrings (or OSSymbols) that specify the property keys to be compared.
    @result Success if <code>compareProperty</code> returns <code>true</code> for each key in the collection; otherwise failure. */

    virtual bool compareProperties( OSDictionary   * matching,
                                    OSCollection   * keys );

    /* Client / provider accessors */

/*! @function attach
    @abstract Attaches an IOService client to a provider in the I/O Registry.
    @discussion This function called in an IOService client enters the client into the I/O Registry as a child of the provider in the service plane. The provider must be active or the attach will fail. Multiple attach calls to the same provider are no-ops and return success. A client may be attached to multiple providers. Entering an object into the I/O Registry retains both the client and provider until they are detached.
    @param provider The IOService object which will serve as this object's provider.
    @result <code>false</code> if the provider is inactive or on a resource failure; otherwise <code>true</code>. */

    virtual bool attach( IOService * provider );
    
/*! @function detach
    @abstract Detaches an IOService client from a provider in the I/O Registry.
    @discussion This function called in an IOService client removes the client as a child of the provider in the service plane of the I/O Registry. If the provider is not a parent of the client this is a no-op, otherwise the I/O Registry releases both the client and provider.
    @param provider The IOService object to detach from. */

    virtual void detach( IOService * provider );

/*! @function getProvider
    @abstract Returns an IOService object's primary provider.
    @discussion This function called in an IOService client will return the provider to which it was first attached. Because the majority of IOService objects have only one provider, this is a useful simplification and also supports caching of the provider when the I/O Registry is unchanged.
    @result The first provider of the client, or zero if the IOService object is not attached into the I/O Registry. The provider is retained while the client is attached, and should not be released by the caller. */

    virtual IOService * getProvider( void ) const;

/*! @function getWorkLoop
    @abstract Returns the current work loop or <code>provider->getWorkLoop</code>.
    @discussion This function returns a valid work loop that a client can use to add an IOCommandGate to. The intention is that an IOService client has data that needs to be protected but doesn't want to pay the cost of a dedicated thread. This data has to be accessed from a provider's call-out context as well. So to achieve both of these goals the client creates an IOCommandGate to lock access to his data but he registers it with the provider's work loop, i.e. the work loop which will make the completion call-outs. This avoids a potential deadlock because the work loop gate uses a recursive lock, which allows the same lock to be held multiple times by a single thread.
    @result A work loop, either the current work loop or it walks up the @link getProvider getProvider@/link chain calling <code>getWorkLoop</code>. Eventually it will reach a valid work loop-based driver or the root of the I/O tree, where it will return a system-wide work loop. Returns 0 if it fails to find (or create) a work loop.*/

    virtual IOWorkLoop * getWorkLoop() const;

/*! @function getProviderIterator
    @abstract Returns an iterator over an IOService object's providers.
    @discussion For those few IOService objects that obtain service from multiple providers, this method supplies an iterator over a client's providers. 
    @result An iterator over the providers of the client, or zero if there is a resource failure. The iterator must be released when the iteration is finished. All objects returned by the iteration are retained while the iterator is valid, though they may no longer be attached during the iteration. */

    virtual OSIterator * getProviderIterator( void ) const;

/*! @function getOpenProviderIterator
    @abstract Returns an iterator over an client's providers that are currently opened by the client.
    @discussion For those few IOService objects that obtain service from multiple providers, this method supplies an iterator over a client's providers, locking each in turn with @link lockForArbitration lockForArbitration@/link and returning those that have been opened by the client. 
    @result An iterator over the providers the client has open, or zero if there is a resource failure. The iterator must be released when the iteration is finished. All objects returned by the iteration are retained while the iterator is valid, and the current entry in the iteration is locked with <code>lockForArbitration</code>, protecting it from state changes. */

    virtual OSIterator * getOpenProviderIterator( void ) const;

/*! @function getClient
    @abstract Returns an IOService object's primary client.
    @discussion This function called in an IOService provider will return the first client to attach to it. For IOService objects which have only only one client, this may be a useful simplification.
    @result The first client of the provider, or zero if the IOService object is not attached into the I/O Registry. The client is retained while it is attached, and should not be released by the caller. */

    virtual IOService * getClient( void ) const;

/*! @function getClientIterator
    @abstract Returns an iterator over an IOService object's clients.
    @discussion For IOService objects that may have multiple clients, this method supplies an iterator over a provider's clients. 
    @result An iterator over the clients of the provider, or zero if there is a resource failure. The iterator must be released when the iteration is finished. All objects returned by the iteration are retained while the iterator is valid, though they may no longer be attached during the iteration. */

    virtual OSIterator * getClientIterator( void ) const;

/*! @function getOpenClientIterator
    @abstract Returns an iterator over a provider's clients that currently have opened the provider.
    @discussion For IOService objects that may have multiple clients, this method supplies an iterator over a provider's clients, locking each in turn with @link lockForArbitration lockForArbitration@/link and returning those that have opened the provider. 
    @result An iterator over the clients that have opened the provider, or zero if there is a resource failure. The iterator must be released when the iteration is finished. All objects returned by the iteration are retained while the iterator is valid, and the current entry in the iteration is locked with <code>lockForArbitration</code>, protecting it from state changes. */

    virtual OSIterator * getOpenClientIterator( void ) const;

/*! @function callPlatformFunction
    @abstract Calls the platform function with the given name.
    @discussion The platform expert or other drivers may implement various functions to control hardware features.  <code>callPlatformFunction</code> allows any IOService object to access these functions. Normally <code>callPlatformFunction</code> is called on a service's provider. The provider services the request or passes it to its provider. The system's IOPlatformExpert subclass catches functions it knows about and redirects them into other parts of the service plane. If the IOPlatformExpert subclass cannot execute the function, the base class is called. The IOPlatformExpert base class attempts to find a service to execute the function by looking up the function name in an IOResources name space. A service may publish a service using <code>publishResource(functionName, this)</code>. If no service can be found to execute the function an error is returned.
    @param functionName Name of the function to be called. When <code>functionName</code> is a C string, <code>callPlatformFunction</code> converts the C string to an OSSymbol and calls the OSSymbol version of <code>callPlatformFunction</code>. This process can block and should not be used from an interrupt context.
    @param waitForFunction If <code>true</code>, <code>callPlatformFunction</code> will not return until the function has been called.
    @result An IOReturn code; <code>kIOReturnSuccess</code> if the function was successfully executed, <code>kIOReturnUnsupported</code> if a service to execute the function could not be found. Other return codes may be returned by the function.*/

    virtual IOReturn callPlatformFunction( const OSSymbol * functionName,
                                           bool waitForFunction,
                                           void *param1, void *param2,
                                           void *param3, void *param4 );

    virtual IOReturn callPlatformFunction( const char * functionName,
                                           bool waitForFunction,
                                           void *param1, void *param2,
                                           void *param3, void *param4 );


    /* Some accessors */

/*! @function getPlatform
    @abstract Returns a pointer to the platform expert instance for the computer.
    @discussion This method provides an accessor to the platform expert instance for the computer. 
    @result A pointer to the IOPlatformExpert instance. It should not be released by the caller. */

    static IOPlatformExpert * getPlatform( void );

/*! @function getPMRootDomain
    @abstract Returns a pointer to the power management root domain instance for the computer.
    @discussion This method provides an accessor to the power management root domain instance for the computer. 
    @result A pointer to the power management root domain instance. It should not be released by the caller. */

    static class IOPMrootDomain * getPMRootDomain( void );

/*! @function getServiceRoot
    @abstract Returns a pointer to the root of the service plane.
    @discussion This method provides an accessor to the root of the service plane for the computer. 
    @result A pointer to the IOService instance at the root of the service plane. It should not be released by the caller. */

    static IOService * getServiceRoot( void );

/*! @function getResourceService
    @abstract Returns a pointer to the IOResources service.
    @discussion IOService maintains a resource service IOResources that allows objects to be published and found globally in the I/O Kit based on a name, using the standard IOService matching and notification calls.
    @result A pointer to the IOResources instance. It should not be released by the caller. */

    static IOService * getResourceService( void );

    /* Allocate resources for a matched service */

/*! @function getResources
    @abstract Allocates any needed resources for a published IOService object before clients attach.
    @discussion This method is called during the registration process for an IOService object if there are successful driver matches, before any clients attach. It allows for lazy allocation of resources to an IOService object when a matching driver is found.
    @result An IOReturn code; <code>kIOReturnSuccess</code> is necessary for the IOService object to be successfully used, otherwise the registration process for the object is halted. */
    
    virtual IOReturn getResources( void );

    /* Device memory accessors */

/*! @function getDeviceMemoryCount
    @abstract Returns a count of the physical memory ranges available for a device.
    @discussion This method returns the count of physical memory ranges, each represented by an IODeviceMemory instance, that have been allocated for a memory mapped device.
    @result An integer count of the number of ranges available. */

    virtual IOItemCount getDeviceMemoryCount( void );

/*! @function getDeviceMemoryWithIndex
    @abstract Returns an instance of IODeviceMemory representing one of a device's memory mapped ranges.
    @discussion This method returns a pointer to an instance of IODeviceMemory for the physical memory range at the given index for a memory mapped device.
    @param index An index into the array of ranges assigned to the device.
    @result A pointer to an instance of IODeviceMemory, or zero if the index is beyond the count available. The IODeviceMemory is retained by the provider, so is valid while attached, or while any mappings to it exist. It should not be released by the caller. See also @link mapDeviceMemoryWithIndex mapDeviceMemoryWithIndex@/link, which creates a device memory mapping. */

    virtual IODeviceMemory * getDeviceMemoryWithIndex( unsigned int index );

/*! @function mapDeviceMemoryWithIndex
    @abstract Maps a physical range of a device.
    @discussion This method creates a mapping for the IODeviceMemory at the given index, with <code>IODeviceMemory::map(options)</code>. The mapping is represented by the returned instance of IOMemoryMap, which should not be released until the mapping is no longer required.
    @param index An index into the array of ranges assigned to the device.
    @result An instance of IOMemoryMap, or zero if the index is beyond the count available. The mapping should be released only when access to it is no longer required. */

    virtual IOMemoryMap * mapDeviceMemoryWithIndex( unsigned int index,
                                                    IOOptionBits options = 0 );

/*! @function getDeviceMemory
    @abstract Returns the array of IODeviceMemory objects representing a device's memory mapped ranges.
    @discussion This method returns an array of IODeviceMemory objects representing the physical memory ranges allocated to a memory mapped device.
    @result An OSArray of IODeviceMemory objects, or zero if none are available. The array is retained by the provider, so is valid while attached. */

    virtual OSArray * getDeviceMemory( void );

/*! @function setDeviceMemory
    @abstract Sets the array of IODeviceMemory objects representing a device's memory mapped ranges.
    @discussion This method sets an array of IODeviceMemory objects representing the physical memory ranges allocated to a memory mapped device.
    @param array An OSArray of IODeviceMemory objects, or zero if none are available. The array will be retained by the object. */

    virtual void setDeviceMemory( OSArray * array );

    /* Interrupt accessors */

/*! @function registerInterrupt
    @abstract Registers a C function interrupt handler for a device supplying interrupts.
    @discussion This method installs a C function interrupt handler to be called at primary interrupt time for a device's interrupt. Only one handler may be installed per interrupt source. IOInterruptEventSource provides a work loop based abstraction for interrupt delivery that may be more appropriate for work loop based drivers.
    @param source The index of the interrupt source in the device.
    @param target An object instance to be passed to the interrupt handler.
    @param handler The C function to be called at primary interrupt time when the interrupt occurs. The handler should process the interrupt by clearing the interrupt, or by disabling the source.
    @param refCon A reference constant for the handler's use.
    @result An IOReturn code.<br><code>kIOReturnNoInterrupt</code> is returned if the source is not valid; <code>kIOReturnNoResources</code> is returned if the interrupt already has an installed handler. */

    virtual IOReturn registerInterrupt(int source, OSObject *target,
                                       IOInterruptAction handler,
                                       void *refCon = 0);
                                       
/*! @function unregisterInterrupt
    @abstract Removes a C function interrupt handler for a device supplying hardware interrupts.
    @discussion This method removes a C function interrupt handler previously installed with @link registerInterrupt registerInterrupt@/link.
    @param source The index of the interrupt source in the device.
    @result An IOReturn code (<code>kIOReturnNoInterrupt</code> is returned if the source is not valid). */

    virtual IOReturn unregisterInterrupt(int source);

/*! @function addInterruptStatistics
    @abstract Adds a statistics object to the IOService for the given interrupt.
    @discussion This method associates a set of statistics and a reporter for those statistics with an interrupt for this IOService, so that we can interrogate the IOService for statistics pertaining to that interrupt.
    @param statistics The IOInterruptAccountingData container we wish to associate the IOService with.
    @param source The index of the interrupt source in the device. */
    IOReturn addInterruptStatistics(IOInterruptAccountingData * statistics, int source);

/*! @function removeInterruptStatistics
    @abstract Removes any statistics from the IOService for the given interrupt.
    @discussion This method disassociates any IOInterruptAccountingData container we may have for the given interrupt from the IOService; this indicates that the the interrupt target (at the moment, likely an IOInterruptEventSource) is being destroyed.
    @param source The index of the interrupt source in the device. */
    IOReturn removeInterruptStatistics(int source);

/*! @function getInterruptType
    @abstract Returns the type of interrupt used for a device supplying hardware interrupts.
    @param source The index of the interrupt source in the device.
    @param interruptType The interrupt type for the interrupt source will be stored here by <code>getInterruptType</code>.<br> <code>kIOInterruptTypeEdge</code> will be returned for edge-trigggered sources.<br><code>kIOInterruptTypeLevel</code> will be returned for level-trigggered sources.
    @result An IOReturn code (<code>kIOReturnNoInterrupt</code> is returned if the source is not valid). */

    virtual IOReturn getInterruptType(int source, int *interruptType);

/*! @function enableInterrupt
    @abstract Enables a device interrupt.
    @discussion It is the caller's responsiblity to keep track of the enable state of the interrupt source.
    @param source The index of the interrupt source in the device.
    @result An IOReturn code (<code>kIOReturnNoInterrupt</code> is returned if the source is not valid). */

    virtual IOReturn enableInterrupt(int source);

/*! @function disableInterrupt
    @abstract Synchronously disables a device interrupt.
    @discussion If the interrupt routine is running, the call will block until the routine completes. It is the caller's responsiblity to keep track of the enable state of the interrupt source.
    @param source The index of the interrupt source in the device.
    @result An IOReturn code (<code>kIOReturnNoInterrupt</code> is returned if the source is not valid). */

    virtual IOReturn disableInterrupt(int source);

/*! @function causeInterrupt
    @abstract Causes a device interrupt to occur.
    @discussion Emulates a hardware interrupt, to be called from task level.
    @param source The index of the interrupt source in the device.
    @result An IOReturn code (<code>kIOReturnNoInterrupt</code> is returned if the source is not valid). */

    virtual IOReturn causeInterrupt(int source);

/*! @function requestProbe
    @abstract Requests that hardware be re-scanned for devices.
    @discussion For bus families that do not usually detect device addition or removal, this method represents an external request (eg. from a utility application) to rescan and publish or remove found devices.
    @param options Family defined options, not interpreted by IOService.
    @result An IOReturn code. */

    virtual IOReturn requestProbe( IOOptionBits options );

    /* Generic API for non-data-path upstream calls */

/*! @function message
    @abstract Receives a generic message delivered from an attached provider.
    @discussion A provider may deliver messages via the <code>message</code> method to its clients informing them of state changes, such as <code>kIOMessageServiceIsTerminated</code> or <code>kIOMessageServiceIsSuspended</code>. Certain messages are defined by the I/O Kit in <code>IOMessage.h</code> while others may be family dependent. This method is implemented in the client to receive messages.
    @param type A type defined in <code>IOMessage.h</code> or defined by the provider family.
    @param provider The provider from which the message originates.
    @param argument An argument defined by the provider family, not used by IOService.
    @result An IOReturn code defined by the message type. */

    virtual IOReturn message( UInt32 type, IOService * provider,
                              void * argument = 0 );
                                
/*! @function messageClient
    @abstract Sends a generic message to an attached client.
    @discussion A provider may deliver messages via the @link message message@/link method to its clients informing them of state changes, such as <code>kIOMessageServiceIsTerminated</code> or <code>kIOMessageServiceIsSuspended</code>. Certain messages are defined by the I/O Kit in <code>IOMessage.h</code> while others may be family dependent. This method may be called in the provider to send a message to the specified client, which may be useful for overrides.
    @param messageType A type defined in <code>IOMessage.h</code> or defined by the provider family.
    @param client A client of the IOService to send the message.
    @param messageArgument An argument defined by the provider family, not used by IOService.
    @param argSize Specifies the size of messageArgument, in bytes.  If argSize is non-zero, messageArgument is treated as a pointer to argSize bytes of data.  If argSize is 0 (the default), messageArgument is treated as an ordinal and passed by value.
    @result The return code from the client message call. */
    
    virtual IOReturn messageClient( UInt32 messageType, OSObject * client,
                                    void * messageArgument = 0, vm_size_t argSize = 0 );

/*! @function messageClients
    @abstract Sends a generic message to all attached clients.
    @discussion A provider may deliver messages via the @link message message@/link method to its clients informing them of state changes, such as <code>kIOMessageServiceIsTerminated</code> or <code>kIOMessageServiceIsSuspended</code>. Certain messages are defined by the I/O Kit in <code>IOMessage.h</code> while others may be family dependent. This method may be called in the provider to send a message to all the attached clients, via the @link messageClient messageClient@/link method.
    @param type A type defined in <code>IOMessage.h</code> or defined by the provider family.
    @param argument An argument defined by the provider family, not used by IOService.
    @param argSize Specifies the size of argument, in bytes.  If argSize is non-zero, argument is treated as a pointer to argSize bytes of data.  If argSize is 0 (the default), argument is treated as an ordinal and passed by value.
    @result Any non-<code>kIOReturnSuccess</code> return codes returned by the clients, or <code>kIOReturnSuccess</code> if all return <code>kIOReturnSuccess</code>. */

    virtual IOReturn messageClients( UInt32 type,
                                     void * argument = 0, vm_size_t argSize = 0 );

    virtual IONotifier * registerInterest( const OSSymbol * typeOfInterest,
                                           IOServiceInterestHandler handler,
                                           void * target, void * ref = 0 );

    virtual void applyToProviders( IOServiceApplierFunction applier,
                                   void * context );

    virtual void applyToClients( IOServiceApplierFunction applier,
                                 void * context );

    virtual void applyToInterested( const OSSymbol * typeOfInterest,
                                    OSObjectApplierFunction applier,
                                    void * context );

    virtual IOReturn acknowledgeNotification( IONotificationRef notification,
                                              IOOptionBits response );

    /* User client create */

/*! @function newUserClient
    @abstract Creates a connection for a non kernel client.
    @discussion A non kernel client may request a connection be opened via the @link //apple_ref/c/func/IOServiceOpen IOServiceOpen@/link library function, which will call this method in an IOService object. The rules and capabilities of user level clients are family dependent, and use the functions of the IOUserClient class for support. IOService's implementation returns <code>kIOReturnUnsupported</code>, so any family supporting user clients must implement this method.
    @param owningTask The Mach task of the client thread in the process of opening the user client. Note that in Mac OS X, each process is based on a Mach task and one or more Mach threads. For more information on the composition of a Mach task and its relationship with Mach threads, see {@linkdoc //apple_ref/doc/uid/TP30000905-CH209-TPXREF103 "Tasks and Threads"}.
    @param securityID A token representing the access level for the task.
    @param type A constant specifying the type of connection to be created, specified by the caller of @link //apple_ref/c/func/IOServiceOpen IOServiceOpen@/link and interpreted only by the family.
    @param handler An instance of an IOUserClient object to represent the connection, which will be released when the connection is closed, or zero if the connection was not opened.    
    @param properties A dictionary of additional properties for the connection.
    @result A return code to be passed back to the caller of <code>IOServiceOpen</code>. */

    virtual IOReturn newUserClient( task_t owningTask, void * securityID,
                                    UInt32 type, OSDictionary * properties,
                                    IOUserClient ** handler );

    virtual IOReturn newUserClient( task_t owningTask, void * securityID,
                                    UInt32 type, IOUserClient ** handler );

    /* Return code utilities */

/*! @function stringFromReturn
    @abstract Supplies a programmer-friendly string from an IOReturn code.
    @discussion Strings are available for the standard return codes in <code>IOReturn.h</code> in IOService, while subclasses may implement this method to interpret family dependent return codes.
    @param rtn The IOReturn code.
    @result A pointer to a constant string, or zero if the return code is unknown. */
    
    virtual const char * stringFromReturn( IOReturn rtn );

/*! @function errnoFromReturn
    @abstract Translates an IOReturn code to a BSD <code>errno</code>.
    @discussion BSD defines its own return codes for its functions in <code>sys/errno.h</code>, and I/O Kit families may need to supply compliant results in BSD shims. Results are available for the standard return codes in <code>IOReturn.h</code> in IOService, while subclasses may implement this method to interpret family dependent return codes.
    @param rtn The IOReturn code.
    @result The BSD <code>errno</code> or <code>EIO</code> if unknown. */
    
    virtual int errnoFromReturn( IOReturn rtn );

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    /* * * * * * * * * * end of IOService API  * * * * * * * */
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* for IOInterruptController implementors */

    int               _numInterruptSources;
    IOInterruptSource *_interruptSources;

    /* overrides */
    virtual bool serializeProperties( OSSerialize * s ) const APPLE_KEXT_OVERRIDE;

#ifdef KERNEL_PRIVATE
    /* Apple only SPI to control CPU low power modes */
    void   setCPUSnoopDelay(UInt32 ns);
    UInt32 getCPUSnoopDelay();
#endif
    void   requireMaxBusStall(UInt32 ns);
    void   requireMaxInterruptDelay(uint32_t ns);

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
    /* * * * * * * * * * * * Internals * * * * * * * * * * * */
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef XNU_KERNEL_PRIVATE
public:
    // called from other xnu components
    static void initialize( void );
    static void setPlatform( IOPlatformExpert * platform);
    static void setPMRootDomain( class IOPMrootDomain * rootDomain );
    static IOReturn catalogNewDrivers( OSOrderedSet * newTables );
    uint64_t getAccumulatedBusyTime( void );
    static void updateConsoleUsers(OSArray * consoleUsers, IOMessage systemMessage);
    static void consoleLockTimer(thread_call_param_t p0, thread_call_param_t p1);
    void setTerminateDefer(IOService * provider, bool defer);
    uint64_t getAuthorizationID( void );
    IOReturn setAuthorizationID( uint64_t authorizationID );
    void cpusRunning(void);
    void scheduleFinalize(bool now);

private:
    static IOReturn waitMatchIdle( UInt32 ms );
    static IONotifier * installNotification(
                            const OSSymbol * type, OSDictionary * matching,
                            IOServiceMatchingNotificationHandler handler,
                            void * target, void * ref,
                            SInt32 priority, OSIterator ** existing );
#if !defined(__LP64__)
    static IONotifier * installNotification(
                            const OSSymbol * type, OSDictionary * matching,
                            IOServiceNotificationHandler handler,
                            void * target, void * ref,
                            SInt32 priority, OSIterator ** existing);
#endif /* !defined(__LP64__) */
#endif

private:
    APPLE_KEXT_COMPATIBILITY_VIRTUAL
        bool checkResources( void );
    APPLE_KEXT_COMPATIBILITY_VIRTUAL
        bool checkResource( OSObject * matching );

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
        void probeCandidates( OSOrderedSet * matches );
    APPLE_KEXT_COMPATIBILITY_VIRTUAL
        bool startCandidate( IOService * candidate );

public:
    APPLE_KEXT_COMPATIBILITY_VIRTUAL
        IOService * getClientWithCategory( const OSSymbol * category )
    APPLE_KEXT_DEPRECATED;
        // copyClientWithCategory is the public replacement

#ifdef XNU_KERNEL_PRIVATE
    /* Callable within xnu source only - but require vtable entries to be visible */
public:
#else
private:
#endif
    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    bool passiveMatch( OSDictionary * matching, bool changesOK = false);
    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    void startMatching( IOOptionBits options = 0 );
    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    void doServiceMatch( IOOptionBits options );
    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    void doServiceTerminate( IOOptionBits options );

private:

    bool matchPassive(OSDictionary * table, uint32_t options);
    bool matchInternal(OSDictionary * table, uint32_t options, unsigned int * did);
    static bool instanceMatch(const OSObject * entry, void * context);

    static OSObject * copyExistingServices( OSDictionary * matching,
                                            IOOptionBits inState, IOOptionBits options = 0 );

    static IONotifier * setNotification(
                            const OSSymbol * type, OSDictionary * matching,
                            IOServiceMatchingNotificationHandler handler,
                            void * target, void * ref,
                            SInt32 priority = 0 );

    static IONotifier * doInstallNotification(
                            const OSSymbol * type, OSDictionary * matching,
                            IOServiceMatchingNotificationHandler handler,
                            void * target, void * ref,
                            SInt32 priority, OSIterator ** existing );

    static bool syncNotificationHandler( void * target, void * ref,
                            IOService * newService, IONotifier * notifier  );

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    void deliverNotification( const OSSymbol * type,
                              IOOptionBits orNewState, IOOptionBits andNewState );

    bool invokeNotifer( class _IOServiceNotifier * notify );

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
        void unregisterAllInterest( void );

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
        IOReturn waitForState( UInt32 mask, UInt32 value,
                               mach_timespec_t * timeout = 0 );

    IOReturn waitForState( UInt32 mask, UInt32 value, uint64_t timeout );

    UInt32 _adjustBusy( SInt32 delta );

    bool terminatePhase1( IOOptionBits options = 0 );
    void scheduleTerminatePhase2( IOOptionBits options = 0 );
    void scheduleStop( IOService * provider );
    static void terminateThread( void * arg, wait_result_t unused );
    static void terminateWorker( IOOptionBits options );
    static void actionWillTerminate( IOService * victim, IOOptionBits options, 
                                     OSArray * doPhase2List, void*, void * );
    static void actionDidTerminate( IOService * victim, IOOptionBits options,
                                    void *, void *, void *);

    static void actionWillStop( IOService * victim, IOOptionBits options, 
				void *, void *, void *);
    static void actionDidStop( IOService * victim, IOOptionBits options,
				void *, void *, void *);

    static void actionFinalize( IOService * victim, IOOptionBits options,
                                void *, void *, void *);
    static void actionStop( IOService * client, IOService * provider,
                            void *, void *, void *);

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
        IOReturn resolveInterrupt(IOService *nub, int source);
    APPLE_KEXT_COMPATIBILITY_VIRTUAL
        IOReturn lookupInterrupt(int source, bool resolve, IOInterruptController **interruptController);

#ifdef XNU_KERNEL_PRIVATE
    /* end xnu internals */
#endif

    /* power management */
public:

/*! @function PMinit
    @abstract Initializes power management for a driver.
    @discussion <code>PMinit</code> allocates and initializes the power management instance variables, and it should be called before accessing those variables or calling the power management methods. This method should be called inside the driver's <code>start</code> routine and must be paired with a call to @link PMstop PMstop@/link.
    Most calls to <code>PMinit</code> are followed by calls to @link joinPMtree joinPMtree@/link and @link registerPowerDriver registerPowerDriver@/link. */

    virtual void PMinit( void );

/*! @function PMstop
    @abstract Stop power managing the driver.
    @discussion Removes the driver from the power plane and stop its power management. This method is synchronous against any power management method invocations (e.g. <code>setPowerState</code> or <code>setAggressiveness</code>), so when this method returns it is guaranteed those power management methods will not be entered. Driver should not call any power management methods after this call.
    Calling <code>PMstop</code> cleans up for the three power management initialization calls: @link PMinit PMinit@/link, @link joinPMtree joinPMtree@/link, and @link registerPowerDriver registerPowerDriver@/link. */

    virtual void PMstop( void );

/*! @function joinPMtree
    @abstract Joins the driver into the power plane of the I/O Registry.
    @discussion A driver uses this method to call its nub when initializing (usually in its <code>start</code> routine after calling @link PMinit PMinit@/link), to be attached into the power management hierarchy (i.e., the power plane). A driver usually calls this method on the driver for the device that provides it power (this is frequently the nub).    
    Before this call returns, the caller will probably be called at @link setPowerParent setPowerParent@/link and @link setAggressiveness setAggressiveness@/link and possibly at @link addPowerChild addPowerChild@/link as it is added to the hierarchy. This method may be overridden by a nub subclass.
    @param driver The driver to be added to the power plane, usually <code>this</code>. */

    virtual void joinPMtree( IOService * driver );

/*! @function registerPowerDriver
    @abstract Registers a set of power states that the driver supports.
    @discussion A driver defines its array of supported power states with power management in its power management initialization (its <code>start</code> routine). If successful, power management will call the driver to instruct it to change its power state through @link setPowerState setPowerState@/link.
    Most drivers do not need to override <code>registerPowerDriver</code>. A nub may override <code>registerPowerDriver</code> if it needs to arrange its children in the power plane differently than the default placement, but this is uncommon.
    @param controllingDriver A pointer to the calling driver, usually <code>this</code>.
    @param powerStates A driver-defined array of power states that the driver and device support. Power states are defined in <code>pwr_mgt/IOPMpowerState.h</code>.
    @param numberOfStates The number of power states in the array.
    @result </code>IOPMNoErr</code>. All errors are logged via <code>kprintf</code>. */

    virtual IOReturn registerPowerDriver(
                        IOService *      controllingDriver,
                        IOPMPowerState * powerStates,
                        unsigned long    numberOfStates );

/*! @function registerInterestedDriver
    @abstract Allows an IOService object to register interest in the changing power state of a power-managed IOService object.
    @discussion Call <code>registerInterestedDriver</code> on the IOService object you are interested in receiving power state messages from, and pass a pointer to the interested driver (<code>this</code>) as an argument.
    The interested driver is retained until the power interest is removed by calling <code>deRegisterInterestedDriver</code>.
    The interested driver should override @link powerStateWillChangeTo powerStateWillChangeTo@/link and @link powerStateDidChangeTo powerStateDidChangeTo@/link to receive these power change messages.
    Interested drivers must acknowledge power changes in <code>powerStateWillChangeTo</code> or <code>powerStateDidChangeTo</code>, either via return value or later calls to @link acknowledgePowerChange acknowledgePowerChange@/link.
    @param theDriver The driver of interest adds this pointer to the list of interested drivers. It informs drivers on this list before and after the power change.
    @result Flags describing the capability of the device in its current power state. If the current power state is not yet defined, zero is returned (this is the case when the driver is not yet in the power domain hierarchy or hasn't fully registered with power management yet). */

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    IOPMPowerFlags registerInterestedDriver( IOService * theDriver );

/*! @function deRegisterInterestedDriver
    @abstract De-registers power state interest from a previous call to <code>registerInterestedDriver</code>.
    @discussion The retain from <code>registerInterestedDriver</code> is released. This method is synchronous against any <code>powerStateWillChangeTo</code> or <code>powerStateDidChangeTo</code> call targeting the interested driver, so when this method returns it is guaranteed those interest handlers will not be entered.
    Most drivers do not need to override <code>deRegisterInterestedDriver</code>.
    @param theDriver The interested driver previously passed into @link registerInterestedDriver registerInterestedDriver@/link.
    @result A return code that can be ignored by the caller. */

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    IOReturn deRegisterInterestedDriver( IOService * theDriver );

/*! @function acknowledgePowerChange
    @abstract Acknowledges an in-progress power state change.
    @discussion When power management informs an interested object (via @link powerStateWillChangeTo powerStateWillChangeTo@/link or @link powerStateDidChangeTo powerStateDidChangeTo@/link), the object can return an immediate acknowledgement via a return code, or it may return an indication that it will acknowledge later by calling <code>acknowledgePowerChange</code>.
    Interested objects are those that have registered as interested drivers, as well as power plane children of the power changing driver. A driver that calls @link registerInterestedDriver registerInterestedDriver@/link must call <code>acknowledgePowerChange</code>, or use an immediate acknowledgement return from <code>powerStateWillChangeTo</code> or <code>powerStateDidChangeTo</code>.
    @param whichDriver A pointer to the calling driver. The called object tracks all interested parties to ensure that all have acknowledged the power state change.
    @result <code>IOPMNoErr</code>. */

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    IOReturn acknowledgePowerChange( IOService * whichDriver );

/*! @function acknowledgeSetPowerState
    @abstract Acknowledges the belated completion of a driver's <code>setPowerState</code> power state change.
    @discussion After power management instructs a driver to change its state via @link setPowerState setPowerState@/link, that driver must acknowledge the change when its device has completed its transition. The acknowledgement may be immediate, via a return code from <code>setPowerState</code>, or delayed, via this call to <code>acknowledgeSetPowerState</code>.
    Any driver that does not return <code>kIOPMAckImplied</code> from its <code>setPowerState</code> implementation must later call <code>acknowledgeSetPowerState</code>.
    @result <code>IOPMNoErr</code>. */

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    IOReturn acknowledgeSetPowerState( void );

/*! @function requestPowerDomainState
    @abstract Tells a driver to adjust its power state.
    @discussion This call is handled internally by power management. It is not intended to be overridden or called by drivers. */

    virtual IOReturn requestPowerDomainState(
                        IOPMPowerFlags desiredState,
                        IOPowerConnection * whichChild,
                        unsigned long specificationFlags );

/*! @function makeUsable
    @abstract Requests that a device become usable.
    @discussion This method is called when some client of a device (or the device's own driver) is asking for the device to become usable. Power management responds by telling the object upon which this method is called to change to its highest power state.
    <code>makeUsable</code> is implemented using @link changePowerStateToPriv changePowerStateToPriv@/link. Subsequent requests for lower power, such as from <code>changePowerStateToPriv</code>, will pre-empt this request.
    @result A return code that can be ignored by the caller. */

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    IOReturn makeUsable( void );

/*! @function temporaryPowerClampOn
    @abstract A driver calls this method to hold itself in the highest power state until it has children.
    @discussion Use <code>temporaryPowerClampOn</code> to hold your driver in its highest power state while waiting for child devices to attach. After children have attached, the clamp is released and the device's power state is controlled by the children's requirements.
    @result A return code that can be ignored by the caller. */

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    IOReturn temporaryPowerClampOn( void );

/*! @function changePowerStateTo
    @abstract Sets a driver's power state.
    @discussion This function is one of several that are used to set a driver's power state. In most circumstances, however, you should call @link changePowerStateToPriv changePowerStateToPriv@/link instead.
    Calls to <code>changePowerStateTo</code>, <code>changePowerStateToPriv</code>, and a driver's power children all affect the power state of a driver. For legacy design reasons, they have overlapping functionality. Although you should call <code>changePowerStateToPriv</code> to change your device's power state, you might need to call <code>changePowerStateTo</code> in the following circumstances:
    <ul><li>If a driver will be using <code>changePowerStateToPriv</code> to change its power state, it should call <code>changePowerStateTo(0)</code> in its <code>start</code> routine to eliminate the influence <code>changePowerStateTo</code> has on power state calculations.
    <li>Call <code>changePowerStateTo</code> in conjunction with @link setIdleTimerPeriod setIdleTimerPeriod@/link and @link activityTickle activityTickle@/link to idle a driver into a low power state. For a driver with 3 power states, for example, <code>changePowerStateTo(1)</code> sets a minimum level of power state 1, such that the idle timer period may not set your device's power any lower than state 1.</ul>
    @param ordinal The number of the desired power state in the power state array. 
    @result A return code that can be ignored by the caller. */

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    IOReturn changePowerStateTo( unsigned long ordinal );

/*! @function currentCapability
    @abstract Finds out the capability of a device's current power state.
    @result A copy of the <code>capabilityFlags</code> field for the current power state in the power state array. */

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    IOPMPowerFlags currentCapability( void );

/*! @function currentPowerConsumption
    @abstract Finds out the current power consumption of a device.
    @discussion Most Mac OS X power managed drivers do not report their power consumption via the <code>staticPower</code> field. Thus this call will not accurately reflect power consumption for most drivers.
    @result A copy of the <code>staticPower</code> field for the current power state in the power state array. */

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    unsigned long currentPowerConsumption( void );

/*! @function activityTickle
    @abstract Informs power management when a power-managed device is in use, so that power management can track when it is idle and adjust its power state accordingly.
    @discussion The <code>activityTickle</code> method is provided for objects in the system (or for the driver itself) to tell a driver that its device is being used.
    The IOService superclass can manage idleness determination with a simple idle timer mechanism and this <code>activityTickle</code> call. To start this up, the driver calls its superclass's <code>setIdleTimerPeriod</code>. This starts a timer for the time interval specified in the call. When the timer expires, the superclass checks to see if there has been any activity since the last timer expiration. (It checks to see if <code>activityTickle</code> has been called). If there has been activity, it restarts the timer, and this process continues. When the timer expires, and there has been no device activity, the superclass lowers the device power state to the next lower state. This can continue until the device is in state zero.
    After the device has been powered down by at least one power state, a subsequent call to <code>activityTickle</code> causes the device to be switched to a higher state required for the activity.    
    If the driver is managing the idleness determination totally on its own, the value of the <code>type</code> parameter should be <code>kIOPMSubclassPolicy</code>, and the driver should override the <code>activityTickle</code> method. The superclass IOService implementation of <code>activityTickle</code> does nothing with the <code>kIOPMSubclassPolicy</code> argument.
    @param type When <code>type</code> is <code>kIOPMSubclassPolicy</code>, <code>activityTickle</code> is not handled in IOService and should be intercepted by the subclass. When <code>type</code> is <code>kIOPMSuperclassPolicy1</code>, an activity flag is set and the device state is checked. If the device has been powered down, it is powered up again.
    @param stateNumber When <code>type</code> is <code>kIOPMSuperclassPolicy1</code>, <code>stateNumber</code> contains the desired power state ordinal for the activity. If the device is in a lower state, the superclass will switch it to this state. This is for devices that can handle some accesses in lower power states; the device is powered up only as far as it needs to be for the activity.
    @result When <code>type</code> is <code>kIOPMSuperclassPolicy1</code>, the superclass returns <code>true</code> if the device is currently in the state specified by <code>stateNumber</code>. If the device is in a lower state and must be powered up, the superclass returns <code>false</code>; in this case the superclass will initiate a power change to power the device up. */

    virtual bool activityTickle(
                        unsigned long type,
                        unsigned long stateNumber = 0 );

/*! @function setAggressiveness
    @abstract Broadcasts an aggressiveness factor from the parent of a driver to the driver.
    @discussion Implement <code>setAggressiveness</code> to receive a notification when an "aggressiveness Aggressiveness factors are a loose set of power management variables that contain values for system sleep timeout, display sleep timeout, whether the system is on battery or AC, and other power management features. There are several aggressiveness factors that can be broadcast and a driver may take action on whichever factors apply to it.
    A driver that has joined the power plane via @link joinPMtree joinPMtree@/link will receive <code>setAgressiveness</code> calls when aggressiveness factors change.
    A driver may override this call if it needs to do something with the new factor (such as change its idle timeout). If overridden, the driver must  call its superclass's <code>setAgressiveness</code> method in its own <code>setAgressiveness</code> implementation.
    Most drivers do not need to implement <code>setAgressiveness</code>.
    @param type The aggressiveness factor type, such as <code>kPMMinutesToDim</code>, <code>kPMMinutesToSpinDown</code>, <code>kPMMinutesToSleep</code>, and <code>kPMPowerSource</code>. (Aggressiveness factors are defined in <code>pwr_mgt/IOPM.h</code>.)
    @param newLevel The aggressiveness factor's new value.
    @result <code>IOPMNoErr</code>. */

    virtual IOReturn setAggressiveness(
                        unsigned long type,
                        unsigned long newLevel );

/*! @function getAggressiveness
    @abstract Returns the current aggressiveness value for the given type.
    @param type The aggressiveness factor to query.
    @param currentLevel Upon successful return, contains the value of aggressiveness factor <code>type</code>.
    @result <code>kIOReturnSuccess</code> upon success; an I/O Kit error code otherwise. */

    virtual IOReturn getAggressiveness(
                        unsigned long type,
                        unsigned long * currentLevel );

#ifndef __LP64__
/*! @function systemWake
    @abstract Tells every driver in the power plane that the system is waking up.
    @discussion This call is handled internally by power management. It is not intended to be overridden or called by drivers. */

    virtual IOReturn systemWake( void )
    APPLE_KEXT_DEPRECATED;

/*! @function temperatureCriticalForZone
    @abstract Alerts a driver to a critical temperature in some thermal zone.
    @discussion This call is unused by power management. It is not intended to be called or overridden. */

    virtual IOReturn temperatureCriticalForZone( IOService * whichZone )
    APPLE_KEXT_DEPRECATED;

/*! @function youAreRoot
    @abstract Informs power management which IOService object is the power plane root.
    @discussion This call is handled internally by power management. It is not intended to be overridden or called by drivers. */

    virtual IOReturn youAreRoot( void )
    APPLE_KEXT_DEPRECATED;

/*! @function setPowerParent
    @abstract This call is handled internally by power management. It is not intended to be overridden or called by drivers. */

    virtual IOReturn setPowerParent(
                        IOPowerConnection * parent,
                        bool stateKnown,
                        IOPMPowerFlags currentState )
    APPLE_KEXT_DEPRECATED;
#endif /* !__LP64__ */

/*! @function addPowerChild
    @abstract Informs a driver that it has a new child.
    @discussion The Platform Expert uses this method to call a driver and introduce it to a new child. This call is handled internally by power management. It is not intended to be overridden or called by drivers.
    @param theChild A pointer to the child IOService object. */

    virtual IOReturn addPowerChild( IOService * theChild );

/*! @function removePowerChild
    @abstract Informs a power managed driver that one of its power plane childen is disappearing.
    @discussion This call is handled internally by power management. It is not intended to be overridden or called by drivers. */

    virtual IOReturn removePowerChild( IOPowerConnection * theChild );

#ifndef __LP64__
/*! @function command_received
    @discussion This call is handled internally by power management. It is not intended to be overridden or called by drivers. */

    virtual void command_received( void *, void * , void * , void * );
#endif

/*! @function start_PM_idle_timer
    @discussion This call is handled internally by power management. It is not intended to be overridden or called by drivers. */

    APPLE_KEXT_COMPATIBILITY_VIRTUAL
    void start_PM_idle_timer( void );

#ifndef __LP64__
/*! @function PM_idle_timer_expiration
    @discussion This call is handled internally by power management. It is not intended to be overridden or called by drivers. */

    virtual void PM_idle_timer_expiration( void )
    APPLE_KEXT_DEPRECATED;

/*! @function PM_Clamp_Timer_Expired
    @discussion This call is handled internally by power management. It is not intended to be overridden or called by drivers. */

    virtual void PM_Clamp_Timer_Expired( void )
    APPLE_KEXT_DEPRECATED;
#endif

/*! @function setIdleTimerPeriod
    @abstract Sets or changes the idle timer period.
    @discussion A driver using the idleness determination provided by IOService calls its superclass with this method to set or change the idle timer period. See @link activityTickle activityTickle@/link for a description of this type of idleness determination.
    @param period The desired idle timer period in seconds.
    @result <code>kIOReturnSuccess</code> upon success; an I/O Kit error code otherwise. */

    virtual IOReturn setIdleTimerPeriod( unsigned long );

#ifndef __LP64__
/*! @function getPMworkloop
    @abstract Returns a pointer to the system-wide power management work loop.
    @availability Deprecated in Mac OS X version 10.6.
    @discussion Most drivers should create their own work loops to synchronize their code; drivers should not run arbitrary code on the power management work loop. */

    virtual IOWorkLoop * getPMworkloop( void )
    APPLE_KEXT_DEPRECATED;
#endif

/*! @function getPowerState
    @abstract Determines a device's power state.
    @discussion A device's "current power state" is updated at the end of each power state transition (e.g. transition from state 1 to state 0, or state 0 to state 2). This transition includes the time spent powering on or off any power plane children. Thus, if a child calls <code>getPowerState</code> on its power parent during system wake from sleep, the call will return the index to the device's off state rather than its on state.
    @result The current power state's index into the device's power state array. */

    UInt32 getPowerState( void );

/*! @function setPowerState
    @abstract Requests a power managed driver to change the power state of its device.
    @discussion A power managed driver must override <code>setPowerState</code> to take part in system power management. After a driver is registered with power management, the system uses <code>setPowerState</code> to power the device off and on for system sleep and wake.
    Calls to @link PMinit PMinit@/link and @link registerPowerDriver registerPowerDriver@/link enable power management to change a device's power state using <code>setPowerState</code>. <code>setPowerState</code> is called in a clean and separate thread context.
    @param powerStateOrdinal The number in the power state array of the state the driver is being instructed to switch to. 
    @param whatDevice A pointer to the power management object which registered to manage power for this device. In most cases, <code>whatDevice</code> will be equal to your driver's own <code>this</code> pointer.
    @result The driver must return <code>IOPMAckImplied</code> if it has complied with the request when it returns. Otherwise if it has started the process of changing power state but not finished it, the driver should return a number of microseconds which is an upper limit of the time it will need to finish. Then, when it has completed the power switch, it should call @link acknowledgeSetPowerState acknowledgeSetPowerState@/link. */

    virtual IOReturn setPowerState(
                        unsigned long powerStateOrdinal,
                        IOService *   whatDevice );

#ifndef __LP64__
/*! @function clampPowerOn
    @abstract Deprecated. Do not use. */

    virtual void clampPowerOn( unsigned long duration );
#endif

/*! @function maxCapabilityForDomainState
    @abstract Determines a driver's highest power state possible for a given power domain state.
    @discussion This happens when the power domain is changing state and power management needs to determine which state the device is capable of in the new domain state.
    Most drivers do not need to implement this method, and can rely upon the default IOService implementation. The IOService implementation scans the power state array looking for the highest state whose <code>inputPowerRequirement</code> field exactly matches the value of the <code>domainState</code> parameter. If more intelligent determination is required, the driver itself should implement the method and override the superclass's implementation.
    @param domainState Flags that describe the character of "domain power"; they represent the <code>outputPowerCharacter</code> field of a state in the power domain's power state array.
    @result A state number. */

    virtual unsigned long maxCapabilityForDomainState( IOPMPowerFlags domainState );

/*! @function initialPowerStateForDomainState
    @abstract Determines which power state a device is in, given the current power domain state.
    @discussion Power management calls this method once, when the driver is initializing power management.
    Most drivers do not need to implement this method, and can rely upon the default IOService implementation. The IOService implementation scans the power state array looking for the highest state whose <code>inputPowerRequirement</code> field exactly matches the value of the <code>domainState</code> parameter. If more intelligent determination is required, the power managed driver should implement the method and override the superclass's implementation.
    @param domainState Flags that describe the character of "domain power"; they represent the <code>outputPowerCharacter</code> field of a state in the power domain's power state array.
    @result A state number. */

    virtual unsigned long initialPowerStateForDomainState( IOPMPowerFlags domainState );

/*! @function powerStateForDomainState
    @abstract Determines what power state the device would be in for a given power domain state.
    @discussion This call is unused by power management. Drivers should override <code>initialPowerStateForDomainState</code> and/or <code>maxCapabilityForDomainState</code> instead to change the default mapping of domain state to driver power state.
    @param domainState Flags that describe the character of "domain power"; they represent the <code>outputPowerCharacter</code> field of a state in the power domain's power state array.
    @result A state number. */

    virtual unsigned long powerStateForDomainState( IOPMPowerFlags domainState );

/*! @function powerStateWillChangeTo
    @abstract Informs interested parties that a device is about to change its power state.
    @discussion Power management informs interested parties that a device is about to change to a different power state. Interested parties are those that have registered for this notification via @link registerInterestedDriver registerInterestedDriver@/link. If you have called <code>registerInterestedDriver</code> on a power managed driver, you must implement <code>powerStateWillChangeTo</code> and @link powerStateDidChangeTo powerStateDidChangeTo@/link to receive the notifications.
    <code>powerStateWillChangeTo</code> is called in a clean and separate thread context. <code>powerStateWillChangeTo</code> is called before a power state transition takes place; <code>powerStateDidChangeTo</code> is called after the transition has completed.
    @param capabilities Flags that describe the capability of the device in the new power state (they come from the <code>capabilityFlags</code> field of the new state in the power state array).
    @param stateNumber The number of the state in the state array that the device is switching to.
    @param whatDevice A pointer to the driver that is changing. It can be used by a driver that is receiving power state change notifications for multiple devices to distinguish between them.
    @result The driver returns <code>IOPMAckImplied</code> if it has prepared for the power change when it returns. If it has started preparing but not finished, it should return a number of microseconds which is an upper limit of the time it will need to finish preparing. Then, when it has completed its preparations, it should call @link acknowledgePowerChange acknowledgePowerChange@/link. */

    virtual IOReturn powerStateWillChangeTo(
                        IOPMPowerFlags  capabilities,
                        unsigned long   stateNumber,
                        IOService *     whatDevice );

/*! @function powerStateDidChangeTo
    @abstract Informs interested parties that a device has changed to a different power state.
    @discussion Power management informs interested parties that a device has changed to a different power state. Interested parties are those that have registered for this notification via @link registerInterestedDriver registerInterestedDriver@/link. If you have called <code>registerInterestedDriver</code> on a power managed driver, you must implemnt @link powerStateWillChangeTo powerStateWillChangeTo@/link and <code>powerStateDidChangeTo</code> to receive the notifications.
    <code>powerStateDidChangeTo</code> is called in a clean and separate thread context. <code>powerStateWillChangeTo</code> is called before a power state transition takes place; <code>powerStateDidChangeTo</code> is called after the transition has completed.
    @param capabilities Flags that describe the capability of the device in the new power state (they come from the <code>capabilityFlags</code> field of the new state in the power state array).
    @param stateNumber The number of the state in the state array that the device is switching to.
    @param whatDevice A pointer to the driver that is changing. It can be used by a driver that is receiving power state change notifications for multiple devices to distinguish between them.
    @result The driver returns <code>IOPMAckImplied</code> if it has prepared for the power change when it returns. If it has started preparing but not finished, it should return a number of microseconds which is an upper limit of the time it will need to finish preparing. Then, when it has completed its preparations, it should call @link acknowledgePowerChange acknowledgePowerChange@/link. */

    virtual IOReturn powerStateDidChangeTo(
                        IOPMPowerFlags  capabilities,
                        unsigned long   stateNumber,
                        IOService *     whatDevice );

#ifndef __LP64__
/*! @function didYouWakeSystem
    @abstract Asks a driver if its device is the one that just woke the system from sleep.
    @availability Deprecated in Mac OS X version 10.6.
    @discussion Power management calls a power managed driver with this method to ask if its device is the one that just woke the system from sleep. If a device is capable of waking the system from sleep, its driver should implement <code>didYouWakeSystem</code> and return <code>true</code> if its device was responsible for waking the system.
    @result <code>true</code> if the driver's device woke the system and <code>false</code> otherwise. */

    virtual bool didYouWakeSystem( void )
    APPLE_KEXT_DEPRECATED;

/*! @function newTemperature
    @abstract Tells a power managed driver that the temperature in the thermal zone has changed.
    @discussion This call is unused by power management. It is not intended to be called or overridden. */

    virtual IOReturn newTemperature( long currentTemp, IOService * whichZone )
    APPLE_KEXT_DEPRECATED;
#endif

    virtual bool askChangeDown( unsigned long );
    virtual bool tellChangeDown( unsigned long );
    virtual void tellNoChangeDown ( unsigned long );
    virtual void tellChangeUp( unsigned long );
    virtual IOReturn allowPowerChange( unsigned long refcon );
    virtual IOReturn cancelPowerChange( unsigned long refcon );

protected:
/*! @function changePowerStateToPriv 
    @abstract Tells a driver's superclass to change the power state of its device.
    @discussion A driver uses this method to tell its superclass to change the power state of the device. This is the recommended way to change the power state of a device.
    Three things affect driver power state: @link changePowerStateTo changePowerStateTo@/link, <code>changePowerStateToPriv</code>, and the desires of the driver's power plane children. Power management puts the device into the maximum state governed by those three entities.
    Drivers may eliminate the influence of the <code>changePowerStateTo</code> method on power state one of two ways. See @link powerOverrideOnPriv powerOverrideOnPriv@/link to ignore the method's influence, or call <code>changePowerStateTo(0)</code> in the driver's <code>start</code> routine to remove the <code>changePowerStateTo</code> method's power request.
    @param ordinal The number of the desired power state in the power state array.
    @result A return code that can be ignored by the caller. */

    IOReturn changePowerStateToPriv( unsigned long ordinal );

/*! @function powerOverrideOnPriv
    @abstract Allows a driver to ignore its children's power management requests and only use changePowerStateToPriv to define its own power state.
    @discussion Power management normally keeps a device at the highest state required by its requests via @link changePowerStateTo changePowerStateTo@/link, @link changePowerStateToPriv changePowerStateToPriv@/link, and its children. However, a driver may ensure a lower power state than otherwise required by itself and its children using <code>powerOverrideOnPriv</code>. When the override is on, power management keeps the device's power state in the state specified by <code>changePowerStateToPriv</code>. Turning on the override will initiate a power change if the driver's <code>changePowerStateToPriv</code> desired power state is different from the maximum of the <code>changePowerStateTo</code> desired power state and the children's desires.
    @result A return code that can be ignored by the caller. */

    IOReturn powerOverrideOnPriv( void );

/*! @function powerOverrideOffPriv
    @abstract Allows a driver to disable a power override.
    @discussion When a driver has enabled an override via @link powerOverrideOnPriv powerOverrideOnPriv@/link, it can disable it again by calling this method in its superclass. Disabling the override reverts to the default algorithm for determining a device's power state. The superclass will now keep the device at the highest state required by <code>changePowerStateTo</code>, <code>changePowerStateToPriv</code>, and its children. Turning off the override will initiate a power change if the driver's desired power state is different from the maximum of the power managed driver's desire and the children's desires.
    @result A return code that can be ignored by the caller. */

    IOReturn powerOverrideOffPriv( void );

/*! @function powerChangeDone
    @abstract Tells a driver when a power state change is complete.
    @discussion Power management uses this method to inform a driver when a power change is completely done, when all interested parties have acknowledged the @link powerStateDidChangeTo powerStateDidChangeTo@/link call. The default implementation of this method is null; the method is meant to be overridden by subclassed power managed drivers. A driver should use this method to find out if a power change it initiated is complete.
    @param stateNumber The number of the state in the state array that the device has switched from. */

    virtual void powerChangeDone( unsigned long stateNumber );
#ifdef XNU_KERNEL_PRIVATE
    /* Power management internals */
public:
    void idleTimerExpired( void );
    void settleTimerExpired( void );
    IOReturn synchronizePowerTree( IOOptionBits options = 0, IOService * notifyRoot = 0 );
    bool assertPMDriverCall( IOPMDriverCallEntry * callEntry, IOOptionBits options = 0, IOPMinformee * inform = 0 );
    void deassertPMDriverCall( IOPMDriverCallEntry * callEntry );
    IOReturn changePowerStateWithOverrideTo( IOPMPowerStateIndex ordinal, IOPMRequestTag tag );
    IOReturn changePowerStateForRootDomain( IOPMPowerStateIndex ordinal );
    IOReturn setIgnoreIdleTimer( bool ignore );
    IOReturn quiescePowerTree( void * target, IOPMCompletionAction action, void * param );
    uint32_t getPowerStateForClient( const OSSymbol * client );
    static const char * getIOMessageString( uint32_t msg );
    static void setAdvisoryTickleEnable( bool enable );
    void reset_watchdog_timer( void );
    void start_watchdog_timer ( void );
    bool stop_watchdog_timer ( void );
    IOReturn registerInterestForNotifer( IONotifier *notify, const OSSymbol * typeOfInterest,
                  IOServiceInterestHandler handler, void * target, void * ref );

#ifdef __LP64__
    static IOWorkLoop * getPMworkloop( void );
#endif

protected:
    bool tellClientsWithResponse( int messageType );
    void tellClients( int messageType );
    void PMDebug( uint32_t event, uintptr_t param1, uintptr_t param2 );

private:
#ifndef __LP64__
    void ack_timer_ticked ( void );
    IOReturn serializedAllowPowerChange2 ( unsigned long );
    IOReturn serializedCancelPowerChange2 ( unsigned long );
    IOReturn powerDomainWillChangeTo( IOPMPowerFlags, IOPowerConnection * );
    IOReturn powerDomainDidChangeTo( IOPMPowerFlags, IOPowerConnection * );
#endif
    void PMfree( void );
    bool tellChangeDown1 ( unsigned long );
    bool tellChangeDown2 ( unsigned long );
    IOReturn startPowerChange( IOPMPowerChangeFlags, IOPMPowerStateIndex, IOPMPowerFlags, IOPowerConnection *, IOPMPowerFlags );
    void setParentInfo ( IOPMPowerFlags, IOPowerConnection *, bool );
    IOReturn notifyAll ( uint32_t nextMS );
    bool notifyChild ( IOPowerConnection * child );
    IOPMPowerStateIndex getPowerStateForDomainFlags( IOPMPowerFlags flags );

    // power change initiated by driver
    void OurChangeStart( void );
    void OurSyncStart ( void );
    void OurChangeTellClientsPowerDown ( void );
    void OurChangeTellUserPMPolicyPowerDown ( void );
    void OurChangeTellPriorityClientsPowerDown ( void );
    void OurChangeTellCapabilityWillChange ( void );
    void OurChangeNotifyInterestedDriversWillChange ( void );
    void OurChangeSetPowerState ( void );
    void OurChangeWaitForPowerSettle ( void );
    void OurChangeNotifyInterestedDriversDidChange ( void );
    void OurChangeTellCapabilityDidChange ( void );
    void OurChangeFinish ( void );

    // downward power change initiated by a power parent
    IOReturn ParentChangeStart( void );
    void ParentChangeTellPriorityClientsPowerDown ( void );
    void ParentChangeTellCapabilityWillChange ( void );
    void ParentChangeNotifyInterestedDriversWillChange ( void );
    void ParentChangeSetPowerState ( void );
    void ParentChangeWaitForPowerSettle ( void );
    void ParentChangeNotifyInterestedDriversDidChange ( void );
    void ParentChangeTellCapabilityDidChange ( void );
    void ParentChangeAcknowledgePowerChange ( void );
    void ParentChangeRootChangeDown( void );

    void all_done ( void );
    void start_ack_timer ( void );
    void stop_ack_timer ( void );
    void start_ack_timer( UInt32 value, UInt32 scale );
    void startSettleTimer( void );
    void start_spindump_timer( const char * delay_type );
    void stop_spindump_timer( void );
    bool checkForDone ( void );
    bool responseValid ( uint32_t x, int pid );
    void computeDesiredState( unsigned long tempDesire, bool computeOnly );
    void trackSystemSleepPreventers( IOPMPowerStateIndex, IOPMPowerStateIndex, IOPMPowerChangeFlags );
    void tellSystemCapabilityChange( uint32_t nextMS );
    void restartIdleTimer( void );

    static void ack_timer_expired( thread_call_param_t, thread_call_param_t );
    static void watchdog_timer_expired ( thread_call_param_t arg0, thread_call_param_t arg1 );
    static void spindump_timer_expired( thread_call_param_t arg0, thread_call_param_t arg1 );
    static IOReturn actionAckTimerExpired(OSObject *, void *, void *, void *, void * );
    static IOReturn watchdog_timer_expired ( OSObject *, void *, void *, void *, void * );
    static IOReturn actionSpinDumpTimerExpired(OSObject *, void *, void *, void *, void * );

    static IOReturn actionDriverCalloutDone(OSObject *, void *, void *, void *, void * );
    static IOPMRequest * acquirePMRequest( IOService * target, IOOptionBits type, IOPMRequest * active = 0 );
    static void releasePMRequest( IOPMRequest * request );
    static void pmDriverCallout( IOService * from );
    static void pmTellAppWithResponse( OSObject * object, void * context );
    static void pmTellClientWithResponse( OSObject * object, void * context );
    static void pmTellCapabilityAppWithResponse ( OSObject * object, void * arg );
    static void pmTellCapabilityClientWithResponse( OSObject * object, void * arg );
    static void submitPMRequest( IOPMRequest * request );
    static void submitPMRequests( IOPMRequest ** request, IOItemCount count );
    bool ackTimerTick( void );
    void addPowerChild1( IOPMRequest * request );
    void addPowerChild2( IOPMRequest * request );
    void addPowerChild3( IOPMRequest * request );
    void adjustPowerState( uint32_t clamp = 0 );
    void handlePMstop( IOPMRequest * request );
    void handleRegisterPowerDriver( IOPMRequest * request );
    bool handleAcknowledgePowerChange( IOPMRequest * request );
    void handlePowerDomainWillChangeTo( IOPMRequest * request );
    void handlePowerDomainDidChangeTo( IOPMRequest * request );
    void handleRequestPowerState( IOPMRequest * request );
    void handlePowerOverrideChanged( IOPMRequest * request );
    void handleActivityTickle( IOPMRequest * request );
    void handleInterestChanged( IOPMRequest * request );
    void handleSynchronizePowerTree( IOPMRequest * request );
    void executePMRequest( IOPMRequest * request );
    bool actionPMWorkQueueInvoke( IOPMRequest * request, IOPMWorkQueue * queue );
    bool actionPMWorkQueueRetire( IOPMRequest * request, IOPMWorkQueue * queue );
    bool actionPMRequestQueue( IOPMRequest * request, IOPMRequestQueue * queue );
    bool actionPMReplyQueue( IOPMRequest * request, IOPMRequestQueue * queue );
    bool actionPMCompletionQueue( IOPMRequest * request, IOPMCompletionQueue * queue );
    bool notifyInterestedDrivers( void );
    void notifyInterestedDriversDone( void );
    bool notifyControllingDriver( void );
    void notifyControllingDriverDone( void );
    void driverSetPowerState( void );
    void driverInformPowerChange( void );
    bool isPMBlocked( IOPMRequest * request, int count );
    void notifyChildren( void );
    void notifyChildrenOrdered( void );
    void notifyChildrenDelayed( void );
    void notifyRootDomain( void );
    void notifyRootDomainDone( void );
    void cleanClientResponses ( bool logErrors );
    void updatePowerClient( const OSSymbol * client, uint32_t powerState );
    void removePowerClient( const OSSymbol * client );
    IOReturn requestPowerState( const OSSymbol * client, uint32_t state );
    IOReturn requestDomainPower( IOPMPowerStateIndex ourPowerState, IOOptionBits options = 0 );
    IOReturn configurePowerStatesReport( IOReportConfigureAction action, void *result );
    IOReturn updatePowerStatesReport( IOReportConfigureAction action, void *result, void *destination );
    IOReturn configureSimplePowerReport(IOReportConfigureAction action, void *result );
    IOReturn updateSimplePowerReport( IOReportConfigureAction action, void *result, void *destination );
    void waitForPMDriverCall( IOService * target = 0 );
#endif /* XNU_KERNEL_PRIVATE */
};

#endif /* ! _IOKIT_IOSERVICE_H */
