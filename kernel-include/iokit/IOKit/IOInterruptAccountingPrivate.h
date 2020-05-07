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

#ifndef __IOKIT_IOINTERRUPTACCOUNTING_PRIVATE_H
#define __IOKIT_IOINTERRUPTACCOUNTING_PRIVATE_H

/*
 * Header containing interrupt accounting related prototypes/defines that should be kept private to
 * xnu itself (no userspace, no kexts, no nothing!).
 */

#include <stdint.h>
#include <IOKit/IOInterruptAccounting.h>
#include <kern/queue.h>

class OSObject;
class IOSimpleReporter;

/*
 * A brief overview.  Interrupt accounting (as implemented in IOKit) pertains to infrastructure for
 * gathering information (currently, statistics only) on interrupts, and allowing them to be reported
 * (either to userspace through IOReporting, or through lldb; lldb macros have yet to be implemented).
 *
 * Currently, interrupt accounting consists of of a relationship between an IOService (a nub, which
 * will contain interrupt specifiers), an IOInterruptEventSource (if we add other interrupt target
 * abstractions, support could be added for them as well), and objects necessary to support them.  An
 * interrupt is "named" by a tuple of {provider, interrupt index}; no nub should ever have more than
 * one interrupt registered for a given index, so this tuple should be unique.
 *
 * The "additional objects" mentioned above consist of an IOReporter object (lazily allocated and
 * tied to the nub; once allocated it will live until the nub is freed), and a statistics object
 * (effectively part of the IOIES in terms of lifecycle).  The statistics object is used by the
 * interrupt codepath itself, and by the nub when it needs to update the reporter; the reporter is
 * used to report values to userspace.
 *
 * As a consequence of the above relationship, we do not track statistics for directly registered
 * interrupt handlers.  We have no guarantees what the handler or the target may be; if you don't
 * follow the generic IOKit interrupt model, you will not be tracked by interrupt accounting.  For
 * now, this means you must use an IOIES to be eligible for interrupt accounting.  We also do not
 * track IOIES' that do not have providers (this is indicative that it is only being used to drive
 * workloop activity, and is not actually handling interrupts).
 */

/*
 * This is meant to let us set up the set of interrupt statistics we are actually interested in, by
 * setting a boot-arg.  If we want to track a statistic, the bit corresponding to the index for that
 * statistic should be set in the bitmask.
 *
 * There is a bit of a mismatch here, in that our IOReporting channel namespace allows for 256 statistics,
 * but this bitmask actually limits it to 32.
 */
extern uint32_t gInterruptAccountingStatisticBitmask;

/*
 * Check the bitmask by statistic index; useful for setting the initial value and conditionalizing code.
 */
#define IA_GET_ENABLE_BIT(statisticIndex) \
    (((uint32_t) 1) << ((uint32_t) statisticIndex))

#define IA_GET_STATISTIC_ENABLED(statisticIndex) \
    (IA_GET_ENABLE_BIT(statisticIndex) & gInterruptAccountingStatisticBitmask)

/*
 * Check if any valid statistics are enabled.
 */
#define IA_ANY_STATISTICS_ENABLED \
    ((IA_GET_ENABLE_BIT(kInterruptAccountingInvalidStatisticIndex) - 1) & gInterruptAccountingStatisticBitmask)

/*
 * Actual string names for the statistics we gather.
 */
#define kInterruptAccountingChannelNameFirstLevelCount       ("               First Level Interrupt Handler Count")
#define kInterruptAccountingChannelNameSecondLevelCount      ("              Second Level Interrupt Handler Count")
#define kInterruptAccountingChannelNameFirstLevelTime        ("        First Level Interrupt Handler Time (MATUs)")
#define kInterruptAccountingChannelNameSecondLevelCPUTime    ("   Second Level Interrupt Handler CPU Time (MATUs)")
#define kInterruptAccountingChannelNameSecondLevelSystemTime ("Second Level Interrupt Handler System Time (MATUs)")
#define kInterruptAccountingChannelNameNoThreadWakeups       ("      Interrupts that did not try to wake a thread")
#define kInterruptAccountingChannelNameTotalThreadWakeups    ("       Sleeping threads woken up by this interrupt")
#define kInterruptAccountingChannelNamePackageWakeups        ("          Package wakeups caused by this interrupt")
#define kInterruptAccountingChannelNameCPUWakeups            ("              CPU wakeups caused by this interrupt")
#define kInterruptAccountingChannelNameIdleExits             ("               Idle exits caused by this interrupt")

static const char * const kInterruptAccountingStatisticNameArray[IA_NUM_INTERRUPT_ACCOUNTING_STATISTICS] = {
	[kInterruptAccountingFirstLevelCountIndex] = kInterruptAccountingChannelNameFirstLevelCount,
	[kInterruptAccountingSecondLevelCountIndex] = kInterruptAccountingChannelNameSecondLevelCount,
	[kInterruptAccountingFirstLevelTimeIndex] = kInterruptAccountingChannelNameFirstLevelTime,
	[kInterruptAccountingSecondLevelCPUTimeIndex] = kInterruptAccountingChannelNameSecondLevelCPUTime,
	[kInterruptAccountingSecondLevelSystemTimeIndex] = kInterruptAccountingChannelNameSecondLevelSystemTime,
	[kInterruptAccountingNoThreadWakeupsIndex] = kInterruptAccountingChannelNameNoThreadWakeups,
	[kInterruptAccountingTotalThreadWakeupsIndex] = kInterruptAccountingChannelNameTotalThreadWakeups,
	[kInterruptAccountingPackageWakeupsIndex] = kInterruptAccountingChannelNamePackageWakeups,
	[kInterruptAccountingCPUWakeupsIndex] = kInterruptAccountingChannelNameCPUWakeups,
	[kInterruptAccountingIdleExitsIndex] = kInterruptAccountingChannelNameIdleExits,
};

/*
 * For updating the statistics in the data structure.  We cannot guarantee all of our platforms will be
 * able to do a 64-bit store in a single transaction.  So, for new platforms, call out to the hardware
 * atomic add routine; it will either be unsupported, or do the right thing.  For architectures or
 * platforms that do support it; just do regular assignment.
 *
 * We use this routine instead of a lock because at the moment, there is no way (in the interrupt context)
 * to reconcile a lock (even a spinlock) with the IOReporting synchonization (as we have no guarantee that
 * IOReporting will not block on a mutex, which would result in a panic if it held a spinlock).  This
 * means that reported values may have a disparity if we update the reporter values while an interrupt is
 * being handled.
 *
 * Atomic modification should not be strictly required, as a given interrupt should not be dispatched to
 * two processors at once (and the interrupt should serve to force out stores), and the second level
 * handler should be synchonized by the work loop it runs on.
 */
#if __x86_64__ || __arm64__
#define IA_ADD_VALUE(target, value) \
    (*(target) += (value))
#else /* !(__x86_64__ || __arm64__) */
#define IA_ADD_VALUE(target, value) \
    (OSAddAtomic64((value), (target)))
#endif /* !(__x86_64__ || __arm64__) */

/*
 * TODO: Should this be an OSObject?  Or properly pull in its methods as member functions?
 */
struct IOInterruptAccountingData {
	OSObject * owner; /* The owner of the statistics; currently always an IOIES or a subclass of it */
	queue_chain_t chain;
	/*
	 * We have no guarantee that the owner will not temporarily mutate its index value (i.e, in setWorkLoop
	 * for IOIES).  To ensure we can properly recalculate our own identity (and our channel IDs for the
	 * reporter), stash the index we set up the reporter with here.
	 *
	 * Note that we should never remap the interrupt (point it to a different specifier).  The mutation of
	 * the index value is usually to negate it; I am uncertain of the reason for this at the moment.  The
	 * practical impact being that we should never need to update the stashed index value; it should stay
	 * valid for the lifetime of the owner.
	 */
	int interruptIndex;

	bool enablePrimaryTimestamp;
	volatile uint64_t primaryTimestamp __attribute__((aligned(8)));

	/*
	 * As long as we are based on the simple reporter, all our channels will be 64 bits.  Align the data
	 * to allow for safe atomic updates (we don't want to cross a cache line on any platform, but for some
	 * it would cause a panic).
	 */
	volatile uint64_t interruptStatistics[IA_NUM_INTERRUPT_ACCOUNTING_STATISTICS] __attribute__((aligned(8)));
};

/*
 * Initializes global values/structures related to interrupt accounting.
 */
void interruptAccountingInit(void);

/*
 * Routines for adding and removing objects from the global queue of IOInterruptAccountingData objects;
 * the queue exists as a debugging aid (no entities other than these routines should care about the
 * queue at runtime).
 */
void interruptAccountingDataAddToList(IOInterruptAccountingData * data);
void interruptAccountingDataRemoveFromList(IOInterruptAccountingData * data);

/*
 * Updates reporter with the statistics contained within data.  Invoked when IOReporting has been asked
 * for updated statistics; requiring explicit synchronization of data between the statistic fields and
 * the reporter helps keep interrupt accounting overhead down.
 */
void interruptAccountingDataUpdateChannels(IOInterruptAccountingData * data, IOSimpleReporter * reporter);

/*
 * Initializes the statistics in data using the statistics currently held by reporter.  Typically invoked
 * when data is first associated with reporter.  The nub that an interrupt is associated with will be
 * longer lived than the interrupt; as a result, our owner may not be the first to register for a
 * particular interrupt index with that nub, so we need to inherit the existing statistics (as we describe
 * statistics in terms of {nub id, index}, not in terms of our owner).
 */
void interruptAccountingDataInheritChannels(IOInterruptAccountingData * data, IOSimpleReporter * reporter);

#endif /* __IOKIT_IOINTERRUPTACCOUNTING_PRIVATE_H */
