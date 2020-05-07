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
#ifndef _IOSYNCER_H
#define _IOSYNCER_H

#include <libkern/c++/OSObject.h>
#include <IOKit/IOTypes.h>
#include <IOKit/IOLocks.h>

class IOSyncer : public OSObject
{
	OSDeclareDefaultStructors(IOSyncer);

private:
// The spin lock that is used to guard the 'threadMustStop' variable.
	IOSimpleLock *guardLock;
	volatile bool threadMustStop;
	IOReturn fResult;
	virtual void free() APPLE_KEXT_OVERRIDE;
	virtual void privateSignal();

public:

	static IOSyncer * create(bool twoRetains = true)
	APPLE_KEXT_DEPRECATED;

	virtual bool init(bool twoRetains)
	APPLE_KEXT_DEPRECATED;
	virtual void reinit()
	APPLE_KEXT_DEPRECATED;
	virtual IOReturn wait(bool autoRelease = true)
	APPLE_KEXT_DEPRECATED;
	virtual void signal(IOReturn res = kIOReturnSuccess,
	    bool autoRelease = true)
	APPLE_KEXT_DEPRECATED;
};

#endif /* !_IOSYNCER */
