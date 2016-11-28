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
/* Copyright (c) 1999 Apple Computer, Inc.  All rights reserved.
 * Copyright (c) 1994-1996 NeXT Software, Inc.  All rights reserved. 
 */

#ifndef _IOKIT_IOCONDITIONLOCK_H
#define _IOKIT_IOCONDITIONLOCK_H

#include <libkern/c++/OSObject.h>
#include <IOKit/IOLib.h>
#include <IOKit/system.h>

#include <IOKit/IOLocks.h>

class IOConditionLock : public OSObject
{
    OSDeclareDefaultStructors(IOConditionLock)

private:
    IOLock *		cond_interlock;		// condition var Simple lock
    volatile int	condition;

    IOLock *		sleep_interlock;	// sleep lock Simple lock
    unsigned char	interruptible;
    volatile bool	want_lock;
    volatile bool	waiting;

public:
    static IOConditionLock *withCondition(int condition, bool inIntr = true);
    virtual bool initWithCondition(int condition, bool inIntr = true);
    virtual void free() APPLE_KEXT_OVERRIDE;

    virtual bool tryLock();	// acquire lock, no waiting
    virtual int  lock();	// acquire lock (enter critical section)
    virtual void unlock();	// release lock (leave critical section)

    virtual bool getInterruptible() const;
    virtual int  getCondition() const;
    virtual int  setCondition(int condition);

    virtual int  lockWhen(int condition); // acquire lock when condition
    virtual void unlockWith(int condition); // set condition & release lock
};

#endif /* _IOKIT_IOCONDITIONLOCK_H */
