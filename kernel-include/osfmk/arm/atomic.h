/*
 * Copyright (c) 2015-2018 Apple Inc. All rights reserved.
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

#ifndef _MACHINE_ATOMIC_H
#error "Do not include <arm/atomic.h> directly, use <machine/atomic.h>"
#endif

#ifndef _ARM_ATOMIC_H_
#define _ARM_ATOMIC_H_

#include <mach/boolean.h>

// Parameter for __builtin_arm_dmb
#define DMB_OSHLD       0x1
#define DMB_OSHST       0x2
#define DMB_OSH         0x3
#define DMB_NSHLD       0x5
#define DMB_NSHST       0x6
#define DMB_NSH         0x7
#define DMB_ISHLD       0x9
#define DMB_ISHST       0xa
#define DMB_ISH         0xb
#define DMB_LD          0xd
#define DMB_ST          0xe
#define DMB_SY          0xf

// Parameter for __builtin_arm_dsb
#define DSB_OSHLD       0x1
#define DSB_OSHST       0x2
#define DSB_OSH         0x3
#define DSB_NSHLD       0x5
#define DSB_NSHST       0x6
#define DSB_NSH         0x7
#define DSB_ISHLD       0x9
#define DSB_ISHST       0xa
#define DSB_ISH         0xb
#define DSB_LD          0xd
#define DSB_ST          0xe
#define DSB_SY          0xf

// Parameter for __builtin_arm_isb
#define ISB_SY          0xf

#undef OS_ATOMIC_HAS_LLSC
#define OS_ATOMIC_HAS_LLSC  1

#if defined(__ARM_ARCH_8_2__) && defined(__arm64__)
#undef OS_ATOMIC_USE_LLSC
#define OS_ATOMIC_USE_LLSC  0
#endif


/*
 * On armv7 & arm64, we do provide fine grained dependency injection, so
 * memory_order_dependency maps to relaxed as far as thread fences are concerned
 */
#undef memory_order_dependency_smp
#define memory_order_dependency_smp  memory_order_relaxed

#define os_atomic_clear_exclusive()  __builtin_arm_clrex()

#if __arm__

#define os_atomic_load_exclusive(p, m)  ({ \
	        _os_atomic_basetypeof(p) _r; \
	        _r = __builtin_arm_ldrex(p); \
	        _os_memory_fence_after_atomic(m); \
	        _os_compiler_barrier_after_atomic(m); \
	        _r; \
})

#define os_atomic_store_exclusive(p, v, m)  ({ \
	        _os_compiler_barrier_before_atomic(m); \
	        _os_memory_fence_before_atomic(m); \
	        !__builtin_arm_strex(p, v); \
})

/*
 * armv7 override of os_atomic_make_dependency
 * documentation for os_atomic_make_dependency is in <machine/atomic.h>
 */
#undef os_atomic_make_dependency
#define os_atomic_make_dependency(v) ({ \
	        os_atomic_dependency_t _dep; \
	        __asm__ __volatile__("and %[_dep], %[_v], #0" \
	                        : [_dep] "=r" (_dep.__opaque_zero) : [_v] "r" (v)); \
	        os_compiler_barrier(acquire); \
	        _dep; \
})

/*
 * armv7 override of os_atomic_rmw_loop
 * documentation for os_atomic_rmw_loop is in <machine/atomic.h>
 */
#undef os_atomic_rmw_loop
#define os_atomic_rmw_loop(p, ov, nv, m, ...)  ({ \
	        int _result = 0; uint32_t _err = 0; \
	        _os_atomic_basetypeof(p) *_p; \
	        _p = (_os_atomic_basetypeof(p) *)(p); \
	        _os_compiler_barrier_before_atomic(m); \
	        for (;;) { \
	                ov = __builtin_arm_ldrex(_p); \
	                __VA_ARGS__; \
	                if (!_err) { \
	/* release barrier only done for the first loop iteration */ \
	                        _os_memory_fence_before_atomic(m); \
	                } \
	                _err = __builtin_arm_strex(nv, _p); \
	                if (__builtin_expect(!_err, 1)) { \
	                        _os_memory_fence_after_atomic(m); \
	                        _result = 1; \
	                        break; \
	                } \
	        } \
	        _os_compiler_barrier_after_atomic(m); \
	        _result; \
	})

/*
 * armv7 override of os_atomic_rmw_loop_give_up
 * documentation for os_atomic_rmw_loop_give_up is in <machine/atomic.h>
 */
#undef os_atomic_rmw_loop_give_up
#define os_atomic_rmw_loop_give_up(...) \
	        ({ os_atomic_clear_exclusive(); __VA_ARGS__; break; })

#else // __arm64__

#define os_atomic_load_exclusive(p, m)  ({ \
	        _os_atomic_basetypeof(p) _r; \
	        if (memory_order_has_acquire(memory_order_##m##_smp)) { \
	                _r = __builtin_arm_ldaex(p); \
	        } else { \
	                _r = __builtin_arm_ldrex(p); \
	        } \
	        _os_compiler_barrier_after_atomic(m); \
	        _r; \
})

#define os_atomic_store_exclusive(p, v, m)  ({ \
	        _os_compiler_barrier_before_atomic(m); \
	        (memory_order_has_release(memory_order_##m##_smp) ? \
	                        !__builtin_arm_stlex(p, v) : !__builtin_arm_strex(p, v)); \
})

/*
 * arm64 override of os_atomic_make_dependency
 * documentation for os_atomic_make_dependency is in <machine/atomic.h>
 */
#undef os_atomic_make_dependency
#define os_atomic_make_dependency(v) ({ \
	        os_atomic_dependency_t _dep; \
	        __asm__ __volatile__("and %[_dep], %[_v], xzr" \
	                        : [_dep] "=r" (_dep.__opaque_zero) : [_v] "r" (v)); \
	        os_compiler_barrier(acquire); \
	        _dep; \
})

#if OS_ATOMIC_USE_LLSC

/*
 * arm64 (without armv81 atomics) override of os_atomic_rmw_loop
 * documentation for os_atomic_rmw_loop is in <machine/atomic.h>
 */
#undef os_atomic_rmw_loop
#define os_atomic_rmw_loop(p, ov, nv, m, ...)  ({ \
	        int _result = 0; \
	        _os_atomic_basetypeof(p) *_p; \
	        _p = (_os_atomic_basetypeof(p) *)(p); \
	        _os_compiler_barrier_before_atomic(m); \
	        do { \
	                if (memory_order_has_acquire(memory_order_##m##_smp)) { \
	                        ov = __builtin_arm_ldaex(_p); \
	                } else { \
	                        ov = __builtin_arm_ldrex(_p); \
	                } \
	                __VA_ARGS__; \
	                if (memory_order_has_release(memory_order_##m##_smp)) { \
	                        _result = !__builtin_arm_stlex(nv, _p); \
	                } else { \
	                        _result = !__builtin_arm_strex(nv, _p); \
	                } \
	        } while (__builtin_expect(!_result, 0)); \
	        _os_compiler_barrier_after_atomic(m); \
	        _result; \
	})

/*
 * arm64 override of os_atomic_rmw_loop_give_up
 * documentation for os_atomic_rmw_loop_give_up is in <machine/atomic.h>
 */
#undef os_atomic_rmw_loop_give_up
#define os_atomic_rmw_loop_give_up(...) \
	        ({ os_atomic_clear_exclusive(); __VA_ARGS__; break; })

#endif // OS_ATOMIC_USE_LLSC

#endif // __arm64__

#endif // _ARM_ATOMIC_H_
