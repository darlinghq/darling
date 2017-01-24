/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
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

#ifndef __OS_INTERNAL_ATOMIC__
#define __OS_INTERNAL_ATOMIC__

#ifndef __OS_EXPOSE_INTERNALS_INDIRECT__
/*
 * Use c11 <stdatomic.h> or c++11 std::atomic from <atomic> instead
 *
 * XXX                           /!\ WARNING /!\                           XXX
 *
 * This header file describes INTERNAL interfaces to libplatform used by other
 * libsystem targets, which are subject to change in future releases of OS X
 * and iOS. Any applications relying on these interfaces WILL break.
 *
 * If you are not a libsystem target, you should NOT EVER use these headers.
 * Not even a little.
 *
 * XXX                           /!\ WARNING /!\                           XXX
 */
#error "Please #include <os/internal/internal_shared.h> instead of this file directly."
#else

// generate error during codegen
#define _os_atomic_unimplemented() \
		({ __asm__(".err unimplemented"); })

#pragma mark -
#pragma mark memory_order

typedef enum _os_atomic_memory_order
{
	_os_atomic_memory_order_relaxed,
	_os_atomic_memory_order_consume,
	_os_atomic_memory_order_acquire,
	_os_atomic_memory_order_release,
	_os_atomic_memory_order_acq_rel,
	_os_atomic_memory_order_seq_cst,
	_os_atomic_memory_order_ordered,
} _os_atomic_memory_order;

#if !OS_ATOMIC_UP

#define os_atomic_memory_order_relaxed \
		_os_atomic_memory_order_relaxed
#define os_atomic_memory_order_acquire \
		_os_atomic_memory_order_acquire
#define os_atomic_memory_order_release \
		_os_atomic_memory_order_release
#define os_atomic_memory_order_acq_rel \
		_os_atomic_memory_order_acq_rel
#define os_atomic_memory_order_seq_cst \
		_os_atomic_memory_order_seq_cst
#define os_atomic_memory_order_ordered \
		_os_atomic_memory_order_seq_cst

#else // OS_ATOMIC_UP

#define os_atomic_memory_order_relaxed \
		_os_atomic_memory_order_relaxed
#define os_atomic_memory_order_acquire \
		_os_atomic_memory_order_relaxed
#define os_atomic_memory_order_release \
		_os_atomic_memory_order_relaxed
#define os_atomic_memory_order_acq_rel \
		_os_atomic_memory_order_relaxed
#define os_atomic_memory_order_seq_cst \
		_os_atomic_memory_order_relaxed
#define os_atomic_memory_order_ordered \
		_os_atomic_memory_order_relaxed

#endif // OS_ATOMIC_UP

#if __has_extension(c_generic_selections)
#define _os_atomic_basetypeof(p) \
		typeof(*_Generic((p), \
		char*: (char*)(p), \
		volatile char*: (char*)(p), \
		signed char*: (signed char*)(p), \
		volatile signed char*: (signed char*)(p), \
		unsigned char*: (unsigned char*)(p), \
		volatile unsigned char*: (unsigned char*)(p), \
		short*: (short*)(p), \
		volatile short*: (short*)(p), \
		unsigned short*: (unsigned short*)(p), \
		volatile unsigned short*: (unsigned short*)(p), \
		int*: (int*)(p), \
		volatile int*: (int*)(p), \
		unsigned int*: (unsigned int*)(p), \
		volatile unsigned int*: (unsigned int*)(p), \
		long*: (long*)(p), \
		volatile long*: (long*)(p), \
		unsigned long*: (unsigned long*)(p), \
		volatile unsigned long*: (unsigned long*)(p), \
		long long*: (long long*)(p), \
		volatile long long*: (long long*)(p), \
		unsigned long long*: (unsigned long long*)(p), \
		volatile unsigned long long*: (unsigned long long*)(p), \
		const void**: (const void**)(p), \
		const void*volatile*: (const void**)(p), \
		default: (void**)(p)))
#endif

#if __has_extension(c_atomic) && __has_extension(c_generic_selections)
#pragma mark -
#pragma mark c11

#define _os_atomic_c11_atomic(p) \
		_Generic((p), \
		char*: (_Atomic(char)*)(p), \
		volatile char*: (volatile _Atomic(char)*)(p), \
		signed char*: (_Atomic(signed char)*)(p), \
		volatile signed char*: (volatile _Atomic(signed char)*)(p), \
		unsigned char*: (_Atomic(unsigned char)*)(p), \
		volatile unsigned char*: (volatile _Atomic(unsigned char)*)(p), \
		short*: (_Atomic(short)*)(p), \
		volatile short*: (volatile _Atomic(short)*)(p), \
		unsigned short*: (_Atomic(unsigned short)*)(p), \
		volatile unsigned short*: (volatile _Atomic(unsigned short)*)(p), \
		int*: (_Atomic(int)*)(p), \
		volatile int*: (volatile _Atomic(int)*)(p), \
		unsigned int*: (_Atomic(unsigned int)*)(p), \
		volatile unsigned int*: (volatile _Atomic(unsigned int)*)(p), \
		long*: (_Atomic(long)*)(p), \
		volatile long*: (volatile _Atomic(long)*)(p), \
		unsigned long*: (_Atomic(unsigned long)*)(p), \
		volatile unsigned long*: (volatile _Atomic(unsigned long)*)(p), \
		long long*: (_Atomic(long long)*)(p), \
		volatile long long*: (volatile _Atomic(long long)*)(p), \
		unsigned long long*: (_Atomic(unsigned long long)*)(p), \
		volatile unsigned long long*: \
				(volatile _Atomic(unsigned long long)*)(p), \
		const void**: (_Atomic(const void*)*)(p), \
		const void*volatile*: (volatile _Atomic(const void*)*)(p), \
		default: (volatile _Atomic(void*)*)(p))

#define _os_atomic_barrier(m) \
		({ __c11_atomic_thread_fence(os_atomic_memory_order_##m); })
#define os_atomic_load(p, m) \
		({ _os_atomic_basetypeof(p) _r = \
		__c11_atomic_load(_os_atomic_c11_atomic(p), \
		os_atomic_memory_order_##m); (typeof(*(p)))_r; })
#define os_atomic_store(p, v, m) \
		({ _os_atomic_basetypeof(p) _v = (v); \
		__c11_atomic_store(_os_atomic_c11_atomic(p), _v, \
		os_atomic_memory_order_##m); })
#define os_atomic_xchg(p, v, m) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = \
		__c11_atomic_exchange(_os_atomic_c11_atomic(p), _v, \
		os_atomic_memory_order_##m); (typeof(*(p)))_r; })
#define os_atomic_cmpxchg(p, e, v, m) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = (e); \
		__c11_atomic_compare_exchange_strong(_os_atomic_c11_atomic(p), \
		&_r, _v, os_atomic_memory_order_##m, os_atomic_memory_order_relaxed); })
#define os_atomic_cmpxchgv(p, e, v, g, m) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = (e); _Bool _b = \
		__c11_atomic_compare_exchange_strong(_os_atomic_c11_atomic(p), \
		&_r, _v, os_atomic_memory_order_##m, os_atomic_memory_order_relaxed); \
		*(g) = (typeof(*(p)))_r; _b; })
#define os_atomic_cmpxchgvw(p, e, v, g, m) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = (e); _Bool _b = \
		__c11_atomic_compare_exchange_weak(_os_atomic_c11_atomic(p), \
		&_r, _v, os_atomic_memory_order_##m, os_atomic_memory_order_relaxed); \
		*(g) = (typeof(*(p)))_r;  _b; })
#define _os_atomic_c11_op(p, v, m, o, op) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = \
		__c11_atomic_fetch_##o(_os_atomic_c11_atomic(p), _v, \
		os_atomic_memory_order_##m); (typeof(*(p)))(_r op _v); })
#define _os_atomic_c11_op_orig(p, v, m, o, op) \
		({ _os_atomic_basetypeof(p) _v = (v), _r = \
		__c11_atomic_fetch_##o(_os_atomic_c11_atomic(p), _v, \
		os_atomic_memory_order_##m); (typeof(*(p)))_r; })

#define os_atomic_add(p, v, m) \
		_os_atomic_c11_op((p), (v), m, add, +)
#define os_atomic_add_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, add, +)
#define os_atomic_sub(p, v, m) \
		_os_atomic_c11_op((p), (v), m, sub, -)
#define os_atomic_sub_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, sub, -)
#define os_atomic_and(p, v, m) \
		_os_atomic_c11_op((p), (v), m, and, &)
#define os_atomic_and_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, and, &)
#define os_atomic_or(p, v, m) \
		_os_atomic_c11_op((p), (v), m, or, |)
#define os_atomic_or_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, or, |)
#define os_atomic_xor(p, v, m) \
		_os_atomic_c11_op((p), (v), m, xor, ^)
#define os_atomic_xor_orig(p, v, m) \
		_os_atomic_c11_op_orig((p), (v), m, xor, ^)

#elif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
#pragma mark -
#pragma mark gnu99

#define _os_atomic_full_barrier()	\
		__sync_synchronize()
#define _os_atomic_barrier(m)	\
		({ switch(os_atomic_memory_order_##m) { \
		case _os_atomic_memory_order_relaxed: \
			break; \
		default: \
			_os_atomic_full_barrier(); break; \
		} })
#define os_atomic_load(p, m) \
		({ typeof(*(p)) _r = *(p); \
		switch(os_atomic_memory_order_##m) { \
		case _os_atomic_memory_order_relaxed: \
		case _os_atomic_memory_order_acquire: \
		case _os_atomic_memory_order_seq_cst: \
			_os_atomic_barrier(m); \
			break; \
		default: \
			_os_atomic_unimplemented(); break; \
		} _r; })
#define os_atomic_store(p, v, m) \
		({ switch(os_atomic_memory_order_##m) { \
		case _os_atomic_memory_order_relaxed: \
		case _os_atomic_memory_order_release: \
		case _os_atomic_memory_order_seq_cst: \
			_os_atomic_barrier(m); \
			*(p) = (v); break; \
		default: \
			_os_atomic_unimplemented(); break; \
		} switch(os_atomic_memory_order_##m) { \
		case _os_atomic_memory_order_seq_cst: \
			_os_atomic_barrier(m); break; \
		default: \
			break; \
		} })
#if __has_builtin(__sync_swap)
#define os_atomic_xchg(p, v, m) \
		((typeof(*(p)))__sync_swap((p), (v)))
#else
#define os_atomic_xchg(p, v, m) \
		((typeof(*(p)))__sync_lock_test_and_set((p), (v)))
#endif
#define os_atomic_cmpxchg(p, e, v, m) \
		__sync_bool_compare_and_swap((p), (e), (v))
#define os_atomic_cmpxchgv(p, e, v, g, m) \
		({ typeof(*(g)) _e = (e), _r = \
		__sync_val_compare_and_swap((p), _e, (v)); \
		bool _b = (_e == _r); *(g) = _r; _b; })
#define os_atomic_cmpxchgvw(p, e, v, g, m) \
		os_atomic_cmpxchgv((p), (e), (v), (g), m)

#define os_atomic_add(p, v, m) \
		__sync_add_and_fetch((p), (v))
#define os_atomic_add_orig(p, v, m) \
		__sync_fetch_and_add((p), (v))
#define os_atomic_sub(p, v, m) \
		__sync_sub_and_fetch((p), (v))
#define os_atomic_sub_orig(p, v, m) \
		__sync_fetch_and_sub((p), (v))
#define os_atomic_and(p, v, m) \
		__sync_and_and_fetch((p), (v))
#define os_atomic_and_orig(p, v, m) \
		__sync_fetch_and_and((p), (v))
#define os_atomic_or(p, v, m) \
		__sync_or_and_fetch((p), (v))
#define os_atomic_or_orig(p, v, m) \
		__sync_fetch_and_or((p), (v))
#define os_atomic_xor(p, v, m) \
		__sync_xor_and_fetch((p), (v))
#define os_atomic_xor_orig(p, v, m) \
		__sync_fetch_and_xor((p), (v))

#if defined(__x86_64__) || defined(__i386__)
// GCC emits nothing for __sync_synchronize() on x86_64 & i386
#undef _os_atomic_full_barrier
#define _os_atomic_full_barrier() \
		({ __asm__ __volatile__( \
		"mfence" \
		: : : "memory"); })
#undef os_atomic_load
#define os_atomic_load(p, m) \
		({ switch(os_atomic_memory_order_##m) { \
		case _os_atomic_memory_order_relaxed: \
		case _os_atomic_memory_order_acquire: \
		case _os_atomic_memory_order_seq_cst: \
			break; \
		default: \
			_os_atomic_unimplemented(); break; \
		} *(p); })
// xchg is faster than store + mfence
#undef os_atomic_store
#define os_atomic_store(p, v, m) \
		({ switch(os_atomic_memory_order_##m) { \
		case _os_atomic_memory_order_relaxed: \
		case _os_atomic_memory_order_release: \
			*(p) = (v); break; \
		case _os_atomic_memory_order_seq_cst: \
			(void)os_atomic_xchg((p), (v), m); break; \
		default:\
			_os_atomic_unimplemented(); break; \
		} })
#endif

#else
#error "Please upgrade to GCC 4.2 or newer."
#endif

#pragma mark -
#pragma mark generic

#define os_atomic_thread_fence(m) _os_atomic_barrier(m)
// see comment in os_once.c
#define os_atomic_maximally_synchronizing_barrier() \
		_os_atomic_barrier(seq_cst)

#define os_atomic_load2o(p, f, m) \
		os_atomic_load(&(p)->f, m)
#define os_atomic_store2o(p, f, v, m) \
		os_atomic_store(&(p)->f, (v), m)
#define os_atomic_xchg2o(p, f, v, m) \
		os_atomic_xchg(&(p)->f, (v), m)
#define os_atomic_cmpxchg2o(p, f, e, v, m) \
		os_atomic_cmpxchg(&(p)->f, (e), (v), m)
#define os_atomic_cmpxchgv2o(p, f, e, v, g, m) \
		os_atomic_cmpxchgv(&(p)->f, (e), (v), (g), m)
#define os_atomic_cmpxchgvw2o(p, f, e, v, g, m) \
		os_atomic_cmpxchgvw(&(p)->f, (e), (v), (g), m)
#define os_atomic_add2o(p, f, v, m) \
		os_atomic_add(&(p)->f, (v), m)
#define os_atomic_add_orig2o(p, f, v, m) \
		os_atomic_add_orig(&(p)->f, (v), m)
#define os_atomic_sub2o(p, f, v, m) \
		os_atomic_sub(&(p)->f, (v), m)
#define os_atomic_sub_orig2o(p, f, v, m) \
		os_atomic_sub_orig(&(p)->f, (v), m)
#define os_atomic_and2o(p, f, v, m) \
		os_atomic_and(&(p)->f, (v), m)
#define os_atomic_and_orig2o(p, f, v, m) \
		os_atomic_and_orig(&(p)->f, (v), m)
#define os_atomic_or2o(p, f, v, m) \
		os_atomic_or(&(p)->f, (v), m)
#define os_atomic_or_orig2o(p, f, v, m) \
		os_atomic_or_orig(&(p)->f, (v), m)
#define os_atomic_xor2o(p, f, v, m) \
		os_atomic_xor(&(p)->f, (v), m)
#define os_atomic_xor_orig2o(p, f, v, m) \
		os_atomic_xor_orig(&(p)->f, (v), m)

#define os_atomic_inc(p, m) \
		os_atomic_add((p), 1, m)
#define os_atomic_inc_orig(p, m) \
		os_atomic_add_orig((p), 1, m)
#define os_atomic_inc2o(p, f, m) \
		os_atomic_add2o(p, f, 1, m)
#define os_atomic_inc_orig2o(p, f, m) \
		os_atomic_add_orig2o(p, f, 1, m)
#define os_atomic_dec(p, m) \
		os_atomic_sub((p), 1, m)
#define os_atomic_dec_orig(p, m) \
		os_atomic_sub_orig((p), 1, m)
#define os_atomic_dec2o(p, f, m) \
		os_atomic_sub2o(p, f, 1, m)
#define os_atomic_dec_orig2o(p, f, m) \
		os_atomic_sub_orig2o(p, f, 1, m)

#define os_atomic_rmw_loop(p, ov, nv, m, ...)  ({ \
		bool _result = false; \
		typeof(p) _p = (p); \
		ov = os_atomic_load(_p, relaxed); \
		do { \
			__VA_ARGS__; \
			_result = os_atomic_cmpxchgvw(_p, ov, nv, &ov, m); \
		} while (os_unlikely(!_result)); \
		_result; \
	})
#define os_atomic_rmw_loop2o(p, f, ov, nv, m, ...) \
		os_atomic_rmw_loop(&(p)->f, ov, nv, m, __VA_ARGS__)
#define os_atomic_rmw_loop_give_up_with_fence(m, expr) \
		({ os_atomic_thread_fence(m); expr; __builtin_unreachable(); })
#define os_atomic_rmw_loop_give_up(expr) \
		os_atomic_rmw_loop_give_up_with_fence(relaxed, expr)

#define os_atomic_tsx_xacq_cmpxchgv(p, e, v, g) \
		os_atomic_cmpxchgv((p), (e), (v), (g), acquire)
#define os_atomic_tsx_xrel_store(p, v) \
		os_atomic_store(p, v, release)
#define os_atomic_tsx_xacq_cmpxchgv2o(p, f, e, v, g) \
		os_atomic_tsx_xacq_cmpxchgv(&(p)->f, (e), (v), (g))
#define os_atomic_tsx_xrel_store2o(p, f, v) \
		os_atomic_tsx_xrel_store(&(p)->f, (v))

#if defined(__x86_64__) || defined(__i386__)
#pragma mark -
#pragma mark x86

#undef os_atomic_maximally_synchronizing_barrier
#ifdef __LP64__
#define os_atomic_maximally_synchronizing_barrier() \
		({ unsigned long _clbr; __asm__ __volatile__( \
		"cpuid" \
		: "=a" (_clbr) : "0" (0) : "rbx", "rcx", "rdx", "cc", "memory"); })
#else
#ifdef __llvm__
#define os_atomic_maximally_synchronizing_barrier() \
		({ unsigned long _clbr; __asm__ __volatile__( \
		"cpuid" \
		: "=a" (_clbr) : "0" (0) : "ebx", "ecx", "edx", "cc", "memory"); })
#else // gcc does not allow inline i386 asm to clobber ebx
#define os_atomic_maximally_synchronizing_barrier() \
		({ unsigned long _clbr; __asm__ __volatile__( \
		"pushl	%%ebx\n\t" \
		"cpuid\n\t" \
		"popl	%%ebx" \
		: "=a" (_clbr) : "0" (0) : "ecx", "edx", "cc", "memory"); })
#endif
#endif


#endif


#endif // __OS_EXPOSE_INTERNALS_INDIRECT__

#endif // __OS_ATOMIC__
