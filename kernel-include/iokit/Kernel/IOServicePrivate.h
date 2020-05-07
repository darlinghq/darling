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


#ifndef _IOKIT_IOSERVICEPRIVATE_H
#define _IOKIT_IOSERVICEPRIVATE_H

// options for getExistingServices()
enum {
	kIONotifyOnce             = 0x00000001,
	kIOServiceExistingSet     = 0x00000002,
	kIOServiceChangesOK       = 0x00000004,
	kIOServiceInternalDone    = 0x00000008,
	kIOServiceClassDone       = 0x00000010,
};

// masks for __state[1]
enum {
	kIOServiceBusyStateMask     = 0x000003ff,
	kIOServiceBusyMax           = 1023,
	kIOServiceNeedConfigState   = 0x80000000,
	kIOServiceSynchronousState  = 0x40000000,
	kIOServiceModuleStallState  = 0x20000000,
	kIOServiceBusyWaiterState   = 0x10000000,

	kIOServiceSyncPubState      = 0x08000000,
	kIOServiceConfigState       = 0x04000000,
	kIOServiceStartState        = 0x02000000,
	kIOServiceTermPhase2State   = 0x01000000,
	kIOServiceTermPhase3State   = 0x00800000,
	kIOServiceTermPhase1State   = 0x00400000,
	kIOServiceTerm1WaiterState  = 0x00200000,
	kIOServiceRecursing         = 0x00100000,
	kIOServiceNeedWillTerminate = 0x00080000,
	kIOServiceWaitDetachState   = 0x00040000,
	kIOServiceConfigRunning     = 0x00020000,
	kIOServiceFinalized         = 0x00010000,
};

// notify state
enum {
	kIOServiceNotifyEnable      = 0x00000001,
	kIOServiceNotifyWaiter      = 0x00000002,
	kIOServiceNotifyBlock       = 0x00000004
};

struct _IOServiceNotifierInvocation {
	IOThread            thread;
	queue_chain_t       link;
};

class _IOServiceNotifier : public IONotifier
{
	friend class IOService;

	OSDeclareDefaultStructors(_IOServiceNotifier);

public:
	OSOrderedSet *                      whence;

	OSDictionary *                      matching;
	const OSSymbol *                    type;
	IOServiceMatchingNotificationHandler handler;
	IOServiceNotificationHandler        compatHandler;
	void *                              target;
	void *                              ref;
	SInt32                              priority;
	queue_head_t                        handlerInvocations;
	IOOptionBits                        state;

	virtual void free() APPLE_KEXT_OVERRIDE;
	virtual void remove() APPLE_KEXT_OVERRIDE;
	virtual bool disable() APPLE_KEXT_OVERRIDE;
	virtual void enable( bool was ) APPLE_KEXT_OVERRIDE;
	virtual void wait();
};

class _IOServiceInterestNotifier : public IONotifier
{
	friend class IOService;

	OSDeclareDefaultStructors(_IOServiceInterestNotifier);

public:
	queue_chain_t               chain;

	IOServiceInterestHandler    handler;
	void *                      target;
	void *                      ref;
	queue_head_t                handlerInvocations;
	IOOptionBits                state;

	virtual void free() APPLE_KEXT_OVERRIDE;
	virtual void remove() APPLE_KEXT_OVERRIDE;
	virtual bool disable() APPLE_KEXT_OVERRIDE;
	virtual void enable( bool was ) APPLE_KEXT_OVERRIDE;
	virtual void wait();
	virtual bool init() APPLE_KEXT_OVERRIDE;
};

class _IOServiceNullNotifier : public IONotifier
{
	OSDeclareDefaultStructors(_IOServiceNullNotifier);

public:
	virtual void taggedRetain(const void *tag) const APPLE_KEXT_OVERRIDE;
	virtual void taggedRelease(const void *tag, const int when) const APPLE_KEXT_OVERRIDE;
	virtual void free() APPLE_KEXT_OVERRIDE;
	virtual void remove() APPLE_KEXT_OVERRIDE;
	virtual bool disable() APPLE_KEXT_OVERRIDE;
	virtual void enable( bool was ) APPLE_KEXT_OVERRIDE;
	virtual void wait();
};

class _IOConfigThread : public OSObject
{
	friend class IOService;

	OSDeclareDefaultStructors(_IOConfigThread);

public:
	static void configThread( int configThreadId );
	static void main( void * arg, wait_result_t result );
};

enum {
	kMaxConfigThreads       = CONFIG_MAX_THREADS,
};

enum {
	kMatchNubJob        = 10,
};

class _IOServiceJob : public OSObject
{
	friend class IOService;

	OSDeclareDefaultStructors(_IOServiceJob);

public:
	int                 type;
	IOService *         nub;
	IOOptionBits        options;

	static _IOServiceJob * startJob( IOService * nub, int type,
	    IOOptionBits options = 0 );
	static void pingConfig( class _IOServiceJob * job );
};

class IOResources : public IOService
{
	friend class IOService;

	OSDeclareDefaultStructors(IOResources);

public:
	static IOService * resources( void );
	virtual bool init( OSDictionary * dictionary = NULL ) APPLE_KEXT_OVERRIDE;
	virtual IOReturn newUserClient(task_t owningTask, void * securityID,
	    UInt32 type, OSDictionary * properties,
	    IOUserClient ** handler) APPLE_KEXT_OVERRIDE;
	virtual IOWorkLoop * getWorkLoop() const APPLE_KEXT_OVERRIDE;
	virtual bool matchPropertyTable( OSDictionary * table ) APPLE_KEXT_OVERRIDE;
	virtual IOReturn setProperties( OSObject * properties ) APPLE_KEXT_OVERRIDE;
};

class IOUserResources : public IOService
{
	friend class IOService;

	OSDeclareDefaultStructors(IOUserResources);

public:
	static IOService * resources( void );
	virtual bool init( OSDictionary * dictionary = NULL ) APPLE_KEXT_OVERRIDE;
	virtual IOReturn newUserClient(task_t owningTask, void * securityID,
	    UInt32 type, OSDictionary * properties,
	    IOUserClient ** handler) APPLE_KEXT_OVERRIDE;
	virtual IOWorkLoop * getWorkLoop() const APPLE_KEXT_OVERRIDE;
	virtual bool matchPropertyTable( OSDictionary * table ) APPLE_KEXT_OVERRIDE;
};

class _IOOpenServiceIterator : public OSIterator
{
	friend class IOService;

	OSDeclareDefaultStructors(_IOOpenServiceIterator);

	OSIterator *        iter;
	const IOService *   client;
	const IOService *   provider;
	IOService *         last;

public:
	static OSIterator * iterator(LIBKERN_CONSUMED OSIterator * _iter,
	    const IOService * client,
	    const IOService * provider );
	virtual void free() APPLE_KEXT_OVERRIDE;
	virtual void reset() APPLE_KEXT_OVERRIDE;
	virtual bool isValid() APPLE_KEXT_OVERRIDE;
	virtual OSObject * getNextObject() APPLE_KEXT_OVERRIDE;
};

extern const OSSymbol * gIOConsoleUsersKey;
extern const OSSymbol * gIOConsoleSessionUIDKey;
extern const OSSymbol * gIOConsoleSessionAuditIDKey;
extern const OSSymbol * gIOConsoleSessionOnConsoleKey;
extern const OSSymbol * gIOConsoleSessionSecureInputPIDKey;


#define _interruptSourcesPrivate(service)   \
    ((IOInterruptSourcePrivate *)(&(service)->_interruptSources[(service)->_numInterruptSources]))

#define sizeofAllIOInterruptSource          \
    (sizeof(IOInterruptSourcePrivate) + sizeof(IOInterruptSource))

#endif /* ! _IOKIT_IOSERVICEPRIVATE_H */
