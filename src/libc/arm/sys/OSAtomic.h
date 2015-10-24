/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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

#ifndef __ARM_OSATOMIC_SHIMS__
#define __ARM_OSATOMIC_SHIMS__

#include <sys/cdefs.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define fastpath(x)	((typeof(x))__builtin_expect((long)(x), ~0l))
#define slowpath(x)	((typeof(x))__builtin_expect((long)(x), 0l))

#if defined(__arm__)
#include <arm/arch.h>

#define _osatomic_load_exclusive(p, r) \
	__asm__ __volatile__( \
	"ldrex	%[_r], %[_p]" \
	: [_r] "=&r" (r) \
	: [_p] "m" (*(p)) : "memory")

#define _osatomic_load_exclusive64(p, r) \
	do { \
		register uint32_t rl asm("r4"), rh asm("r5"); \
		__asm__ __volatile__( \
			"ldrexd	%[_rl], %[_rh], %[_p]" \
			: [_rl] "=r" (rl), [_rh] "=r" (rh) \
			: [_p] "m" (*(p)) : "memory"); \
		r = (typeof(r))(((uint64_t)rh << 32) | (uint64_t)rl); \
	} while (0);

#define _osatomic_store_exclusive(p, r, t) \
	__asm__ __volatile__( \
	"strex	%[_t], %[_r], %[_p]" \
	: [_t] "=&r" (t) \
	: [_p] "m" (*(p)), [_r] "r" (r) : "memory")

#define _osatomic_store_exclusive64(p, r, t) \
	do { \
		register uint32_t rl asm("r4"), rh asm("r5"); \
		rh = (uint32_t)(r >> 32); \
		rl = (uint32_t)(r); \
		__asm__ __volatile__( \
			"strexd	%[_t], %[_rl], %[_rh], %[_p]" \
			: [_t] "=&r" (t) \
			: [_p] "m" (*(p)), [_rl] "r" (rl), [_rh] "r" (rh) : "memory"); \
	} while (0);

#if defined(_ARM_ARCH_7)

#ifdef _OSATOMIC_WFE
#	define MP_SPIN_TRIES		10
#	define _osatomic_pause()	__asm__ __volatile__("wfe")
#else
#	define MP_SPIN_TRIES		1000
#	define _osatomic_pause()	__asm__ __volatile__("yield")
#endif // _OSATOMIC_WFE

#define _osatomic_barrier() \
	__asm__ __volatile__( \
	"dmb	ish" \
	: : : "memory")

#define _osatomic_store_barrier() \
	__asm__ __volatile__( \
	"dmb	ishst" \
	: : : "memory")

#elif defined(_ARM_ARCH_6) && !defined(__thumb__) // _ARM_ARCH_7

#define _osatomic_barrier() \
	__asm__ __volatile__( \
	"mcr	p15, 0, %0, c7, c10, 5" \
	: : "r" (0) : "memory")

#define _osatomic_store_barrier() _osatomic_barrier()
#define _osatomic_pause()

#elif defined(_ARM_ARCH_6) && defined(__thumb__)
#error Atomic operations not available on ARMv6 Thumb
#endif // _ARM_ARCH_7

#if !defined(_ARM_ARCH_7)
#	ifndef _OSATOMIC_NO_BARRIERS
#		define __OSATOMIC_SUFFIX	""
#		define __OSATOMIC_BARRIER	"Barrier"
#		define _OSATOMIC_ALIAS_NB(x)
#	else
#		define _OSATOMIC_EXTRAS			1
#		define __OSATOMIC_SUFFIX	""
#		define __OSATOMIC_BARRIER	""
#		define _OSATOMIC_ALIAS_NB(x)
#	endif // _OSATOMIC_NO_BARRIERS
#else
#	define _OSATOMIC_EXTRAS			1
#	ifndef _OSATOMIC_NO_BARRIERS
#		ifdef _OSATOMIC_WFE
#			define __OSATOMIC_SUFFIX "$VARIANT$wfe"
#		else
#			define __OSATOMIC_SUFFIX "$VARIANT$mp"
#		endif // _OSATOMIC_ONLY_WFE
#		define __OSATOMIC_BARRIER "Barrier" __OSATOMIC_SUFFIX
#		define _OSATOMIC_ALIAS_NB(x)
#	else
#		define __OSATOMIC_SUFFIX "$VARIANT$up"
#		define __OSATOMIC_BARRIER "Barrier" __OSATOMIC_SUFFIX
// Undefine the barrier operations
#		undef _osatomic_barrier
#		define _osatomic_barrier()
#		undef _osatomic_store_barrier
#		define _osatomic_store_barrier()
// Aliases to alias non-barrier operations to UP variant names
#		if defined(__thumb__)
#			define _OSATOMIC_ALIAS_NB(sym)	__asm __volatile__(".globl _" __STRING(sym) "; .thumb_func _" __STRING(sym) " ; .set _" __STRING(sym) ", _" __STRING(sym) __OSATOMIC_BARRIER)
#		else
#			define _OSATOMIC_ALIAS_NB(sym)	__asm __volatile__(".globl _" __STRING(sym) "; .set _" __STRING(sym) ", _" __STRING(sym) __OSATOMIC_BARRIER)
#		endif // __thumb__
#	endif // _OSATOMIC_NO_BARRIERS
#endif // !_ARM_ARCH_7

// Allow us to mark up functions as being uni/multiprocessor.
#define _OSATOMIC_VARIANT(sym)	__asm("_" __STRING(sym) __OSATOMIC_SUFFIX)
#define _OSATOMIC_VARIANT_B(sym)	__asm("_" __STRING(sym) __OSATOMIC_BARRIER)

#if defined(__thumb__)
	// For aliasing CompareAndSwap operations of the same size
	#define _OSATOMIC_ALIAS_B(newsym, sym)	\
		__asm __volatile__( \
			".globl _" __STRING(newsym) __OSATOMIC_BARRIER ";" \
			".thumb_func _" __STRING(newsym) __OSATOMIC_BARRIER ";" \
			".set _" __STRING(newsym) __OSATOMIC_BARRIER ", _" __STRING(sym) __OSATOMIC_BARRIER \
		)

	#define _OSATOMIC_ALIAS(newsym, sym)	\
		__asm __volatile__( \
			".globl _" __STRING(newsym) __OSATOMIC_SUFFIX ";" \
			".thumb_func _" __STRING(newsym) __OSATOMIC_SUFFIX ";" \
			".set _" __STRING(newsym) __OSATOMIC_SUFFIX ", _" __STRING(sym) __OSATOMIC_SUFFIX \
		)
#else
	// For aliasing CompareAndSwap operations of the same size
	#define _OSATOMIC_ALIAS_B(newsym, sym)	\
		__asm __volatile__( \
			".globl _" __STRING(newsym) __OSATOMIC_BARRIER ";" \
			".set _" __STRING(newsym) __OSATOMIC_BARRIER ", _" __STRING(sym) __OSATOMIC_BARRIER \
		)

	#define _OSATOMIC_ALIAS(newsym, sym)	\
		__asm __volatile__( \
			".globl _" __STRING(newsym) __OSATOMIC_SUFFIX ";" \
			".set _" __STRING(newsym) __OSATOMIC_SUFFIX ", _" __STRING(sym) __OSATOMIC_SUFFIX \
		)
#endif // __thumb__

#endif // __arm__

#endif // __ARM_OSATOMIC_SHIMS__
