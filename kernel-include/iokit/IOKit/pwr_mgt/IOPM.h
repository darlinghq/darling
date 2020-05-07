/*
 * Copyright (c) 1998-2005 Apple Computer, Inc. All rights reserved.
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
#ifndef _IOKIT_IOPM_H
#define _IOKIT_IOPM_H

#include <IOKit/IOTypes.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOReturn.h>

/*! @header IOPM.h
 *   @abstract Defines power management constants and keys used by both in-kernel and user space power management.
 *   @discussion IOPM.h defines a range of power management constants used in several in-kernel and user space APIs. Most significantly, the IOPMPowerFlags used to specify the fields of an IOPMPowerState struct are defined here.
 *
 *   Most of the constants defined in IOPM.h are deprecated or for Apple internal use only, and are not elaborated on in headerdoc.
 */

enum {
	kIOPMMaxPowerStates = 10,
	IOPMMaxPowerStates = kIOPMMaxPowerStates
};

/*! @enum IOPMPowerFlags
 *   @abstract Bits are used in defining capabilityFlags, inputPowerRequirements, and outputPowerCharacter in the IOPMPowerState structure.
 *   @discussion These bits may be bitwise-OR'd together in the IOPMPowerState capabilityFlags field, the outputPowerCharacter field, and/or the inputPowerRequirement field.
 *
 *   The comments clearly mark whether each flag should be used in the capabilityFlags field, outputPowerCharacter field, and inputPowerRequirement field, or all three.
 *
 *   The value of capabilityFlags, inputPowerRequirement or outputPowerCharacter may be 0. Most drivers implement their 'OFF' state, used when asleep, by defininf each of the 3 fields as 0.
 *
 *   The bits listed below are only the most common bits used to define a device's power states. Your device's IO family may require that your device specify other input or output power flags to interact properly. Consult family-specific documentation to determine if your IOPower plane parents or children require other power flags; they probably don't.
 *
 *   @constant kIOPMPowerOn Indicates the device is on, requires power, and provides power. Useful as a: Capability, InputPowerRequirement, OutputPowerCharacter
 *
 *   @constant kIOPMDeviceUsable Indicates the device is usable in this state. Useful only as a Capability
 *
 *   @constant kIOPMLowPower
 *   Indicates device is in a low power state. May be bitwis-OR'd together
 *   with kIOPMDeviceUsable flag, to indicate the device is still usable.
 *
 *   A device with a capability of kIOPMLowPower may:
 *      Require either 0 or kIOPMPowerOn from its power parent
 *      Offer either kIOPMLowPower, kIOPMPowerOn, or 0 (no power at all)
 *        to its power plane children.
 *
 *   Useful only as a Capability, although USB drivers should consult USB family documentation for other valid circumstances to use the kIOPMLowPower bit.
 *
 *   @constant kIOPMPreventIdleSleep
 *   In the capability field of a power state, disallows idle system sleep while the device is in that state.
 *
 *   For example, displays and disks set this capability for their ON power state; since the system may not idle sleep while the display (and thus keyboard or mouse) or the disk is active.
 *
 *   Useful only as a Capability.
 *
 *   @constant kIOPMSleepCapability
 *   Used only by certain IOKit Families (USB). Not defined or used by generic Power Management. Read your family documentation to see if you should define a powerstate using these capabilities.
 *
 *   @constant kIOPMRestartCapability
 *   Used only by certain IOKit Families (USB). Not defined or used by generic Power Management. Read your family documentation to see if you should define a powerstate using these capabilities.
 *
 *   @constant kIOPMSleep
 *   Used only by certain IOKit Families (USB). Not defined or used by generic Power Management. Read your family documentation to see if you should define a powerstate using these capabilities.
 *
 *   @constant kIOPMRestart
 *   Used only by certain IOKit Families (USB). Not defined or used by generic Power Management. Read your family documentation to see if you should define a powerstate using these capabilities.
 *
 *   @constant kIOPMInitialDeviceState
 *   Indicates the initial power state for the device. If <code>initialPowerStateForDomainState()</code> returns a power state with this flag set in the capability field, then the initial power change is performed without calling the driver's <code>setPowerState()</code>.
 *
 *   @constant kIOPMRootDomainState
 *   An indication that the power flags represent the state of the root power
 *   domain. This bit must not be set in the IOPMPowerState structure.
 *   Power Management may pass this bit to initialPowerStateForDomainState()
 *   to map from a global system state to the desired device state.
 */
typedef unsigned long IOPMPowerFlags;
enum {
	kIOPMPowerOn                    = 0x00000002,
	kIOPMDeviceUsable               = 0x00008000,
	kIOPMLowPower                   = 0x00010000,
#if PRIVATE
#if !(defined(RC_HIDE_N144) || defined(RC_HIDE_N146))
	kIOPMAOTPower                   = 0x00020000,
	kIOPMAOTCapability              = kIOPMAOTPower,
#endif /* !(defined(RC_HIDE_N144) || defined(RC_HIDE_N146)) */
#endif /* PRIVATE */
	kIOPMPreventIdleSleep           = 0x00000040,
	kIOPMSleepCapability            = 0x00000004,
	kIOPMRestartCapability          = 0x00000080,
	kIOPMSleep                      = 0x00000001,
	kIOPMRestart                    = 0x00000080,
	kIOPMInitialDeviceState         = 0x00000100,
	kIOPMRootDomainState            = 0x00000200
};

/*
 * Private IOPMPowerFlags
 *
 * For Apple use only
 * Not for use with non-Apple drivers
 * Their behavior is undefined
 */
enum {
	kIOPMClockNormal                = 0x0004,
	kIOPMClockRunning               = 0x0008,
	kIOPMPreventSystemSleep         = 0x0010,
	kIOPMDoze                       = 0x0400,
	kIOPMChildClamp                 = 0x0080,
	kIOPMChildClamp2                = 0x0200,
	kIOPMNotPowerManaged            = 0x0800
};

/*
 * Deprecated IOPMPowerFlags
 * Their behavior is undefined when used in IOPMPowerState
 * Capability, InputPowerRequirement, or OutputPowerCharacter fields.
 */
enum {
	kIOPMMaxPerformance             = 0x4000,
	kIOPMPassThrough                = 0x0100,
	kIOPMAuxPowerOn                 = 0x0020,
	kIOPMNotAttainable              = 0x0001,
	kIOPMContextRetained            = 0x2000,
	kIOPMConfigRetained             = 0x1000,
	kIOPMStaticPowerValid           = 0x0800,
	kIOPMSoftSleep                  = 0x0400,
	kIOPMCapabilitiesMask =     kIOPMPowerOn | kIOPMDeviceUsable |
	    kIOPMMaxPerformance | kIOPMContextRetained |
	    kIOPMConfigRetained | kIOPMSleepCapability |
	    kIOPMRestartCapability
};

/*
 * Support for old names of IOPMPowerFlag constants
 */
enum {
	IOPMNotAttainable           = kIOPMNotAttainable,
	IOPMPowerOn                 = kIOPMPowerOn,
	IOPMClockNormal             = kIOPMClockNormal,
	IOPMClockRunning            = kIOPMClockRunning,
	IOPMAuxPowerOn              = kIOPMAuxPowerOn,
	IOPMDeviceUsable            = kIOPMDeviceUsable,
	IOPMMaxPerformance          = kIOPMMaxPerformance,
	IOPMContextRetained         = kIOPMContextRetained,
	IOPMConfigRetained          = kIOPMConfigRetained,
	IOPMNotPowerManaged         = kIOPMNotPowerManaged,
	IOPMSoftSleep               = kIOPMSoftSleep
};


enum {
	kIOPMNextHigherState        = 1,
	kIOPMHighestState           = 2,
	kIOPMNextLowerState         = 3,
	kIOPMLowestState            = 4
};

enum {
	IOPMNextHigherState         = kIOPMNextHigherState,
	IOPMHighestState            = kIOPMHighestState,
	IOPMNextLowerState          = kIOPMNextLowerState,
	IOPMLowestState             = kIOPMLowestState
};

// Internal commands used by power managment command queue
enum {
	kIOPMBroadcastAggressiveness = 1,
	kIOPMUnidleDevice
};

// Power consumption unknown value
enum {
	kIOPMUnknown = 0xFFFF
};

/*******************************************************************************
 *
 * Root Domain property keys of interest
 *
 ******************************************************************************/

/* AppleClamshellState
 * reflects the state of the clamshell (lid) on a portable.
 * It has a boolean value.
 *  true        == clamshell is closed
 *  false       == clamshell is open
 *  not present == no clamshell on this hardware
 */
#define kAppleClamshellStateKey             "AppleClamshellState"

/* AppleClamshellCausesSleep
 * reflects the clamshell close behavior on a portable.
 * It has a boolean value.
 *  true        == system will sleep when clamshell is closed
 *  false       == system will not sleep on clamshell close
 *                  (typically external display mode)
 *  not present == no clamshell on this hardware
 */
#define kAppleClamshellCausesSleepKey       "AppleClamshellCausesSleep"

/* kIOPMSleepWakeUUIDKey
 * Key refers to a CFStringRef that will uniquely identify
 * a sleep/wake cycle for logging & tracking.
 * The key becomes valid at the beginning of a sleep cycle - before we
 * initiate any sleep/wake notifications.
 * The key becomes invalid at the completion of a system wakeup. The
 * property will not be present in the IOPMrootDomain's registry entry
 * when it is invalid.
 *
 * See IOPMrootDomain notification kIOPMMessageSleepWakeUUIDChange
 */
 #define kIOPMSleepWakeUUIDKey              "SleepWakeUUID"

/* kIOPMBootSessionUUIDKey
 * Key refers to a CFStringRef that will uniquely identify
 * a boot cycle.
 * The key becomes valid at boot time and remains valid
 * till shutdown. The property value will remain same across
 * sleep/wake/hibernate cycle.
 */
#define kIOPMBootSessionUUIDKey             "BootSessionUUID"

/* kIOPMDeepSleepEnabledKey
 * Indicates the Deep Sleep enable state.
 * It has a boolean value.
 *  true        == Deep Sleep is enabled
 *  false       == Deep Sleep is disabled
 *  not present == Deep Sleep is not supported on this hardware
 */
#define kIOPMDeepSleepEnabledKey            "Standby Enabled"

/* kIOPMDeepSleepDelayKey
 * Key refers to a CFNumberRef that represents the delay in seconds before
 * entering Deep Sleep state when on battery power and when remaining
 * battery capacity is below a particular threshold (e.g., 50%.) The
 * property is not present if Deep Sleep is unsupported.
 */
#define kIOPMDeepSleepDelayKey              "Standby Delay"

/* kIOPMDeepSleepDelayHighKey
 * Key refers to a CFNumberRef that represents the delay in seconds before
 * entering Deep Sleep state. This is used instead of the value specified by
 * kIOPMDeepSleepDelayKey if the remaining battery capacity is above a
 * particular threshold (e.g. 50%) or on AC power. The property is not
 * present if Deep Sleep is unsupported.
 */
#define kIOPMDeepSleepDelayHighKey          "High Standby Delay"

/* kIOPMLowBatteryThresholdKey
 * Key refers to a CFNumberRef that represents the threshold used to choose
 * between the normal deep sleep delay and the high deep sleep delay (as a
 * percentage of total battery capacity remaining.) The property is not
 * present if Deep Sleep is unsupported.
 */
#define kIOPMStandbyBatteryThresholdKey     "Standby Battery Threshold"

/* kIOPMDestroyFVKeyOnStandbyKey
 * Specifies if FileVault key can be stored when going to standby mode
 * It has a boolean value,
 *  true        == Destroy FV key when going to standby mode
 *  false       == Retain FV key when going to standby mode
 *  not present == Retain FV key when going to standby mode
 */
#define kIOPMDestroyFVKeyOnStandbyKey       "DestroyFVKeyOnStandby"

/*******************************************************************************
 *
 * Properties that can control power management behavior
 *
 ******************************************************************************/

/* kIOPMResetPowerStateOnWakeKey
 * If an IOService publishes this key with the value of kOSBooleanTrue,
 * then PM will disregard the influence from changePowerStateToPriv() or
 * any activity tickles that occurred before system sleep when resolving
 * the initial device power state on wake. Influences from power children
 * and changePowerStateTo() are not eliminated. At the earliest opportunity
 * upon system wake, PM will query the driver for a new power state to be
 * installed as the initial changePowerStateToPriv() influence, by calling
 * initialPowerStateForDomainState() with both kIOPMRootDomainState and
 * kIOPMPowerOn flags set. The default implementation will always return
 * the lowest power state. Drivers can override this default behavior to
 * immediately raise the power state when there are work blocked on the
 * power change, and cannot afford to wait until the next activity tickle.
 * This property should be statically added to a driver's plist or set at
 * runtime before calling PMinit().
 */
#define kIOPMResetPowerStateOnWakeKey       "IOPMResetPowerStateOnWake"

/*******************************************************************************
 *
 * Driver PM Assertions
 *
 ******************************************************************************/

/* Driver Assertion bitfield description
 * Driver PM assertions are defined by these bits.
 */
enum {
	/*! kIOPMDriverAssertionCPUBit
	 * When set, PM kernel will prefer to leave the CPU and core hardware
	 * running in "Dark Wake" state, instead of sleeping.
	 */
	kIOPMDriverAssertionCPUBit                      = 0x01,

	/*! kIOPMDriverAssertionPreventSystemIdleSleepBit
	 * When set, the system should not idle sleep. This does not prevent
	 * demand sleep.
	 */
	kIOPMDriverAssertionPreventSystemIdleSleepBit   = 0x02,

	/*! kIOPMDriverAssertionUSBExternalDeviceBit
	 * When set, driver is informing PM that an external USB device is attached.
	 */
	kIOPMDriverAssertionUSBExternalDeviceBit        = 0x04,

	/*! kIOPMDriverAssertionBluetoothHIDDevicePairedBit
	 * When set, driver is informing PM that a Bluetooth HID device is paired.
	 */
	kIOPMDriverAssertionBluetoothHIDDevicePairedBit = 0x08,

	/*! kIOPMDriverAssertionExternalMediaMountedBit
	 * When set, driver is informing PM that an external media is mounted.
	 */
	kIOPMDriverAssertionExternalMediaMountedBit     = 0x10,

	/*! kIOPMDriverAssertionReservedBit5
	 * Reserved for Thunderbolt.
	 */
	kIOPMDriverAssertionReservedBit5                = 0x20,

	/*! kIOPMDriverAssertionPreventDisplaySleepBit
	 * When set, the display should remain powered on while the system's awake.
	 */
	kIOPMDriverAssertionPreventDisplaySleepBit      = 0x40,

	/*! kIOPMDriverAssertionReservedBit7
	 * Reserved for storage family.
	 */
	kIOPMDriverAssertionReservedBit7                = 0x80,

	/*! kIOPMDriverAssertionMagicPacketWakeEnabledBit
	 * When set, driver is informing PM that magic packet wake is enabled.
	 */
	kIOPMDriverAssertionMagicPacketWakeEnabledBit   = 0x100,

	/*! kIOPMDriverAssertionNetworkKeepAliveActiveBit
	 * When set, driver is informing PM that it is holding the network
	 * interface up to do TCPKeepAlive
	 */
	kIOPMDriverAssertionNetworkKeepAliveActiveBit   = 0x200
};

/* kIOPMAssertionsDriverKey
 * This kIOPMrootDomain key refers to a CFNumberRef property, containing
 * a bitfield describing the aggregate PM assertion levels.
 * Example: A value of 0 indicates that no driver has asserted anything.
 * Or, a value of <link>kIOPMDriverAssertionCPUBit</link>
 *   indicates that a driver (or drivers) have asserted a need for CPU and video.
 */
#define kIOPMAssertionsDriverKey            "DriverPMAssertions"

/* kIOPMAssertionsDriverKey
 * This kIOPMrootDomain key refers to a CFNumberRef property, containing
 * a bitfield describing the aggregate PM assertion levels.
 * Example: A value of 0 indicates that no driver has asserted anything.
 * Or, a value of <link>kIOPMDriverAssertionCPUBit</link>
 *   indicates that a driver (or drivers) have asserted a need for CPU and video.
 */
#define kIOPMAssertionsDriverDetailedKey    "DriverPMAssertionsDetailed"

/*******************************************************************************
 *
 * Kernel Driver assertion detailed dictionary keys
 *
 * Keys decode the Array & dictionary data structure under IOPMrootDomain property
 *  kIOPMAssertionsDriverKey.
 *
 */
#define kIOPMDriverAssertionIDKey               "ID"
#define kIOPMDriverAssertionCreatedTimeKey      "CreatedTime"
#define kIOPMDriverAssertionModifiedTimeKey     "ModifiedTime"
#define kIOPMDriverAssertionOwnerStringKey      "Owner"
#define kIOPMDriverAssertionOwnerServiceKey     "ServicePtr"
#define kIOPMDriverAssertionRegistryEntryIDKey  "RegistryEntryID"
#define kIOPMDriverAssertionLevelKey            "Level"
#define kIOPMDriverAssertionAssertedKey         "Assertions"

/*******************************************************************************
 *
 * Root Domain general interest messages
 *
 * Available by registering for interest type 'gIOGeneralInterest'
 * on IOPMrootDomain.
 *
 ******************************************************************************/

/* kIOPMMessageClamshellStateChange
 * Delivered as a general interest notification on the IOPMrootDomain
 * IOPMrootDomain sends this message when state of either AppleClamshellState
 * or AppleClamshellCausesSleep changes. If this clamshell change results in
 * a sleep, the sleep will initiate soon AFTER delivery of this message.
 * The state of both variables is encoded in a bitfield argument sent with
 * the message. Check bits 0 and 1 using kClamshellStateBit & kClamshellSleepBit
 */
enum {
	kClamshellStateBit = (1 << 0),
	kClamshellSleepBit = (1 << 1)
};

#define kIOPMMessageClamshellStateChange   \
	        iokit_family_msg(sub_iokit_powermanagement, 0x100)

/* kIOPMMessageFeatureChange
 * Delivered when the set of supported features ("Supported Features" dictionary
 * under IOPMrootDomain registry) changes in some way. Typically addition or
 * removal of a supported feature.
 * RootDomain passes no argument with this message.
 */
#define kIOPMMessageFeatureChange           \
	        iokit_family_msg(sub_iokit_powermanagement, 0x110)

/* kIOPMMessageInflowDisableCancelled
 * The battery has drained completely to its "Fully Discharged" state.
 * If a user process has disabled battery inflow for battery
 * calibration, we forcibly re-enable Inflow at this point.
 * If inflow HAS been forcibly re-enabled, bit 0
 * (kInflowForciblyEnabledBit) will be set.
 */
enum {
	kInflowForciblyEnabledBit = (1 << 0)
};

/* kIOPMMessageInternalBatteryFullyDischarged
 * The battery has drained completely to its "Fully Discharged" state.
 */
#define kIOPMMessageInternalBatteryFullyDischarged  \
	        iokit_family_msg(sub_iokit_powermanagement, 0x120)

/* kIOPMMessageSystemPowerEventOccurred
 * Some major system thermal property has changed, and interested clients may
 * modify their behavior.
 */
#define kIOPMMessageSystemPowerEventOccurred  \
	        iokit_family_msg(sub_iokit_powermanagement, 0x130)

/* kIOPMMessageSleepWakeUUIDChange
 * Either a new SleepWakeUUID has been specified at the beginning of a sleep,
 * or we're removing the existing property upon completion of a wakeup.
 */
#define kIOPMMessageSleepWakeUUIDChange  \
	        iokit_family_msg(sub_iokit_powermanagement, 0x140)

/* kIOPMMessageSleepWakeUUIDSet
 * Argument accompanying the kIOPMMessageSleepWakeUUIDChange notification when
 * a new UUID has been specified.
 */
#define kIOPMMessageSleepWakeUUIDSet                    ((void *)1)

/* kIOPMMessageSleepWakeUUIDCleared
 * Argument accompanying the kIOPMMessageSleepWakeUUIDChange notification when
 * the current UUID has been removed.
 */
#define kIOPMMessageSleepWakeUUIDCleared                ((void *)NULL)

/*! kIOPMMessageDriverAssertionsChanged
 *  Sent when kernel PM driver assertions have changed.
 */
#define kIOPMMessageDriverAssertionsChanged  \
	        iokit_family_msg(sub_iokit_powermanagement, 0x150)

/*! kIOPMMessageDarkWakeThermalEmergency
 * Sent when machine becomes unsustainably warm in DarkWake.
 * Kernel PM might choose to put the machine back to sleep right after.
 */
#define kIOPMMessageDarkWakeThermalEmergency \
	        iokit_family_msg(sub_iokit_powermanagement, 0x160)

/*******************************************************************************
 *
 * Power commands issued to root domain
 * Use with IOPMrootDomain::receivePowerNotification()
 *
 * These commands are issued from system drivers only:
 *      ApplePMU, AppleSMU, IOGraphics, AppleACPIFamily
 *
 * TODO: deprecate kIOPMAllowSleep and kIOPMPreventSleep
 ******************************************************************************/
enum {
	kIOPMSleepNow                 = (1 << 0),// put machine to sleep now
	kIOPMAllowSleep               = (1 << 1),// allow idle sleep
	kIOPMPreventSleep             = (1 << 2),// do not allow idle sleep
	kIOPMPowerButton              = (1 << 3),// power button was pressed
	kIOPMClamshellClosed          = (1 << 4),// clamshell was closed
	kIOPMPowerEmergency           = (1 << 5),// battery dangerously low
	kIOPMDisableClamshell         = (1 << 6),// do not sleep on clamshell closure
	kIOPMEnableClamshell          = (1 << 7),// sleep on clamshell closure
	kIOPMProcessorSpeedChange     = (1 << 8),// change the processor speed
	kIOPMOverTemp                 = (1 << 9),// system dangerously hot
	kIOPMClamshellOpened          = (1 << 10),// clamshell was opened
	kIOPMDWOverTemp               = (1 << 11),// DarkWake thermal limits exceeded.
	kIOPMPowerButtonUp            = (1 << 12),// Power button up
	kIOPMProModeEngaged           = (1 << 13),// Fans entered 'ProMode'
	kIOPMProModeDisengaged        = (1 << 14) // Fans exited 'ProMode'
};


/*******************************************************************************
 *
 * Power Management Return Codes
 *
 ******************************************************************************/
enum {
	kIOPMNoErr                  = 0,

	// Returned by driver's setPowerState(), powerStateWillChangeTo(),
	// powerStateDidChangeTo(), or acknowledgeSetPowerState() to
	// implicitly acknowledge power change upon function return.
	kIOPMAckImplied             = 0,

	// Deprecated
	kIOPMWillAckLater           = 1,

	// Returned by requestPowerDomainState() to indicate
	// unrecognized specification parameter.
	kIOPMBadSpecification       = 4,

	// Returned by requestPowerDomainState() to indicate
	// no power state matches search specification.
	kIOPMNoSuchState            = 5,

	// Deprecated
	kIOPMCannotRaisePower       = 6,

	// Deprecated
	kIOPMParameterError         = 7,

	// Returned when power management state is accessed
	// before driver has called PMinit().
	kIOPMNotYetInitialized      = 8,

	// And the old constants; deprecated
	IOPMNoErr = kIOPMNoErr,
	IOPMAckImplied = kIOPMAckImplied,
	IOPMWillAckLater = kIOPMWillAckLater,
	IOPMBadSpecification = kIOPMBadSpecification,
	IOPMNoSuchState = kIOPMNoSuchState,
	IOPMCannotRaisePower = kIOPMCannotRaisePower,
	IOPMParameterError = kIOPMParameterError,
	IOPMNotYetInitialized = kIOPMNotYetInitialized
};


// IOPMPowerSource class descriptive strings
// Power Source state is published as properties to the IORegistry under these
// keys.
#define kIOPMPSExternalConnectedKey                 "ExternalConnected"
#define kIOPMPSExternalChargeCapableKey             "ExternalChargeCapable"
#define kIOPMPSBatteryInstalledKey                  "BatteryInstalled"
#define kIOPMPSIsChargingKey                        "IsCharging"
#define kIOPMFullyChargedKey                        "FullyCharged"
#define kIOPMPSAtWarnLevelKey                       "AtWarnLevel"
#define kIOPMPSAtCriticalLevelKey                   "AtCriticalLevel"
#define kIOPMPSCurrentCapacityKey                   "CurrentCapacity"
#define kIOPMPSMaxCapacityKey                       "MaxCapacity"
#define kIOPMPSDesignCapacityKey                    "DesignCapacity"
#define kIOPMPSTimeRemainingKey                     "TimeRemaining"
#define kIOPMPSAmperageKey                          "Amperage"
#define kIOPMPSVoltageKey                           "Voltage"
#define kIOPMPSCycleCountKey                        "CycleCount"
#define kIOPMPSMaxErrKey                            "MaxErr"
#define kIOPMPSAdapterInfoKey                       "AdapterInfo"
#define kIOPMPSLocationKey                          "Location"
#define kIOPMPSErrorConditionKey                    "ErrorCondition"
#define kIOPMPSManufacturerKey                      "Manufacturer"
#define kIOPMPSManufactureDateKey                   "ManufactureDate"
#define kIOPMPSModelKey                             "Model"
#define kIOPMPSSerialKey                            "Serial"
#define kIOPMDeviceNameKey                          "DeviceName"
#define kIOPMPSLegacyBatteryInfoKey                 "LegacyBatteryInfo"
#define kIOPMPSBatteryHealthKey                     "BatteryHealth"
#define kIOPMPSHealthConfidenceKey                  "HealthConfidence"
#define kIOPMPSCapacityEstimatedKey                 "CapacityEstimated"
#define kIOPMPSBatteryChargeStatusKey               "ChargeStatus"
#define kIOPMPSBatteryTemperatureKey                "Temperature"
#define kIOPMPSAdapterDetailsKey                    "AdapterDetails"
#define kIOPMPSChargerConfigurationKey              "ChargerConfiguration"

// kIOPMPSBatteryChargeStatusKey may have one of the following values, or may have
// no value. If kIOPMBatteryChargeStatusKey has a NULL value (or no value) associated with it
// then charge is proceeding normally. If one of these battery charge status reasons is listed,
// then the charge may have been interrupted.
#define kIOPMBatteryChargeStatusTooHot              "HighTemperature"
#define kIOPMBatteryChargeStatusTooCold             "LowTemperature"
#define kIOPMBatteryChargeStatusTooHotOrCold        "HighOrLowTemperature"
#define kIOPMBatteryChargeStatusGradient            "BatteryTemperatureGradient"

// Definitions for battery location, in case of multiple batteries.
// A location of 0 is unspecified
// Location is undefined for single battery systems
enum {
	kIOPMPSLocationLeft = 1001,
	kIOPMPSLocationRight = 1002
};

// Battery quality health types, specified by BatteryHealth and HealthConfidence
// properties in an IOPMPowerSource battery kext.
enum {
	kIOPMUndefinedValue = 0,
	kIOPMPoorValue      = 1,
	kIOPMFairValue      = 2,
	kIOPMGoodValue      = 3
};

// Keys for kIOPMPSAdapterDetailsKey dictionary
#define kIOPMPSAdapterDetailsIDKey                  "AdapterID"
#define kIOPMPSAdapterDetailsWattsKey               "Watts"
#define kIOPMPSAdapterDetailsRevisionKey            "AdapterRevision"
#define kIOPMPSAdapterDetailsSerialNumberKey        "SerialNumber"
#define kIOPMPSAdapterDetailsFamilyKey              "FamilyCode"
#define kIOPMPSAdapterDetailsAmperageKey            "Current"
#define kIOPMPSAdapterDetailsDescriptionKey         "Description"
#define kIOPMPSAdapterDetailsPMUConfigurationKey    "PMUConfiguration"
#define kIOPMPSAdapterDetailsVoltage                "Voltage"
#define kIOPMPSAdapterDetailsSourceIDKey            "Source"
#define kIOPMPSAdapterDetailsErrorFlagsKey          "ErrorFlags"
#define kIOPMPSAdapterDetailsSharedSourceKey        "SharedSource"
#define kIOPMPSAdapterDetailsCloakedKey             "CloakedSource"

// values for kIOPSPowerAdapterFamilyKey
enum {
	kIOPSFamilyCodeDisconnected     = 0,
	kIOPSFamilyCodeUnsupported      = kIOReturnUnsupported,
	kIOPSFamilyCodeFirewire     = iokit_family_err(sub_iokit_firewire, 0),
	kIOPSFamilyCodeUSBHost      = iokit_family_err(sub_iokit_usb, 0),
	kIOPSFamilyCodeUSBHostSuspended   = iokit_family_err(sub_iokit_usb, 1),
	kIOPSFamilyCodeUSBDevice      = iokit_family_err(sub_iokit_usb, 2),
	kIOPSFamilyCodeUSBAdapter     = iokit_family_err(sub_iokit_usb, 3),
	kIOPSFamilyCodeUSBChargingPortDedicated = iokit_family_err(sub_iokit_usb, 4),
	kIOPSFamilyCodeUSBChargingPortDownstream  = iokit_family_err(sub_iokit_usb, 5),
	kIOPSFamilyCodeUSBChargingPort    = iokit_family_err(sub_iokit_usb, 6),
	kIOPSFamilyCodeUSBUnknown     = iokit_family_err(sub_iokit_usb, 7),
	kIOPSFamilyCodeUSBCBrick      = iokit_family_err(sub_iokit_usb, 8),
	kIOPSFamilyCodeUSBCTypeC      = iokit_family_err(sub_iokit_usb, 9),
	kIOPSFamilyCodeUSBCPD         = iokit_family_err(sub_iokit_usb, 10),
	kIOPSFamilyCodeAC       = iokit_family_err(sub_iokit_pmu, 0),
	kIOPSFamilyCodeExternal     = iokit_family_err(sub_iokit_pmu, 1),
	kIOPSFamilyCodeExternal2     = iokit_family_err(sub_iokit_pmu, 2),
	kIOPSFamilyCodeExternal3     = iokit_family_err(sub_iokit_pmu, 3),
	kIOPSFamilyCodeExternal4     = iokit_family_err(sub_iokit_pmu, 4),
	kIOPSFamilyCodeExternal5     = iokit_family_err(sub_iokit_pmu, 5),
};

// values for kIOPMPSAdapterDetailsErrorFlagsKey
enum {
	kIOPSAdapterErrorFlagNoErrors                     = 0,
	kIOPSAdapterErrorFlagInsufficientAvailablePower   = (1 << 1),
	kIOPSAdapterErrorFlagForeignObjectDetected        = (1 << 2),
	kIOPSAdapterErrorFlagDeviceNeedsToBeRepositioned  = (1 << 3),
};

// Battery's time remaining estimate is invalid this long (seconds) after a wake
#define kIOPMPSInvalidWakeSecondsKey           "BatteryInvalidWakeSeconds"

// Battery must wait this long (seconds) after being completely charged before
// the battery is settled.
#define kIOPMPSPostChargeWaitSecondsKey        "PostChargeWaitSeconds"

// Battery must wait this long (seconds) after being completely discharged
// before the battery is settled.
#define kIOPMPSPostDishargeWaitSecondsKey      "PostDischargeWaitSeconds"


/* CPU Power Management status keys
 * Pass as arguments to IOPMrootDomain::systemPowerEventOccurred
 * Or as arguments to IOPMSystemPowerEventOccurred()
 * Or to decode the dictionary obtained from IOPMCopyCPUPowerStatus()
 * These keys reflect restrictions placed on the CPU by the system
 * to bring the CPU's power consumption within allowable thermal and
 * power constraints.
 */


/* kIOPMGraphicsPowerLimitsKey
 *   The key representing the dictionary of graphics power limits.
 *   The dictionary contains the other kIOPMCPUPower keys & their associated
 *   values (e.g. Speed limit, Processor Count, and Schedule limits).
 */
#define kIOPMGraphicsPowerLimitsKey                     "Graphics_Power_Limits"

/* kIOPMGraphicsPowerLimitPerformanceKey
 *   The key representing the percent of overall performance made available
 *   by the graphics chip as a percentage (integer 0 - 100).
 */
#define kIOPMGraphicsPowerLimitPerformanceKey           "Graphics_Power_Performance"



/* kIOPMCPUPowerLimitsKey
 *   The key representing the dictionary of CPU Power Limits.
 *   The dictionary contains the other kIOPMCPUPower keys & their associated
 *   values (e.g. Speed limit, Processor Count, and Schedule limits).
 */
#define kIOPMCPUPowerLimitsKey                          "CPU_Power_Limits"

/* kIOPMCPUPowerLimitProcessorSpeedKey defines the speed & voltage limits placed
 *   on the CPU.
 *   Represented as a percentage (0-100) of maximum CPU speed.
 */
#define kIOPMCPUPowerLimitProcessorSpeedKey             "CPU_Speed_Limit"

/* kIOPMCPUPowerLimitProcessorCountKey reflects how many, if any, CPUs have been
 *   taken offline. Represented as an integer number of CPUs (0 - Max CPUs).
 */
#define kIOPMCPUPowerLimitProcessorCountKey             "CPU_Available_CPUs"

/* kIOPMCPUPowerLimitSchedulerTimeKey represents the percentage (0-100) of CPU time
 *   available. 100% at normal operation. The OS may limit this time for a percentage
 *   less than 100%.
 */
#define kIOPMCPUPowerLimitSchedulerTimeKey              "CPU_Scheduler_Limit"


/* Thermal Level Warning Key
 * Indicates the thermal constraints placed on the system. This value may
 * cause clients to action to consume fewer system resources.
 * The value associated with this warning is defined by the platform.
 */
#define kIOPMThermalLevelWarningKey                     "Thermal_Level_Warning"

/* Thermal Warning Level values
 *      kIOPMThermalLevelNormal   - under normal operating conditions
 *      kIOPMThermalLevelDanger   - thermal pressure may cause system slowdown
 *      kIOPMThermalLevelCritical - thermal conditions may cause imminent shutdown
 *
 * The platform may define additional thermal levels if necessary.
 * Platform specific values are defined from 100 and above
 */
enum {
	kIOPMThermalLevelNormal    = 0,
	kIOPMThermalLevelDanger    = 5,
	kIOPMThermalLevelCritical  = 10,

	kIOPMThermalLevelWarning = 100,
	kIOPMThermalLevelTrap    = 110,

	kIOPMThermalLevelUnknown = 255,
};

#define kIOPMThermalWarningLevelNormal kIOPMThermalLevelNormal
#define kIOPMThermalWarningLevelDanger kIOPMThermalLevelWarning
#define kIOPMThermalWarningLevelCrisis kIOPMThermalLevelCritical

// PM Settings Controller setting types
// Settings types used primarily with:
//      IOPMrootDomain::registerPMSettingController
// The values are identical to the similarly named keys for use in user space
// PM settings work. Those keys are defined in IOPMLibPrivate.h.
#define kIOPMSettingWakeOnRingKey                   "Wake On Modem Ring"
#define kIOPMSettingRestartOnPowerLossKey           "Automatic Restart On Power Loss"
#define kIOPMSettingWakeOnACChangeKey               "Wake On AC Change"
#define kIOPMSettingSleepOnPowerButtonKey           "Sleep On Power Button"
#define kIOPMSettingWakeOnClamshellKey              "Wake On Clamshell Open"
#define kIOPMSettingReduceBrightnessKey             "ReduceBrightness"
#define kIOPMSettingDisplaySleepUsesDimKey          "Display Sleep Uses Dim"
#define kIOPMSettingTimeZoneOffsetKey               "TimeZoneOffsetSeconds"
#define kIOPMSettingMobileMotionModuleKey           "MobileMotionModule"
#define kIOPMSettingGraphicsSwitchKey               "GPUSwitch"
#define kIOPMSettingProModeControl                  "ProModeControl"
#define kIOPMSettingProModeDefer                    "ProModeDefer"

// Setting controlling drivers can register to receive scheduled wake data
// Either in "CF seconds" type, or structured calendar data in a formatted
// IOPMCalendarStruct defined below.
#define kIOPMSettingAutoWakeSecondsKey              "wake"
#define kIOPMSettingAutoWakeCalendarKey             "WakeByCalendarDate"
#define kIOPMSettingAutoPowerSecondsKey             "poweron"
#define kIOPMSettingAutoPowerCalendarKey            "PowerByCalendarDate"

// Debug seconds auto wake
// Used by sleep cycling debug tools
#define kIOPMSettingDebugWakeRelativeKey            "WakeRelativeToSleep"
#define kIOPMSettingDebugPowerRelativeKey           "PowerRelativeToShutdown"

// Maintenance wake calendar.
#define kIOPMSettingMaintenanceWakeCalendarKey      "MaintenanceWakeCalendarDate"


struct IOPMCalendarStruct {
	UInt32      year;
	UInt8       month;
	UInt8       day;
	UInt8       hour;
	UInt8       minute;
	UInt8       second;
	UInt8       selector;
};
typedef struct IOPMCalendarStruct IOPMCalendarStruct;

// SetAggressiveness types
enum {
	kPMGeneralAggressiveness = 0,
	kPMMinutesToDim,
	kPMMinutesToSpinDown,
	kPMMinutesToSleep,
	kPMEthernetWakeOnLANSettings,
	kPMSetProcessorSpeed,
	kPMPowerSource,
	kPMMotionSensor,
	kPMLastAggressivenessType
};
#define kMaxType (kPMLastAggressivenessType-1)

// SetAggressiveness values for the kPMPowerSource aggressiveness type
enum {
	kIOPMInternalPower = 1,
	kIOPMExternalPower
};

#define kIOREMSleepEnabledKey               "REMSleepEnabled"

// Strings for deciphering the dictionary returned from IOPMCopyBatteryInfo
#define kIOBatteryInfoKey                   "IOBatteryInfo"
#define kIOBatteryCurrentChargeKey          "Current"
#define kIOBatteryCapacityKey               "Capacity"
#define kIOBatteryFlagsKey                  "Flags"
#define kIOBatteryVoltageKey                "Voltage"
#define kIOBatteryAmperageKey               "Amperage"
#define kIOBatteryCycleCountKey             "Cycle Count"

enum {
	kIOBatteryInstalled         = (1 << 2),
	kIOBatteryCharge            = (1 << 1),
	kIOBatteryChargerConnect    = (1 << 0)
};

// Private power management message indicating battery data has changed
// Indicates new data resides in the IORegistry
#define kIOPMMessageBatteryStatusHasChanged         iokit_family_msg(sub_iokit_pmu, 0x100)

// Apple private Legacy messages for re-routing AutoWake and AutoPower messages to the PMU
// through newer user space IOPMSchedulePowerEvent API
#define kIOPMUMessageLegacyAutoWake                 iokit_family_msg(sub_iokit_pmu, 0x200)
#define kIOPMUMessageLegacyAutoPower                iokit_family_msg(sub_iokit_pmu, 0x210)

// For use with IOPMPowerSource bFlags
#define IOPM_POWER_SOURCE_REV   2
enum {
	kIOPMACInstalled      = kIOBatteryChargerConnect,
	kIOPMBatteryCharging  = kIOBatteryCharge,
	kIOPMBatteryInstalled = kIOBatteryInstalled,
	kIOPMUPSInstalled     = (1 << 3),
	kIOPMBatteryAtWarn    = (1 << 4),
	kIOPMBatteryDepleted  = (1 << 5),
	kIOPMACnoChargeCapability = (1 << 6), // AC adapter cannot charge battery
	kIOPMRawLowBattery    = (1 << 7), // used only by  Platform Expert
	kIOPMForceLowSpeed    = (1 << 8), // set by Platfm Expert, chk'd by Pwr Plugin
	kIOPMClosedClamshell  = (1 << 9), // set by PMU - reflects state of the clamshell
	kIOPMClamshellStateOnWake = (1 << 10) // used only by Platform Expert
};

// **********************************************
// Internal power management data structures
// **********************************************

#if KERNEL && __cplusplus
class IOService;

enum {
	kIOPowerEmergencyLevel = 1000
};

enum {
	kIOPMSubclassPolicy,
	kIOPMSuperclassPolicy1
#ifdef KERNEL_PRIVATE
	, kIOPMActivityTickleTypeAdvisory = 128
#endif
};

struct stateChangeNote {
	IOPMPowerFlags    stateFlags;
	unsigned long    stateNum;
	void *         powerRef;
};
typedef struct stateChangeNote stateChangeNote;

#endif /* KERNEL && __cplusplus */
struct IOPowerStateChangeNotification {
	void *        powerRef;
	unsigned long    returnValue;
	unsigned long    stateNumber;
	IOPMPowerFlags    stateFlags;
};
typedef struct IOPowerStateChangeNotification IOPowerStateChangeNotification;
typedef IOPowerStateChangeNotification sleepWakeNote;

/*! @struct IOPMSystemCapabilityChangeParameters
 *   @abstract A structure describing a system capability change.
 *   @discussion A system capability change is a system level transition from a set
 *       of system capabilities to a new set of system capabilities. Power management
 *       sends a <code>kIOMessageSystemCapabilityChange</code> message and provides
 *       this structure as the message data (by reference) to
 *       <code>gIOPriorityPowerStateInterest</code> clients when system capability
 *       changes.
 *   @field notifyRef An identifier for this message notification. Clients with pending
 *       I/O can signal completion by calling <code>allowPowerChange()</code> with this
 *       value as the argument. Clients that are able to process the notification
 *       synchronously should ignore this field.
 *   @field maxWaitForReply A return value to the caller indicating the maximum time in
 *       microseconds to wait for the <code>allowPowerChange()</code> call. The default
 *       value is zero, which indicates the client processing has finished, and power
 *       management should not wait for an <code>allowPowerChange()</code> call.
 *   @field changeFlags Flags will be set to indicate whether the notification precedes
 *       the capability change (<code>kIOPMSystemCapabilityWillChange</code>), or after
 *       the capability change has occurred (<code>kIOPMSystemCapabilityDidChange</code>).
 *   @field __reserved1 Set to zero.
 *   @field fromCapabilities The system capabilities at the start of the transition.
 *   @field toCapabilities The system capabilities at the end of the transition.
 *   @field __reserved2 Set to zero.
 */
struct IOPMSystemCapabilityChangeParameters {
	uint32_t    notifyRef;
	uint32_t    maxWaitForReply;
	uint32_t    changeFlags;
	uint32_t    __reserved1;
	uint32_t    fromCapabilities;
	uint32_t    toCapabilities;
	uint32_t    __reserved2[4];
};

/*! @enum IOPMSystemCapabilityChangeFlags
 *   @constant kIOPMSystemCapabilityWillChange Indicates the system capability will change.
 *   @constant kIOPMSystemCapabilityDidChange Indicates the system capability has changed.
 */
enum {
	kIOPMSystemCapabilityWillChange = 0x01,
	kIOPMSystemCapabilityDidChange  = 0x02
};

enum {
	kIOPMSystemCapabilityCPU        = 0x01,
	kIOPMSystemCapabilityGraphics   = 0x02,
	kIOPMSystemCapabilityAudio      = 0x04,
	kIOPMSystemCapabilityNetwork    = 0x08
};

#endif /* ! _IOKIT_IOPM_H */
