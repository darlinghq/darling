/*
 * Copyright (c) 2014 Apple Computer, Inc. All rights reserved.
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

#ifndef __IOKIT_IOINTERRUPTACCOUNTING_H
#define __IOKIT_IOINTERRUPTACCOUNTING_H

/*
 * This header contains definitions that will be needed by userspace clients of the interrupt accounting
 * mechanisms.
 */

#define IA_INDEX_MAX (255)
#define IA_INDEX_MASK (0xFFULL)
#define IA_STATISTIC_INDEX_SHIFT (0ULL)
#define IA_INTERRUPT_INDEX_SHIFT (16ULL)

/*
 * For the moment, the indices aren't preprocessor visible, so any changes to this code will need to be
 * careful to ensure that the defined number of statistics matches (or is greater than) the actual number
 * of statistics, to avoid channel ID collisions... because that would be bad.
 */
#define IA_NUM_INTERRUPT_ACCOUNTING_STATISTICS (10)

/*
 * Channel ID related definitions.  These serve to denote the namespace of interrupt accounting in the
 * context of IOReporter-based clients.  Interrupt accounting distinguishes between interrupts based on
 * the nub the interrupt was registered relative to, and the nub relative interrupt index (i.e, the
 * namespace for interrupt accounting is {nub ID, interrupt index}).  IOReporting already knows about
 * nubs (in the context of IOService), however it has no built in knowledge of interrupt accounting.
 * As a result of this, it is the responsibility of the IOReporting client to request any desired
 * statistics on a per-index basis (i.e, if you want to get the first level interrupt count for all
 * interrupts, you must express an interest in the first level interrupt count statistic for index 0, 1,
 * 2, and so on, to a reasonable maximum).
 */

/*
 * These delimit the channel ID namespace for interrupt accounting.
 */
#define IA_BASE_CHANNEL_ID IOREPORT_MAKEID('I', 'n', 't', 'r', 0, 0, 0, 0) /* First valid channel ID */
#define IA_MAX_CHANNEL_ID IOREPORT_MAKEID('I', 'n', 't', 'r', 0xFF, 0xFF, 0xFF, 0xFF) /* Last valid channel ID */

/*
 * Given a nub-relative interrupt index (an index into the interrupt specifiers), and the index of a
 * interrupt accounting statistic, returns the channel id for that statistic.
 */
#define IA_GET_CHANNEL_ID(interruptIndex, statisticIndex) \
    ((IA_BASE_CHANNEL_ID) + \
    ((interruptIndex % IA_INDEX_MASK) << IA_INTERRUPT_INDEX_SHIFT) + \
    ((statisticIndex % IA_INDEX_MASK) << IA_STATISTIC_INDEX_SHIFT))

/*
 * Extracts the interrupt index, given a channel ID.
 */
#define IA_GET_INTERRUPT_INDEX(channelID) \
    (((channelID - IA_BASE_CHANNEL_ID) >> IA_INTERRUPT_INDEX_SHIFT) % IA_INDEX_MASK)

/*
 * Extracts the statistic index, given a channel ID.
 */
#define IA_GET_STATISTIC_INDEX(channelID) \
    (((channelID - IA_BASE_CHANNEL_ID) >> IA_STATISTIC_INDEX_SHIFT) % IA_INDEX_MASK)

/*
 * This enum defines the basic statistics we gather for each interrupt.  Currently, the information
 * we gather falls into roughly three buckets: interrupt related (counts, times), scheduler related
 * (thread wakeups), and power related (package/cpu state changes).
 *
 * First Level Count: This represents the number of times that this interrupt has fired (more
 *   specifically, the number of times we have run the handler we expect to run in interrupt context).
 *
 * Second Level Count: This represents the number of times we have run any second level handler for
 *   this interrupt (more specifically, the handler we expect to be run in the regular kernel context).
 *
 * First Level Time: This represents the aggregate time spent running the first level handler.  For
 *   some interrupts, this value will be 0, as this is not meant to track time spent running generic
 *   IOKit code for the interrupt (i.e, IOInterruptEventSource code), but instead any custom code run
 *   at the interrupt context (i.e, the filter installed for an IOFilterInterruptEventSource).
 *
 * Second Level CPU Time: This represents the aggregate time spent actually running the second level
 *   handler on the CPU.  As the second level handler may block or be preempted, it is meaningful to
 *   distinguish this from the system time spent handling the interrupt.  As was the case for the
 *   first level handler, this does not attempt to track the time spent running generic IOKit code
 *   (i.e, IOInterruptEventSource code or IOWorkLoop code), but instead attempts to track the time
 *   spent running the handler that was installed.
 *
 * Second Level System Time: This represents the aggregate time spent in the second level handler;
 *   this will include time where the handler was blocked or had been preempted.  This should equate
 *   to the wall time spent handling an interrupt (as long as we don't allow the system to go to
 *   sleep while a second level handler is running).
 *
 * No Thread Wakeups: The number of times that the interrupt did not attempt to wake up a thread
 *   (typically the work loop for the interrupt source is woken up).
 *
 * Total Thread Wakeups: The aggregate number of threads (non-unique) woken up by the interrupt.
 *   If no threads were actually woken up for an interrupt (i.e, the work loop thread was already
 *   runnable), this value will not change.
 *
 * Package Wakeups: The number of times that this interrupt woke up the package (as defined by the
 *   scheduler).
 *
 * CPU Wakeups: The number of times that this interrupt woke up a CPU (forcing it to go through the
 *   reset path).
 *
 * Idle Exits: The number of times that this interrupt forced a CPU out of the idle loop (the CPU
 *   had to exit an idle state to handle the interrupt, but it did not need to go through the reset
 *   path).
 */
enum {
    kInterruptAccountingFirstLevelCountIndex = 0, /* Number of times we invoked the top level handler */
    kInterruptAccountingSecondLevelCountIndex, /* Number of times we invoked the workloop action */
    kInterruptAccountingFirstLevelTimeIndex, /* Time spent in the top level handler, if one was installed */
    kInterruptAccountingSecondLevelCPUTimeIndex, /* CPU time spent in the workloop action */
    kInterruptAccountingSecondLevelSystemTimeIndex, /* System time spent in the workloop action */
    kInterruptAccountingNoThreadWakeupsIndex, /* Number of first level (filter) invocations that did not wake up a thread */
    kInterruptAccountingTotalThreadWakeupsIndex, /* Number of actual thread wakeups caused by this interrupt */
    kInterruptAccountingPackageWakeupsIndex, /* Number of times this interrupt woke up the package */
    kInterruptAccountingCPUWakeupsIndex, /* Number of times this interrupt woke up a CPU */
    kInterruptAccountingIdleExitsIndex, /* Number of times this interrupt forced a CPU out of the idle loop */
    kInterruptAccountingInvalidStatisticIndex /* Sentinel value for checking for a nonsensical index */
};

/*
 * IOReporting group name; exposed publicly for the purpose of getting channels by group
 * name; other strings (subgroup names, statistic names) are not exposed, as we may want
 * to change them in the future.
 */
#define kInterruptAccountingGroupName "Interrupt Statistics (by index)"

#endif /* __IOKIT_IOINTERRUPTACCOUNTING_PRIVATE_H */

