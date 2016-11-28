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
#ifndef _IOKIT_ROOTDOMAIN_H
#define _IOKIT_ROOTDOMAIN_H

#include <IOKit/IOService.h>
#include <IOKit/pwr_mgt/IOPM.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <sys/vnode.h>

#ifdef XNU_KERNEL_PRIVATE
struct AggressivesRecord;
struct IOPMMessageFilterContext;
struct IOPMActions;
struct IOPMSystemSleepParameters;
class PMSettingObject;
class PMTraceWorker;
class IOPMPowerStateQueue;
class RootDomainUserClient;
class PMAssertionsTracker;

#define OBFUSCATE(x) \
    (((((uintptr_t)(x)) >= VM_MIN_KERNEL_AND_KEXT_ADDRESS) && (((uintptr_t)(x)) < VM_MAX_KERNEL_ADDRESS)) ? \
        ((void *)(VM_KERNEL_ADDRPERM(x))) : (void *)(x))

#endif

/*!
 * Types for PM Assertions
 * For creating, releasing, and getting PM assertion levels.
 */

/*! IOPMDriverAssertionType
 * A bitfield describing a set of assertions. May be used to specify which assertions
 * to set with <link>IOPMrootDomain::createPMAssertion</link>; or to query which
 * assertions are set with <link>IOPMrootDomain::releasePMAssertion</link>.
 */
typedef uint64_t IOPMDriverAssertionType;

/* IOPMDriverAssertionID
 * Drivers may create PM assertions to request system behavior (keep the system awake,
 *  or keep the display awake). When a driver creates an assertion via
 *  <link>IOPMrootDomain::createPMAssertion</link>, PM returns a handle to
 *  the assertion of type IOPMDriverAssertionID.
 */
typedef uint64_t IOPMDriverAssertionID;
#define kIOPMUndefinedDriverAssertionID       0

/* IOPMDriverAssertionLevel
 * Possible values for IOPMDriverAssertionLevel are <link>kIOPMDriverAssertionLevelOff</link>
 * and <link>kIOPMDriverAssertionLevelOn</link>
 */
typedef uint32_t IOPMDriverAssertionLevel;
#define kIOPMDriverAssertionLevelOff          0
#define kIOPMDriverAssertionLevelOn           255

/*
 * Flags for get/setSleepSupported()
 */
enum {
    kRootDomainSleepNotSupported    = 0x00000000,
    kRootDomainSleepSupported         = 0x00000001,
    kFrameBufferDeepSleepSupported    = 0x00000002,
    kPCICantSleep                   = 0x00000004
};

/*
 *IOPMrootDomain registry property keys
 */
#define kRootDomainSupportedFeatures        "Supported Features"
#define kRootDomainSleepReasonKey           "Last Sleep Reason"
#define kRootDomainSleepOptionsKey          "Last Sleep Options"
#define kIOPMRootDomainWakeReasonKey        "Wake Reason"
#define kIOPMRootDomainWakeTypeKey          "Wake Type"
#define kIOPMRootDomainPowerStatusKey       "Power Status"

/*
 * Possible sleep reasons found under kRootDomainSleepReasonsKey
 */
#define kIOPMClamshellSleepKey              "Clamshell Sleep"
#define kIOPMPowerButtonSleepKey            "Power Button Sleep"
#define kIOPMSoftwareSleepKey               "Software Sleep"
#define kIOPMOSSwitchHibernationKey         "OS Switch Sleep"
#define kIOPMIdleSleepKey                   "Idle Sleep"
#define kIOPMLowPowerSleepKey               "Low Power Sleep"
#define kIOPMThermalEmergencySleepKey       "Thermal Emergency Sleep"
#define kIOPMMaintenanceSleepKey            "Maintenance Sleep"

/*
 * String constants for communication with PM CPU
 */
#define kIOPMRootDomainLidCloseCString      "LidClose"
#define kIOPMRootDomainBatPowerCString      "BatPower"

/*
 * Supported Feature bitfields for IOPMrootDomain::publishFeature()
 */
enum {
    kIOPMSupportedOnAC      = (1<<0),
    kIOPMSupportedOnBatt    = (1<<1),
    kIOPMSupportedOnUPS     = (1<<2)
};

typedef IOReturn (*IOPMSettingControllerCallback)
                    (OSObject *target, const OSSymbol *type,
                     OSObject *val, uintptr_t refcon);

__BEGIN_DECLS
IONotifier *    registerSleepWakeInterest(
                    IOServiceInterestHandler, void *, void * = 0);

IONotifier *    registerPrioritySleepWakeInterest(
                    IOServiceInterestHandler handler,
                    void * self, void * ref = 0);

IOReturn        acknowledgeSleepWakeNotification(void * );

IOReturn        vetoSleepWakeNotification(void * PMrefcon);
__END_DECLS

#define IOPM_ROOTDOMAIN_REV        2

class IOPMrootDomain: public IOService
{
    OSDeclareFinalStructors(IOPMrootDomain)

public:
    static IOPMrootDomain * construct( void );

    virtual bool        start( IOService * provider ) APPLE_KEXT_OVERRIDE;
    virtual IOReturn    setAggressiveness( unsigned long, unsigned long ) APPLE_KEXT_OVERRIDE;
    virtual IOReturn    getAggressiveness( unsigned long, unsigned long * ) APPLE_KEXT_OVERRIDE;

    virtual IOReturn    sleepSystem( void );
    IOReturn            sleepSystemOptions( OSDictionary *options );

    virtual IOReturn    setProperties( OSObject * ) APPLE_KEXT_OVERRIDE;
    virtual bool        serializeProperties( OSSerialize * s ) const APPLE_KEXT_OVERRIDE;
    virtual OSObject *  copyProperty( const char * aKey ) const APPLE_KEXT_OVERRIDE;

/*! @function systemPowerEventOccurred
    @abstract Other drivers may inform IOPMrootDomain of system PM events
    @discussion systemPowerEventOccurred is a richer alternative to receivePowerNotification()
        Only Apple-owned kexts should have reason to call systemPowerEventOccurred.
    @param event An OSSymbol describing the type of power event.
    @param value A 32-bit integer value associated with the event.
    @param shouldUpdate indicates whether the root domain should send a notification
        to interested parties. Pass false if you're calling systemPowerEventOccurred
        several times in succession; and pass true only on the last invocatino.
    @result kIOReturnSuccess on success */

    IOReturn            systemPowerEventOccurred(
                                    const OSSymbol *event,
                                    uint32_t intValue );

    IOReturn            systemPowerEventOccurred(
                                    const OSSymbol *event,
                                    OSObject *value );

#ifdef XNU_KERNEL_PRIVATE   // Hide doc from public headers
/*! @function claimSystemWakeEvent
    @abstract   Apple-internal SPI to describe system wake events.
    @discussion IOKit drivers may call claimSystemWakeEvent() during system wakeup to
                provide human readable debug information describing the event(s) that
                caused the system to wake.

                - Drivers should call claimSystemWakeEvent before completing
                  their setPowerState() acknowledgement. IOPMrootDomain stops
                  collecting wake events when driver wake is complete.

                - It is only appropriate to claim a wake event when the driver
                  can positively identify its hardware has generated an event
                  that can wake the system.

                - This call tracks wake events from a non-S0 state (S0i, S3, S4) into S0.
                - This call does not track wake events from DarkWake(S0) to FullWake(S0).

	Examples:
		(reason  = "WiFi.TCPData",
         details = "TCPKeepAlive packet arrived from IP 16.2.1.1")
		(reason  = "WiFi.ScanOffload",
         details = "WiFi station 'AppleWiFi' signal dropped below threshold")
		(reason  = "Enet.LinkToggle",
         details = "Ethernet attached")

    @param device   The device/nub that is associated with the wake event.

    @param flags    Pass kIOPMWakeEventSource if the device is the source
                    of the wake event. Pass zero if the device is forwarding or
                    aggregating wake events from multiple sources, e.g. an USB or
                    Thunderbolt host controller.

    @param reason   Caller should pass a human readable C string describing the
                    wake reason. Please use a string from the list below, or create
                    your own string matching this format:
                              [Hardware].[Event]
                              WiFi.MagicPacket
                              WiFi.ScanOffload
                              WiFi.mDNSConflict
                              WiFi.mDNSService
                              WiFi.TCPData
                              WiFi.TCPTimeout
                              WiFi.FirmwareCrash
                              Enet.MagicPacket
                              Enet.mDNSConflict
                              Enet.mDNSService
                              Enet.TCPData
                              Enet.TCPTimeout
                              Enet.Service
                              Enet.LinkToggle
                              Enet.ConflictResolution
                              Enet.PatternMatch
                              Enet.Timer
                              Enet.LinkUpTimeout
                              Enet.LinkDown
                              USB.DeviceAttach
                              USB.DeviceDetach

     @param details Optional details further describing the wake event.
                    Please pass an OSString defining the event.
     */
#endif
    void 				claimSystemWakeEvent( IOService     *device,
                                              IOOptionBits  flags,
                                              const char    *reason,
                                              OSObject      *details = 0 );

    virtual IOReturn    receivePowerNotification( UInt32 msg );

    virtual void        setSleepSupported( IOOptionBits flags );

    virtual IOOptionBits getSleepSupported( void );

    void                wakeFromDoze( void );

    // KEXT driver announces support of power management feature

    void                publishFeature( const char *feature );

    // KEXT driver announces support of power management feature
    // And specifies power sources with kIOPMSupportedOn{AC/Batt/UPS} bitfield.
    // Returns a unique uint32_t identifier for later removing support for this
    // feature.
    // NULL is acceptable for uniqueFeatureID for kexts without plans to unload.

    void                publishFeature( const char *feature,
                                        uint32_t supportedWhere,
                                        uint32_t *uniqueFeatureID);

    // KEXT driver announces removal of a previously published power management
    // feature. Pass 'uniqueFeatureID' returned from publishFeature()

    IOReturn            removePublishedFeature( uint32_t removeFeatureID );

/*! @function copyPMSetting
    @abstract Copy the current value for a PM setting. Returns an OSNumber or
        OSData depending on the setting.
    @param whichSetting Name of the desired setting.
    @result OSObject value if valid, NULL otherwise. */

    OSObject *          copyPMSetting( OSSymbol *whichSetting );

/*! @function registerPMSettingController
    @abstract Register for callbacks on changes to certain PM settings.
    @param settings NULL terminated array of C strings, each string for a PM
        setting that the caller is interested in and wants to get callbacks for.
    @param callout C function ptr or member function cast as such.
    @param target The target of the callback, usually 'this'
    @param refcon Will be passed to caller in callback; for caller's use.
    @param handle Caller should keep the OSObject * returned here. If non-NULL,
        handle will have a retain count of 1 on return. To deregister, pass to
        unregisterPMSettingController()
    @result kIOReturnSuccess on success. */

    IOReturn            registerPMSettingController(
                                 const OSSymbol *settings[],
                                 IOPMSettingControllerCallback callout,
                                 OSObject   *target,
                                 uintptr_t  refcon,
                                 OSObject   **handle);    // out param

/*! @function registerPMSettingController
    @abstract Register for callbacks on changes to certain PM settings.
    @param settings NULL terminated array of C strings, each string for a PM
        setting that the caller is interested in and wants to get callbacks for.
    @param supportedPowerSources bitfield indicating which power sources these
        settings are supported for (kIOPMSupportedOnAC, etc.)
    @param callout C function ptr or member function cast as such.
    @param target The target of the callback, usually 'this'
    @param refcon Will be passed to caller in callback; for caller's use.
    @param handle Caller should keep the OSObject * returned here. If non-NULL,
        handle will have a retain count of 1 on return. To deregister, pass to
        unregisterPMSettingController()
    @result kIOReturnSuccess on success. */

    IOReturn            registerPMSettingController(
                                 const OSSymbol *settings[],
                                 uint32_t   supportedPowerSources,
                                 IOPMSettingControllerCallback callout,
                                 OSObject   *target,
                                 uintptr_t  refcon,
                                 OSObject   **handle);    // out param

    virtual IONotifier * registerInterest(
                                const OSSymbol * typeOfInterest,
                                IOServiceInterestHandler handler,
                                void * target, void * ref = 0 ) APPLE_KEXT_OVERRIDE;

    virtual IOReturn    callPlatformFunction(
                                const OSSymbol *functionName,
                                bool waitForFunction,
                                void *param1, void *param2,
                                void *param3, void *param4 ) APPLE_KEXT_OVERRIDE;

/*! @function createPMAssertion
    @abstract Creates an assertion to influence system power behavior.
    @param whichAssertionBits A bitfield specify the assertion that the caller requests.
    @param assertionLevel An integer detailing the initial assertion level, kIOPMDriverAssertionLevelOn
        or kIOPMDriverAssertionLevelOff.
    @param ownerService A pointer to the caller's IOService class, for tracking.
    @param ownerDescription A reverse-DNS string describing the caller's identity and reason.
    @result On success, returns a new assertion of type IOPMDriverAssertionID
*/
    IOPMDriverAssertionID createPMAssertion(
                                IOPMDriverAssertionType whichAssertionsBits,
                                IOPMDriverAssertionLevel assertionLevel,
                                IOService *ownerService,
                                const char *ownerDescription);

/* @function setPMAssertionLevel
   @abstract Modify the level of a pre-existing assertion.
   @discussion Change the value of a PM assertion to influence system behavior,
    without undergoing the work required to create or destroy an assertion. Suggested
    for clients who will assert and de-assert needs for PM behavior several times over
    their lifespan.
   @param assertionID An assertion ID previously returned by <link>createPMAssertion</link>
   @param assertionLevel The new assertion level.
   @result kIOReturnSuccess if it worked; kIOReturnNotFound or other IOReturn error on failure.
*/
    IOReturn setPMAssertionLevel(IOPMDriverAssertionID assertionID, IOPMDriverAssertionLevel assertionLevel);

/*! @function getPMAssertionLevel
    @absract Returns the active level of the specified assertion(s).
    @discussion Returns <link>kIOPMDriverAssertionLevelOff</link> or
        <link>kIOPMDriverAssertionLevelOn</link>. If multiple assertions are specified
        in the bitfield, only returns <link>kIOPMDriverAssertionLevelOn</link>
        if all assertions are active.
    @param whichAssertionBits Bits defining the assertion or assertions the caller is interested in
        the level of. If in doubt, pass <link>kIOPMDriverAssertionCPUBit</link> as the argument.
    @result Returns <link>kIOPMDriverAssertionLevelOff</link> or
        <link>kIOPMDriverAssertionLevelOn</link> indicating the specified assertion's levels, if available.
        If the assertions aren't supported on this machine, or aren't recognized by the OS, the
        result is undefined.
*/
    IOPMDriverAssertionLevel getPMAssertionLevel(IOPMDriverAssertionType whichAssertionBits);

/*! @function releasePMAssertion
    @abstract Removes an assertion to influence system power behavior.
    @result On success, returns a new assertion of type IOPMDriverAssertionID *
*/
    IOReturn releasePMAssertion(IOPMDriverAssertionID releaseAssertion);

/*! @function restartWithStackshot
    @abstract Take a stackshot of the system and restart the system.
    @result Return kIOReturnSuccess if it work, kIOReturnError if the service is not available.
*/
    IOReturn restartWithStackshot();

private:
    virtual IOReturn    changePowerStateTo( unsigned long ordinal ) APPLE_KEXT_COMPATIBILITY_OVERRIDE;
    virtual IOReturn    changePowerStateToPriv( unsigned long ordinal );
    virtual IOReturn    requestPowerDomainState( IOPMPowerFlags, IOPowerConnection *, unsigned long ) APPLE_KEXT_OVERRIDE;
    virtual void        powerChangeDone( unsigned long ) APPLE_KEXT_OVERRIDE;
    virtual bool        tellChangeDown( unsigned long ) APPLE_KEXT_OVERRIDE;
    virtual bool        askChangeDown( unsigned long ) APPLE_KEXT_OVERRIDE;
    virtual void        tellChangeUp( unsigned long ) APPLE_KEXT_OVERRIDE;
    virtual void        tellNoChangeDown( unsigned long ) APPLE_KEXT_OVERRIDE;
    virtual IOReturn configureReport(IOReportChannelList   *channels,
                                    IOReportConfigureAction action,
                                    void                    *result,
                                    void                    *destination) APPLE_KEXT_OVERRIDE;
    virtual IOReturn updateReport(IOReportChannelList      *channels,
                                  IOReportUpdateAction     action,
                                  void                     *result,
                                  void                     *destination) APPLE_KEXT_OVERRIDE;

    void             configureReportGated(uint64_t channel_id,
                                          uint64_t action,
                                          void     *result);
    IOReturn         updateReportGated(uint64_t ch_id, 
                                       void *result, 
                                       IOBufferMemoryDescriptor *dest);

#ifdef XNU_KERNEL_PRIVATE
    /* Root Domain internals */
public:
    void        tagPowerPlaneService(
                    IOService *     service,
                    IOPMActions *   actions );

    void        overrideOurPowerChange(
                    IOService *             service,
                    IOPMActions *           actions,
                    IOPMPowerStateIndex *   inOutPowerState,
                    IOPMPowerChangeFlags *  inOutChangeFlags,
                    IOPMRequestTag          requestTag );

    void        handleOurPowerChangeStart(
                    IOService *             service,
                    IOPMActions *           actions,
                    IOPMPowerStateIndex     powerState,
                    IOPMPowerChangeFlags *  inOutChangeFlags,
                    IOPMRequestTag          requestTag );

    void        handleOurPowerChangeDone(
                    IOService *             service,
                    IOPMActions *           actions,
                    IOPMPowerStateIndex     powerState,
                    IOPMPowerChangeFlags    changeFlags,
                    IOPMRequestTag          requestTag );

    void        overridePowerChangeForUIService(
                    IOService *             service,
                    IOPMActions *           actions,
                    IOPMPowerStateIndex *   inOutPowerState,
                    IOPMPowerChangeFlags *  inOutChangeFlags );

    void        handleActivityTickleForDisplayWrangler(
                    IOService *             service,
                    IOPMActions *           actions );

    void        handleUpdatePowerClientForDisplayWrangler(
                    IOService *             service,
                    IOPMActions *           actions,
                    const OSSymbol *        powerClient,
                    IOPMPowerStateIndex     oldPowerState,
                    IOPMPowerStateIndex     newPowerState );

    bool        shouldDelayChildNotification(
                    IOService *     service );

    void        handlePowerChangeStartForPCIDevice(
                    IOService *             service,
                    IOPMActions *           actions,
                    IOPMPowerStateIndex     powerState,
                    IOPMPowerChangeFlags *  inOutChangeFlags );

    void        handlePowerChangeDoneForPCIDevice(
                    IOService *             service,
                    IOPMActions *           actions,
                    IOPMPowerStateIndex     powerState,
                    IOPMPowerChangeFlags    changeFlags );

    void        askChangeDownDone(
                    IOPMPowerChangeFlags * inOutChangeFlags,
                    bool * cancel );

    void        handlePublishSleepWakeUUID(
                    bool shouldPublish);

    void        handleQueueSleepWakeUUID(
                    OSObject *obj);

    void        handleDisplayPowerOn( );

    void        willNotifyPowerChildren( IOPMPowerStateIndex newPowerState );

    IOReturn    setMaintenanceWakeCalendar(
                    const IOPMCalendarStruct * calendar );

    IOReturn    getSystemSleepType( uint32_t * sleepType );

    // Handle callbacks from IOService::systemWillShutdown()
    void        acknowledgeSystemWillShutdown( IOService * from );

    // Handle platform halt and restart notifications
    void        handlePlatformHaltRestart( UInt32 pe_type );

    IOReturn    shutdownSystem( void );
    IOReturn    restartSystem( void );
    void        handleSleepTimerExpiration( void );

    bool        activitySinceSleep(void);
    bool        abortHibernation(void);
    void        updateConsoleUsers(void);

    IOReturn    joinAggressiveness( IOService * service );
    void        handleAggressivesRequests( void );

    void        tracePoint( uint8_t point );
    void        tracePoint( uint8_t point, uint8_t data );
    void        traceDetail( uint32_t data32 );

    bool        systemMessageFilter(
                    void * object, void * arg1, void * arg2, void * arg3 );

    bool        updatePreventIdleSleepList(
                    IOService * service, bool addNotRemove );
    void        updatePreventSystemSleepList(
                    IOService * service, bool addNotRemove );

    void        publishPMSetting(
                    const OSSymbol * feature, uint32_t where, uint32_t * featureID );

    void        pmStatsRecordEvent(
                                int             eventIndex,
                                AbsoluteTime    timestamp);

    void        pmStatsRecordApplicationResponse(
                                const OSSymbol      *response,
                                const char          *name,
                                int                 messageType,
                                uint32_t            delay_ms,
                                int                 app_pid,
                                OSObject            *object,
                                IOPMPowerStateIndex ps=0);

    void        copyWakeReasonString( char * outBuf, size_t bufSize );

#if HIBERNATION
    bool        getHibernateSettings(
                    uint32_t *  hibernateMode,
                    uint32_t *  hibernateFreeRatio,
                    uint32_t *  hibernateFreeTime );
#endif
    void        takeStackshot(bool restart, bool isOSXWatchdog, bool isSpinDump);
    void        sleepWakeDebugTrig(bool restart);
    void        sleepWakeDebugEnableWdog();
    bool        sleepWakeDebugIsWdogEnabled();
    static void saveTimeoutAppStackShot(void *p0, void *p1);
    void        sleepWakeDebugSaveSpinDumpFile();

private:
    friend class PMSettingObject;
    friend class RootDomainUserClient;
    friend class PMAssertionsTracker;

    static IOReturn sysPowerDownHandler( void * target, void * refCon,
                                    UInt32 messageType, IOService * service,
                                    void * messageArgument, vm_size_t argSize );

    static IOReturn displayWranglerNotification( void * target, void * refCon,
                                    UInt32 messageType, IOService * service,
                                    void * messageArgument, vm_size_t argSize );

    static IOReturn rootBusyStateChangeHandler( void * target, void * refCon,
                                    UInt32 messageType, IOService * service,
                                    void * messageArgument, vm_size_t argSize );

    static bool displayWranglerMatchPublished( void * target, void * refCon,
                                    IOService * newService,
                                    IONotifier * notifier);

    static bool IONVRAMMatchPublished( void * target, void * refCon,
                                    IOService * newService,
                                    IONotifier * notifier);

    static bool batteryPublished( void * target, void * refCon,
                                    IOService * resourceService,
                                    IONotifier * notifier);

    void initializeBootSessionUUID( void );

    void fullWakeDelayedWork( void );

    IOService *             wrangler;
    OSDictionary *          wranglerIdleSettings;

    IOLock                  *featuresDictLock;  // guards supportedFeatures
    IOLock                  *wakeEventLock;
    IOPMPowerStateQueue     *pmPowerStateQueue;

    OSArray                 *allowedPMSettings;
    OSArray                 *noPublishPMSettings;
    PMTraceWorker           *pmTracer;
    PMAssertionsTracker     *pmAssertions;

    // Settings controller info
    IOLock                  *settingsCtrlLock;
    OSDictionary            *settingsCallbacks;
    OSDictionary            *fPMSettingsDict;

    IONotifier              *_batteryPublishNotifier;
    IONotifier              *_displayWranglerNotifier;

    // Statistics
    const OSSymbol          *_statsNameKey;
    const OSSymbol          *_statsPIDKey;
    const OSSymbol          *_statsTimeMSKey;
    const OSSymbol          *_statsResponseTypeKey;
    const OSSymbol          *_statsMessageTypeKey;
    const OSSymbol          *_statsPowerCapsKey;
    uint32_t                sleepCnt;
    uint32_t                darkWakeCnt;
    uint32_t                displayWakeCnt;

    OSString                *queuedSleepWakeUUIDString;
    OSArray                 *pmStatsAppResponses;
    IOLock                  *pmStatsLock;   // guards pmStatsAppResponses

    void                    *sleepDelaysReport;     // report to track time taken to go to sleep
    uint32_t                sleepDelaysClientCnt;   // Number of interested clients in sleepDelaysReport
    uint64_t                ts_sleepStart;
    uint64_t                wake2DarkwakeDelay;      // Time taken to change from full wake -> Dark wake


    void                    *assertOnWakeReport;    // report to track time spent without any assertions held after wake
    uint32_t                assertOnWakeClientCnt;  // Number of clients interested in assertOnWakeReport
    clock_sec_t             assertOnWakeSecs;       // Num of secs after wake for first assertion

    bool                    uuidPublished;

    // Pref: idle time before idle sleep
    unsigned long           sleepSlider;
    unsigned long           idleSeconds;
    uint64_t                autoWakeStart;
    uint64_t                autoWakeEnd;

    // Difference between sleepSlider and longestNonSleepSlider
    unsigned long           extraSleepDelay;

    // Used to wait between say display idle and system idle
    thread_call_t           extraSleepTimer;
    thread_call_t           diskSyncCalloutEntry;
    thread_call_t           fullWakeThreadCall;
    thread_call_t           hibDebugSetupEntry;
    thread_call_t           updateConsoleUsersEntry;

    // Track system capabilities.
    uint32_t                _desiredCapability;
    uint32_t                _currentCapability;
    uint32_t                _pendingCapability;
    uint32_t                _highestCapability;
    OSSet *                 _joinedCapabilityClients;
    uint32_t                _systemStateGeneration;

    // Type of clients that can receive system messages.
    enum {
        kSystemMessageClientPowerd    = 0x01,
        kSystemMessageClientLegacyApp = 0x02,
        kSystemMessageClientKernel    = 0x04,
        kSystemMessageClientAll       = 0x07
    };
    uint32_t                _systemMessageClientMask;

    // Power state and capability change transitions.
    enum {
        kSystemTransitionNone         = 0,
        kSystemTransitionSleep        = 1,
        kSystemTransitionWake         = 2,
        kSystemTransitionCapability   = 3,
        kSystemTransitionNewCapClient = 4
    }                       _systemTransitionType;

    unsigned int            systemBooting           :1;
    unsigned int            systemShutdown          :1;
    unsigned int            systemDarkWake          :1;
    unsigned int            clamshellExists         :1;
    unsigned int            clamshellClosed         :1;
    unsigned int            clamshellDisabled       :1;
    unsigned int            desktopMode             :1;
    unsigned int            acAdaptorConnected      :1;

    unsigned int            clamshellSleepDisabled  :1;
    unsigned int            idleSleepTimerPending   :1;
    unsigned int            userDisabledAllSleep    :1;
    unsigned int            ignoreTellChangeDown    :1;
    unsigned int            wranglerAsleep          :1;
    unsigned int            wranglerTickled         :1;
    unsigned int            _preventUserActive      :1;
    unsigned int            graphicsSuppressed      :1;

    unsigned int            capabilityLoss          :1;
    unsigned int            pciCantSleepFlag        :1;
    unsigned int            pciCantSleepValid       :1;
    unsigned int            logGraphicsClamp        :1;
    unsigned int            darkWakeToSleepASAP     :1;
    unsigned int            darkWakeMaintenance     :1;
    unsigned int            darkWakeSleepService    :1;
    unsigned int            darkWakePostTickle      :1;

    unsigned int            sleepTimerMaintenance   :1;
    unsigned int            sleepToStandby          :1;
    unsigned int            lowBatteryCondition     :1;
    unsigned int            hibernateDisabled       :1;
    unsigned int            hibernateRetry          :1;
    unsigned int            wranglerTickleLatched   :1;
    unsigned int            userIsActive            :1;
    unsigned int            userWasActive           :1;

    unsigned int            displayIdleForDemandSleep :1;
    unsigned int            darkWakeHibernateError  :1;
    unsigned int            thermalWarningState:1;
    unsigned int            toldPowerdCapWillChange :1;
    unsigned int            displayPowerOnRequested:1;

    uint8_t                 tasksSuspended;
    uint32_t                hibernateMode;
    AbsoluteTime            userActivityTime;
    AbsoluteTime            userActivityTime_prev;
    uint32_t                userActivityCount;
    uint32_t                userActivityAtSleep;
    uint32_t                lastSleepReason;
    uint32_t                fullToDarkReason;
    uint32_t                hibernateAborted;

    enum FullWakeReason {
        kFullWakeReasonNone = 0,
        kFullWakeReasonLocalUser = 1,
        kFullWakeReasonDisplayOn = 2,
        fFullWakeReasonDisplayOnAndLocalUser = 3
    };
    uint32_t                fullWakeReason;

    // Info for communicating system state changes to PMCPU
    int32_t                 idxPMCPUClamshell;
    int32_t                 idxPMCPULimitedPower;

    IOOptionBits            platformSleepSupport;
    uint32_t                _debugWakeSeconds;
    uint32_t                _lastDebugWakeSeconds;

    queue_head_t            aggressivesQueue;
    thread_call_t           aggressivesThreadCall;
    OSData *                aggressivesData;

    AbsoluteTime            userBecameInactiveTime;
    AbsoluteTime            systemWakeTime;

    // PCI top-level PM trace
    IOService *             pciHostBridgeDevice;
    IOService *             pciHostBridgeDriver;

    IONotifier *            systemCapabilityNotifier;

    typedef struct {
        uint32_t            pid;
        uint32_t            refcount;
    } PMNotifySuspendedStruct;

    uint32_t                pmSuspendedCapacity;
    uint32_t                pmSuspendedSize;
    PMNotifySuspendedStruct *pmSuspendedPIDS;

    OSSet *                 preventIdleSleepList;
    OSSet *                 preventSystemSleepList;

    UInt32                  _scheduledAlarms;
    UInt32                  _userScheduledAlarm;

#if HIBERNATION
    clock_sec_t             _standbyTimerResetSeconds;
#endif
    volatile uint32_t   swd_lock;    /* Lock to access swd_buffer & and its header */
    void  *             swd_buffer;  /* Memory allocated for dumping sleep/wake logs */
    uint8_t             swd_flags;   /* Flags defined in IOPMPrivate.h */
    void  *             swd_spindump_buffer;

    IOMemoryMap  *      swd_logBufMap; /* Memory with sleep/wake logs from previous boot */

    // Wake Event Reporting
    OSArray *               _systemWakeEventsArray;
    bool                    _acceptSystemWakeEvents;

    int         findSuspendedPID(uint32_t pid, uint32_t *outRefCount);

    // IOPMrootDomain internal sleep call
    IOReturn    privateSleepSystem( uint32_t sleepReason );
    void        reportUserInput( void );
    void        setDisableClamShellSleep( bool );
    bool        checkSystemSleepAllowed( IOOptionBits options,
                                         uint32_t sleepReason );
    bool        checkSystemSleepEnabled( void );
    bool        checkSystemCanSleep( uint32_t sleepReason );
    bool        checkSystemCanSustainFullWake( void );

    void        adjustPowerState( bool sleepASAP = false );
    void        setQuickSpinDownTimeout( void );
    void        restoreUserSpinDownTimeout( void );

    bool        shouldSleepOnClamshellClosed(void );
    void        sendClientClamshellNotification( void );

    // Inform PMCPU of changes to state like lid, AC vs. battery
    void        informCPUStateChange( uint32_t type, uint32_t value );

    void        dispatchPowerEvent( uint32_t event, void * arg0, uint64_t arg1 );
    void        handlePowerNotification( UInt32 msg );

    IOReturn    setPMSetting(const OSSymbol *, OSObject *);

    void        startIdleSleepTimer( uint32_t inSeconds );
    void        cancelIdleSleepTimer( void );
    uint32_t    getTimeToIdleSleep( void );

    IOReturn    setAggressiveness(
                        unsigned long type,
                        unsigned long value,
                        IOOptionBits  options );

    void        synchronizeAggressives(
                        queue_head_t * services,
                        const AggressivesRecord * array,
                        int count );

    void        broadcastAggressives(
                        const AggressivesRecord * array,
                        int count );

    IOReturn    setPMAssertionUserLevels(IOPMDriverAssertionType);

    void        publishSleepWakeUUID( bool shouldPublish );

    void        evaluatePolicy( int stimulus, uint32_t arg = 0 );
    void        requestFullWake( FullWakeReason reason );
    void        willEnterFullWake( void );

    void        evaluateAssertions(IOPMDriverAssertionType newAssertions,
                                   IOPMDriverAssertionType oldAssertions);

    void        deregisterPMSettingObject( PMSettingObject * pmso );

    void        checkForValidDebugData(const char *fname, vfs_context_t *ctx, 
                                            void *tmpBuf, struct vnode **vp);
    void        sleepWakeDebugMemAlloc( );
    void        sleepWakeDebugSpinDumpMemAlloc( );
    void        sleepWakeDebugDumpFromMem(IOMemoryMap *logBufMap);
    void        sleepWakeDebugDumpFromFile( );
    IOMemoryMap *sleepWakeDebugRetrieve();
    errno_t     sleepWakeDebugSaveFile(const char *name, char *buf, int len);
    errno_t     sleepWakeDebugCopyFile( struct vnode *srcVp,
                               vfs_context_t srcCtx,
                               char *tmpBuf, uint64_t tmpBufSize,
                               uint64_t srcOffset, 
                               const char *dstFname, 
                               uint64_t numBytes,
                               uint32_t crc);


#if HIBERNATION
    bool        getSleepOption( const char * key, uint32_t * option );
    bool        evaluateSystemSleepPolicy( IOPMSystemSleepParameters * p,
                                           int phase, uint32_t * hibMode );
    void        evaluateSystemSleepPolicyEarly( void );
    void        evaluateSystemSleepPolicyFinal( void );
#endif /* HIBERNATION */

    bool        latchDisplayWranglerTickle( bool latch );
    void        setDisplayPowerOn( uint32_t options );

    void        acceptSystemWakeEvents( bool accept );
    void        systemDidNotSleep( void );
    void        preventTransitionToUserActive( bool prevent );
    void        setThermalState(OSObject *value);
    void        copySleepPreventersList(OSArray  **idleSleepList, OSArray  **systemSleepList);
#endif /* XNU_KERNEL_PRIVATE */
};

#ifdef XNU_KERNEL_PRIVATE
class IORootParent: public IOService
{
    OSDeclareFinalStructors(IORootParent)

public:
    static void initialize( void );
    virtual OSObject * copyProperty( const char * aKey ) const APPLE_KEXT_OVERRIDE;
    bool start( IOService * nub ) APPLE_KEXT_OVERRIDE;
    void shutDownSystem( void );
    void restartSystem( void );
    void sleepSystem( void );
    void dozeSystem( void );
    void sleepToDoze( void );
    void wakeSystem( void );
};
#endif /* XNU_KERNEL_PRIVATE */

#endif /* _IOKIT_ROOTDOMAIN_H */
