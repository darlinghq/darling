/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
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
#ifndef _IOKIT_IOPMPRIVATE_H
#define _IOKIT_IOPMPRIVATE_H

#include <IOKit/pwr_mgt/IOPM.h>

/* @constant kIOPMEventTypeIntermediateFlag
 * @abstract This bit indicates the event is an intermediate event
 *      which must occur within a major system power event.
 */
#define kIOPMEventTypeIntermediateFlag              0x10000000

/* @enum SystemEventTypes
 * @abstract Potential system events logged in the system event record.
 */
enum {
    kIOPMEventTypeUndefined                     = 0,

    /* Event types mark driver events
     */
    kIOPMEventTypeSetPowerStateImmediate        = 1001,
    kIOPMEventTypeSetPowerStateDelayed          = 1002,
    kIOPMEventTypePSWillChangeTo                = 1003,
    kIOPMEventTypePSDidChangeTo                 = 1004,
    kIOPMEventTypeAppResponse                   = 1005,


    /* Start and stop event types bracket major
     * system power management events.
     */
    kIOPMEventTypeSleep                         = 2001,
    kIOPMEventTypeSleepDone                     = 2002,
    kIOPMEventTypeWake                          = 3001,
    kIOPMEventTypeWakeDone                      = 3002,
    kIOPMEventTypeDoze                          = 4001,
    kIOPMEventTypeDozeDone                      = 4002,
    kIOPMEventTypeUUIDSet                       = 6001,
    kIOPMEventTypeUUIDClear                     = 6002,

    /* Intermediate events that may only occur within the bounds
     * of a major system event (between the event's initiation and its "done event".)
     * e.g. chronologically kIOPMEventTypeSleep may be followed by one or more
     *      intermediate events, which then must be followed by kIOPMEventTypeSleepDone.
     *
     * The intermediate events below will always occur in a Sleep or Wake event, and may
     *      or may not occur for any of the other events.
     */
    kIOPMEventTypeAppNotificationsFinished      = 501 | kIOPMEventTypeIntermediateFlag,
    kIOPMEventTypeDriverNotificationsFinished   = 502 | kIOPMEventTypeIntermediateFlag,
    kIOPMEventTypeCalTimeChange                 = 503 | kIOPMEventTypeIntermediateFlag
};

/*****************************************************************************
 *
 * Private Root Domain general interest messages
 *
 * Check IOPM.h when adding new messages to avoid conflict.
 *
 *****************************************************************************/

/* kIOPMMessageUserIsActiveChanged
 * User became active or inactive. Message sent after the kIOPMUserIsActiveKey
 * property was updated with a new value.
 */
#define kIOPMMessageUserIsActiveChanged \
                iokit_family_msg(sub_iokit_powermanagement, 0x400)

/*
 * Private IOMessage notifications shared between kernel and userspace PM policy
 */
#define kIOPMMessageLastCallBeforeSleep \
                iokit_family_msg(sub_iokit_powermanagement, 0x410)

#define kIOPMMessageIdleSleepPreventers \
                iokit_family_msg(sub_iokit_powermanagement, 0x420)

#define kIOPMMessageSystemSleepPreventers \
                iokit_family_msg(sub_iokit_powermanagement, 0x430)

#define kIOPMMessageLaunchBootSpinDump \
                iokit_family_msg(sub_iokit_powermanagement, 0x440)

#define kIOPMMessageProModeStateChange \
                iokit_family_msg(sub_iokit_powermanagement, 0x450)

/* @enum SystemSleepReasons
 * @abstract The potential causes for system sleep as logged in the system event record.
 */
enum {
    kIOPMSleepReasonClamshell                   = 101,
    kIOPMSleepReasonPowerButton                 = 102,
    kIOPMSleepReasonSoftware                    = 103,
    kIOPMSleepReasonOSSwitchHibernate           = 104,
    kIOPMSleepReasonIdle                        = 105,
    kIOPMSleepReasonLowPower                    = 106,
    kIOPMSleepReasonThermalEmergency            = 107,
    kIOPMSleepReasonMaintenance                 = 108,
    kIOPMSleepReasonSleepServiceExit            = 109,
    kIOPMSleepReasonDarkWakeThermalEmergency    = 110
};

/*
 * Possible C-string sleep reasons found under kRootDomainSleepReasonsKey
 */
#define kIOPMClamshellSleepKey                      "Clamshell Sleep"
#define kIOPMPowerButtonSleepKey                    "Power Button Sleep"
#define kIOPMSoftwareSleepKey                       "Software Sleep"
#define kIOPMOSSwitchHibernationKey                 "OS Switch Sleep"
#define kIOPMIdleSleepKey                           "Idle Sleep"
#define kIOPMLowPowerSleepKey                       "Low Power Sleep"
#define kIOPMThermalEmergencySleepKey               "Thermal Emergency Sleep"
#define kIOPMSleepServiceExitKey                    "Sleep Service Back to Sleep"
#define kIOPMDarkWakeThermalEmergencyKey            "Dark Wake Thermal Emergency"

/*! kIOPMPSRestrictedModeKey
 *  An IOPMPowerSource property key
 *  Its property has an integer value.
 *  - value = 1 when the device is in a low power state and not fully functional.
 *  - value = 0, or property missing altogether, when the device is
 *      not in a restricted mode power state.
*/
#define kIOPMPSRestrictedModeKey                    "RestrictedMode"

#pragma mark Stray Bitfields
// Private power commands issued to root domain
// bits 0-7 in IOPM.h

enum {
    kIOPMSetValue                   = (1<<16),
    // don't sleep on clamshell closure on a portable with AC connected
    kIOPMSetDesktopMode             = (1<<17),
    // set state of AC adaptor connected
    kIOPMSetACAdaptorConnected      = (1<<18)
};

/*****************************************************************************/
/*****************************************************************************/

/*
 * PM notification types
 */

/*! @constant kIOPMSleepServiceScheduleImmediate
 *
 * Setting type used in calls to IOPMrootDomain::registerPMSettingController
 * Use this type between powerd and IOKit.framework
 *
 */
#define kIOPMSleepServiceScheduleImmediate     "SleepServiceImmediate"

/*! @constant kIOPMSettingSleepServiceScheduleImmediate
 *
 * Setting type used in calls to IOPMrootDomain::registerPMSettingController
 * Use this type between xnu and AppleRTC
 */
#define kIOPMSettingSleepServiceWakeCalendarKey     "SleepServiceWakeCalendarKey"

/*! @constant kIOPMCalendarWakeTypes
 *
 * These are valid values for IOPM.h:IOPMCalendarStruct->selector
 */
enum {
    kPMCalendarTypeInvalid = 0,
    kPMCalendarTypeMaintenance = 1,
    kPMCalendarTypeSleepService = 2
};


/* @constant kIOPMStateConsoleShutdown
 * @abstract Notification of GUI shutdown state available to kexts.
 * @discussion This type can be passed as arguments to registerPMSettingController()
 * to receive callbacks.
 */
#define kIOPMStateConsoleShutdown   "ConsoleShutdown"

/* @enum ShutdownValues
 * @abstract Potential values shared with key kIOPMStateConsoleShutdown
 */
enum {
/* @constant kIOPMStateConsoleShutdownNone
 * @abstract System shutdown (or restart) hasn't started; system is ON.
 * @discussion Next state: 2
 */
    kIOPMStateConsoleShutdownNone   = 1,
/* @constant kIOPMStateConsoleShutdownPossible
 * @abstract User has been presented with the option to shutdown or restart. Shutdown may be cancelled.
 * @discussion Next state may be: 1, 4
 */
    kIOPMStateConsoleShutdownPossible = 2,
/* @constant kIOPMStateConsoleShutdownUnderway
 * @abstract Shutdown or restart is proceeding. It may still be cancelled.
 * @discussion Next state may be: 1, 4. This state is currently unused.
 */
    kIOPMStateConsoleShutdownUnderway = 3,
/* @constant kIOPMStateConsoleShutdownCertain
 * @abstract Shutdown is in progress and irrevocable.
 * @discussion State remains 4 until power is removed from CPU.
 */
    kIOPMStateConsoleShutdownCertain = 4,
/* @constant kIOPMStateConsoleSULogoutInitiated
   Indicates that LoginWindow has initiated a software update restart.
   The next logout will not immediately lead to a shutdown.
 */
    kIOPMStateConsoleSULogoutInitiated = 5
};

/* @constant kIOPMSettingSilentRunningKey
 * @abstract Notification of silent running mode changes to kexts.
 * @discussion This key can be passed as an argument to registerPMSettingController()
 * and also identifies the type of PMSetting notification callback.
 */
#define kIOPMSettingSilentRunningKey    "SilentRunning"
#define kIOPMFeatureSilentRunningKey    kIOPMSettingSilentRunningKey

/* @enum SilentRunningFlags
 * @abstract The kIOPMSettingSilentRunningKey notification provides an OSNumber
 * object with a value described by the following flags.
 */
enum {
    kIOPMSilentRunningModeOn = 0x00000001
};

/*****************************************************************************/
/*****************************************************************************/

/* PM Statistics - event indices
 * These are arguments to IOPMrootDomain::pmStatsRecordEvent().
 */
enum {
    kIOPMStatsHibernateImageWrite         = 1,
    kIOPMStatsHibernateImageRead,
    kIOPMStatsDriversNotify,
    kIOPMStatsApplicationNotify,
    kIOPMStatsLateDriverAcknowledge,
    kIOPMStatsLateAppAcknowledge,

    // To designate if you're specifying the start or stop end of
    // each of the above events, do a bitwise OR of the appropriate
    // Start/Stop flag and pass the result to IOPMrootDomain to record
    // the event.
    kIOPMStatsEventStartFlag              = (1 << 24),
    kIOPMStatsEventStopFlag               = (1 << 25)
};

// Keys for IOPMrootDomain registry properties
#define kIOPMSleepStatisticsKey                 "SleepStatistics"
#define kIOPMSleepStatisticsAppsKey             "AppStatistics"
#define kIOPMIdleSleepPreventersKey             "IdleSleepPreventers"
#define kIOPMSystemSleepPreventersKey           "SystemSleepPreventers"
#define kIOPMIdleSleepPreventersWithIDKey       "IdleSleepPreventersWithID"
#define kIOPMSystemSleepPreventersWithIDKey     "SystemSleepPreventersWithID"

// Application response statistics
#define kIOPMStatsNameKey                       "Name"
#define kIOPMStatsPIDKey                        "Pid"
#define kIOPMStatsTimeMSKey                     "TimeMS"
#define kIOPMStatsApplicationResponseTypeKey    "ResponseType"
#define kIOPMStatsMessageTypeKey                "MessageType"
#define kIOPMStatsPowerCapabilityKey            "PowerCaps"
#define kIOPMStatsSystemTransitionKey           "TransitionType"

// PM Statistics: potential values for the key kIOPMStatsApplicationResponseTypeKey
// entry in the application results array.
#define kIOPMStatsResponseTimedOut      "ResponseTimedOut"
#define kIOPMStatsResponseCancel        "ResponseCancel"
#define kIOPMStatsResponseSlow          "ResponseSlow"
#define kIOPMStatsResponsePrompt        "ResponsePrompt"
#define kIOPMStatsDriverPSChangeSlow    "DriverPSChangeSlow"

struct PMStatsBounds{
    uint64_t start;
    uint64_t stop;
};
typedef struct {

    struct PMStatsBounds    hibWrite;
    struct PMStatsBounds    hibRead;
//    bounds    driverNotifySleep;
//    bounds    driverNotifyWake;
//    bounds    appNotifySleep;
//    bounds    appNotifyWake;
//    OSDictionary    *tardyApps;
//    OSDictionary    *tardyDrivers;
} PMStatsStruct;

/*****************************************************************************/

/* PM RootDomain tracePoints
 *
 * In the sleep/wake process, we expect the sleep trace points to proceed
 * in increasing order. Once sleep begins with code kIOPMTracePointSleepStarted,
 * we expect sleep to continue in a monotonically increasing order of tracepoints
 * to kIOPMTracePointSystemLoginwindowPhase. After trace point SystemLoginWindowPhase,
 * the system will return to kIOPMTracePointSystemUp.
 *
 * If the trace point decreases (instead of increasing) before reaching kIOPMTracePointSystemUp,
 * that indicates that the sleep process was cancelled. The cancel reason shall be indicated
 * in the cancel tracepoint. (TBD)
 */

enum {
/* When kTracePointSystemUp is the latest tracePoint,
 * the system is awake. It is not asleep, sleeping, or waking.
 *
 * Phase begins: At boot, at completion of wake from sleep,
 *      immediately following kIOPMTracePointSystemLoginwindowPhase.
 * Phase ends: When a sleep attempt is initiated.
 */
    kIOPMTracePointSystemUp                     = 0,

/* When kIOPMTracePointSleepStarted is the latest tracePoint,
 * sleep has been initiated.
 *
 * Phase begins: At initiation of system sleep (idle or forced).
 * Phase ends: PM starts to notify applications of system sleep.
 */
    kIOPMTracePointSleepStarted                 = 0x10,

/* When kIOPMTracePointSleepApplications is the latest tracePoint,
 * a system sleep has been initiated and PM waits for responses
 * from notified applications.
 *
 * Phase begins: Begin to asynchronously fire kIOMessageSystemWillSleep
 *      notifications, and also kIOMessageCanSystemSleep for the idle sleep case.
 * Phase ends: When PM has received all application responses.
 */
    kIOPMTracePointSleepApplications            = 0x11,

/* When kIOPMTracePointSleepPriorityClients is the latest tracePoint,
 * PM is notifying priority clients and in-kernel system capability
 * clients, and waiting for any asynchronous completions.
 *
 * Phase begins: Synchronous delivery of kIOMessageSystemWillSleep notifications.
 * Phase ends: All notified clients have acknowledged.
 */
    kIOPMTracePointSleepPriorityClients         = 0x12,

/* When kIOPMTracePointSleepWillChangeInterests is the latest tracePoint,
 * PM is calling powerStateWillChangeTo() on interested drivers of root domain.
 *
 * Phase begins: Dispatch a callout thread to call interested drivers.
 * Phase ends: Callout thread work done, and acknowledgePowerChange() called
 *      by drivers that indicated asynchronous completion.
 */
    kIOPMTracePointSleepWillChangeInterests     = 0x13,

/* When kIOPMTracePointSleepPowerPlaneDrivers is the latest tracePoint,
 * PM is directing power plane drivers to power off in leaf-to-root order.
 *
 * Phase begins: Root domain informs its power children that it will drop to
 *      sleep state. This has a cascade effect and triggers all drivers in
 *      the power plane to transition to a lower power state if necessary.
 * Phase ends: All power transitions in response to the root domain power
 *      change have completed.
 */
    kIOPMTracePointSleepPowerPlaneDrivers       = 0x14,

/* When kIOPMTracePointSleepDidChangeInterests is the latest tracePoint,
 * PM is calling powerStateDidChangeTo() on interested drivers of root domain.
 *
 * Phase begins: Dispatch a callout thread to call interested drivers.
 * Phase ends: Callout thread work done, and acknowledgePowerChange() called
 *      by drivers that indicated asynchronous completion.
 */
    kIOPMTracePointSleepDidChangeInterests      = 0x15,

/* When kIOPMTracePointSleepCapabilityClients is the latest tracePoint,
 * PM is notifying system capability clients about system sleep.
 *
 * Phase begins: Send kIOMessageSystemCapabilityChange notifications to inform
 *      capability clients that system has lost all capabilities.
 * Phase ends: Finished sending notifications.
 */
    kIOPMTracePointSleepCapabilityClients       = 0x16,

/* When kIOPMTracePointSleepPlatformActions is the latest tracePoint,
 * PM is calling drivers that have registered a platform sleep action.
 */
    kIOPMTracePointSleepPlatformActions         = 0x17,

/* When kIOPMTracePointSleepCPUs is the latest tracePoint,
 * PM is shutting down all non-boot processors.
 *
 * Phase begins: Shutdown all non-boot processors.
 * Phase ends: Reduced to only the boot processor running.
 */
    kIOPMTracePointSleepCPUs                    = 0x18,

/* When kIOPMTracePointSleepPlatformDriver is the latest tracePoint,
 * PM is executing platform dependent code to prepare for system sleep.
 */
    kIOPMTracePointSleepPlatformDriver          = 0x19,

/* When kIOPMTracePointHibernate is the latest tracePoint,
 * PM is writing the hibernate image to disk.
 */
    kIOPMTracePointHibernate                    = 0x1a,

/* When kIOPMTracePointSystemSleep is the latest tracePoint,
 * PM has recorded the final trace point before the hardware platform
 * enters sleep state, or low level wakeup is underway - such as restoring
 * the hibernate image from disk.
 *
 * Note: If a system is asleep and then loses power, and it does not have a
 * hibernate image to restore from (e.g. hibernatemode = 0), then OS X will
 * interpret this power loss as a failure in kIOPMTracePointSystemSleep.
 *
 * Phase begins: Before the OS directs the hardware to enter sleep state.
 * Phase ends: Control returns to the OS on wake, but before recording the first
 *      wake trace point.
 */
    kIOPMTracePointSystemSleep                  = 0x1f,

/* When kIOPMTracePointWakePlatformDriver is the latest tracePoint,
 * PM is executing platform dependent code to prepare for system wake.
 */
    kIOPMTracePointWakePlatformDriver           = 0x21,

/* When kIOPMTracePointWakePlatformActions is the latest tracePoint,
 * PM is calling drivers that have registered a platform wake action.
 */
    kIOPMTracePointWakePlatformActions          = 0x22,

/* When kIOPMTracePointWakeCPUs is the latest tracePoint,
 * PM is bringing all non-boot processors online.
 */
    kIOPMTracePointWakeCPUs                     = 0x23,

/* When kIOPMTracePointWakeWillPowerOnClients is the latest tracePoint,
 * PM is sending kIOMessageSystemWillPowerOn to both kernel clients and
 * applications. PM also notifies system capability clients about the
 * proposed capability change.
 *
 * Phase begins: Send kIOMessageSystemWillPowerOn and
 *      kIOMessageSystemCapabilityChange notifications.
 * Phase ends: Finished sending notifications.
 */
    kIOPMTracePointWakeWillPowerOnClients       = 0x24,

/* When kIOPMTracePointWakeWillChangeInterests is the latest tracePoint,
 * PM is calling powerStateWillChangeTo() on interested drivers of root domain.
 *
 * Phase begins: Dispatch a callout thread to call interested drivers.
 * Phase ends: Callout thread work done, and acknowledgePowerChange() called
 *      by drivers that indicated asynchronous completion.
 */
    kIOPMTracePointWakeWillChangeInterests      = 0x25,

/* When kIOPMTracePointWakeDidChangeInterests is the latest tracePoint,
 * PM is calling powerStateDidChangeTo() on interested drivers of root domain.
 *
 * Phase begins: Dispatch a callout thread to call interested drivers.
 * Phase ends: Callout thread work done, and acknowledgePowerChange() called
 *      by drivers that indicated asynchronous completion.
 */
    kIOPMTracePointWakeDidChangeInterests       = 0x26,

/* When kIOPMTracePointWakePowerPlaneDrivers is the latest tracePoint,
 * PM is directing power plane drivers to power up in root-to-leaf order.
 *
 * Phase begins: Root domain informs its power children that it transitioned
 *      to ON state. This has a cascade effect and triggers all drivers in
 *      the power plane to re-evaluate and potentially change power state.
 * Phase ends: All power transitions in response to the root domain power
 *      change have completed.
 */
    kIOPMTracePointWakePowerPlaneDrivers        = 0x27,

/* When kIOPMTracePointWakeCapabilityClients is the latest tracePoint,
 * PM is notifying system capability clients about system wake, and waiting
 * for any asynchronous completions.
 *
 * Phase begins: Inform capability clients that system has gained capabilities.
 * Phase ends: All notified clients have acknowledged.
 */
    kIOPMTracePointWakeCapabilityClients        = 0x28,

/* When kIOPMTracePointWakeApplications is the latest tracePoint,
 * System CPU is powered, PM has powered on each driver.
 *
 * Phase begins: Send asynchronous kIOMessageSystemHasPoweredOn notifications.
 * Phase ends: Finished sending asynchronous notifications.
 */
    kIOPMTracePointWakeApplications             = 0x29,

/* kIOPMTracePointSystemLoginwindowPhase
 * This phase represents a several minute window after the system has powered on.
 * Higher levels of system diagnostics are in a heightened state of alert in this phase,
 * in case any user errors occurred that we could not detect in software.
 *
 * Phase begins: After IOPMrootDomain sends kIOMessageSystemHasPoweredOn message.
 * Phase ends: When loginwindow calls IOPMSleepWakeSetUUID(NULL) the system shall
 *      be considered awake and usable. The next phase shall be kIOPMTracePointSystemUp.
 */
    kIOPMTracePointSystemLoginwindowPhase       = 0x30,

/* When kIOPMTracePointDarkWakeEntry is the latest tracePoint,
 * PM has started a transition from full wake to dark wake.
 *
 * Phase begins: Start transition to dark wake.
 * Phase ends: System in dark wake. Before recording kIOPMTracePointSystemUp.
 */
    kIOPMTracePointDarkWakeEntry                = 0x31,

/* When kIOPMTracePointDarkWakeExit is the latest tracePoint,
 * PM has started a transition from dark wake to full wake.
 *
 * Phase begins: Start transition to full wake.
 * Phase ends: System in full wake. Before recording kIOPMTracePointSystemUp.
 */
    kIOPMTracePointDarkWakeExit                 = 0x32
};

/*****************************************************************************/

/*
 * Component wake progress keys
 *
 * Certain components have the ability to log their wake progress with
 * root domain using the keys provided below.
 *
 * LoginWindow - 4 bits
 * CoreDisplay - 8 bits
 * CoreGraphics - 8 bits
 *
 * These bits are stored with the trace phase that gets logged to
 * the RTC register.
 */

// Values that should be passed in to IOPMLogWakeProgress
enum {
    kIOPMLoginWindowProgress    = 1,
    kIOPMCoreDisplayProgress    = 2,
    kIOPMCoreGraphicsProgress   = 3
};

enum {
    kIOPMLoginWindowProgressMask    = 0x0f,
    kIOPMCoreDisplayProgressMask    = 0xff,
    kIOPMCoreGraphicsProgressMask   = 0xff
};

/*
 * kIOPMLoginWindowProgressKey - identifies PM debug data specific to LoginWindow
 *  for use with IOPMrootDomain. Only 4 bits of data are allotted.
 */
#define kIOPMLoginWindowProgressKey             "LoginWindowProgress"

/*
 * kIOPMCoreDisplayProgressKey - identifies PM debug data specific to CoreDisplay
 *  for use with IOPMrootDomain. Only 8 bits of data are allotted.
 */
#define kIOPMCoreDisplayProgressKey             "CoreDisplayProgress"

/*
 * kIOPMCoreGraphicsProgressKey - identifies PM debug data specific to CoreGraphics
 *  for use with IOPMrootDomain. Only 8 bits of data are allotted.
 */
#define kIOPMCoreGraphicsProgressKey            "CoreGraphicsProgress"

// For PM internal use only - key to locate sleep failure results within SCDynamicStore.
#define kIOPMDynamicStoreSleepFailureKey        "SleepFailure"

/*****************************************************************************/

// For IOPMLibPrivate.h
#define kIOPMSleepWakeFailureKey            "PMFailurePhase"
#define kIOPMSleepWakeFailureCodeKey        "PMStatusCode"
#define kIOPMSleepWakeFailureLoginKey       "LWFailurePhase"
#define kIOPMSleepWakeFailureUUIDKey        "UUID"
#define kIOPMSleepWakeFailureDateKey        "Date"
#define kIOPMSleepWakeWdogRebootKey         "SWWdogTriggeredRestart"
#define kIOPMSleepWakeWdogLogsValidKey      "SWWdogLogsValid"

/*****************************************************************************
 *
 * Root Domain private property keys
 *
 *****************************************************************************/

/* kIOPMFeatureAutoPowerOffKey
 * Feature published if Auto Power Off is supported
 */
#define kIOPMFeatureAutoPowerOffKey         "AutoPowerOff"

/* kIOPMAutoPowerOffEnabledKey
 * Indicates if Auto Power Off is enabled.
 * It has a boolean value.
 *  true        == Auto Power Off is enabled
 *  false       == Auto Power Off is disabled
 *  not present == Auto Power Off is not supported on this hardware
 */
#define kIOPMAutoPowerOffEnabledKey         "AutoPowerOff Enabled"

/* kIOPMAutoPowerOffDelayKey
 * Key refers to a CFNumberRef that represents the delay in seconds before
 * entering the Auto Power Off state.  The property is not present if Auto
 * Power Off is unsupported.
 */
#define kIOPMAutoPowerOffDelayKey           "AutoPowerOff Delay"

/* kIOPMAutoPowerOffTimerKey
 * Key refers to a CFNumberRef that indicates the time in seconds until the
 * expiration of the Auto Power Off delay period. This value should be used
 * to program a wake alarm before system sleep.
 */
#define kIOPMAutoPowerOffTimerKey           "AutoPowerOff Timer"

/* kIOPMDeepSleepTimerKey
 * Key refers to a CFNumberRef that indicates the time in seconds until the
 * expiration of the Standby delay period. This value should be used
 * to program a wake alarm before system sleep.
 */
#define kIOPMDeepSleepTimerKey                "Standby Timer"

/* kIOPMUserWakeAlarmScheduledKey
 * Key refers to a boolean value that indicates if an user alarm was scheduled
 * or pending.
 */
#define kIOPMUserWakeAlarmScheduledKey      "UserWakeAlarmScheduled"

/* kIOPMDeepIdleSupportedKey
 * Presence of this key indicates Deep Idle is supported on this platform.
 * Key will always refer to a value of kOSBooleanTrue.
 */
#define kIOPMDeepIdleSupportedKey           "IOPMDeepIdleSupported"

/* kIOPMUserTriggeredFullWakeKey
 * Key refers to a boolean value that indicates if the first full wake since
 * last system sleep was triggered by the local user. This property is set
 * before the initial full wake transition, and removed after powering down
 * drivers for system sleep.
 */
#define kIOPMUserTriggeredFullWakeKey       "IOPMUserTriggeredFullWake"

/* kIOPMUserIsActiveKey
 * Key refers to a boolean value that indicates if the user is active.
 */
#define kIOPMUserIsActiveKey                "IOPMUserIsActive"

/* kIOPMDriverWakeEventsKey
 * Key refers to a property that should only be examined by powerd.
 */
#define kIOPMDriverWakeEventsKey            "IOPMDriverWakeEvents"

/*****************************************************************************
 *
 * IOPMDriverWakeEvents dictionary keys
 *
 *****************************************************************************/

#define kIOPMWakeEventTimeKey               "Time"
#define kIOPMWakeEventFlagsKey              "Flags"
#define kIOPMWakeEventReasonKey             "Reason"
#define kIOPMWakeEventDetailsKey            "Details"

/* kIOPMFeatureProModeKey
 * Feature published if ProMode is supported
 */
#define kIOPMFeatureProModeKey              "ProMode"

/*****************************************************************************
 *
 * Wake event flags reported to IOPMrootDomain::claimSystemWakeEvent()
 *
 *****************************************************************************/

#define kIOPMWakeEventSource                0x00000001

#if !(defined(RC_HIDE_N144) || defined(RC_HIDE_N146))
/*****************************************************************************
 *
 * AOT defs
 *
 *****************************************************************************/

// signals the device should wake up to user space running
#define kIOPMWakeEventAOTExit                   0x00000002

// will start a 400 ms timer before sleeping
#define kIOPMWakeEventAOTPossibleExit           0x00000004

// signals the device should wake up to user space running
#define kIOPMWakeEventAOTConfirmedPossibleExit  0x00000008

// signals the device should go back to AOT
#define kIOPMWakeEventAOTRejectedPossibleExit   0x00000010

// signals the device should go back to AOT
#define kIOPMWakeEventAOTExpiredPossibleExit    0x00000020

#define kIOPMWakeEventAOTFlags \
                                 (kIOPMWakeEventAOTExit \
                                | kIOPMWakeEventAOTPossibleExit \
                                | kIOPMWakeEventAOTConfirmedPossibleExit \
                                | kIOPMWakeEventAOTRejectedPossibleExit \
                                | kIOPMWakeEventAOTExpiredPossibleExit)

#define kIOPMWakeEventAOTPossibleFlags \
                                 (kIOPMWakeEventAOTPossibleExit \
                                | kIOPMWakeEventAOTConfirmedPossibleExit \
                                | kIOPMWakeEventAOTRejectedPossibleExit \
                                | kIOPMWakeEventAOTExpiredPossibleExit)

#define kIOPMWakeEventAOTPerCycleFlags \
                                 (kIOPMWakeEventAOTPossibleExit \
                                | kIOPMWakeEventAOTRejectedPossibleExit \
                                | kIOPMWakeEventAOTExpiredPossibleExit)

#define kIOPMWakeEventAOTExitFlags \
                                 (kIOPMWakeEventAOTExit \
                                | kIOPMWakeEventAOTConfirmedPossibleExit)

enum {
    kIOPMAOTModeEnable        = 0x00000001,
    kIOPMAOTModeCycle         = 0x00000002,
    kIOPMAOTModeAddEventFlags = 0x00000004,
    kIOPMAOTModeRespectTimers = 0x00000008,
    kIOPMAOTModeDefault       = (kIOPMAOTModeEnable | kIOPMAOTModeAddEventFlags | kIOPMAOTModeRespectTimers)
};

enum {
    kIOPMAOTMetricsKernelWakeCountMax = 24
};

struct IOPMAOTMetrics
{
    uint32_t sleepCount;
    uint32_t possibleCount;
    uint32_t confirmedPossibleCount;
    uint32_t rejectedPossibleCount;
    uint32_t expiredPossibleCount;
    uint32_t noTimeSetCount;
    uint32_t rtcAlarmsCount;
    uint32_t softwareRequestCount;
    uint64_t totalTime;

	char     kernelWakeReason[kIOPMAOTMetricsKernelWakeCountMax][64];
	// 54:10 secs:ms calendar time
    uint64_t kernelSleepTime[kIOPMAOTMetricsKernelWakeCountMax];
    uint64_t kernelWakeTime[kIOPMAOTMetricsKernelWakeCountMax];
};

#define kIOPMAOTPowerKey    "aot-power"

#endif /* !(defined(RC_HIDE_N144) || defined(RC_HIDE_N146)) */

/*****************************************************************************
 *
 * System Sleep Policy
 *
 *****************************************************************************/

#define kIOPMSystemSleepPolicySignature     0x54504c53
#define kIOPMSystemSleepPolicyVersion       2

/*!
 * @defined kIOPMSystemSleepTypeKey
 * @abstract Indicates the type of system sleep.
 * @discussion An OSNumber property of root domain that describes the type
 * of system sleep. This property is set after notifying priority sleep/wake
 * clients, but before informing interested drivers and shutting down power
 * plane drivers. On a hibernate abort or failure, this property will not be
 * updated and will indicate the failed sleep type.
 */
#define kIOPMSystemSleepTypeKey             "IOPMSystemSleepType"

struct IOPMSystemSleepPolicyVariables
{
    uint32_t    signature;                  // kIOPMSystemSleepPolicySignature
    uint32_t    version;                    // kIOPMSystemSleepPolicyVersion

    uint64_t    currentCapability;          // current system capability bits
    uint64_t    highestCapability;          // highest system capability bits

    uint64_t    sleepFactors;               // sleep factor bits
    uint32_t    sleepReason;                // kIOPMSleepReason*
    uint32_t    sleepPhase;                 // identify the sleep phase
    uint32_t    hibernateMode;              // current hibernate mode

    uint32_t    standbyDelay;               // standby delay in seconds
    uint32_t    standbyTimer;               // standby timer in seconds
    uint32_t    poweroffDelay;              // auto-poweroff delay in seconds
    uint32_t    scheduledAlarms;            // bitmask of scheduled alarm types
    uint32_t    poweroffTimer;              // auto-poweroff timer in seconds

    uint32_t    reserved[49];               // pad sizeof 256 bytes
};

enum {
    kIOPMAlarmBitDebugWake                  = 0x01,
    kIOPMAlarmBitCalendarWake               = 0x02,
    kIOPMAlarmBitMaintenanceWake            = 0x04,
    kIOPMAlarmBitSleepServiceWake           = 0x08
};

enum {
    kIOPMSleepPhase0 = 0,
    kIOPMSleepPhase1,
    kIOPMSleepPhase2
};

// Sleep Factor Mask / Bits
enum {
    kIOPMSleepFactorSleepTimerWake          = 0x00000001ULL,
    kIOPMSleepFactorLidOpen                 = 0x00000002ULL,
    kIOPMSleepFactorACPower                 = 0x00000004ULL,
    kIOPMSleepFactorBatteryLow              = 0x00000008ULL,
    kIOPMSleepFactorStandbyNoDelay          = 0x00000010ULL,
    kIOPMSleepFactorStandbyForced           = 0x00000020ULL,
    kIOPMSleepFactorStandbyDisabled         = 0x00000040ULL,
    kIOPMSleepFactorUSBExternalDevice       = 0x00000080ULL,
    kIOPMSleepFactorBluetoothHIDDevice      = 0x00000100ULL,
    kIOPMSleepFactorExternalMediaMounted    = 0x00000200ULL,
    kIOPMSleepFactorThunderboltDevice       = 0x00000400ULL,
    kIOPMSleepFactorRTCAlarmScheduled       = 0x00000800ULL,
    kIOPMSleepFactorMagicPacketWakeEnabled  = 0x00001000ULL,
    kIOPMSleepFactorHibernateForced         = 0x00010000ULL,
    kIOPMSleepFactorAutoPowerOffDisabled    = 0x00020000ULL,
    kIOPMSleepFactorAutoPowerOffForced      = 0x00040000ULL,
    kIOPMSleepFactorExternalDisplay         = 0x00080000ULL,
    kIOPMSleepFactorNetworkKeepAliveActive  = 0x00100000ULL,
    kIOPMSleepFactorLocalUserActivity       = 0x00200000ULL,
    kIOPMSleepFactorHibernateFailed         = 0x00400000ULL,
    kIOPMSleepFactorThermalWarning          = 0x00800000ULL,
    kIOPMSleepFactorDisplayCaptured         = 0x01000000ULL
};

// System Sleep Types
enum {
    kIOPMSleepTypeInvalid                   = 0,
    kIOPMSleepTypeAbortedSleep              = 1,
    kIOPMSleepTypeNormalSleep               = 2,
    kIOPMSleepTypeSafeSleep                 = 3,
    kIOPMSleepTypeHibernate                 = 4,
    kIOPMSleepTypeStandby                   = 5,
    kIOPMSleepTypePowerOff                  = 6,
    kIOPMSleepTypeDeepIdle                  = 7,
    kIOPMSleepTypeLast                      = 8
};

// System Sleep Flags
enum {
    kIOPMSleepFlagDisableHibernateAbort     = 0x00000001,
    kIOPMSleepFlagDisableUSBWakeEvents      = 0x00000002,
    kIOPMSleepFlagDisableBatlowAssertion    = 0x00000004,
    kIOPMSleepFlagDisableS4WakeSources      = 0x00000008
};

// System Wake Events
enum {
    kIOPMWakeEventLidOpen                   = 0x00000001,
    kIOPMWakeEventLidClose                  = 0x00000002,
    kIOPMWakeEventACAttach                  = 0x00000004,
    kIOPMWakeEventACDetach                  = 0x00000008,
    kIOPMWakeEventCDInsert                  = 0x00000010,
    kIOPMWakeEventCDEject                   = 0x00000020,
    kIOPMWakeEventHPDAttach                 = 0x00000040,
    kIOPMWakeEventHPDDetach                 = 0x00000080,
    kIOPMWakeEventPowerButton               = 0x00000100,
    kIOPMWakeEventG3PowerOn                 = 0x00000200,
    kIOPMWakeEventUserPME                   = 0x00000400,
    kIOPMWakeEventSleepTimer                = 0x00000800,
    kIOPMWakeEventBatteryLow                = 0x00001000,
    kIOPMWakeEventDarkPME                   = 0x00002000,
    kIOPMWakeEventWifi                      = 0x00004000,
    kIOPMWakeEventRTCSystem                 = 0x00008000,  // Maintenance RTC wake
    kIOPMWakeEventUSBCPlugin                = 0x00010000,  // USB-C Plugin
    kIOPMWakeEventHID                       = 0x00020000,
    kIOPMWakeEventBluetooth                 = 0x00040000,
    kIOPMWakeEventDFR                       = 0x00080000,
    kIOPMWakeEventSD                        = 0x00100000,  // SD card
    kIOPMWakeEventLANWake                   = 0x00200000,  // Wake on Lan
    kIOPMWakeEventLANPlugin                 = 0x00400000,  // Ethernet media sense
    kIOPMWakeEventThunderbolt               = 0x00800000,
    kIOPMWakeEventRTCUser                   = 0x01000000,  // User requested RTC wake
};

/*!
 * @defined kIOPMSystemSleepParametersKey
 * @abstract Sleep parameters describing the upcoming sleep
 * @discussion Root domain updates this OSData property before system sleep
 * to pass sleep parameters to the platform driver.  Some of the parameters
 * are based on the chosen entry in the system sleep policy table.
 */
#define kIOPMSystemSleepParametersKey       "IOPMSystemSleepParameters"
#define kIOPMSystemSleepParametersVersion   2

struct IOPMSystemSleepParameters
{
    uint16_t    version;
    uint16_t    reserved1;
    uint32_t    sleepType;
    uint32_t    sleepFlags;
    uint32_t    ecWakeEvents;
    uint32_t    ecWakeTimer;
    uint32_t    ecPoweroffTimer;
    uint32_t    reserved2[10];
} __attribute__((packed));


/*
 * Sleep Wake debug buffer header
 */
typedef struct {
   uint32_t signature;
   uint32_t alloc_size;
   uint32_t          crc;             /* CRC for spindump & following data.*/
   uint32_t          spindump_offset; /* Offset at which spindump offset is stored */
   uint32_t          spindump_size;
   uint8_t           is_osx_watchdog;

   /* All members from UUID onwards are saved into log file */
   char             UUID[44];
   char             spindump_status[24];   /* stackshot status*/
   char             PMStatusCode[32];
   char             reason[32];
} swd_hdr;


#define SWD_HDR_SIGNATURE       0xdeb8da2a

#define SWD_STACKSHOT_SIZE      (40*PAGE_SIZE)
#define SWD_COMPRESSED_BUFSIZE  (5*PAGE_SIZE)
#define SWD_ZLIB_BUFSIZE        (10*PAGE_SIZE)
#define SWD_STACKSHOT_VAR_PREFIX    "sleepwake_diags"

#define SWD_SPINDUMP_SIZE          (256*1024)
#define SWD_INITIAL_SPINDUMP_SIZE  ((SWD_SPINDUMP_SIZE/2)-sizeof(swd_hdr))
#define SWD_MAX_STACKSHOTS          (10)

/* Bits in swd_flags */
#define SWD_WDOG_ENABLED        0x01
#define SWD_BOOT_BY_SW_WDOG     0x02
#define SWD_BOOT_BY_OSX_WDOG    0x04
#define SWD_VALID_LOGS          0x08
#define SWD_LOGS_IN_FILE        0x10
#define SWD_LOGS_IN_MEM         0x20
#define SWD_PWR_BTN_STACKSHOT   0x30

#define SWD_DATA_CRC_ERROR      0x010000
#define SWD_BUF_SIZE_ERROR      0x020000
#define SWD_HDR_SIZE_ERROR      0x040000
#define SWD_FILEOP_ERROR        0x080000
#define SWD_HDR_SIGNATURE_ERROR 0x100000
#define SWD_INTERNAL_FAILURE    0x200000


/* Filenames associated with the stackshots/logs generated by the SWD */
#define kOSWatchdogStacksFilename           "/var/log/OSXWatchdogStacks.gz"
#define kOSWatchdogFailureStringFile        "/var/log/OSWatchdogFailureString.txt"
#define kSleepWakeStacksFilename            "/var/log/SleepWakeStacks.gz"
#define kSleepWakeFailureStringFile         "/var/log/SleepWakeFailureString.txt"


/* RootDomain IOReporting channels */
#define kSleepCntChID IOREPORT_MAKEID('S','l','e','e','p','C','n','t')
#define kDarkWkCntChID IOREPORT_MAKEID('G','U','I','W','k','C','n','t')
#define kUserWkCntChID IOREPORT_MAKEID('D','r','k','W','k','C','n','t')

/*
 * kAssertDelayChID - Histogram of time elapsed before assertion after wake.
 */
#define kAssertDelayBcktCnt     11
#define kAssertDelayBcktSize    3
#define kAssertDelayChID IOREPORT_MAKEID('r','d','A','s','r','t','D','l')

/*
 * kSleepDelaysChID - Histogram of time taken to put system to sleep
 */
#define kSleepDelaysBcktCnt     13
#define kSleepDelaysBcktSize    10
#define kSleepDelaysChID IOREPORT_MAKEID('r','d','S','l','p','D','l','y')

/* Sleep Options/settings */
#define kSleepOptionDisplayCapturedModeKey         "DisplayCapturedMode"


#if defined(KERNEL) && defined(__cplusplus)

/*!
 * @defined kIOPMInstallSystemSleepPolicyHandlerKey
 * @abstract Name of the platform function to install a sleep policy handler.
 * @discussion Pass to IOPMrootDomain::callPlatformFunction(), with a pointer
 * to the C-function handler at param1, and an optional target at param2, to
 * register a sleep policy handler. Only a single sleep policy handler can
 * be installed.
 */
#define kIOPMInstallSystemSleepPolicyHandlerKey \
        "IOPMInstallSystemSleepPolicyHandler"

typedef IOReturn (*IOPMSystemSleepPolicyHandler)(
        void * target,
        const IOPMSystemSleepPolicyVariables * vars,
        IOPMSystemSleepParameters * params );

#endif /* KERNEL */

/*****************************************************************************
 *
 * Performance Warning
 *
 *****************************************************************************/

/* Performance Warning Key
 * Key for performance warning event published using IOPMrootDomain::
 * systemPowerEventOccurred()
 */
#define kIOPMPerformanceWarningKey          "Performance_Warning"

/* Performance warning values */
enum {
  kIOPMPerformanceNormal    = 0,
  kIOPMPerformanceWarning   = 100
};

#endif /* ! _IOKIT_IOPMPRIVATE_H */
