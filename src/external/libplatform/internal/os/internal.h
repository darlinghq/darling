/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __OS_INTERNAL_H__
#define __OS_INTERNAL_H__

#define __OS_ALLOC_INDIRECT__

#include <TargetConditionals.h>
#include <machine/cpu_capabilities.h>

#include "os/base_private.h"
#include "os/semaphore_private.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#if defined(__arm__)
#include <arm/arch.h>
#endif
#include <mach/thread_switch.h>

#define likely(x) os_likely(x)
#define unlikely(x) os_unlikely(x)

#define __OS_CRASH__(rc, msg)  ({ \
		_os_set_crash_log_cause_and_message(rc, msg); \
		os_prevent_tail_call_optimization(); \
		__builtin_trap(); \
	})

#define __LIBPLATFORM_CLIENT_CRASH__(rc, msg) \
		__OS_CRASH__(rc, "BUG IN CLIENT OF LIBPLATFORM: " msg)
#define __LIBPLATFORM_INTERNAL_CRASH__(rc, msg) \
		__OS_CRASH__(rc, "BUG IN LIBPLATFORM: " msg)


#define __OS_EXPOSE_INTERNALS__ 1
#include "os/internal/internal_shared.h"
#include "yield.h"

#if !VARIANT_NO_RESOLVERS
#if defined(_ARM_ARCH_7) && !defined(__ARM_ARCH_7S__)
#if OS_ATOMIC_UP
#define OS_VARIANT_SELECTOR up
#else
#define OS_VARIANT_SELECTOR mp
#endif
#endif
#if !defined(OS_VARIANT_SELECTOR) && defined(VARIANT_NO_RESOLVERS)
// forced up variant for no-barrier OSAtomics
#define OS_ATOMIC_NO_BARRIER_ONLY 1
#define OS_VARIANT_SELECTOR up
#endif
#if (defined(_ARM_ARCH_7) || defined(__arm64__)) && \
		(!defined(OS_ATOMIC_WFE) && !OS_ATOMIC_UP)
#define OS_ATOMIC_WFE 0
#endif
#ifdef OS_ATOMIC_WFE
#if OS_ATOMIC_WFE
#define OS_LOCK_VARIANT_SELECTOR wfe
#else
#define OS_LOCK_VARIANT_SELECTOR mp
#endif
#endif
#endif // !VARIANT_NO_RESOLVERS

#define OS_VARIANT(f, v) OS_CONCAT(f, OS_CONCAT($VARIANT$, v))

#ifdef OS_VARIANT_SELECTOR
#define _OS_ATOMIC_ALIAS_PRIVATE_EXTERN(n) \
		".private_extern _" OS_STRINGIFY(n) "\n\t"
#define OS_ATOMIC_EXPORT
#else
#define _OS_ATOMIC_ALIAS_PRIVATE_EXTERN(n)
#define OS_ATOMIC_EXPORT OS_EXPORT
#endif
#define _OS_ATOMIC_ALIAS_GLOBL(n) \
		".globl _" OS_STRINGIFY(n) "\n\t"
#ifdef __thumb__
#define _OS_ATOMIC_ALIAS_THUMB(n) \
		".thumb_func _" OS_STRINGIFY(n) "\n\t"
#else
#define _OS_ATOMIC_ALIAS_THUMB(n)
#endif
#define _OS_ATOMIC_ALIAS_SET(n, o) \
		".set _" OS_STRINGIFY(n) ", _" OS_STRINGIFY(o)

#define OS_ATOMIC_ALIAS(n, o) __asm__( \
		_OS_ATOMIC_ALIAS_PRIVATE_EXTERN(n) \
		_OS_ATOMIC_ALIAS_GLOBL(n) \
		_OS_ATOMIC_ALIAS_THUMB(n) \
		_OS_ATOMIC_ALIAS_SET(n, o))

#define OS_ATOMIC_EXPORT_ALIAS(n, o) __asm__( \
		_OS_ATOMIC_ALIAS_GLOBL(n) \
		_OS_ATOMIC_ALIAS_THUMB(n) \
		_OS_ATOMIC_ALIAS_SET(n, o))

#define _OS_VARIANT_RESOLVER(s, v, ...) \
	__attribute__((visibility(OS_STRINGIFY(v)))) extern void* s(void); \
	void* s(void) { \
	__asm__(".symbol_resolver _" OS_STRINGIFY(s)); \
		__VA_ARGS__ \
	}

#define _OS_VARIANT_UPMP_RESOLVER(s, v) \
	_OS_VARIANT_RESOLVER(s, v, \
		uint32_t *_c = (void*)(uintptr_t)_COMM_PAGE_CPU_CAPABILITIES; \
		if (*_c & kUP) { \
			extern void OS_VARIANT(s, up)(void); \
			return &OS_VARIANT(s, up); \
		} else { \
			extern void OS_VARIANT(s, mp)(void); \
			return &OS_VARIANT(s, mp); \
		})

#define OS_VARIANT_UPMP_RESOLVER(s) \
	_OS_VARIANT_UPMP_RESOLVER(s, default)

#define OS_VARIANT_UPMP_RESOLVER_INTERNAL(s) \
	_OS_VARIANT_UPMP_RESOLVER(s, hidden)

#endif // __OS_INTERNAL_H__
