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

#ifndef _IOKIT_IOPMPOWERSTATE_H
#define _IOKIT_IOPMPOWERSTATE_H

#include <IOKit/pwr_mgt/IOPM.h>

/*! @header IOPMpowerState.h
    @abstract Defines the struct IOPMPowerState that power managed drivers should use to describe their power states.
*/

/*! @struct IOPMPowerState
    @abstract Describes a device's power state.
    @discussion To take part in system power management, drivers should define an array of 2 or more power states and pass it to kernel power management through IOService::registerPowerDriver.
    @field version Defines version number of this struct. Just use the value "1" when defining an IOPMPowerState.
    @field	capabilityFlags Describes the capability of the device in this state.
    @field	outputPowerCharacter Describes the power provided in this state.
    @field	inputPowerRequirement Describes the input power required in this state.
    @field	staticPower Describes average consumption in milliwatts. Unused; drivers may specify 0.
    @field	stateOrder Valid in version kIOPMPowerStateVersion2 or greater of this structure. Defines ordering of power states independently of the power state ordinal.
    @field	powerToAttain Describes dditional power to attain this state from next lower state (in milliWatts). Unused; drivers may specify 0.
    @field	timeToAttain Describes time required to enter this state from next lower state (in microseconds). Unused; drivers may specify 0.
    @field	settleUpTime Describes settle time required after entering this state from next lower state (microseconds). Unused; drivers may specify 0.
    @field timeToLower Describes time required to enter next lower state from this one (microseconds). Unused; drivers may specify 0.
    @field	settleDownTime Settle time required after entering next lower state from this state (microseconds). Unused; drivers may specify 0.
    @field	powerDomainBudget Describes power in milliWatts a domain in this state can deliver to its children. Unused; drivers may specify 0.
}
*/

struct IOPMPowerState
{
    unsigned long	version;
    IOPMPowerFlags	capabilityFlags;
    IOPMPowerFlags	outputPowerCharacter;
    IOPMPowerFlags	inputPowerRequirement;
    unsigned long	staticPower;
    unsigned long	stateOrder;
    unsigned long	powerToAttain;
    unsigned long	timeToAttain;
    unsigned long	settleUpTime;
    unsigned long	timeToLower;
    unsigned long	settleDownTime;
    unsigned long	powerDomainBudget;
};

typedef struct IOPMPowerState IOPMPowerState;

enum {
    kIOPMPowerStateVersion1 = 1,
    kIOPMPowerStateVersion2 = 2
};

#endif /* _IOKIT_IOPMPOWERSTATE_H */
