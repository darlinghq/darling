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
/*
 * Copyright (c) 1998-9 Apple Computer, Inc.  All rights reserved.
 *
 *  DRI: Josh de Cesare
 *
 */

#ifndef _IOKIT_APPLENMI_H
#define _IOKIT_APPLENMI_H

#include <IOKit/IOService.h>
#include <IOKit/IOInterrupts.h>

// NMI Interrupt Constants
enum
{
    kExtInt9_NMIIntSource      = 0x800506E0,
    kNMIIntLevelMask           = 0x00004000,
    kNMIIntMask                = 0x00000080
};


class AppleNMI : public IOService
{
  OSDeclareDefaultStructors(AppleNMI);

private:
  bool enable_debugger;
  bool mask_NMI;

  struct ExpansionData { };
  ExpansionData * reserved;	// Reserved for future use

public:
  IOService *rootDomain;
  virtual bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
  virtual IOReturn initNMI(IOInterruptController *parentController, OSData *parentSource);
  virtual IOReturn handleInterrupt(void *refCon, IOService *nub, int source);

  // Power handling methods:
  virtual IOReturn powerStateWillChangeTo(IOPMPowerFlags, unsigned long, IOService*) APPLE_KEXT_OVERRIDE;

  OSMetaClassDeclareReservedUnused(AppleNMI,  0);
  OSMetaClassDeclareReservedUnused(AppleNMI,  1);
  OSMetaClassDeclareReservedUnused(AppleNMI,  2);
  OSMetaClassDeclareReservedUnused(AppleNMI,  3);
};

#endif /* ! _IOKIT_APPLENMI_H */
