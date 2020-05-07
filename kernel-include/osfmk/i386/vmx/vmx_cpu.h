/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef _I386_VMX_CPU_H_
#define _I386_VMX_CPU_H_

#include <mach/machine/vm_types.h>
#include <mach/boolean.h>
#include <i386/vmx/vmx_asm.h>

/*
 * Physical CPU's VMX specifications
 *
 */
typedef struct vmx_specs {
	boolean_t       initialized;    /* the specs have already been read */
	boolean_t       vmx_present;    /* VMX feature available and enabled */
	boolean_t       vmx_on;                 /* VMX is active */
	uint32_t        vmcs_id;                /* VMCS revision identifier */
	/*
	 * Fixed control register bits are specified by a pair of
	 * bitfields: 0-settings contain 0 bits corresponding to
	 * CR bits that may be 0; 1-settings contain 1 bits
	 * corresponding to CR bits that may be 1.
	 */
	uint32_t        cr0_fixed_0;    /* allowed 0-settings for CR0 */
	uint32_t        cr0_fixed_1;    /* allowed 1-settings for CR0 */

	uint32_t        cr4_fixed_0;    /* allowed 0-settings for CR4 */
	uint32_t        cr4_fixed_1;    /* allowed 1-settings for CR4 */
} vmx_specs_t;

typedef struct vmx_cpu {
	vmx_specs_t     specs;          /* this phys CPU's VMX specifications */
	void            *vmxon_region;  /* the logical address of the VMXON region page */
} vmx_cpu_t;

void vmx_init(void);
void vmx_cpu_init(void);
void vmx_resume(boolean_t is_wake_from_hibernate);
void vmx_suspend(void);

#define VMX_BASIC_TRUE_CTLS                                     (1ull << 55)
#define VMX_TRUE_PROCBASED_SECONDARY_CTLS       (1ull << 31)
#define VMX_PROCBASED_CTLS2_EPT                         (1ull << 1)
#define VMX_PROCBASED_CTLS2_UNRESTRICTED        (1ull << 7)

#define VMX_CAP(msr, shift, mask) (rdmsr64(msr) & ((mask) << (shift)))

boolean_t vmx_hv_support(void);

/*
 *	__vmxoff -- Leave VMX Operation
 *
 */
extern int __vmxoff(void);

/*
 *	__vmxon -- Enter VMX Operation
 *
 */
extern int __vmxon(addr64_t v);

#endif  /* _I386_VMX_CPU_H_ */
