/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 1997-2003 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _SYS_LOCKSTAT_H
#define	_SYS_LOCKSTAT_H
#endif

/* #pragma ident	"@(#)lockstat.h	1.6	05/06/08 SMI" */

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Spin locks - we have less variants
 */
#define	LS_LCK_SPIN_LOCK_ACQUIRE	0
#define	LS_LCK_SPIN_LOCK_SPIN		1
#define	LS_LCK_SPIN_UNLOCK_RELEASE	2
/*
 * Mutexes can also have interlock-spin events, which are
 * unique to our lock implementation.
 */
#define	LS_LCK_MTX_LOCK_ACQUIRE			3
#define	LS_LCK_MTX_LOCK_BLOCK			5
#define	LS_LCK_MTX_LOCK_SPIN			6
#define	LS_LCK_MTX_LOCK_ILK_SPIN		7
#define	LS_LCK_MTX_TRY_LOCK_ACQUIRE		8
#define	LS_LCK_MTX_TRY_SPIN_LOCK_ACQUIRE	9
#define	LS_LCK_MTX_UNLOCK_RELEASE		10

#define LS_LCK_MTX_LOCK_SPIN_ACQUIRE		39
/*
 * Provide a parallel set for indirect mutexes
 */
#define	LS_LCK_MTX_EXT_LOCK_ACQUIRE			17
#define	LS_LCK_MTX_EXT_LOCK_BLOCK			18
#define	LS_LCK_MTX_EXT_LOCK_SPIN			19
#define	LS_LCK_MTX_EXT_LOCK_ILK_SPIN		20
#define	LS_LCK_MTX_TRY_EXT_LOCK_ACQUIRE		21
#define	LS_LCK_MTX_EXT_UNLOCK_RELEASE		22
/*
 * Our reader-writer locks support a blocking upgrade primitive, as
 * well as the possibility of spinning on the interlock.
 */
#define	LS_LCK_RW_LOCK_SHARED_ACQUIRE		23
#define	LS_LCK_RW_LOCK_SHARED_BLOCK		24
#define	LS_LCK_RW_LOCK_SHARED_SPIN		25

#define	LS_LCK_RW_LOCK_EXCL_ACQUIRE		26
#define	LS_LCK_RW_LOCK_EXCL_BLOCK		27
#define	LS_LCK_RW_LOCK_EXCL_SPIN		28

#define	LS_LCK_RW_DONE_RELEASE			29

#define	LS_LCK_RW_TRY_LOCK_SHARED_ACQUIRE	30
#define	LS_LCK_RW_TRY_LOCK_SHARED_SPIN		31

#define	LS_LCK_RW_TRY_LOCK_EXCL_ACQUIRE		32
#define	LS_LCK_RW_TRY_LOCK_EXCL_ILK_SPIN	33

#define	LS_LCK_RW_LOCK_SHARED_TO_EXCL_UPGRADE	34
#define	LS_LCK_RW_LOCK_SHARED_TO_EXCL_SPIN	35
#define	LS_LCK_RW_LOCK_SHARED_TO_EXCL_BLOCK	36

#define	LS_LCK_RW_LOCK_EXCL_TO_SHARED_DOWNGRADE	37
#define	LS_LCK_RW_LOCK_EXCL_TO_SHARED_ILK_SPIN	38

#define	LS_NPROBES			40
#define LS_LCK_INVALID			LS_NPROBES

/*
 * Name the various locking functions...
 */
#define	LS_LCK_MTX_LOCK			"lck_mtx_lock"
#define	LS_LCK_MTX_SPIN_LOCK		"lck_mtx_spin_lock"
#define	LS_LCK_MTX_UNLOCK		"lck_mtx_unlock"
#define	LS_LCK_MTX_TRY_LOCK		"lck_mtx_try_lock"
#define	LS_LCK_MTX_TRY_SPIN_LOCK	"lck_mtx_try_spin_lock"
#define	LS_LCK_MTX_EXT_LOCK		"lck_mtx_ext_lock"
#define	LS_LCK_MTX_EXT_UNLOCK		"lck_mtx_ext_unlock"
#define	LS_LCK_MTX_EXT_TRY_LOCK		"lck_mtx_ext_try_lock"
#define	LS_LCK_MTX_LOCK_SPIN_LOCK	"lck_mtx_lock_spin"

#define	LS_LCK_SPIN_LOCK		"lck_spin_lock"
#define	LS_LCK_SPIN_TRY_LOCK		"lck_spin_try_lock"
#define	LS_LCK_SPIN_UNLOCK		"lck_spin_unlock"
#define	LS_LCK_RW_LOCK_SHARED		"lck_rw_lock_shared"
#define	LS_LCK_RW_LOCK_EXCL		"lck_rw_lock_exclusive"
#define	LS_LCK_RW_DONE			"lck_rw_done"
#define	LS_LCK_RW_TRY_LOCK_EXCL		"lck_rw_try_lock_exclusive"
#define	LS_LCK_RW_TRY_LOCK_SHARED	"lck_rw_try_lock_shared"
#define	LS_LCK_RW_LOCK_SHARED_TO_EXCL	"lck_rw_shared_to_exclusive"
#define	LS_LCK_RW_LOCK_EXCL_TO_SHARED	"lck_rw_exclusive_to_shared"

#define	LS_ACQUIRE			"acquire"
#define	LS_RELEASE			"release"
#define	LS_SPIN				"spin"
#define	LS_BLOCK			"block"
#define	LS_UPGRADE			"upgrade"
#define	LS_DOWNGRADE			"downgrade"

#define	LS_TYPE_ADAPTIVE		"adaptive"
#define	LS_TYPE_SPIN			"spin"
#define	LS_TYPE_ILK			"interlock"	/* OS X only */
#define	LS_TYPE_THREAD			"thread"	/* Solaris only */
#define	LS_TYPE_RW			"rw"
#define	LS_TYPE_RWUPGRADE		"rwupgrade"	/* OS X only */

#define	LSA_ACQUIRE			(LS_TYPE_ADAPTIVE "-" LS_ACQUIRE)
#define	LSA_RELEASE			(LS_TYPE_ADAPTIVE "-" LS_RELEASE)
#define	LSA_SPIN			(LS_TYPE_ADAPTIVE "-" LS_SPIN)
#define	LSA_BLOCK			(LS_TYPE_ADAPTIVE "-" LS_BLOCK)
#define	LSA_ILK_SPIN			(LS_TYPE_ILK "-" LS_SPIN)
#define	LSS_ACQUIRE			(LS_TYPE_SPIN "-" LS_ACQUIRE)
#define	LSS_RELEASE			(LS_TYPE_SPIN "-" LS_RELEASE)
#define	LSS_SPIN			(LS_TYPE_SPIN "-" LS_SPIN)
#define	LSR_ACQUIRE			(LS_TYPE_RW "-" LS_ACQUIRE)
#define	LSR_RELEASE			(LS_TYPE_RW "-" LS_RELEASE)
#define	LSR_BLOCK			(LS_TYPE_RW "-" LS_BLOCK)
#define	LSR_SPIN			(LS_TYPE_RW "-" LS_SPIN)
#define	LSR_UPGRADE			(LS_TYPE_RW "-" LS_UPGRADE)
#define	LSR_UPGRADE_BLOCK		(LS_TYPE_RWUPGRADE "-" LS_BLOCK)
#define	LSR_DOWNGRADE			(LS_TYPE_RW "-" LS_DOWNGRADE)
#define	LST_SPIN			(LS_TYPE_THREAD "-" LS_SPIN)

#ifndef _ASM

#include <stdint.h>
#ifdef KERNEL

#ifndef _KERNEL
#define _KERNEL /* Solaris vs. Darwin */
#endif

/*
 * Platform-independent kernel support for the lockstat driver.
 */
#if defined(NEED_DTRACE_DEFS)
typedef uint32_t dtrace_id_t;		/* probe identifier - also in dtrace.h! */
typedef uint64_t	u_longlong_t;	/* also in dtrace.h! */
#endif

extern dtrace_id_t lockstat_probemap[LS_NPROBES];
extern void (*lockstat_probe)(dtrace_id_t, uint64_t, uint64_t,
    uint64_t, uint64_t, uint64_t);



#ifdef _KERNEL

#if	CONFIG_DTRACE

extern void (lockstat_probe_wrapper)(int, uintptr_t, int);
	
/*
 * Macros to record lockstat probes.
 */
#define	LOCKSTAT_RECORD4(probe, lp, arg0, arg1, arg2, arg3)		\
	{								\
		dtrace_id_t id;						\
		if ((id = lockstat_probemap[(probe)])) {		\
			(*lockstat_probe)(id, (uintptr_t)(lp), (arg0),	\
			    (arg1), (arg2), (arg3));			\
		}							\
	}

#define	LOCKSTAT_RECORD2(probe, lp, arg1, arg2)	\
	LOCKSTAT_RECORD4(probe, lp, arg1, arg2, 0, 0)

#define	LOCKSTAT_RECORD(probe, lp, arg)	\
	LOCKSTAT_RECORD4(probe, lp, arg, 0, 0, 0)

#define	LOCKSTAT_RECORD0(probe, lp)	\
	LOCKSTAT_RECORD4(probe, lp, 0, 0, 0, 0)
#else
	/* No Lockstat provider */

#define LOCKSTAT_RECORD()
#define	LOCKSTAT_RECORD0()
#define	LOCKSTAT_RECORD2()
#define	LOCKSTAT_RECORD4()

#endif /* !CONFIG_DTRACE */

#endif /* _KERNEL */

#endif /* _ASM */

#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_LOCKSTAT_H */

