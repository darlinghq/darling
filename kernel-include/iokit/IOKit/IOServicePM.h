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

#ifndef _IOKIT_IOSERVICEPM_H
#define _IOKIT_IOSERVICEPM_H

#include <IOKit/pwr_mgt/IOPM.h>

class IOService;
class IOServicePM;
class IOPowerConnection;
class IOWorkLoop;
class IOCommandGate;
class IOTimerEventSource;
class IOPlatformExpert;

#ifdef XNU_KERNEL_PRIVATE
class IOPMinformee;
class IOPMinformeeList;
class IOPMWorkQueue;
class IOPMRequest;
class IOPMRequestQueue;
class IOPMCompletionQueue;

typedef void (*IOPMCompletionAction)(void * target, void * param);

// PM channels for IOReporting
#ifndef kPMPowerStatesChID
#define kPMPowerStatesChID  IOREPORT_MAKEID('P','M','S','t','H','i','s','t')
#endif

#ifndef kPMCurrStateChID
#define kPMCurrStateChID  IOREPORT_MAKEID( 'P','M','C','u','r','S','t','\0' )
#endif

// state_id details in PM channels
#define kPMReportPowerOn       0x01
#define kPMReportDeviceUsable  0x02
#define kPMReportLowPower      0x04


typedef unsigned long       IOPMPowerStateIndex;
typedef uint32_t            IOPMPowerChangeFlags;
typedef uint32_t            IOPMRequestTag;

struct IOPMDriverCallEntry {
    queue_chain_t   link;
    thread_t        thread;
    IOService *     target;
};

// Power clients (desires)
extern const OSSymbol *     gIOPMPowerClientDevice;
extern const OSSymbol *     gIOPMPowerClientDriver;
extern const OSSymbol *     gIOPMPowerClientChildProxy;
extern const OSSymbol *     gIOPMPowerClientChildren;
extern const OSSymbol *     gIOPMPowerClientRootDomain;

/* Binary compatibility with drivers that access pm_vars */
#ifdef __LP64__
#define PM_VARS_SUPPORT     0
#else
#define PM_VARS_SUPPORT     1
#endif

#if PM_VARS_SUPPORT
/* Deprecated in version 10.5 */
class IOPMprot : public OSObject
{
    friend class IOService;
    
    OSDeclareDefaultStructors(IOPMprot)

public:
    const char *            ourName;
    IOPlatformExpert *      thePlatform;
    unsigned long           theNumberOfPowerStates;
    IOPMPowerState          thePowerStates[IOPMMaxPowerStates];
    IOService *             theControllingDriver;
    unsigned long           aggressiveness;
    unsigned long           current_aggressiveness_values[kMaxType+1];
    bool                    current_aggressiveness_valid[kMaxType+1];
    unsigned long           myCurrentState;
};
#endif /* PM_VARS_SUPPORT */
#endif /* XNU_KERNEL_PRIVATE */
#endif /* !_IOKIT_IOSERVICEPM_H */
