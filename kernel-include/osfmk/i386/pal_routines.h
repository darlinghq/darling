/*
 * Copyright (c) 2009-2012 Apple Inc. All rights reserved.
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
#ifndef _I386_PAL_ROUTINES_H
#define _I386_PAL_ROUTINES_H

#include <stdint.h>
#include <mach/kern_return.h>
#include <mach/mach_types.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* PAL routines exported to kexts */

/*
 * Load registers with these values. In 32-bit mode,
 * only the low-order half is loaded (if applicable)
 */
struct pal_efi_registers {
	uint64_t rcx;
	uint64_t rdx;
	uint64_t r8;
	uint64_t r9;
	uint64_t rax;
};

/*
 * Load registers and stack with these values before
 * executing "call" instruction
 */
kern_return_t
pal_efi_call_in_64bit_mode(uint64_t func,
    struct pal_efi_registers *efi_reg,
    void *stack_contents,
    size_t stack_contents_size,                        /* 16-byte multiple */
    uint64_t *efi_status);

kern_return_t
pal_efi_call_in_32bit_mode(uint32_t func,
    struct pal_efi_registers *efi_reg,
    void *stack_contents,
    size_t stack_contents_size,                        /* 16-byte multiple */
    uint32_t *efi_status);

/* Go into ACPI sleep */

boolean_t pal_machine_sleep(uint8_t type_a,
    uint8_t type_b,
    uint32_t bit_position,
    uint32_t disable_mask,
    uint32_t enable_mask);

/* xnu internal PAL routines */
#ifdef XNU_KERNEL_PRIVATE

/* Define any PAL-specific types for x86 */
typedef uint64_t pal_cr_t;

struct pal_cpu_data; /* Defined per-platform */
struct pal_pcb; /* Defined per-platform */
struct pal_apic_table; /* Defined per-platform */

/* For use by APIC kext */
extern struct pal_apic_table *apic_table;

/* serial / debug output routines */
extern int  pal_serial_init(void);
extern void pal_serial_putc(char);
extern void pal_serial_putc_nocr(char);
extern int  pal_serial_getc(void);

/* Generic I386 PAL functions go here */
extern void pal_i386_init(void);
extern void pal_set_signal_delivery(thread_t);

/* Get values for cr0..4 */
extern void pal_get_control_registers( pal_cr_t *cr0, pal_cr_t *cr2,
    pal_cr_t *cr3, pal_cr_t *cr4 );

/* Debug hook invoked in the page-fault path */
extern void pal_dbg_page_fault( thread_t thread, user_addr_t vadddr,
    kern_return_t kr );

/* Set a task's name in the platform kernel debugger */
extern void pal_dbg_set_task_name( task_t task );

/* wind-back to the start of a system call */
void pal_syscall_restart(thread_t thread, x86_saved_state_t *state);

/* Hook for non-vfork exec */
void pal_execve_return(thread_t thread);

/* Called by thread_terminate_self() */
void pal_thread_terminate_self(thread_t thread);

/* Called by ast_check() */
void pal_ast_check(thread_t thread);

/* Called by sync_iss_to_iks */
extern void pal_get_kern_regs( x86_saved_state_t *state ) __dead2;

/*
 * Platform-specific hlt/sti.
 */
extern void pal_hlt(void);
extern void pal_sti(void);
extern void pal_cli(void);

/*
 * Mark in-memory thread register cache state validity.
 */
typedef enum { DIRTY, VALID } pal_cache_state_t;
void pal_register_cache_state(thread_t thread, pal_cache_state_t state);


/* Catch code running on the except thread that shouldn't be */
void pal_preemption_assert(void);

/* Include a PAL-specific header, too, for xnu-internal overrides */
#include <i386/pal_native.h>


extern boolean_t virtualized;
#define PAL_VIRTUALIZED_PROPERTY_VALUE 4

/* Allow for tricky IOKit property matching */
#define PAL_AICPM_PROPERTY_NAME "intel_cpupm_matching"
static inline void
pal_get_resource_property(const char **property_name, int *property_value)
{
	*property_name = PAL_AICPM_PROPERTY_NAME;
	*property_value = PAL_AICPM_PROPERTY_VALUE;
	if (virtualized) {
		*property_value = PAL_VIRTUALIZED_PROPERTY_VALUE;
	}
}

/* assembly function to update TSC / timebase info */
extern void _pal_rtc_nanotime_store(
	uint64_t                tsc,
	uint64_t                nsec,
	uint32_t                scale,
	uint32_t                shift,
	struct pal_rtc_nanotime *dst);

/* global nanotime info */
extern struct pal_rtc_nanotime pal_rtc_nanotime_info;

#endif /* XNU_KERNEL_PRIVATE */

#if defined(__cplusplus)
}
#endif

#endif /* _I386_PAL_ROUTINES_H */
