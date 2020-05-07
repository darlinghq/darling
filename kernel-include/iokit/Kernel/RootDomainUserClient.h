/*
 * Copyright (c) 1998-2019 Apple Inc. All rights reserved.
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
 * Copyright (c) 1999 Apple Computer, Inc.  All rights reserved.
 *
 * HISTORY
 *
 */


#ifndef _IOKIT_ROOTDOMAINUSERCLIENT_H
#define _IOKIT_ROOTDOMAINUSERCLIENT_H

#include <IOKit/IOUserClient.h>
#include <IOKit/pwr_mgt/IOPM.h>
#include <IOKit/pwr_mgt/RootDomain.h>
#include <IOKit/pwr_mgt/IOPMLibDefs.h>


class RootDomainUserClient : public IOUserClient
{
	OSDeclareDefaultStructors(RootDomainUserClient);

	friend class IOPMrootDomain;
private:
	IOPMrootDomain *    fOwner;
	task_t              fOwningTask;

	IOReturn            secureSleepSystem( uint32_t *return_code );

	IOReturn            secureSleepSystemOptions( const void  *inOptions,
	    IOByteCount  inOptionsSize,
	    uint32_t  *returnCode);

	IOReturn            secureSetAggressiveness( unsigned long type,
	    unsigned long newLevel,
	    int *return_code );

	IOReturn            secureSetMaintenanceWakeCalendar(
		IOPMCalendarStruct  *inCalendar,
		uint32_t            *returnCode);

	IOReturn            secureSetUserAssertionLevels(uint32_t    assertionBitfield);

	IOReturn            secureGetSystemSleepType( uint32_t *sleepType, uint32_t *sleepTimer);

public:

	virtual IOReturn clientClose( void ) APPLE_KEXT_OVERRIDE;

	virtual IOReturn externalMethod( uint32_t selector,
	    IOExternalMethodArguments * arguments,
	    IOExternalMethodDispatch * dispatch,
	    OSObject * target,
	    void * reference ) APPLE_KEXT_OVERRIDE;

	virtual bool start( IOService * provider ) APPLE_KEXT_OVERRIDE;

	virtual bool initWithTask(task_t owningTask, void *security_id,
	    UInt32 type, OSDictionary * properties) APPLE_KEXT_OVERRIDE;

// Unused - retained for symbol compatibility
	void setPreventative(UInt32 on_off, UInt32 types_of_sleep);

// Unused - retained for symbol compatibility
	virtual IOExternalMethod * getTargetAndMethodForIndex( IOService ** targetP, UInt32 index ) APPLE_KEXT_OVERRIDE;
	virtual void stop( IOService *provider) APPLE_KEXT_OVERRIDE;
};

#endif /* ! _IOKIT_ROOTDOMAINUSERCLIENT_H */
