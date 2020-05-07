/*
 * Copyright (c) 2017-2019 Apple Inc. All rights reserved.
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

#ifndef SYS_MONOTONIC_H
#define SYS_MONOTONIC_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

struct mt_cpu_inscyc {
	uint64_t mtci_instructions;
	uint64_t mtci_cycles;
};

__END_DECLS

#if !MACH_KERNEL_PRIVATE

#include <sys/ioccom.h>

__BEGIN_DECLS

/*
 * XXX These declarations are subject to change at any time.
 */

#define MT_IOC(x) _IO('m', (x))
#define MT_IOC_RESET MT_IOC(0)
#define MT_IOC_ADD MT_IOC(1)
#define MT_IOC_ENABLE MT_IOC(2)
#define MT_IOC_COUNTS MT_IOC(3)
#define MT_IOC_GET_INFO MT_IOC(4)

__END_DECLS

#endif /* !MACH_KERNEL_PRIVATE */

__BEGIN_DECLS

struct monotonic_config {
	uint64_t event;
	uint64_t allowed_ctr_mask;
	uint64_t cpu_mask;
};

union monotonic_ctl_add {
	struct {
		struct monotonic_config config;
	} in;

	struct {
		uint32_t ctr;
	} out;
};

union monotonic_ctl_enable {
	struct {
		bool enable;
	} in;
};


union monotonic_ctl_counts {
	struct {
		uint64_t ctr_mask;
	} in;

	struct {
		uint64_t counts[1];
	} out;
};


union monotonic_ctl_info {
	struct {
		unsigned int nmonitors;
		unsigned int ncounters;
	} out;
};

__END_DECLS

#if XNU_KERNEL_PRIVATE

#if MONOTONIC

#include <kern/monotonic.h>
#include <machine/monotonic.h>
#include <sys/kdebug.h>
#include <kern/locks.h>

__BEGIN_DECLS

#ifdef MT_CORE_INSTRS
#define COUNTS_INSTRS __counts[MT_CORE_INSTRS]
#else /* defined(MT_CORE_INSTRS) */
#define COUNTS_INSTRS 0
#endif /* !defined(MT_CORE_INSTRS) */

/*
 * MT_KDBG_TMP* macros are meant for temporary (i.e. not checked-in)
 * performance investigations.
 */

/*
 * Record the current CPU counters.
 *
 * Preemption must be disabled.
 */
#define MT_KDBG_TMPCPU_EVT(CODE) \
	KDBG_EVENTID(DBG_MONOTONIC, DBG_MT_TMPCPU, CODE)

#define MT_KDBG_TMPCPU_(CODE, FUNC) \
	do { \
	        if (kdebug_enable && \
	                        kdebug_debugid_enabled(MT_KDBG_TMPCPU_EVT(CODE))) { \
	                uint64_t __counts[MT_CORE_NFIXED]; \
	                mt_fixed_counts(__counts); \
	                KDBG(MT_KDBG_TMPCPU_EVT(CODE) | (FUNC), COUNTS_INSTRS, \
	                                __counts[MT_CORE_CYCLES]); \
	        } \
	} while (0)

#define MT_KDBG_TMPCPU(CODE) MT_KDBG_TMPCPU_(CODE, DBG_FUNC_NONE)
#define MT_KDBG_TMPCPU_START(CODE) MT_KDBG_TMPCPU_(CODE, DBG_FUNC_START)
#define MT_KDBG_TMPCPU_END(CODE) MT_KDBG_TMPCPU_(CODE, DBG_FUNC_END)

/*
 * Record the current thread counters.
 *
 * Interrupts must be disabled.
 */
#define MT_KDBG_TMPTH_EVT(CODE) \
	KDBG_EVENTID(DBG_MONOTONIC, DBG_MT_TMPTH, CODE)

#define MT_KDBG_TMPTH_(CODE, FUNC) \
	do { \
	        if (kdebug_enable && \
	                        kdebug_debugid_enabled(MT_KDBG_TMPTH_EVT(CODE))) { \
	                uint64_t __counts[MT_CORE_NFIXED]; \
	                mt_cur_thread_fixed_counts(__counts); \
	                KDBG(MT_KDBG_TMPTH_EVT(CODE) | (FUNC), COUNTS_INSTRS, \
	                                __counts[MT_CORE_CYCLES]); \
	        } \
	} while (0)

#define MT_KDBG_TMPTH(CODE) MT_KDBG_TMPTH_(CODE, DBG_FUNC_NONE)
#define MT_KDBG_TMPTH_START(CODE) MT_KDBG_TMPTH_(CODE, DBG_FUNC_START)
#define MT_KDBG_TMPTH_END(CODE) MT_KDBG_TMPTH_(CODE, DBG_FUNC_END)

extern lck_grp_t * mt_lock_grp;

int mt_dev_init(void);

struct mt_device {
	const char *mtd_name;
	int(*const mtd_init)(struct mt_device *dev);
	int(*const mtd_add)(struct monotonic_config *config, uint32_t *ctr_out);
	void(*const mtd_reset)(void);
	void(*const mtd_enable)(bool enable);
	int(*const mtd_read)(uint64_t ctr_mask, uint64_t *counts_out);
	decl_lck_mtx_data(, mtd_lock);

	uint8_t mtd_nmonitors;
	uint8_t mtd_ncounters;
	bool mtd_inuse;
};
typedef struct mt_device *mt_device_t;

extern struct mt_device mt_devices[];

__END_DECLS

#endif /* MONOTONIC */

#endif /* XNU_KERNEL_PRIVATE */

#endif /* !defined(SYS_MONOTONIC_H) */
