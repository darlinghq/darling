/*
 * Copyright (c) 2004-2012 Apple Inc. All rights reserved.
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

#ifndef _I386_LOCKS_H_
#define _I386_LOCKS_H_

#include <sys/appleapiopts.h>
#include <kern/kern_types.h>
#include <kern/assert.h>

#ifdef  MACH_KERNEL_PRIVATE

#include <i386/hw_lock_types.h>

extern  unsigned int    LcksOpts;
#if DEVELOPMENT || DEBUG
extern  unsigned int    LckDisablePreemptCheck;
#endif

#define enaLkDeb                0x00000001      /* Request debug in default attribute */
#define enaLkStat               0x00000002      /* Request statistic in default attribute */
#define disLkRWPrio             0x00000004      /* Disable RW lock priority promotion */
#define enaLkTimeStat           0x00000008      /* Request time statistics in default attribute */

#endif /* MACH_KERNEL_PRIVATE */

#if     defined(MACH_KERNEL_PRIVATE)
typedef struct {
	volatile uintptr_t      interlock;
#if     MACH_LDEBUG
	unsigned long   lck_spin_pad[9];        /* XXX - usimple_lock_data_t */
#endif
} lck_spin_t;

#define LCK_SPIN_TAG_DESTROYED          0x00002007      /* lock marked as Destroyed */

#else /* MACH_KERNEL_PRIVATE */
#ifdef  KERNEL_PRIVATE
typedef struct {
	unsigned long    opaque[10];
} lck_spin_t;
#else /* KERNEL_PRIVATE */
typedef struct __lck_spin_t__   lck_spin_t;
#endif
#endif

#ifdef  MACH_KERNEL_PRIVATE
/* The definition of this structure, including the layout of the
 * state bitfield, is tailored to the asm implementation in i386_lock.s
 */
typedef struct _lck_mtx_ {
	union {
		struct {
			volatile uintptr_t              lck_mtx_owner;
			union {
				struct {
					volatile uint32_t
					    lck_mtx_waiters:16,
					    lck_mtx_pri:8, // unused
					    lck_mtx_ilocked:1,
					    lck_mtx_mlocked:1,
					    lck_mtx_promoted:1, // unused
					    lck_mtx_spin:1,
					    lck_mtx_is_ext:1,
					    lck_mtx_pad3:3;
				};
				uint32_t        lck_mtx_state;
			};
			/* Pad field used as a canary, initialized to ~0 */
			uint32_t                        lck_mtx_pad32;
		};
		struct {
			struct _lck_mtx_ext_            *lck_mtx_ptr;
			uint32_t                        lck_mtx_tag;
			uint32_t                        lck_mtx_pad32_2;
		};
	};
} lck_mtx_t;

#define LCK_MTX_WAITERS_MSK             0x0000ffff
#define LCK_MTX_WAITER                  0x00000001
#define LCK_MTX_PRIORITY_MSK            0x00ff0000
#define LCK_MTX_ILOCKED_MSK             0x01000000
#define LCK_MTX_MLOCKED_MSK             0x02000000
#define LCK_MTX_SPIN_MSK                0x08000000

/* This pattern must subsume the interlocked, mlocked and spin bits */
#define LCK_MTX_TAG_INDIRECT                    0x07ff1007      /* lock marked as Indirect  */
#define LCK_MTX_TAG_DESTROYED                   0x07fe2007      /* lock marked as Destroyed */

/* Adaptive spin before blocking */
extern uint64_t         MutexSpin;

typedef enum lck_mtx_spinwait_ret_type {
	LCK_MTX_SPINWAIT_ACQUIRED = 0,
	LCK_MTX_SPINWAIT_SPUN = 1,
	LCK_MTX_SPINWAIT_NO_SPIN = 2,
} lck_mtx_spinwait_ret_type_t;

extern lck_mtx_spinwait_ret_type_t              lck_mtx_lock_spinwait_x86(lck_mtx_t *mutex);
struct turnstile;
extern void                                     lck_mtx_lock_wait_x86(lck_mtx_t *mutex, struct turnstile **ts);
extern void                                     lck_mtx_lock_acquire_x86(lck_mtx_t *mutex);

extern void                                     lck_mtx_lock_slow(lck_mtx_t *lock);
extern boolean_t                                lck_mtx_try_lock_slow(lck_mtx_t *lock);
extern void                                     lck_mtx_unlock_slow(lck_mtx_t *lock);
extern void                                     lck_mtx_lock_spin_slow(lck_mtx_t *lock);
extern boolean_t                                lck_mtx_try_lock_spin_slow(lck_mtx_t *lock);
extern void                                     hw_lock_byte_init(volatile uint8_t *lock_byte);
extern void                                     hw_lock_byte_lock(volatile uint8_t *lock_byte);
extern void                                     hw_lock_byte_unlock(volatile uint8_t *lock_byte);

typedef struct {
	unsigned int            type;
	unsigned int            pad4;
	vm_offset_t             pc;
	vm_offset_t             thread;
} lck_mtx_deb_t;

#define MUTEX_TAG       0x4d4d

typedef struct {
	unsigned int            lck_mtx_stat_data;
} lck_mtx_stat_t;

typedef struct _lck_mtx_ext_ {
	lck_mtx_t               lck_mtx;
	struct _lck_grp_        *lck_mtx_grp;
	unsigned int            lck_mtx_attr;
	unsigned int            lck_mtx_pad1;
	lck_mtx_deb_t           lck_mtx_deb;
	uint64_t                lck_mtx_stat;
	unsigned int            lck_mtx_pad2[2];
} lck_mtx_ext_t;

#define LCK_MTX_ATTR_DEBUG      0x1
#define LCK_MTX_ATTR_DEBUGb     0
#define LCK_MTX_ATTR_STAT       0x2
#define LCK_MTX_ATTR_STATb      1

#define LCK_MTX_EVENT(lck)        ((event_t)(((unsigned int*)(lck))+(sizeof(lck_mtx_t)-1)/sizeof(unsigned int)))
#define LCK_EVENT_TO_MUTEX(event) ((lck_mtx_t *)(uintptr_t)(((unsigned int *)(event)) - ((sizeof(lck_mtx_t)-1)/sizeof(unsigned int))))

#else /* MACH_KERNEL_PRIVATE */
#ifdef  XNU_KERNEL_PRIVATE
typedef struct {
	unsigned long           opaque[2];
} lck_mtx_t;

typedef struct {
	unsigned long           opaque[10];
} lck_mtx_ext_t;
#else
#ifdef  KERNEL_PRIVATE
typedef struct {
	unsigned long           opaque[2];
} lck_mtx_t;

typedef struct {
	unsigned long           opaque[10];
} lck_mtx_ext_t;

#else
typedef struct __lck_mtx_t__            lck_mtx_t;
typedef struct __lck_mtx_ext_t__        lck_mtx_ext_t;
#endif
#endif
#endif

#ifdef  MACH_KERNEL_PRIVATE
typedef union _lck_rw_t_internal_ {
	struct {
		volatile uint16_t       lck_rw_shared_count;    /* No. of accepted readers */
		volatile uint8_t        lck_rw_interlock;       /* Interlock byte */
		volatile uint8_t
		    lck_rw_priv_excl:1,                         /* Writers prioritized if set */
		    lck_rw_want_upgrade:1,                      /* Read-to-write upgrade waiting */
		    lck_rw_want_write:1,                        /* Writer waiting or locked for write */
		    lck_r_waiting:1,                            /* Reader is sleeping on lock */
		    lck_w_waiting:1,                            /* Writer is sleeping on lock */
		    lck_rw_can_sleep:1,                         /* Can attempts to lock go to sleep? */
		    lck_rw_padb6:2;                             /* padding */
		uint32_t                lck_rw_tag;             /* This can be obsoleted when stats are in */
		thread_t                lck_rw_owner;           /* Unused */
	};
	struct {
		uint32_t                data;                   /* Single word for count, ilk, and bitfields */
		uint32_t                lck_rw_pad4;
		uint32_t                lck_rw_pad8;
		uint32_t                lck_rw_pad12;
	};
} lck_rw_t;
#define LCK_RW_T_SIZE           16

static_assert(sizeof(lck_rw_t) == LCK_RW_T_SIZE);

#define LCK_RW_SHARED_SHIFT      0
#define LCK_RW_INTERLOCK_BIT    16
#define LCK_RW_PRIV_EXCL_BIT    24
#define LCK_RW_WANT_UPGRADE_BIT 25
#define LCK_RW_WANT_EXCL_BIT    26
#define LCK_RW_R_WAITING_BIT    27
#define LCK_RW_W_WAITING_BIT    28
#define LCK_RW_CAN_SLEEP_BIT    29

#define LCK_RW_INTERLOCK        (1 << LCK_RW_INTERLOCK_BIT)
#define LCK_RW_WANT_UPGRADE     (1 << LCK_RW_WANT_UPGRADE_BIT)
#define LCK_RW_WANT_EXCL        (1 << LCK_RW_WANT_EXCL_BIT)
#define LCK_RW_R_WAITING        (1 << LCK_RW_R_WAITING_BIT)
#define LCK_RW_W_WAITING        (1 << LCK_RW_W_WAITING_BIT)
#define LCK_RW_PRIV_EXCL        (1 << LCK_RW_PRIV_EXCL_BIT)
#define LCK_RW_TAG_VALID        (1 << LCK_RW_TAG_VALID_BIT)
#define LCK_RW_SHARED_MASK      (0xffff << LCK_RW_SHARED_SHIFT)
#define LCK_RW_SHARED_READER    (1 << LCK_RW_SHARED_SHIFT)

#define LCK_RW_WANT_WRITE       LCK_RW_WANT_EXCL


#define LCK_RW_ATTR_DEBUG       0x1
#define LCK_RW_ATTR_DEBUGb      0
#define LCK_RW_ATTR_STAT        0x2
#define LCK_RW_ATTR_STATb       1
#define LCK_RW_ATTR_READ_PRI    0x3
#define LCK_RW_ATTR_READ_PRIb   2
#define LCK_RW_ATTR_DIS_THREAD  0x40000000
#define LCK_RW_ATTR_DIS_THREADb 30
#define LCK_RW_ATTR_DIS_MYLOCK  0x10000000
#define LCK_RW_ATTR_DIS_MYLOCKb 28

#define LCK_RW_TAG_DESTROYED            0x00002007      /* lock marked as Destroyed */

#define RW_LOCK_READER_EVENT(x)   ((event_t) (((unsigned char*) (x)) + (offsetof(lck_rw_t, lck_rw_tag))))
#define RW_LOCK_WRITER_EVENT(x)   ((event_t) (((unsigned char*) (x)) + (offsetof(lck_rw_t, lck_rw_pad8))))
#define READ_EVENT_TO_RWLOCK(x)   ((lck_rw_t *)(((unsigned char*)(x) - (offsetof(lck_rw_t, lck_rw_tag)))))
#define WRITE_EVENT_TO_RWLOCK(x)  ((lck_rw_t *)(((unsigned char*)(x) - (offsetof(lck_rw_t, lck_rw_pad8)))))

#if LOCK_PRIVATE

#define disable_preemption_for_thread(t) ((cpu_data_t GS_RELATIVE *)0UL)->cpu_preemption_level++
#define preemption_disabled_for_thread(t) (((cpu_data_t GS_RELATIVE *)0UL)->cpu_preemption_level > 0)

#define LCK_MTX_THREAD_TO_STATE(t)      ((uintptr_t)t)
#define PLATFORM_LCK_ILOCK              0

#define LOCK_SNOOP_SPINS        1000
#define LOCK_PRETEST            1

/* hw_lock_lock static panic deadline, in timebase units. hw_lock_to() uses
 * LockTimeoutTSC computed at startup
 */
#define LOCK_PANIC_TIMEOUT      0xf000000  /* 251e6 TSC ticks */

#endif  // LOCK_PRIVATE

#else
#ifdef  KERNEL_PRIVATE
#pragma pack(1)
typedef struct {
	uint32_t                opaque[3];
	uint32_t                opaque4;
} lck_rw_t;
#pragma pack()
#else
typedef struct __lck_rw_t__     lck_rw_t;
#endif
#endif

#ifdef MACH_KERNEL_PRIVATE

extern void             kernel_preempt_check(void);

#endif /* MACH_KERNEL_PRIVATE */
#endif  /* _I386_LOCKS_H_ */
