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

#ifndef _IOKIT_STATISTICS_H
#define _IOKIT_STATISTICS_H

#define IOSTATISTICS_SIG            'IOST'
#define IOSTATISTICS_SIG_USERCLIENT 'IOSU'
#define IOSTATISTICS_SIG_WORKLOOP   'IOSW'

/* Update when the binary format changes */
#define IOSTATISTICS_VER  			0x2

enum {
	kIOStatisticsDriverNameLength  = 64,
	kIOStatisticsClassNameLength   = 64,
	kIOStatisticsProcessNameLength = 20
};

enum {
	kIOStatisticsDerivedEventSourceCounter = 0,
	kIOStatisticsTimerEventSourceCounter,
	kIOStatisticsCommandGateCounter,
	kIOStatisticsCommandQueueCounter,
	kIOStatisticsInterruptEventSourceCounter,
	kIOStatisticsFilterInterruptEventSourceCounter
};

typedef uint32_t IOStatisticsCounterType;

enum {
	kIOStatisticsGeneral = 0,
	kIOStatisticsWorkLoop,
	kIOStatisticsUserClient
};

/* Keep our alignments as intended */

#pragma pack(4)

/* Event Counters */ 

typedef struct IOStatisticsInterruptEventSources {
	uint32_t created;
	uint32_t produced;
	uint32_t checksForWork;	
} IOStatisticsInterruptEventSources;

typedef struct IOStatisticsTimerEventSources {
	uint32_t created;
	uint32_t openGateCalls;	
	uint32_t closeGateCalls;
	uint64_t timeOnGate;
	uint32_t timeouts;
	uint32_t checksForWork;
} IOStatisticsTimerEventSources;

typedef struct IOStatisticsDerivedEventSources {
	uint32_t created;
	uint32_t openGateCalls;
	uint32_t closeGateCalls;
	uint64_t timeOnGate;
} IOStatisticsDerivedEventSources;

typedef struct IOStatisticsCommandGates {
	uint32_t created;
	uint32_t openGateCalls;
	uint32_t closeGateCalls;
	uint64_t timeOnGate;
	uint32_t actionCalls;
} IOStatisticsCommandGates;
 
typedef struct IOStatisticsCommandQueues {
	uint32_t created;
	uint32_t actionCalls;
} IOStatisticsCommandQueues;
 
typedef struct IOStatisticsUserClients {
	uint32_t created;
	uint32_t clientCalls;
} IOStatisticsUserClients;

/* General mode */

typedef struct IOStatisticsHeader {
	uint32_t sig; /* 'IOST' */
	uint32_t ver; /* incremented with every data revision */

	uint32_t seq; /* sequence ID */

	uint32_t globalStatsOffset;
	uint32_t kextStatsOffset;
	uint32_t memoryStatsOffset;
	uint32_t classStatsOffset;
	uint32_t counterStatsOffset;
	uint32_t kextIdentifiersOffset;
	uint32_t classNamesOffset;

	/* struct IOStatisticsGlobal */
	/* struct IOStatisticsKext */
	/* struct IOStatisticsMemory */
	/* struct IOStatisticsClass */
	/* struct IOStatisticsCounter */
	/* struct IOStatisticsKextIdentifier */
	/* struct IOStatisticsClassName */
} IOStatisticsHeader;

typedef struct IOStatisticsGlobal {
	uint32_t kextCount;
	uint32_t classCount;
	uint32_t workloops;
} IOStatisticsGlobal;

typedef struct IOStatisticsKext {
	uint32_t loadTag;
	uint32_t loadSize;
	uint32_t wiredSize;
	uint32_t classes; /* Number of classes owned */
	uint32_t classIndexes[]; /* Variable length array of owned class indexes */
} IOStatisticsKext;

typedef struct IOStatisticsMemory {
	uint32_t allocatedSize;
	uint32_t freedSize;
	uint32_t allocatedAlignedSize;
	uint32_t freedAlignedSize;
	uint32_t allocatedContiguousSize;
	uint32_t freedContiguousSize;
	uint32_t allocatedPageableSize;
	uint32_t freedPageableSize;
} IOStatisticsMemory;

typedef struct IOStatisticsClass {
	uint32_t classID;
	uint32_t superClassID;
	uint32_t classSize;
} IOStatisticsClass;

typedef struct IOStatisticsCounter {
	uint32_t classID;
	uint32_t classInstanceCount;
	struct IOStatisticsUserClients userClientStatistics;
	struct IOStatisticsInterruptEventSources interruptEventSourceStatistics;
	struct IOStatisticsInterruptEventSources filterInterruptEventSourceStatistics;
	struct IOStatisticsTimerEventSources timerEventSourceStatistics;
	struct IOStatisticsCommandGates commandGateStatistics;
	struct IOStatisticsCommandQueues commandQueueStatistics;
	struct IOStatisticsDerivedEventSources derivedEventSourceStatistics;
} IOStatisticsCounter;

typedef struct IOStatisticsKextIdentifier {
	char identifier[kIOStatisticsDriverNameLength];
} IOStatisticsKextIdentifier;

typedef struct IOStatisticsClassName {
	char name[kIOStatisticsClassNameLength];
} IOStatisticsClassName;

/* WorkLoop mode */

typedef struct IOStatisticsWorkLoop {
	uint32_t attachedEventSources;
	uint64_t timeOnGate;
	uint32_t kextLoadTag;
	uint32_t dependentKexts;
	uint32_t dependentKextLoadTags[];
} IOStatisticsWorkLoop;

typedef struct IOStatisticsWorkLoopHeader {
	uint32_t sig; /* 'IOSW */
	uint32_t ver; /* incremented with every data revision */
	uint32_t seq; /* sequence ID */
	uint32_t workloopCount;
	struct IOStatisticsWorkLoop workLoopStats;
} IOStatisticsWorkLoopHeader;

/* UserClient mode */

typedef struct IOStatisticsUserClientCall {
	char processName[kIOStatisticsProcessNameLength];
	int32_t pid;
	uint32_t calls;
} IOStatisticsUserClientCall;

typedef struct IOStatisticsUserClientHeader {
	uint32_t sig; /* 'IOSU */
	uint32_t ver; /* incremented with every data revision */	
	uint32_t seq; /* sequence ID */
	uint32_t processes;
	struct IOStatisticsUserClientCall userClientCalls[];
} IOStatisticsUserClientHeader;

#pragma pack()

#endif /* _IOKIT_STATISTICS_H */
