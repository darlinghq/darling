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
# error os_overflow expects type-generic builtins
#endif /* __has_builtin(...) */

/* os_add3_overflow(a, b, c) -> (a + b + c) */
#define os_add3_overflow(a, b, c, res) __os_warn_unused(__extension__({ \
	__typeof(*(res)) _tmp; \
	bool _s, _t; \
	_s = os_add_overflow((a), (b), &_tmp); \
	_t = os_add_overflow((c), _tmp, (res)); \
	_s | _t; \
}))

/* os_mul3_overflow(a, b, c) -> (a * b * c) */
#define os_mul3_overflow(a, b, c, res) __os_warn_unused(__extension__({ \
	__typeof(*(res)) _tmp; \
	bool _s, _t; \
	_s = os_mul_overflow((a), (b), &_tmp); \
	_t = os_mul_overflow((c), _tmp, (res)); \
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

/* os_convert_overflow(a) -> a [converted to the result type] */
#define os_convert_overflow(a, res) os_add_overflow((a), 0, (res))

/* os_inc_overflow(res) -> *res += 1 */
#define os_inc_overflow(res) __os_warn_unused(__extension__({ \
	__typeof((res)) _tmp = (res); \
	os_add_overflow(*_tmp, 1, _tmp); \
}))

/* os_dec_overflow(res) -> *res -= 1 */
#define os_dec_overflow(res) __os_warn_unused(__extension__({ \
	__typeof((res)) _tmp = (res); \
	os_sub_overflow(*_tmp, 1, _tmp); \
}))


#endif /* _OS_OVERFLOW_H */
