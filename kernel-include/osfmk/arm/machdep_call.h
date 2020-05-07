/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
/*
 * Copyright (c) 1992 NeXT Computer, Inc.
 *
 * Machine dependent kernel call table defines.
 *
 * HISTORY
 *
 * 17 June 1992 ? at NeXT
 *	Created.
 */

typedef union {
	kern_return_t           (*args_0)(void);
	kern_return_t           (*args_1)(vm_address_t);
	kern_return_t           (*args_2)(vm_address_t, vm_address_t);
	kern_return_t           (*args_3)(vm_address_t, vm_address_t, vm_address_t);
	kern_return_t           (*args_4)(vm_address_t, vm_address_t, vm_address_t, vm_address_t);
	kern_return_t           (*args_var)(vm_address_t, ...);
} machdep_call_routine_t;

#define MACHDEP_CALL_ROUTINE(func, args) \
	{ { .args_ ## args = func }, args }

typedef struct {
	machdep_call_routine_t      routine;
	int                         nargs;
} machdep_call_t;

extern const machdep_call_t             machdep_call_table[];
extern int                      machdep_call_count;

extern vm_address_t             thread_get_cthread_self(void);
extern kern_return_t            thread_set_cthread_self(vm_address_t);

// Read and write raw TPIDRURO / TPIDRRO_EL0
uintptr_t                       get_tpidrro(void);
void                            set_tpidrro(uintptr_t);
