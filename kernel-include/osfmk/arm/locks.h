/*
 * Copyright (c) 2007-2017 Apple Inc. All rights reserved.
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

#ifndef _ARM_LOCKS_H_
#define _ARM_LOCKS_H_

#include <kern/kern_types.h>
#ifdef  MACH_KERNEL_PRIVATE
#include <arm/hw_lock_types.h>
#endif


#ifdef  MACH_KERNEL_PRIVATE

extern  unsigned int    LcksOpts;

#define enaLkDeb                0x00000001      /* Request debug in default attribute */
#define enaLkStat               0x00000002      /* Request statistic in default attribute */
#define disLkRWPrio             0x00000004      /* Disable RW lock priority promotion */
#define enaLkTimeStat           0x00000008      /* Request time statistics in default attribute */

#define disLkType               0x80000000      /* Disable type checking */
#define disLktypeb              0
#define disLkThread             0x40000000      /* Disable ownership checking */
#define disLkThreadb    1
#define enaLkExtStck    0x20000000      /* Enable extended backtrace */
#define enaLkExtStckb   2
#define disLkMyLck              0x10000000      /* Disable recursive lock dectection */
#define disLkMyLckb             3

#endif

#ifdef  MACH_KERNEL_PRIVATE
typedef struct {
	struct hslock   hwlock;
	uintptr_t               type;
} lck_spin_t;

#define lck_spin_data hwlock.lock_data

#define LCK_SPIN_TAG_DESTROYED  0xdead  /* lock marked as Destroyed */

#define LCK_SPIN_TYPE                   0x00000011

#else
#ifdef  KERNEL_PRIVATE

typedef struct {
	uintptr_t               opaque[2];
} lck_spin_t;

#else
typedef struct __lck_spin_t__   lck_spin_t;
#endif  // KERNEL_PRIVATE
#endif  // MACH_KERNEL_PRIVATE

#ifdef  MACH_KERNEL_PRIVATE
typedef struct _lck_mtx_ {
	union {
		uintptr_t                                       lck_mtx_data;   /* Thread pointer plus lock bits */
		uintptr_t                                       lck_mtx_tag;    /* Tag for type */
	};                                                                                              /* arm: 4   arm64: 8 */
	union {
		struct {
			uint16_t                                lck_mtx_waiters;/* Number of waiters */
			uint8_t                                 lck_mtx_pri;    /* unused */
			uint8_t                                 lck_mtx_type;   /* Type */
		};
		struct {
			struct _lck_mtx_ext_    *lck_mtx_ptr;   /* Indirect pointer */
		};
	};                                                                                              /* arm: 4   arm64: 8 */
} lck_mtx_t;                                                                            /* arm: 8  arm64: 16 */

/* Shared between mutex and read-write locks */
#define LCK_ILOCK_BIT           0
#define ARM_LCK_WAITERS_BIT     1
#define LCK_ILOCK                       (1 << LCK_ILOCK_BIT)
#define ARM_LCK_WAITERS         (1 << ARM_LCK_WAITERS_BIT)

#define LCK_MTX_TYPE                                    0x22            /* lock type */

#define LCK_MTX_TAG_INDIRECT                    0x00001007      /* lock marked as Indirect  */
#define LCK_MTX_TAG_DESTROYED                   0x00002007      /* lock marked as Destroyed */

#define LCK_FRAMES_MAX  8

extern uint64_t         MutexSpin;

typedef struct {
	unsigned int            type;
	vm_offset_t                     stack[LCK_FRAMES_MAX];
	vm_offset_t                     thread;
} lck_mtx_deb_t;

#define MUTEX_TAG       0x4d4d

typedef struct {
	unsigned int            lck_mtx_stat_data;
} lck_mtx_stat_t;

typedef struct _lck_mtx_ext_ {
	lck_mtx_t                       lck_mtx;        /* arm: 12  arm64: 24 */
	struct _lck_grp_        *lck_mtx_grp;   /* arm: 4   arm64: 8 */
	unsigned int            lck_mtx_attr;   /* arm: 4   arm64: 4 */
	lck_mtx_stat_t          lck_mtx_stat;   /* arm: 4   arm64: 4 */
	lck_mtx_deb_t           lck_mtx_deb;    /* arm: 40  arm64: 80 */
} lck_mtx_ext_t;                        /* arm: 64  arm64: 120 */

#define LCK_MTX_ATTR_DEBUG      0x1
#define LCK_MTX_ATTR_DEBUGb     31
#define LCK_MTX_ATTR_STAT       0x2
#define LCK_MTX_ATTR_STATb      30

#define LCK_MTX_EVENT(lck)        ((event_t)(((unsigned int*)(lck))+((sizeof(lck_mtx_t)-1)/sizeof(unsigned int))))
#define LCK_EVENT_TO_MUTEX(event) ((lck_mtx_t *)(uintptr_t)(((unsigned int *)(event)) - ((sizeof(lck_mtx_t)-1)/sizeof(unsigned int))))

#else
#ifdef  KERNEL_PRIVATE
typedef struct {
	uintptr_t        opaque[2];
} lck_mtx_t;

typedef struct {
#if defined(__arm64__)
	unsigned long       opaque[16];
#else /* __arm__ */
	unsigned int            opaque[16];
#endif
} lck_mtx_ext_t;

#else
typedef struct __lck_mtx_t__    lck_mtx_t;
#endif
#endif

#ifdef  MACH_KERNEL_PRIVATE

typedef union {
	struct {
		uint16_t        shared_count;           /* No. of shared granted request */
		uint16_t        interlock:              1,      /* Interlock */
		    priv_excl:              1,                          /* priority for Writer */
		    want_upgrade:   1,                          /* Read-to-write upgrade waiting */
		    want_excl:              1,                          /* Writer is waiting, or locked for write */
		    r_waiting:              1,                          /* Someone is sleeping on lock */
		    w_waiting:              1,                          /* Writer is sleeping on lock */
		    can_sleep:              1,                          /* Can attempts to lock go to sleep? */
		    _pad2:                  8,                          /* padding */
		    tag_valid:              1;                          /* Field is actually a tag, not a bitfield */
#if __arm64__
		uint32_t        _pad4;
#endif
	};
	struct {
		uint32_t        data;                                           /* Single word version of bitfields and shared count */
#if __arm64__
		uint32_t        lck_rw_pad4;
#endif
	};
} lck_rw_word_t;

typedef struct {
	lck_rw_word_t   word;
	thread_t                lck_rw_owner;
} lck_rw_t;     /* arm: 8  arm64: 16 */

#define lck_rw_shared_count     word.shared_count
#define lck_rw_interlock        word.interlock
#define lck_rw_priv_excl        word.priv_excl
#define lck_rw_want_upgrade     word.want_upgrade
#define lck_rw_want_excl        word.want_excl
#define lck_r_waiting           word.r_waiting
#define lck_w_waiting           word.w_waiting
#define lck_rw_can_sleep        word.can_sleep
#define lck_rw_data                     word.data
// tag and data reference the same memory. When the tag_valid bit is set,
// the data word should be treated as a tag instead of a bitfield.
#define lck_rw_tag_valid        word.tag_valid
#define lck_rw_tag                      word.data

#define LCK_RW_SHARED_READER_OFFSET      0
#define LCK_RW_INTERLOCK_BIT            16
#define LCK_RW_PRIV_EXCL_BIT            17
#define LCK_RW_WANT_UPGRADE_BIT         18
#define LCK_RW_WANT_EXCL_BIT            19
#define LCK_RW_R_WAITING_BIT            20
#define LCK_RW_W_WAITING_BIT            21
#define LCK_RW_CAN_SLEEP_BIT            22
//									23-30
#define LCK_RW_TAG_VALID_BIT            31

#define LCK_RW_INTERLOCK                (1U << LCK_RW_INTERLOCK_BIT)
#define LCK_RW_R_WAITING                (1U << LCK_RW_R_WAITING_BIT)
#define LCK_RW_W_WAITING                (1U << LCK_RW_W_WAITING_BIT)
#define LCK_RW_WANT_UPGRADE             (1U << LCK_RW_WANT_UPGRADE_BIT)
#define LCK_RW_WANT_EXCL                (1U << LCK_RW_WANT_EXCL_BIT)
#define LCK_RW_TAG_VALID                (1U << LCK_RW_TAG_VALID_BIT)
#define LCK_RW_PRIV_EXCL                (1U << LCK_RW_PRIV_EXCL_BIT)
#define LCK_RW_SHARED_MASK              (0xffff << LCK_RW_SHARED_READER_OFFSET)
#define LCK_RW_SHARED_READER    (0x1 << LCK_RW_SHARED_READER_OFFSET)

#define LCK_RW_TAG_DESTROYED            ((LCK_RW_TAG_VALID | 0xdddddeadu))      /* lock marked as Destroyed */

#define LCK_RW_WRITER_EVENT(lck)                (event_t)((uintptr_t)(lck)+1)
#define LCK_RW_READER_EVENT(lck)                (event_t)((uintptr_t)(lck)+2)
#define WRITE_EVENT_TO_RWLOCK(event)    ((lck_rw_t *)((uintptr_t)(event)-1))
#define READ_EVENT_TO_RWLOCK(event)             ((lck_rw_t *)((uintptr_t)(event)-2))

#if __ARM_ENABLE_WFE_

#define wait_for_event()        __builtin_arm_wfe()
#if __arm__
#define set_event()                     do{__builtin_arm_dsb(DSB_ISHST);__builtin_arm_sev();}while(0)
#define LOCK_SNOOP_SPINS        4
#else
#define set_event()                     do{}while(0)    // arm64 sev is implicit in stlxr
#define LOCK_SNOOP_SPINS        0x300
#endif

#else

#define wait_for_event()        __builtin_arm_clrex()
#define set_event()             do{}while(0)
#define LOCK_SNOOP_SPINS        0x300

#endif // __ARM_ENABLE_WFE_

#if LOCK_PRIVATE

#define LOCK_PANIC_TIMEOUT      0xc00000        // 12.5 m ticks ~= 524ms with 24MHz OSC

#define PLATFORM_LCK_ILOCK LCK_ILOCK

#if defined(__ARM_ARCH_8_2__)
#define __ARM_ATOMICS_8_1       1       // ARMv8.1 atomic instructions are available
#endif

/*
 * Lock state to thread pointer
 * Clear the bottom bits
 */
#define LCK_MTX_STATE_TO_THREAD(s)      (thread_t)(s & ~(LCK_ILOCK | ARM_LCK_WAITERS))
/*
 * Thread pointer to lock state
 * arm thread pointers are aligned such that the bottom two bits are clear
 */
#define LCK_MTX_THREAD_TO_STATE(t)      ((uintptr_t)t)
/*
 * Thread pointer mask
 */
#define LCK_MTX_THREAD_MASK (~(uintptr_t)(LCK_ILOCK | ARM_LCK_WAITERS))

#define disable_preemption_for_thread(t) os_atomic_store(&(t->machine.preemption_count), t->machine.preemption_count + 1, compiler_acq_rel)
#define preemption_disabled_for_thread(t) (t->machine.preemption_count > 0)


__unused static void
disable_interrupts_noread(void)
{
#if __arm__
	__asm__ volatile ("cpsid if" ::: "memory"); // Mask IRQ FIQ
#else
	__builtin_arm_wsr64("DAIFSet", (DAIFSC_IRQF | DAIFSC_FIQF));    // Mask IRQ FIQ
#endif
}

__unused static inline long
get_interrupts(void)
{
	long    state;

#if __arm__
	__asm__ volatile ("mrs %[state], cpsr" :[state] "=r" (state));  // Read cpsr
#else
	state = __builtin_arm_rsr64("DAIF");    // Read interrupt state
#endif
	return state;
}

__unused static inline long
disable_interrupts(void)
{
	long    state;

	state = get_interrupts();               // Get previous state
	disable_interrupts_noread();    // Disable
	return state;
}

__unused static inline void
restore_interrupts(long state)
{
#if __arm__
	__asm__ volatile ("msr  cpsr, %[state]" :: [state] "r" (state) : "cc", "memory"); // Restore CPSR
#elif __arm64__
	__builtin_arm_wsr64("DAIF", state);     // Restore masks
#endif
}

#endif // LOCK_PRIVATE

#else
#ifdef  KERNEL_PRIVATE
typedef struct {
	uintptr_t                    opaque[2];
} lck_rw_t;
#else
typedef struct __lck_rw_t__     lck_rw_t;
#endif
#endif

#endif  /* _ARM_LOCKS_H_ */
