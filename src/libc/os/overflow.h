/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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
 * Facilities for performing type- and overflow-checked arithmetic. These
 * functions return non-zero if overflow occured, zero otherwise. In either case,
 * the potentially overflowing operation is fully performed, mod the size of the
 * output type. See:
 * http://clang.llvm.org/docs/LanguageExtensions.html#checked-arithmetic-builtins
 * for full details.
 *
 * The compiler enforces that users of os_*_overflow() check the return value to
 * determine whether overflow occured.
 */

#ifndef _OS_OVERFLOW_H
#define _OS_OVERFLOW_H

#include <sys/cdefs.h>
#include <stdbool.h>
#include <os/base.h>

bool __header_always_inline OS_WARN_RESULT
__os_warn_unused(__const bool x)
{
	return x;
}

#if __has_builtin(__builtin_add_overflow) && \
    __has_builtin(__builtin_sub_overflow) && \
    __has_builtin(__builtin_mul_overflow)

#define os_add_overflow(a, b, res) __os_warn_unused(__builtin_add_overflow((a), (b), (res)))
#define os_sub_overflow(a, b, res) __os_warn_unused(__builtin_sub_overflow((a), (b), (res)))
#define os_mul_overflow(a, b, res) __os_warn_unused(__builtin_mul_overflow((a), (b), (res)))

#else

/* compile-time assertion that 'x' and 'y' are equivalent types */
#ifdef __cplusplus
#define __OS_TYPE_CHECK(x, y) do { \
	__typeof__(x) _x; \
	__typeof__(y) _y; \
	(void)(&_x == &_y, "overflow arithmetic: incompatible types"); \
} while (0)
#else
#define __OS_TYPE_CHECK(x, y) do { \
	_Static_assert(__builtin_types_compatible_p(__typeof(x),__typeof(y)), \
			"overflow arithmetic: incompatible types"); \
} while (0)
#endif

#define __os_add_overflow_func(T,U,V) _Generic((T), \
		unsigned:           __builtin_uadd_overflow, \
		unsigned long:      __builtin_uaddl_overflow, \
		unsigned long long: __builtin_uaddll_overflow, \
		int:                __builtin_sadd_overflow, \
		long:               __builtin_saddl_overflow, \
		long long:          __builtin_saddll_overflow \
	)(T,U,V)

#define __os_sub_overflow_func(T,U,V) _Generic((T), \
		unsigned:           __builtin_usub_overflow, \
		unsigned long:      __builtin_usubl_overflow, \
		unsigned long long: __builtin_usubll_overflow, \
		int:                __builtin_ssub_overflow, \
		long:               __builtin_ssubl_overflow, \
		long long:          __builtin_ssubll_overflow \
	)(T,U,V)

#define __os_mul_overflow_func(T,U,V) _Generic((T), \
		unsigned:           __builtin_umul_overflow, \
		unsigned long:      __builtin_umull_overflow, \
		unsigned long long: __builtin_umulll_overflow, \
		int:                __builtin_smul_overflow, \
		long:               __builtin_smull_overflow, \
		long long:          __builtin_smulll_overflow \
	)(T,U,V)

#define os_add_overflow(a, b, res) __os_warn_unused(__extension__({ \
	__OS_TYPE_CHECK((a), (b)); \
	__OS_TYPE_CHECK((b), *(res)); \
	__os_add_overflow_func((a), (b), (res)); \
}))

#define os_sub_overflow(a, b, res) __os_warn_unused(__extension__({ \
	__OS_TYPE_CHECK((a), (b)); \
	__OS_TYPE_CHECK((b), *(res)); \
	__os_sub_overflow_func((a), (b), (res)); \
}))

#define os_mul_overflow(a, b, res) __os_warn_unused(__extension__({ \
	__OS_TYPE_CHECK((a), (b)); \
	__OS_TYPE_CHECK((b), *(res)); \
	__os_mul_overflow_func((a), (b), (res)); \
}))

#endif /* __has_builtin(...) */

/* os_add3_overflow(a, b, c) -> (a + b + c) */
#define os_add3_overflow(a, b, c, res) __os_warn_unused(__extension__({ \
	__typeof(*(res)) _tmp; \
	bool _s, _t; \
	_s = os_add_overflow((a), (b), &_tmp); \
	_t = os_add_overflow((c), _tmp, (res)); \
	_s | _t; \
}))

/* os_add_and_mul_overflow(a, b, x) -> (a + b)*x */
#define os_add_and_mul_overflow(a, b, x, res) __os_warn_unused(__extension__({ \
	__typeof(*(res)) _tmp; \
	bool _s, _t; \
	_s = os_add_overflow((a), (b), &_tmp); \
	_t = os_mul_overflow((x), _tmp, (res)); \
	_s | _t; \
}))

/* os_mul_and_add_overflow(a, x, b) -> a*x + b */
#define os_mul_and_add_overflow(a, x, b, res) __os_warn_unused(__extension__({ \
	__typeof(*(res)) _tmp; \
	bool _s, _t; \
	_s = os_mul_overflow((a), (x), &_tmp); \
	_t = os_add_overflow((b), _tmp, (res)); \
	_s | _t; \
}))

#endif /* _OS_OVERFLOW_H */
