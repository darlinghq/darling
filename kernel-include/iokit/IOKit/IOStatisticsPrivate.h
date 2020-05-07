/*
 * Copyright (c) 2010 Apple Computer, Inc. All rights reserved.
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

#ifndef __IOKIT_STATISTICS_PRIVATE_H
#define __IOKIT_STATISTICS_PRIVATE_H

#if IOKITSTATS

#include <sys/queue.h>
#include <sys/tree.h>

#include <libkern/c++/OSKext.h>
#include <libkern/OSDebug.h>

#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IOStatistics.h>

#ifndef KERNEL
#error IOStatisticsPrivate.h is for kernel use only
#endif

/* Defines */
#define IOKIT_STATISTICS_RECORDED_USERCLIENT_PROCS 20

#ifndef __probable
#define __probable(x) x
#endif

/* Forward declarations */
class IOWorkLoop;
class IOUserClient;
class IOEventSource;

struct IOEventSourceCounter;
struct IOUserClientCounter;
struct IOWorkLoopCounter;
struct IOUserClientProcessEntry;

struct KextNode;

/* Allocation tracking */

enum {
	kIOStatisticsMalloc = 0,
	kIOStatisticsFree,
	kIOStatisticsMallocAligned,
	kIOStatisticsFreeAligned,
	kIOStatisticsMallocContiguous,
	kIOStatisticsFreeContiguous,
	kIOStatisticsMallocPageable,
	kIOStatisticsFreePageable,
	kIOStatisticsAllocCount
};

TAILQ_HEAD(ProcessEntryList, IOUserClientProcessEntry);

/* Tree and list structs */

typedef struct ClassNode {
	RB_ENTRY(ClassNode) tLink;
	SLIST_ENTRY(ClassNode) lLink;
	struct KextNode *parentKext;
	uint32_t classID;
	uint32_t superClassID;
	const OSMetaClass *metaClass;
	SLIST_HEAD(, IOEventSourceCounter) counterList;
	SLIST_HEAD(, IOUserClientCounter) userClientList;
} ClassNode;

typedef struct KextNode {
	RB_ENTRY(KextNode) link;
	RB_ENTRY(KextNode) addressLink;
	OSKext *kext;
	OSKextLoadTag loadTag;
	vm_offset_t address;
	vm_offset_t address_end;
	uint32_t memoryCounters[kIOStatisticsAllocCount];
	uint32_t classes;
	SLIST_HEAD(, ClassNode) classList;
	SLIST_HEAD(, IOWorkLoopCounter) workLoopList;
	ProcessEntryList userClientCallList;
} KextNode;

/* User client tracing */

typedef struct IOUserClientProcessEntry {
	TAILQ_ENTRY(IOUserClientProcessEntry) link;
	char processName[kIOStatisticsProcessNameLength];
	int32_t pid;
	uint32_t calls;
} IOUserClientProcessEntry;

/* Counters */

typedef struct IOInterruptEventSourceCounter {
	uint32_t produced;
	uint32_t checksForWork;
} IOInterruptEventSourceCounter;

typedef struct IOTimerEventSourceCounter {
	uint32_t timeouts;
	uint32_t checksForWork;
} IOTimerEventSourceCounter;

typedef struct IOCommandGateCounter {
	uint32_t actionCalls;
} IOCommandGateCounter;

typedef struct IOCommandQueueCounter {
	uint32_t actionCalls;
} IOCommandQueueCounter;

typedef struct IOEventSourceCounter {
	SLIST_ENTRY(IOEventSourceCounter) link;
	ClassNode *parentClass;
	IOStatisticsCounterType type;
	uint64_t startTimeStamp;
	uint64_t timeOnGate;
	uint32_t closeGateCalls;
	uint32_t openGateCalls;
	union {
		IOInterruptEventSourceCounter interrupt;
		IOInterruptEventSourceCounter filter;
		IOTimerEventSourceCounter timer;
		IOCommandGateCounter commandGate;
		IOCommandQueueCounter commandQueue;
	} u;
} IOEventSourceCounter;

typedef struct IOWorkLoopDependency {
	RB_ENTRY(IOWorkLoopDependency) link;
	OSKextLoadTag loadTag;
} IOWorkLoopDependency;

typedef struct IOWorkLoopCounter {
	SLIST_ENTRY(IOWorkLoopCounter) link;
	KextNode *parentKext;
	int attachedEventSources;
	IOWorkLoop *workLoop;
	uint64_t startTimeStamp;
	uint64_t timeOnGate;
	uint32_t closeGateCalls;
	uint32_t openGateCalls;
	typedef RB_HEAD(DependencyTree, IOWorkLoopDependency) DependencyTreeHead;
	DependencyTreeHead dependencyHead;
	static int loadTagCompare(IOWorkLoopDependency *e1, IOWorkLoopDependency *e2);
	RB_PROTOTYPE_SC(static, DependencyTree, IOWorkLoopDependency, dependencyLink, KextTagCompare);
} IOWorkLoopCounter;

typedef struct IOUserClientCounter {
	SLIST_ENTRY(IOUserClientCounter) link;
	ClassNode *parentClass;
	uint32_t clientCalls;
} IOUserClientCounter;

class IOStatistics {
	static bool enabled;

	static IORWLock *lock;

	static uint32_t sequenceID;

	static uint32_t lastKextIndex;
	static uint32_t lastClassIndex;

	static uint32_t loadedKexts;
	static uint32_t registeredClasses;
	static uint32_t registeredCounters;
	static uint32_t registeredWorkloops;

	static uint32_t attachedEventSources;

	static KextNode *kextHint;

	static IOWorkLoopDependency *nextWorkLoopDependency;

	typedef RB_HEAD(KextTree, KextNode) KextTreeHead;
	static KextTreeHead kextHead;
	static int kextNodeCompare(KextNode *e1, KextNode *e2);
	RB_PROTOTYPE_SC(static, KextTree, KextNode, link, kextNodeCompare);

	typedef RB_HEAD(KextAddressTree, KextNode) KextAddressTreeHead;
	static KextAddressTreeHead kextAddressHead;
	static int kextAddressNodeCompare(KextNode *e1, KextNode *e2);
	RB_PROTOTYPE_SC(static, KextAddressTree, KextNode, addressLink, kextAddressNodeCompare);

	typedef RB_HEAD(ClassTree, ClassNode) ClassTreeHead;
	static ClassTreeHead classHead;
	static int classNodeCompare(ClassNode *e1, ClassNode *e2);
	RB_PROTOTYPE_SC(static, ClassTree, ClassNode, tLink, classNodeCompare);

	static int oid_sysctl(__unused struct sysctl_oid *oidp, __unused void *arg1, int arg2, struct sysctl_req *req);

	static uint32_t copyGlobalStatistics(IOStatisticsGlobal *stats);
	static uint32_t copyKextStatistics(IOStatisticsKext *stats);
	static uint32_t copyMemoryStatistics(IOStatisticsMemory *stats);
	static uint32_t copyClassStatistics(IOStatisticsClass *stats);
	static uint32_t copyCounterStatistics(IOStatisticsCounter *stats);
	static uint32_t copyKextIdentifiers(IOStatisticsKextIdentifier *kextIDs);
	static uint32_t copyClassNames(IOStatisticsClassName *classNames);

	static uint32_t copyWorkLoopStatistics(IOStatisticsWorkLoop *workLoopStats);

	static uint32_t copyUserClientStatistics(IOStatisticsUserClientHeader *stats, uint32_t loadTag);

	static void updateAllocationCounter(vm_offset_t address, uint32_t index, vm_size_t size);

	static void storeUserClientCallInfo(IOUserClient *userClient, IOUserClientCounter *counter);

	static KextNode *getKextNodeFromBacktrace(boolean_t write);
	static void releaseKextNode(KextNode *node);

public:

	static void initialize();

	static void onKextLoad(OSKext *kext, kmod_info_t *kmod_info);
	static void onKextUnload(OSKext *kext);
	static void onClassAdded(OSKext *parentKext, OSMetaClass *metaClass);
	static void onClassRemoved(OSKext *parentKext, OSMetaClass *metaClass);

	static IOEventSourceCounter *registerEventSource(OSObject *inOwner);
	static void unregisterEventSource(IOEventSourceCounter *counter);

	static IOWorkLoopCounter *registerWorkLoop(IOWorkLoop *workLoop);
	static void unregisterWorkLoop(IOWorkLoopCounter *counter);

	static IOUserClientCounter *registerUserClient(IOUserClient *userClient);
	static void unregisterUserClient(IOUserClientCounter *counter);

	static int getStatistics(sysctl_req *req);
	static int getWorkLoopStatistics(sysctl_req *req);
	static int getUserClientStatistics(sysctl_req *req);

/* Inlines for counter manipulation.
 *
 * NOTE: counter access is not expressly guarded here so as not to incur performance penalties
 * in the instrumented parent objects. Writes are arranged so as to be protected by pre-existing
 * locks in the parent where appropriate, but reads have no such guarantee. Counters should
 * therefore be regarded as providing an indication of current state, rather than precisely
 * accurate statistics.
 */

	static inline void
	setCounterType(IOEventSourceCounter *counter, IOStatisticsCounterType type)
	{
		if (counter) {
			counter->type = type;
		}
	}

	static inline void
	countOpenGate(IOEventSourceCounter *counter)
	{
		if (counter) {
			counter->timeOnGate += mach_absolute_time() - counter->startTimeStamp;
			counter->openGateCalls++;
		}
	}

	static inline void
	countCloseGate(IOEventSourceCounter *counter)
	{
		if (counter) {
			counter->startTimeStamp = mach_absolute_time();
			counter->closeGateCalls++;
		}
	}

/* Interrupt */
	static inline void
	countInterruptCheckForWork(IOEventSourceCounter *counter)
	{
		if (counter) {
			counter->u.interrupt.checksForWork++;
		}
	}

	static inline void
	countInterrupt(IOEventSourceCounter *counter)
	{
		if (counter) {
			counter->u.interrupt.produced++;
		}
	}

/* CommandQueue */
	static inline void
	countCommandQueueActionCall(IOEventSourceCounter *counter)
	{
		if (counter) {
			counter->u.commandQueue.actionCalls++;
		}
	}

/* CommandGate */
	static inline void
	countCommandGateActionCall(IOEventSourceCounter *counter)
	{
		if (counter) {
			counter->u.commandGate.actionCalls++;
		}
	}

/* Timer */
	static inline void
	countTimerTimeout(IOEventSourceCounter *counter)
	{
		if (counter) {
			counter->u.timer.timeouts++;
		}
	}

/* WorkLoop */
	static void attachWorkLoopEventSource(IOWorkLoopCounter *wlc, IOEventSourceCounter *esc);
	static void detachWorkLoopEventSource(IOWorkLoopCounter *wlc, IOEventSourceCounter *esc);

	static inline void
	countWorkLoopOpenGate(IOWorkLoopCounter *counter)
	{
		if (counter) {
			counter->timeOnGate += mach_absolute_time() - counter->startTimeStamp;
			counter->openGateCalls++;
		}
	}

	static inline void
	countWorkLoopCloseGate(IOWorkLoopCounter *counter)
	{
		if (counter) {
			counter->startTimeStamp = mach_absolute_time();
			counter->closeGateCalls++;
		}
	}

/* IOLib allocations */
	static void countAlloc(uint32_t index, vm_size_t size);

/* UserClient */
	static void countUserClientCall(IOUserClient *client);
};

#else

/* Statistics disabled */

class IOStatistics {
public:
	static void
	initialize()
	{
	}
};

#endif /* IOKITSTATS */

#endif /* __IOKIT_STATISTICS_PRIVATE_H */
