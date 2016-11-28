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
 * Copyright (c) 1998-2000 Apple Computer, Inc.  All rights reserved. 
 *
 * HISTORY
 *
 */


#ifndef _IOKIT_APPLEPLATFORM_H
#define _IOKIT_APPLEPLATFORM_H

#include <IOKit/IOPlatformExpert.h>

enum {
  kBootROMTypeOldWorld = 0,
  kBootROMTypeNewWorld
};

enum {
  kChipSetTypePowerSurge = 0,
  kChipSetTypePowerStar,
  kChipSetTypeGossamer,
  kChipSetTypePowerExpress,
  kChipSetTypeCore99,
  kChipSetTypeCore2001
};

enum {
  kMachineTypeUnknown = 0
};

extern const OSSymbol *gGetDefaultBusSpeedsKey;

class ApplePlatformExpert : public IODTPlatformExpert
{
  OSDeclareAbstractStructors(ApplePlatformExpert);
  
private:    
  SInt32 _timeToGMT;

  struct ExpansionData { };
  ExpansionData *reserved;

public:
  virtual bool start( IOService * provider ) APPLE_KEXT_OVERRIDE;
  virtual bool configure( IOService * provider ) APPLE_KEXT_OVERRIDE;
  virtual const char * deleteList( void ) APPLE_KEXT_OVERRIDE;
  virtual const char * excludeList( void ) APPLE_KEXT_OVERRIDE;
  
  virtual void registerNVRAMController( IONVRAMController * nvram ) APPLE_KEXT_OVERRIDE;
  
  virtual long getGMTTimeOfDay(void) APPLE_KEXT_OVERRIDE;
  virtual void setGMTTimeOfDay(long secs) APPLE_KEXT_OVERRIDE;
  
  virtual bool getMachineName(char *name, int maxLength) APPLE_KEXT_OVERRIDE;

  OSMetaClassDeclareReservedUnused(ApplePlatformExpert,  0);
  OSMetaClassDeclareReservedUnused(ApplePlatformExpert,  1);
  OSMetaClassDeclareReservedUnused(ApplePlatformExpert,  2);
  OSMetaClassDeclareReservedUnused(ApplePlatformExpert,  3);
};


#endif /* ! _IOKIT_APPLEPLATFORM_H */
