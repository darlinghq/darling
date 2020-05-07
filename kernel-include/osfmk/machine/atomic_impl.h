/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
 * This header provides some gory details to implement the <machine/atomic.h>
 * interfaces. Nothing in this header should be called directly, no promise is
 * made to keep this interface stable.
 */

#ifndef _MACHINE_ATOMIC_H
#error "Do not include <machine/atomic_impl.h> directly, use <machine/atomic.h>"
#endif

#ifndef _MACHINE_ATOMIC_IMPL_H
#define _MACHINE_ATOMIC_IMPL_H

#include <stdatomic.h>
#include <machine/smp.h>

static inline int
memory_order_has_acquire(enum memory_order ord)
{
	switch (ord) {
	case memory_order_consume:
	case memory_order_acquire:
	case memory_order_acq_rel:
	case memory_order_seq_cst:
		return 1;
	default:
		return 0;
	}
}

static inline int
memory_order_has_release(enum memory_order ord)
{
	switch (ord) {
	case memory_order_release:
	case memory_order_acq_rel:
	case memory_order_seq_cst:
		return 1;
	default:
		return 0;
	}
}

#if __SMP__

#define memory_order_relaxed_smp            memory_order_relaxed
#define memory_order_compiler_acquire_smp   memory_order_relaxed
#define memory_order_compiler_release_smp   memory_order_relaxed
#define memory_order_compiler_acq_rel_smp   memory_order_relaxed
#define memory_order_consume_smp            memory_order_consume
#define memory_order_dependency_smp         memory_order_acquire
#define memory_order_acquire_smp            memory_order_acquire
#define memory_order_release_smp            memory_order_release
#define memory_order_acq_rel_smp            memory_order_acq_rel
#define memory_order_seq_cst_smp            memory_order_seq_cst

#else

#define memory_order_relaxed_smp            memory_order_relaxed
#define memory_order_compiler_acquire_smp   memory_order_relaxed
#define memory_order_compiler_release_smp   memory_order_relaxed
#define memory_order_compiler_acq_rel_smp   memory_order_relaxed
#define memory_order_consume_smp            memory_order_relaxed
#define memory_order_dependency_smp         memory_order_relaxed
#define memory_order_acquire_smp            memory_order_relaxed
#define memory_order_release_smp            memory_order_relaxed
#define memory_order_acq_rel_smp            memory_order_relaxed
#define memory_order_seq_cst_smp            memory_order_relaxed

#endif

/*
 * Hack needed for os_compiler_barrier() to work (including with empty argument)
 */
#define _os_compiler_barrier_relaxed        memory_order_relaxed
#define _os_compiler_barrier_acquire        memory_order_acquire
#define _os_compiler_barrier_release        memory_order_release
#define _os_compiler_barrier_acq_rel        memory_order_acq_rel
#define _os_compiler_barrier_               memory_order_acq_rel

/*
 * Mapping between compiler barrier/memory orders and:
 * - compiler barriers before atomics ("rel_barrier")
 * - compiler barriers after atomics ("acq_barrier")
 */
#define _os_rel_barrier_relaxed             memory_order_relaxed
#define _os_rel_barrier_compiler_acquire    memory_order_relaxed
#define _os_rel_barrier_compiler_release    memory_order_release
#define _os_rel_barrier_compiler_acq_rel    memory_order_release
#define _os_rel_barrier_consume             memory_order_relaxed
#define _os_rel_barrier_dependency          memory_order_relaxed
#define _os_rel_barrier_acquire             memory_order_relaxed
#define _os_rel_barrier_release             memory_order_release
#define _os_rel_barrier_acq_rel             memory_order_release
#define _os_rel_barrier_seq_cst             memory_order_release

#define _os_acq_barrier_relaxed             memory_order_relaxed
#define _os_acq_barrier_compiler_acquire    memory_order_acquire
#define _os_acq_barrier_compiler_release    memory_order_relaxed
#define _os_acq_barrier_compiler_acq_rel    memory_order_acquire
#define _os_acq_barrier_consume             memory_order_acquire
#define _os_acq_barrier_dependency          memory_order_acquire
#define _os_acq_barrier_acquire             memory_order_acquire
#define _os_acq_barrier_release             memory_order_relaxed
#define _os_acq_barrier_acq_rel             memory_order_acquire
#define _os_acq_barrier_seq_cst             memory_order_acquire

#define _os_compiler_barrier_before_atomic(m) \
	        atomic_signal_fence(_os_rel_barrier_##m)
#define _os_compiler_barrier_after_atomic(m) \
	        atomic_signal_fence(_os_acq_barrier_##m)

/*
 * Mapping between compiler barrier/memmory orders and:
 * - memory fences before atomics ("rel_fence")
 * - memory fences after atomics ("acq_fence")
 */
#define _os_rel_fence_relaxed               memory_order_relaxed
#define _os_rel_fence_compiler_acquire      memory_order_relaxed
#define _os_rel_fence_compiler_release      memory_order_release
#define _os_rel_fence_compiler_acq_rel      memory_order_release
#define _os_rel_fence_consume               memory_order_relaxed_smp
#define _os_rel_fence_dependency            memory_order_relaxed_smp
#define _os_rel_fence_acquire               memory_order_relaxed_smp
#define _os_rel_fence_release               memory_order_release_smp
#define _os_rel_fence_acq_rel               memory_order_release_smp
#define _os_rel_fence_seq_cst               memory_order_release_smp

#define _os_acq_fence_relaxed               memory_order_relaxed
#define _os_acq_fence_compiler_acquire      memory_order_relaxed
#define _os_acq_fence_compiler_release      memory_order_relaxed
#define _os_acq_fence_compiler_acq_rel      memory_order_relaxed
#define _os_acq_fence_consume               memory_order_acquire_smp
#define _os_acq_fence_dependency            memory_order_dependency_smp
#define _os_acq_fence_acquire               memory_order_acquire_smp
#define _os_acq_fence_release               memory_order_relaxed_smp
#define _os_acq_fence_acq_rel               memory_order_acquire_smp
#define _os_acq_fence_seq_cst               memory_order_acquire_smp

#define _os_memory_fence_before_atomic(m) \
	        atomic_thread_fence(_os_rel_fence_##m)
#define _os_memory_fence_after_atomic(m) \
	        atomic_thread_fence(_os_acq_fence_##m)

/*
 * Misc. helpers
 */

/*
 * For this implementation, we make sure the compiler cannot coalesce any of the
 * os_atomic calls by casting all atomic variables to `volatile _Atomic`.
 *
 * At the time this decision was taken, clang has been treating all `_Atomic`
 * accesses as if qualified `volatile _Atomic`, so the cast below freezes that
 * aspect of the codegen in time.
 *
 * When/if clang starts coalescing non-volatile _Atomics, we may decide to add
 * coalescing orderings, e.g. {relaxed,acquire,release,acq_rel,seq_cst}_nv.
 */
#define _os_atomic_c11_atomic(p) \
	        ((typeof(*(p)) volatile _Atomic *)(p))

#define _os_atomic_basetypeof(p) \
	        typeof(atomic_load(_os_atomic_c11_atomic(p)))

#define _os_atomic_op_orig(p, v, m, o)  ({ \
	        _os_atomic_basetypeof(p) _r; \
	        _os_compiler_barrier_before_atomic(m); \
	        _r = o(_os_atomic_c11_atomic(p), v, memory_order_##m##_smp); \
	        _os_compiler_barrier_after_atomic(m); \
	        _r; \
})

#define _os_atomic_c11_op_orig(p, v, m, o) \
	        _os_atomic_op_orig(p, v, m, atomic_##o##_explicit)

#define _os_atomic_c11_op(p, v, m, o, op) \
	        ({ typeof(v) _v = (v); _os_atomic_c11_op_orig(p, _v, m, o) op _v; })

#define _os_atomic_clang_op_orig(p, v, m, o) \
	        _os_atomic_op_orig(p, v, m, __atomic_##o)

#define _os_atomic_clang_op(p, v, m, o, op) \
	        ({ typeof(v) _v = (v); _os_atomic_basetypeof(p) _r = \
	                _os_atomic_clang_op_orig(p, _v, m, o); op(_r, _v); })

#define _os_atomic_auto_dependency(e) \
	        _Generic(e, \
	                os_atomic_dependency_t: (e), \
	                default: os_atomic_make_dependency(e))

#endif /* _MACHINE_ATOMIC_IMPL_H */
