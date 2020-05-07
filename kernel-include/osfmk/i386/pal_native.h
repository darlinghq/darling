/*
 * Copyright (c) 2009-2016 Apple Inc. All rights reserved.
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
#ifndef _I386_PAL_I386_H
#define _I386_PAL_I386_H

#ifdef XNU_KERNEL_PRIVATE

/* No-op on bare-metal */
#define pal_dbg_page_fault(x, y, z)
#define pal_dbg_set_task_name( x )
#define pal_set_signal_delivery( x )

#define pal_is_usable_memory(b, t)      (TRUE)

#define pal_hlt()                       __asm__ volatile ("sti; hlt")
#define pal_sti()                       __asm__ volatile ("sti")
#define pal_cli()                       __asm__ volatile ("cli")

static inline void
pal_stop_cpu(boolean_t cli)
{
	if (cli) {
		__asm__ volatile ( "cli");
	}
	__asm__ volatile ( "wbinvd; hlt");
}

#define pal_register_cache_state(t, v)

#define pal_execve_return(t)
#define pal_thread_terminate_self(t)
#define pal_ast_check(t)

#define panic_display_pal_info() do { } while(0)
#define pal_kernel_announce() do { } while(0)

#define PAL_AICPM_PROPERTY_VALUE 0

#define pal_pmc_swi() __asm__ __volatile__("int %0"::"i"(LAPIC_PMC_SWI_VECTOR):"memory")

/* Macro used by non-native xnus for access to low globals when it may
 * have moved.
 */
#define PAL_KDP_ADDR(x) (x)

struct pal_rtc_nanotime {
	volatile uint64_t       tsc_base;       /* timestamp */
	volatile uint64_t       ns_base;        /* nanoseconds */
	uint32_t                scale;          /* tsc -> nanosec multiplier */
	uint32_t                shift;          /* shift is nonzero only on "slow" machines, */
	                                        /* ie where tscFreq <= SLOW_TSC_THRESHOLD */
	volatile uint32_t       generation;     /* 0 == being updated */
	uint32_t                spare1;
};


#ifdef MACH_KERNEL_PRIVATE

struct pal_cpu_data {
};

struct pal_pcb {
};

struct pal_apic_table {
};

#endif /* MACH_KERNEL_PRIVATE */

#endif /* XNU_KERNEL_PRIVATE */

#endif /* _I386_PAL_I386_H */
