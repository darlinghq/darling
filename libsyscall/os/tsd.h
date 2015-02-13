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

#include <stdint.h>

/* The low nine slots of the TSD are reserved for libsyscall usage. */
#define __TSD_RESERVED_BASE 0
#define __TSD_RESERVED_MAX 9

#define __TSD_THREAD_SELF 0
#define __TSD_ERRNO 1
#define __TSD_MIG_REPLY 2
#define __TSD_SEMAPHORE_CACHE 9


__attribute__((always_inline))
static __inline__ unsigned int
_os_cpu_number(void)
{
	/* Not yet implemented */
	return 0;
}

__attribute__((always_inline))
static __inline__ void*
_os_tsd_get_direct(unsigned long slot)
{
	void *ret;
#if defined(__i386__) || defined(__x86_64__)
	__asm__("mov %%gs:%1, %0" : "=r" (ret) : "m" (*(void **)(slot * sizeof(void *))));
#endif


	return ret;
}

__attribute__((always_inline))
static __inline__ int
_os_tsd_set_direct(unsigned long slot, void* val)
{
#if defined(__i386__) && defined(__PIC__)
	__asm__("movl %1, %%gs:%0" : "=m" (*(void **)(slot * sizeof(void *))) : "rn" (val));
#elif defined(__i386__) && !defined(__PIC__)
	__asm__("movl %1, %%gs:%0" : "=m" (*(void **)(slot * sizeof(void *))) : "ri" (val));
#elif defined(__x86_64__)
	__asm__("movq %1, %%gs:%0" : "=m" (*(void **)(slot * sizeof(void *))) : "rn" (val));
#endif

	return 0;
}

#endif
