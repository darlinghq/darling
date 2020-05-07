/*
 * Copyright (c) 2006-2010 Apple Inc. All rights reserved.
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
#ifdef KERNEL_PRIVATE
#ifndef _I386_PMCPU_H_
#define _I386_PMCPU_H_

#include <i386/cpu_topology.h>

#ifndef ASSEMBLER

/*
 * This value should be changed each time that pmDispatch_t or pmCallBacks_t
 * changes.
 */
#define PM_DISPATCH_VERSION     102

/*
 * Dispatch table for functions that get installed when the power
 * management KEXT loads.
 *
 * pmDispatch_t is the set of functions that the kernel can use to call
 * into the power management KEXT.
 *
 * pmCallBacks_t is the set of functions that the power management kext
 * can call to get at specific kernel functions.
 */
typedef struct{
	kern_return_t       (*pmCPUStateInit)(void);
	void                (*cstateInit)(void);
	uint64_t            (*MachineIdle)(uint64_t maxIdleDuration);
	uint64_t            (*GetDeadline)(x86_lcpu_t *lcpu);
	uint64_t            (*SetDeadline)(x86_lcpu_t *lcpu, uint64_t);
	void                (*Deadline)(x86_lcpu_t *lcpu);
	boolean_t           (*exitIdle)(x86_lcpu_t *lcpu);
	void                (*markCPURunning)(x86_lcpu_t *lcpu);
	int                 (*pmCPUControl)(uint32_t cmd, void *datap);
	void                (*pmCPUHalt)(void);
	uint64_t            (*getMaxSnoop)(void);
	void                (*setMaxBusDelay)(uint64_t time);
	uint64_t            (*getMaxBusDelay)(void);
	void                (*setMaxIntDelay)(uint64_t time);
	uint64_t            (*getMaxIntDelay)(void);
	void                (*pmCPUSafeMode)(x86_lcpu_t *lcpu, uint32_t flags);
	void                (*pmTimerStateSave)(void);
	void                (*pmTimerStateRestore)(void);
	kern_return_t       (*exitHalt)(x86_lcpu_t *lcpu);
	kern_return_t       (*exitHaltToOff)(x86_lcpu_t *lcpu);
	void                (*markAllCPUsOff)(void);
	void                (*pmSetRunCount)(uint32_t count);
	boolean_t           (*pmIsCPUUnAvailable)(x86_lcpu_t *lcpu);
	int                 (*pmChooseCPU)(int startCPU, int endCPU, int preferredCPU);
	int                 (*pmIPIHandler)(void *state);
	void                (*pmThreadTellUrgency)(int urgency, uint64_t rt_period, uint64_t rt_deadline);
	void                (*pmActiveRTThreads)(boolean_t active);
	boolean_t           (*pmInterruptPrewakeApplicable)(void);
	void                (*pmThreadGoingOffCore)(thread_t old_thread, boolean_t transfer_load,
	    uint64_t last_dispatch, boolean_t thread_runnable);
} pmDispatch_t;

/* common time fields exported to PM code. This structure may be
 * allocated on the stack, so avoid making it unnecessarily large.
 */
typedef struct pm_rtc_nanotime {
	uint64_t        tsc_base;               /* timestamp */
	uint64_t        ns_base;                /* nanoseconds */
	uint32_t        scale;                  /* tsc -> nanosec multiplier */
	uint32_t        shift;                  /* tsc -> nanosec shift/div */
	uint32_t        generation;             /* 0 == being updated */
} pm_rtc_nanotime_t;

typedef struct {
	uint64_t            (*setRTCPop)(uint64_t time);
	void                (*resyncDeadlines)(int cpu);
	void                (*initComplete)(void);
	x86_lcpu_t          *(*GetLCPU)(int cpu);
	x86_core_t          *(*GetCore)(int cpu);
	x86_die_t           *(*GetDie)(int cpu);
	x86_pkg_t           *(*GetPackage)(int cpu);
	x86_lcpu_t          *(*GetMyLCPU)(void);
	x86_core_t          *(*GetMyCore)(void);
	x86_die_t           *(*GetMyDie)(void);
	x86_pkg_t           *(*GetMyPackage)(void);
	x86_pkg_t           *(*GetPkgRoot)(void);
	void                (*LockCPUTopology)(int lock);
	boolean_t           (*GetHibernate)(int cpu);
	processor_t         (*LCPUtoProcessor)(int lcpu);
	processor_t         (*ThreadBind)(processor_t proc);
	uint32_t            (*GetSavedRunCount)(void);
	void                (*pmSendIPI)(int cpu);
	void                (*GetNanotimeInfo)(pm_rtc_nanotime_t *);
	int                 (*ThreadGetUrgency)(uint64_t *rt_period, uint64_t *rt_deadline);
	uint32_t            (*timerQueueMigrate)(int cpu);
	void                (*RTCClockAdjust)(uint64_t adjustment);
	x86_topology_parameters_t   *topoParms;
	boolean_t           (*InterruptPending)(void);
	boolean_t           (*IsInterrupting)(uint8_t vector);
	void                (*InterruptStats)(uint64_t intrs[256]);
	void                (*DisableApicTimer)(void);
} pmCallBacks_t;

extern pmDispatch_t     *pmDispatch;

void power_management_init(void);
void pmKextRegister(uint32_t version, pmDispatch_t *cpuFuncs,
    pmCallBacks_t *callbacks);
void pmUnRegister(pmDispatch_t *cpuFuncs);
void pmCPUStateInit(void);
uint64_t pmCPUGetDeadline(struct cpu_data *cpu);
uint64_t pmCPUSetDeadline(struct cpu_data *cpu, uint64_t deadline);
void pmCPUDeadline(struct cpu_data *cpu);
boolean_t pmCPUExitIdle(struct cpu_data *cpu);
void pmCPUMarkRunning(struct cpu_data *cpu);
void pmMarkAllCPUsOff(void);
int pmCPUControl(uint32_t cmd, void *datap);
void pmCPUHalt(uint32_t reason);
void pmTimerSave(void);
void pmTimerRestore(void);
kern_return_t pmCPUExitHalt(int cpu);
kern_return_t pmCPUExitHaltToOff(int cpu);
uint32_t pmTimerQueueMigrate(int);

#define PM_HALT_NORMAL          0               /* normal halt path */
#define PM_HALT_DEBUG           1               /* debug code wants to halt */
#define PM_HALT_PANIC           2               /* panic code wants to halt */
#define PM_HALT_SLEEP           3               /* sleep code wants to halt */

void pmSafeMode(x86_lcpu_t *lcpu, uint32_t flags);

#define PM_SAFE_FL_NORMAL       0x00000001      /* put CPU into "normal" power mode */
#define PM_SAFE_FL_SAFE         0x00000002      /* put CPU into a "safe" power mode */
#define PM_SAFE_FL_PAUSE        0x00000010      /* pause execution on the CPU */
#define PM_SAFE_FL_RESUME       0x00000020      /* resume execution on the CPU */

extern int pmsafe_debug;
/* Default urgency timing threshold for the DEBUG build */
#define         URGENCY_NOTIFICATION_ASSERT_NS (5 * 1000 * 1000)
extern uint64_t urgency_notification_assert_abstime_threshold;

x86_lcpu_t *
pmGetLogicalCPU(int cpu);
x86_lcpu_t *
pmGetMyLogicalCPU(void);
processor_t
pmLCPUtoProcessor(int lcpu);
x86_pkg_t *
pmGetPkgRoot(void);


/******************************************************************************
*
* All of the following are deprecated interfaces and no longer used.
*
******************************************************************************/


#endif /* ASSEMBLER */

#endif /* _I386_PMCPU_H_ */
#endif /* KERNEL_PRIVATE */
