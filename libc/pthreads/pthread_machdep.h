/*
 * Copyright (c) 2003-2004, 2008, 2011 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991  
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */

/* Machine-dependent definitions for pthread internals. */

#ifndef _POSIX_PTHREAD_MACHDEP_H
#define _POSIX_PTHREAD_MACHDEP_H

#ifndef __ASSEMBLER__

#include <System/machine/cpu_capabilities.h>
#ifdef __arm__
#include <arm/arch.h>
#endif
#include <TargetConditionals.h>
#include <stdint.h>

/*
** Define macros for inline pthread_getspecific() usage.
** We reserve a number of slots for Apple internal use.
** This number can grow dynamically, no need to fix it.
*/

/* This header contains pre defined thread specific keys */
/* 0 is used for pthread_self */
#define _PTHREAD_TSD_SLOT_PTHREAD_SELF		0
/* Keys 1- 9 for use by dyld, directly or indirectly */
#define _PTHREAD_TSD_SLOT_DYLD_1		1
#define _PTHREAD_TSD_SLOT_DYLD_2		2
#define _PTHREAD_TSD_SLOT_DYLD_3		3
#define _PTHREAD_TSD_RESERVED_SLOT_COUNT	4
/* To mirror the usage by dyld for Unwind_SjLj */
#define _PTHREAD_TSD_SLOT_DYLD_8		8

/* Keys 10 - 29 are for Libc/Libsystem internal ussage */
/* used as __pthread_tsd_first + Num  */
#define __PTK_LIBC_LOCALE_KEY		10
#define __PTK_LIBC_TTYNAME_KEY		11
#define __PTK_LIBC_LOCALTIME_KEY	12
#define __PTK_LIBC_GMTIME_KEY		13
#define __PTK_LIBC_GDTOA_BIGINT_KEY	14
#define __PTK_LIBC_PARSEFLOAT_KEY	15
/* for usage by dyld */
#define __PTK_LIBC_DYLD_Unwind_SjLj_Key	18

/* Keys 20-25 for libdispactch usage */
#define __PTK_LIBDISPATCH_KEY0		20
#define __PTK_LIBDISPATCH_KEY1		21
#define __PTK_LIBDISPATCH_KEY2		22
#define __PTK_LIBDISPATCH_KEY3		23
#define __PTK_LIBDISPATCH_KEY4		24
#define __PTK_LIBDISPATCH_KEY5		25

/* Keys 30-255 for Non Libsystem usage */

/* Keys 30-39 for Graphic frameworks usage */
#define _PTHREAD_TSD_SLOT_OPENGL	30	/* backwards compat sake */
#define __PTK_FRAMEWORK_OPENGL_KEY	30
#define __PTK_FRAMEWORK_GRAPHICS_KEY1	31
#define __PTK_FRAMEWORK_GRAPHICS_KEY2	32
#define __PTK_FRAMEWORK_GRAPHICS_KEY3	33
#define __PTK_FRAMEWORK_GRAPHICS_KEY4	34
#define __PTK_FRAMEWORK_GRAPHICS_KEY5	35
#define __PTK_FRAMEWORK_GRAPHICS_KEY6	36
#define __PTK_FRAMEWORK_GRAPHICS_KEY7	37
#define __PTK_FRAMEWORK_GRAPHICS_KEY8	38
#define __PTK_FRAMEWORK_GRAPHICS_KEY9	39

/* Keys 40-49 for Objective-C runtime usage */
#define __PTK_FRAMEWORK_OBJC_KEY0	40
#define __PTK_FRAMEWORK_OBJC_KEY1	41
#define __PTK_FRAMEWORK_OBJC_KEY2	42
#define __PTK_FRAMEWORK_OBJC_KEY3	43
#define __PTK_FRAMEWORK_OBJC_KEY4	44
#define __PTK_FRAMEWORK_OBJC_KEY5	45
#define __PTK_FRAMEWORK_OBJC_KEY6	46
#define __PTK_FRAMEWORK_OBJC_KEY7	47
#define __PTK_FRAMEWORK_OBJC_KEY8	48
#define __PTK_FRAMEWORK_OBJC_KEY9	49

/* Keys 50-59 for Core Foundation usage */
#define __PTK_FRAMEWORK_COREFOUNDATION_KEY0	50
#define __PTK_FRAMEWORK_COREFOUNDATION_KEY1	51
#define __PTK_FRAMEWORK_COREFOUNDATION_KEY2	52
#define __PTK_FRAMEWORK_COREFOUNDATION_KEY3	53
#define __PTK_FRAMEWORK_COREFOUNDATION_KEY4	54
#define __PTK_FRAMEWORK_COREFOUNDATION_KEY5	55
#define __PTK_FRAMEWORK_COREFOUNDATION_KEY6	56
#define __PTK_FRAMEWORK_COREFOUNDATION_KEY7	57
#define __PTK_FRAMEWORK_COREFOUNDATION_KEY8	58
#define __PTK_FRAMEWORK_COREFOUNDATION_KEY9	59

/* Keys 60-69 for Foundation usage */
#define __PTK_FRAMEWORK_FOUNDATION_KEY0		60
#define __PTK_FRAMEWORK_FOUNDATION_KEY1		61
#define __PTK_FRAMEWORK_FOUNDATION_KEY2		62
#define __PTK_FRAMEWORK_FOUNDATION_KEY3		63
#define __PTK_FRAMEWORK_FOUNDATION_KEY4		64
#define __PTK_FRAMEWORK_FOUNDATION_KEY5		65
#define __PTK_FRAMEWORK_FOUNDATION_KEY6		66
#define __PTK_FRAMEWORK_FOUNDATION_KEY7		67
#define __PTK_FRAMEWORK_FOUNDATION_KEY8		68
#define __PTK_FRAMEWORK_FOUNDATION_KEY9		69

/* Keys 70-79 for Core Animation/QuartzCore usage */
#define __PTK_FRAMEWORK_QUARTZCORE_KEY0		70
#define __PTK_FRAMEWORK_QUARTZCORE_KEY1		71
#define __PTK_FRAMEWORK_QUARTZCORE_KEY2		72
#define __PTK_FRAMEWORK_QUARTZCORE_KEY3		73
#define __PTK_FRAMEWORK_QUARTZCORE_KEY4		74
#define __PTK_FRAMEWORK_QUARTZCORE_KEY5		75
#define __PTK_FRAMEWORK_QUARTZCORE_KEY6		76
#define __PTK_FRAMEWORK_QUARTZCORE_KEY7		77
#define __PTK_FRAMEWORK_QUARTZCORE_KEY8		78
#define __PTK_FRAMEWORK_QUARTZCORE_KEY9		79


/* Keys 80-89 for Garbage Collection */
#define __PTK_FRAMEWORK_OLDGC_KEY0		80
#define __PTK_FRAMEWORK_OLDGC_KEY1		81
#define __PTK_FRAMEWORK_OLDGC_KEY2		82
#define __PTK_FRAMEWORK_OLDGC_KEY3		83
#define __PTK_FRAMEWORK_OLDGC_KEY4		84
#define __PTK_FRAMEWORK_OLDGC_KEY5		85
#define __PTK_FRAMEWORK_OLDGC_KEY6		86
#define __PTK_FRAMEWORK_OLDGC_KEY7		87
#define __PTK_FRAMEWORK_OLDGC_KEY8		88
#define __PTK_FRAMEWORK_OLDGC_KEY9		89

/* Keys 90-94 for JavaScriptCore Collection */
#define __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY0		90
#define __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY1		91
#define __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY2		92
#define __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY3		93
#define __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY4		94
/* Keys 95 for CoreText */
#define __PTK_FRAMEWORK_CORETEXT_KEY0			95

/* Keys 110-119 for Garbage Collection */
#define __PTK_FRAMEWORK_GC_KEY0		110
#define __PTK_FRAMEWORK_GC_KEY1		111
#define __PTK_FRAMEWORK_GC_KEY2		112
#define __PTK_FRAMEWORK_GC_KEY3		113
#define __PTK_FRAMEWORK_GC_KEY4		114
#define __PTK_FRAMEWORK_GC_KEY5		115
#define __PTK_FRAMEWORK_GC_KEY6		116
#define __PTK_FRAMEWORK_GC_KEY7		117
#define __PTK_FRAMEWORK_GC_KEY8		118
#define __PTK_FRAMEWORK_GC_KEY9		119

/*
** Define macros for inline pthread_getspecific() usage.
** We reserve a number of slots for Apple internal use.
** This number can grow dynamically, no need to fix it.
*/


#if defined(__cplusplus)
extern "C" {
#endif

extern void *pthread_getspecific(unsigned long);
extern int pthread_setspecific(unsigned long, const void *);
/* setup destructor function for static key as it is not created with pthread_key_create() */
int       pthread_key_init_np(int, void (*)(void *));

#if defined(__cplusplus)
}
#endif

typedef int pthread_lock_t;

__inline__ static int
_pthread_has_direct_tsd(void)
{
#if TARGET_IPHONE_SIMULATOR
	/* Simulator will use the host implementation, so bypass the macro that is in the target code */
	return 0;
#elif defined(__ppc__)
	int *caps = (int *)_COMM_PAGE_CPU_CAPABILITIES;
	if (*caps & kFastThreadLocalStorage) {
		return 1;
	} else {
		return 0;
	}
#else
	return 1;
#endif
}

#if TARGET_IPHONE_SIMULATOR || defined(__ppc__) || defined(__ppc64__) || \
	(defined(__arm__) && !defined(_ARM_ARCH_7) && defined(_ARM_ARCH_6) && defined(__thumb__))

#define _pthread_getspecific_direct(key) pthread_getspecific((key))
#define _pthread_setspecific_direct(key, val) pthread_setspecific((key), (val))

#else

/* To be used with static constant keys only */
__inline__ static void *
_pthread_getspecific_direct(unsigned long slot)
{
	void *ret;
#if defined(__i386__) || defined(__x86_64__)
	__asm__("mov %%gs:%1, %0" : "=r" (ret) : "m" (*(void **)(slot * sizeof(void *))));
#elif (defined(__arm__) && (defined(_ARM_ARCH_6) || defined(_ARM_ARCH_5))) 
	void **__pthread_tsd;
#if defined(__arm__) && defined(_ARM_ARCH_6)
	uintptr_t __pthread_tpid;
	__asm__("mrc p15, 0, %0, c13, c0, 3" : "=r" (__pthread_tpid));
	__pthread_tsd = (void**)(__pthread_tpid & ~0x3ul);
#elif defined(__arm__) && defined(_ARM_ARCH_5)
	register uintptr_t __pthread_tpid asm ("r9");
	__pthread_tsd = (void**)__pthread_tpid;
#endif
	ret = __pthread_tsd[slot];
#else
#error no _pthread_getspecific_direct implementation for this arch
#endif
	return ret;
}

/* To be used with static constant keys only */
__inline__ static int
_pthread_setspecific_direct(unsigned long slot, void * val)
{
#if defined(__i386__)
#if defined(__PIC__)
	__asm__("movl %1,%%gs:%0" : "=m" (*(void **)(slot * sizeof(void *))) : "rn" (val));
#else
	__asm__("movl %1,%%gs:%0" : "=m" (*(void **)(slot * sizeof(void *))) : "ri" (val));
#endif
#elif defined(__x86_64__)
	/* PIC is free and cannot be disabled, even with: gcc -mdynamic-no-pic ... */
	__asm__("movq %1,%%gs:%0" : "=m" (*(void **)(slot * sizeof(void *))) : "rn" (val));
#elif (defined(__arm__) && (defined(_ARM_ARCH_6) || defined(_ARM_ARCH_5))) 
	void **__pthread_tsd;
#if defined(__arm__) && defined(_ARM_ARCH_6)
	uintptr_t __pthread_tpid;
	__asm__("mrc p15, 0, %0, c13, c0, 3" : "=r" (__pthread_tpid));
	__pthread_tsd = (void**)(__pthread_tpid & ~0x3ul);
#elif defined(__arm__) && defined(_ARM_ARCH_5)
	register uintptr_t __pthread_tpid asm ("r9");
	__pthread_tsd = (void**)__pthread_tpid;
#endif
	__pthread_tsd[slot] = val;
#else
#error no _pthread_setspecific_direct implementation for this arch
#endif
	return 0;
}

#endif

#define LOCK_INIT(l)	((l) = 0)
#define LOCK_INITIALIZER 0

#endif /* ! __ASSEMBLER__ */
#endif /* _POSIX_PTHREAD_MACHDEP_H */
