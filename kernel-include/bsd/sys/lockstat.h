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
#define _SYS_LOCKSTAT_H

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * Name the various locking functions...
 */
#define LS_LCK_MTX_LOCK                 "lck_mtx_lock"
#define LS_LCK_MTX_SPIN_LOCK            "lck_mtx_spin_lock"
#define LS_LCK_MTX_UNLOCK               "lck_mtx_unlock"
#define LS_LCK_MTX_TRY_LOCK             "lck_mtx_try_lock"
#define LS_LCK_MTX_TRY_SPIN_LOCK        "lck_mtx_try_spin_lock"
#define LS_LCK_MTX_EXT_LOCK             "lck_mtx_ext_lock"
#define LS_LCK_MTX_EXT_UNLOCK           "lck_mtx_ext_unlock"
#define LS_LCK_MTX_EXT_TRY_LOCK         "lck_mtx_ext_try_lock"
#define LS_LCK_MTX_LOCK_SPIN_LOCK       "lck_mtx_lock_spin"

#define LS_LCK_SPIN_LOCK                "lck_spin_lock"
#define LS_LCK_SPIN_TRY_LOCK            "lck_spin_try_lock"
#define LS_LCK_SPIN_UNLOCK              "lck_spin_unlock"
#define LS_LCK_RW_LOCK_SHARED           "lck_rw_lock_shared"
#define LS_LCK_RW_LOCK_EXCL             "lck_rw_lock_exclusive"
#define LS_LCK_RW_DONE                  "lck_rw_done"
#define LS_LCK_RW_TRY_LOCK_EXCL         "lck_rw_try_lock_exclusive"
#define LS_LCK_RW_TRY_LOCK_SHARED       "lck_rw_try_lock_shared"
#define LS_LCK_RW_LOCK_SHARED_TO_EXCL   "lck_rw_shared_to_exclusive"
#define LS_LCK_RW_LOCK_EXCL_TO_SHARED   "lck_rw_exclusive_to_shared"

#define LS_ACQUIRE                      "acquire"
#define LS_RELEASE                      "release"
#define LS_SPIN                         "spin"
#define LS_BLOCK                        "block"
#define LS_UPGRADE                      "upgrade"
#define LS_DOWNGRADE                    "downgrade"

#define LS_TYPE_ADAPTIVE                "adaptive"
#define LS_TYPE_SPIN                    "spin"
#define LS_TYPE_ILK                     "interlock"     /* OS X only */
#define LS_TYPE_THREAD                  "thread"        /* Solaris only */
#define LS_TYPE_RW                      "rw"
#define LS_TYPE_RWUPGRADE               "rwupgrade"     /* OS X only */

#define LSA_ACQUIRE                     (LS_TYPE_ADAPTIVE "-" LS_ACQUIRE)
#define LSA_RELEASE                     (LS_TYPE_ADAPTIVE "-" LS_RELEASE)
#define LSA_SPIN                        (LS_TYPE_ADAPTIVE "-" LS_SPIN)
#define LSA_BLOCK                       (LS_TYPE_ADAPTIVE "-" LS_BLOCK)
#define LSA_ILK_SPIN                    (LS_TYPE_ILK "-" LS_SPIN)
#define LSS_ACQUIRE                     (LS_TYPE_SPIN "-" LS_ACQUIRE)
#define LSS_RELEASE                     (LS_TYPE_SPIN "-" LS_RELEASE)
#define LSS_SPIN                        (LS_TYPE_SPIN "-" LS_SPIN)
#define LSR_ACQUIRE                     (LS_TYPE_RW "-" LS_ACQUIRE)
#define LSR_RELEASE                     (LS_TYPE_RW "-" LS_RELEASE)
#define LSR_BLOCK                       (LS_TYPE_RW "-" LS_BLOCK)
#define LSR_SPIN                        (LS_TYPE_RW "-" LS_SPIN)
#define LSR_UPGRADE                     (LS_TYPE_RW "-" LS_UPGRADE)
#define LSR_UPGRADE_BLOCK               (LS_TYPE_RWUPGRADE "-" LS_BLOCK)
#define LSR_DOWNGRADE                   (LS_TYPE_RW "-" LS_DOWNGRADE)
#define LST_SPIN                        (LS_TYPE_THREAD "-" LS_SPIN)

#ifdef  __cplusplus
}
#endif

#endif  /* _SYS_LOCKSTAT_H */
