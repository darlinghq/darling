/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifndef OS_TSD_H
#define OS_TSD_H

/* The low nine slots of the TSD are reserved for libsyscall usage. */
#define __TSD_RESERVED_BASE 0
#define __TSD_RESERVED_MAX 9

#define __TSD_THREAD_SELF 0
#define __TSD_ERRNO 1
#define __TSD_MIG_REPLY 2
#define __TSD_MACH_THREAD_SELF 3
#define __TSD_THREAD_QOS_CLASS 4
#define __TSD_RETURN_TO_KERNEL 5
/* slot 6 is reserved for Windows/WINE compatibility reasons */
#define __TSD_PTR_MUNGE 7
#define __TSD_MACH_SPECIAL_REPLY 8
#define __TSD_SEMAPHORE_CACHE 9

#ifndef __ASSEMBLER__

#include <stdint.h>

#ifdef __arm__
#include <arm/arch.h>
#endif

extern void _thread_set_tsd_base(void *tsd_base);

__attribute__((always_inline))
static __inline__ unsigned int
_os_cpu_number(void)
{
#if defined(__arm__)
	uintptr_t p;
	__asm__("mrc	p15, 0, %[p], c13, c0, 3" : [p] "=&r" (p));
	return (unsigned int)(p & 0x3ul);
#elif defined(__arm64__)
	uint64_t p;
	__asm__("mrs	%[p], TPIDRRO_EL0" : [p] "=&r" (p));
	return (unsigned int)p & 0x7;
#elif defined(__x86_64__) || defined(__i386__)
	struct { uintptr_t p1, p2; } p;
	__asm__("sidt %[p]" : [p] "=&m" (p));
	return (unsigned int)(p.p1 & 0xfff);
#else
#error _os_cpu_number not implemented on this architecture
#endif
}

#if defined(__i386__) || defined(__x86_64__)

#if defined(__has_attribute)
#if __has_attribute(address_space)
#define OS_GS_RELATIVE  __attribute__((address_space(256)))
#endif
#endif

#ifdef OS_GS_RELATIVE
#define _os_tsd_get_base() ((void * OS_GS_RELATIVE *)0)
#else
__attribute__((always_inline))
static __inline__ void*
_os_tsd_get_direct(unsigned long slot)
{
	void *ret;
	__asm__("mov %%gs:%1, %0" : "=r" (ret) : "m" (*(void **)(slot * sizeof(void *))));
	return ret;
}

__attribute__((always_inline))
static __inline__ int
_os_tsd_set_direct(unsigned long slot, void *val)
{
#if defined(__i386__) && defined(__PIC__)
	__asm__("movl %1, %%gs:%0" : "=m" (*(void **)(slot * sizeof(void *))) : "rn" (val));
#elif defined(__i386__) && !defined(__PIC__)
	__asm__("movl %1, %%gs:%0" : "=m" (*(void **)(slot * sizeof(void *))) : "ri" (val));
#else
	__asm__("movq %1, %%gs:%0" : "=m" (*(void **)(slot * sizeof(void *))) : "rn" (val));
#endif
	return 0;
}
#endif

#elif defined(__arm__) || defined(__arm64__)

__attribute__((always_inline, pure))
static __inline__ void**
_os_tsd_get_base(void)
{
#if defined(__arm__)
	uintptr_t tsd;
	__asm__("mrc p15, 0, %0, c13, c0, 3\n"
                "bic %0, %0, #0x3\n" : "=r" (tsd));
	/* lower 2-bits contain CPU number */
#elif defined(__arm64__)
	uint64_t tsd;
	__asm__("mrs %0, TPIDRRO_EL0\n"
                "bic %0, %0, #0x7\n" : "=r" (tsd));
	/* lower 3-bits contain CPU number */
#endif

	return (void**)(uintptr_t)tsd;
}
#define _os_tsd_get_base()  _os_tsd_get_base()

#else
#error _os_tsd_get_base not implemented on this architecture
#endif

#ifdef _os_tsd_get_base
__attribute__((always_inline))
static __inline__ void*
_os_tsd_get_direct(unsigned long slot)
{
	return _os_tsd_get_base()[slot];
}

__attribute__((always_inline))
static __inline__ int
_os_tsd_set_direct(unsigned long slot, void *val)
{
	_os_tsd_get_base()[slot] = val;
	return 0;
}
#endif

__attribute__((always_inline, pure))
static __inline__ uintptr_t
_os_ptr_munge_token(void)
{
	return (uintptr_t)_os_tsd_get_direct(__TSD_PTR_MUNGE);
}

__attribute__((always_inline, pure))
static __inline__ uintptr_t
_os_ptr_munge(uintptr_t ptr)
{
	return ptr ^ _os_ptr_munge_token();
}
#define _OS_PTR_MUNGE(_ptr) _os_ptr_munge((uintptr_t)(_ptr))
#define _OS_PTR_UNMUNGE(_ptr) _os_ptr_munge((uintptr_t)(_ptr))

#else // __ASSEMBLER__

#define _OS_TSD_OFFSET(_key) \
	((__POINTER_WIDTH__/__CHAR_BIT__)*_key)

#if defined(__i386__) || defined(__x86_64__)

#define _OS_PTR_MUNGE(_reg) \
	xor %gs:_OS_TSD_OFFSET(__TSD_PTR_MUNGE), _reg

#define _OS_PTR_UNMUNGE(_reg) \
	_OS_PTR_MUNGE(_reg)

#elif defined(__arm__) || defined(__arm64__)

#if defined(__arm__)

#define _OS_PTR_MUNGE_TOKEN(_reg, _token) \
	mrc p15, 0, _reg, c13, c0, 3; \
	bic	_reg, _reg, #3; \
	ldr	_token, [ _reg,  #_OS_TSD_OFFSET(__TSD_PTR_MUNGE) ]

#elif defined(__arm64__)

#define _OS_PTR_MUNGE_TOKEN(_reg, _token) \
	mrs _reg, TPIDRRO_EL0 %% \
	and	_reg, _reg, #~0x7 %% \
	ldr	_token, [ _reg,  #_OS_TSD_OFFSET(__TSD_PTR_MUNGE) ]

#endif // defined(__arm64__)

#define _OS_PTR_MUNGE(_regdest, _regsrc, _token) \
	eor _regdest, _regsrc, _token

#define _OS_PTR_UNMUNGE(_regdest, _regsrc, _token) \
	_OS_PTR_MUNGE(_regdest, _regsrc, _token)

#endif // defined(__arm__) || defined(__arm64__)

#endif // __ASSEMBLER__

#endif // OS_TSD_H
