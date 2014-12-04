/*
 * Copyright (c) 2003-2010 Apple Inc. All rights reserved.
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

#if defined(__i386__) || defined(__x86_64__)

#ifndef _I386_PLATFUNC_H
#define _I386_PLATFUNC_H

#ifndef	__ASSEMBLER__
#include <stdint.h>
#endif /* __ASSEMBLER__ */

// This is a shared macro which calls PLATFUNC_VARIANT_NAME which has different
// implementations in __ASSEMBLER__ and !__ASSEMBLER__
#define PLATFUNC_DESCRIPTOR_NAME(name, variant) \
	PLATFUNC_VARIANT_NAME(platfunc_ ## name, variant)

#ifdef	__ASSEMBLER__

/* When trying to acquire a spinlock or mutex, we will spin in
 * user mode for awhile, before entering the kernel to relinquish.
 * MP_SPIN_TRIES is the initial value of _COMM_PAGE_SPIN_COUNT.
 * The idea is that _COMM_PAGE_SPIN_COUNT will be adjusted up or
 * down as the machine is plugged in/out, etc.
 * At present spinlocks do not use _COMM_PAGE_SPIN_COUNT.
 * They use MP_SPIN_TRIES directly.
 */
#define	MP_SPIN_TRIES	1000

#define PLATFUNC_VARIANT_NAME(name, variant) _ ## name ## $VARIANT$ ## variant

#if defined (__i386__)
#define PLATFUNC_DESCRIPTOR_FIELD_POINTER .long
#define PLATFUNC_DESCRIPTOR_REFERENCE(name, variant) \
	.long PLATFUNC_DESCRIPTOR_NAME(name, variant)
#elif defined (__x86_64__)
#define PLATFUNC_DESCRIPTOR_FIELD_POINTER .quad
#define PLATFUNC_DESCRIPTOR_REFERENCE(name, variant) \
	.quad PLATFUNC_DESCRIPTOR_NAME(name, variant)
#else
#error unsupported architecture
#endif

#ifdef VARIANT_DYLD

#define PLATFUNC_FUNCTION_START_GENERIC(name, variant, codetype, alignment) \
	PLATFUNC_FUNCTION_START(name, variant, codetype, alignment) \
	.globl _ ## name ;\
	_ ## name ## :

#define	PLATFUNC_DESCRIPTOR(name, variant, must, cant)

#else /* VARIANT_DYLD */

#define PLATFUNC_FUNCTION_START_GENERIC PLATFUNC_FUNCTION_START

#define	PLATFUNC_DESCRIPTOR(name, variant, must, cant) \
	.const_data ;\
	.private_extern PLATFUNC_DESCRIPTOR_NAME(name, variant) ;\
	PLATFUNC_DESCRIPTOR_NAME(name, variant) ## : ;\
	PLATFUNC_DESCRIPTOR_FIELD_POINTER PLATFUNC_VARIANT_NAME(name, variant) ;\
	.long must ;\
	.long cant ;\
	.text

#endif /* VARIANT_DYLD */

#define PLATFUNC_FUNCTION_START(name, variant, codetype, alignment) \
	.text ;\
	.align alignment, 0x90 ;\
	.private_extern PLATFUNC_VARIANT_NAME(name, variant) ;\
	PLATFUNC_VARIANT_NAME(name, variant) ## :

#else /* __ASSEMBLER__ */

#define PLATFUNC_VARIANT_NAME(name, variant) name ## $VARIANT$ ## variant
#define PLATFUNC_DESCRIPTOR_PROTOTYPE(name, variant) extern const platfunc_descriptor PLATFUNC_DESCRIPTOR_NAME(name, variant);
#define PLATFUNC_DESCRIPTOR_REFERENCE(name, variant) &PLATFUNC_DESCRIPTOR_NAME(name, variant)

#define PLATFUNC_DESCRIPTOR(name, variant, must, cant) \
	extern void PLATFUNC_VARIANT_NAME(name, variant) (void); \
	const platfunc_descriptor PLATFUNC_DESCRIPTOR_NAME(name, variant) = { \
		.address = PLATFUNC_VARIANT_NAME(name, variant), \
		.musthave = must, \
		.canthave = cant \
	}

/*
 * Each potential platfunc routine is described by one of these.
 * Note that the PLATFUNC_DESCRIPTOR macro (above), used in
 * assembly language, must agree with this.
 */
 
typedef	struct	platfunc_descriptor	{
	void       *address; // address of code
	uint32_t   musthave; // _cpu_capability bits we must have
	uint32_t   canthave; // _cpu_capability bits we can't have
} platfunc_descriptor;

void *find_platform_function(const platfunc_descriptor *descriptors[]);

#endif	/* __ASSEMBLER__ */

#endif /* _I386_PLATFUNC_H */

#endif /* __i386__ || __x86_64__ */
