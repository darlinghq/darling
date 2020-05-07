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

#ifndef __IOKIT_IOMESSAGE_H
#define __IOKIT_IOMESSAGE_H

#include <IOKit/IOReturn.h>
#include <IOKit/IOTypes.h>

/*!
 * @header IOMessage.h
 *
 * Defines message type constants for several IOKit messaging API's.
 *
 */

typedef UInt32 IOMessage;

#define iokit_common_msg(message)          (UInt32)(sys_iokit|sub_iokit_common|message)
#define iokit_family_msg(sub, message)      (UInt32)(sys_iokit|sub|message)

/*!
 * @defined         iokit_vendor_specific_msg
 * @discussion      iokit_vendor_specific_msg passes messages in the sub_iokit_vendor_specific
 *                  subsystem. It can be used to generate messages that are used for private
 *                  communication between vendor specific code with the IOService::message() etc. APIs.
 */
#define iokit_vendor_specific_msg(message) (UInt32)(sys_iokit|sub_iokit_vendor_specific|message)

#define kIOMessageServiceIsTerminated      iokit_common_msg(0x010)
#define kIOMessageServiceIsSuspended       iokit_common_msg(0x020)
#define kIOMessageServiceIsResumed         iokit_common_msg(0x030)

#define kIOMessageServiceIsRequestingClose iokit_common_msg(0x100)
#define kIOMessageServiceIsAttemptingOpen  iokit_common_msg(0x101)
#define kIOMessageServiceWasClosed         iokit_common_msg(0x110)

#define kIOMessageServiceBusyStateChange   iokit_common_msg(0x120)

#define kIOMessageConsoleSecurityChange    iokit_common_msg(0x128)

#define kIOMessageServicePropertyChange    iokit_common_msg(0x130)

#define kIOMessageCopyClientID             iokit_common_msg(0x330)

#define kIOMessageSystemCapabilityChange   iokit_common_msg(0x340)
#define kIOMessageDeviceSignaledWakeup     iokit_common_msg(0x350)

#ifdef KERNEL_PRIVATE
// sent to IOUserClients with the property kIOUserClientMessageAppSuspendedKey
// when their task's app suspend state changes;
// use task_is_app_suspended() to retrieve the owning task's current state
#define kIOMessageTaskAppSuspendedChange   iokit_common_msg(0x800)
#endif

/*!
 * @defined         kIOMessageDeviceWillPowerOff
 * @discussion      Indicates the device is about to move to a lower power state.
 *                  Sent to IOKit interest notification clients of type <code>kIOAppPowerStateInterest</code>
 *                  and <code>kIOGeneralInterest</code>.
 */
#define kIOMessageDeviceWillPowerOff       iokit_common_msg(0x210)

/*!
 * @defined         kIOMessageDeviceHasPoweredOn
 * @discussion      Indicates the device has just moved to a higher power state.
 *                  Sent to IOKit interest notification clients of type <code>kIOAppPowerStateInterest</code>
 *                  and <code>kIOGeneralInterest</code>.
 */
#define kIOMessageDeviceHasPoweredOn       iokit_common_msg(0x230)

/*! @group          In-kernel system shutdown and restart notifications
 */

/*!
 * @defined         kIOMessageSystemWillPowerOff
 * @discussion      Indicates an imminent system shutdown. Recipients have a limited
 *                  amount of time to respond, otherwise the system will timeout and
 *                  shutdown even without a response.
 *                  Delivered to in-kernel IOKit drivers via <code>IOService::systemWillShutdown()</code>,
 *                  and to clients of <code>registerPrioritySleepWakeInterest()</code>.
 *                  Never delivered to user space notification clients.
 */
#define kIOMessageSystemWillPowerOff       iokit_common_msg(0x250)

/*!
 * @defined         kIOMessageSystemWillRestart
 * @discussion      Indicates an imminent system restart. Recipients have a limited
 *                  amount of time to respond, otherwise the system will timeout and
 *                  restart even without a response.
 *                  Delivered to in-kernel IOKit drivers via <code>IOService::systemWillShutdown()</code>,
 *                  and to clients of <code>registerPrioritySleepWakeInterest()</code>.
 *                  Never delivered to user space notification clients.
 */
#define kIOMessageSystemWillRestart        iokit_common_msg(0x310)

/*!
 * @defined         kIOMessageSystemPagingOff
 * @discussion      Indicates an imminent system shutdown, paging device now unavailable.
 *					Recipients have a limited amount of time to respond, otherwise the
 *					system will timeout and shutdown even without a response.
 *                  Delivered to clients of <code>registerPrioritySleepWakeInterest()</code>.
 *                  Never delivered to user space notification clients.
 */
#define kIOMessageSystemPagingOff       iokit_common_msg(0x255)


/*! @group          System sleep and wake notifications
 */

/*!
 * @defined         kIOMessageCanSystemSleep
 * @discussion      Announces/Requests permission to proceed to system sleep.
 *                  Delivered to in-kernel IOKit drivers via <code>kIOGeneralInterest</code>
 *                  and <code>kIOPriorityPowerStateInterest</code>.
 *                  Delivered to user clients of <code>IORegisterForSystemPower</code>.
 */
#define kIOMessageCanSystemSleep           iokit_common_msg(0x270)

/*!
 * @defined         kIOMessageSystemWillNotSleep
 * @discussion      Announces that the system has retracted a previous attempt to sleep;
 *                  it follows <code>kIOMessageCanSystemSleep</code>.
 *                  Delivered to in-kernel IOKit drivers via <code>kIOGeneralInterest</code>
 *                  and <code>kIOPriorityPowerStateInterest</code>.
 *                  Delivered to user clients of <code>IORegisterForSystemPower</code>.
 */
#define kIOMessageSystemWillNotSleep       iokit_common_msg(0x290)

/*!
 * @defined         kIOMessageSystemWillSleep
 * @discussion      Announces that sleep is beginning.
 *                  Delivered to in-kernel IOKit drivers via <code>kIOGeneralInterest</code>
 *                  and <code>kIOPriorityPowerStateInterest</code>.
 *                  Delivered to user clients of <code>IORegisterForSystemPower</code>.
 */
#define kIOMessageSystemWillSleep          iokit_common_msg(0x280)

/*!
 * @defined         kIOMessageSystemWillPowerOn
 * @discussion      Announces that the system is beginning to power the device tree; most
 *                  devices are unavailable at this point..
 *                  Delivered to in-kernel IOKit drivers via <code>kIOGeneralInterest</code>
 *                  and <code>kIOPriorityPowerStateInterest</code>.
 *                  Delivered to user clients of <code>IORegisterForSystemPower</code>.
 */
#define kIOMessageSystemWillPowerOn        iokit_common_msg(0x320)

/*!
 * @defined         kIOMessageSystemHasPoweredOn
 * @discussion      Announces that the system and its devices have woken up.
 *                  Delivered to in-kernel IOKit drivers via <code>kIOGeneralInterest</code>
 *                  and <code>kIOPriorityPowerStateInterest</code>.
 *                  Delivered to user clients of <code>IORegisterForSystemPower</code>.
 */
#define kIOMessageSystemHasPoweredOn       iokit_common_msg(0x300)

/*! @group          Unused and deprecated notifications
 */

/*!
 * @defined         kIOMessageCanDevicePowerOff
 * @discussion      Delivered to <code>kIOAppPowerStateInterest</code> clients of
 *                  devices that implement their own idle timeouts.
 *                  This message type is almost never used.
 */
#define kIOMessageCanDevicePowerOff        iokit_common_msg(0x200)

/*!
 * @defined         kIOMessageDeviceWillNotPowerOff
 * @discussion      This IOKit interest notification is largely unused;
 *                  it's not very interesting.
 */
#define kIOMessageDeviceWillNotPowerOff    iokit_common_msg(0x220)

/*!
 * @defined         kIOMessageSystemWillNotPowerOff
 * @deprecated      This IOKit message is unused.
 */
#define kIOMessageSystemWillNotPowerOff    iokit_common_msg(0x260)

/*!
 * @defined         kIOMessageCanSystemPowerOff
 * @deprecated      This IOKit message is unused.
 */
#define kIOMessageCanSystemPowerOff        iokit_common_msg(0x240)

/*!
 * @defined         kIOMessageDeviceWillPowerOn
 * @discussion      IOService power mgt does not send kIOMessageDeviceWillPowerOn.
 */
#define kIOMessageDeviceWillPowerOn        iokit_common_msg(0x215)

/*!
 * @defined         kIOMessageDeviceHasPoweredOff
 * @discussion      IOService power mgt does not send kIOMessageDeviceHasPoweredOff.
 */
#define kIOMessageDeviceHasPoweredOff      iokit_common_msg(0x225)


#endif /* ! __IOKIT_IOMESSAGE_H */
