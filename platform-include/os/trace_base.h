/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#ifndef __OS_TRACE_BASE_H__
#define __OS_TRACE_BASE_H__

#include <mach-o/loader.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <os/availability.h>
#include <os/base.h>
#include <os/object.h>

#define OS_LOG_FORMATLIKE(x, y)  __attribute__((format(os_log, x, y)))

#if     (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_10_0) \
     || (defined(__WATCH_OS_VERSION_MIN_REQUIRED) &&  __WATCH_OS_VERSION_MIN_REQUIRED >= __WATCHOS_3_0) \
     || (defined(__TV_OS_VERSION_MIN_REQUIRED) && __TV_OS_VERSION_MIN_REQUIRED >= __TVOS_10_0) \
     || (defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_12)
#define OS_LOG_TARGET_HAS_10_12_FEATURES 1
#else
#define OS_LOG_TARGET_HAS_10_12_FEATURES 0
#endif

#ifndef OS_COUNT_ARGS
#define OS_COUNT_ARGS(...) OS_COUNT_ARGS1(, ##__VA_ARGS__, _8, _7, _6, _5, _4, _3, _2, _1, _0)
#define OS_COUNT_ARGS1(z, a, b, c, d, e, f, g, h, cnt, ...) cnt
#endif

#ifdef OS_LOG_FORMAT_WARNINGS
#define OS_LOG_FORMAT_ERRORS _Pragma("clang diagnostic warning \"-Wformat\"")
#else
#define OS_LOG_FORMAT_ERRORS _Pragma("clang diagnostic error \"-Wformat\"")
#endif

#define OS_LOG_PRAGMA_PUSH \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wvla\"") \
        OS_LOG_FORMAT_ERRORS

#define OS_LOG_PRAGMA_POP  _Pragma("clang diagnostic pop")

#define OS_LOG_STRING(_ns, _var, _str) \
        _Static_assert(__builtin_constant_p(_str), \
                "formatters/labels/descriptions must be a constant string"); \
        __attribute__((section("__TEXT,__oslogstring,cstring_literals"),internal_linkage)) \
        static const char _var[] __asm(OS_STRINGIFY(OS_CONCAT(LOS_##_ns, __COUNTER__))) = _str


#define OS_LOG_REMOVE_PARENS(...) __VA_ARGS__

#define OS_LOG_CALL_WITH_FORMAT(fun, fun_args, fmt, ...) __extension__({ \
        OS_LOG_PRAGMA_PUSH OS_LOG_STRING(LOG, _os_fmt_str, fmt); \
        uint8_t _os_fmt_buf[__builtin_os_log_format_buffer_size(fmt, ##__VA_ARGS__)]; \
        fun(OS_LOG_REMOVE_PARENS fun_args, _os_fmt_str, \
                (uint8_t *)__builtin_os_log_format(_os_fmt_buf, fmt, ##__VA_ARGS__), \
                (uint32_t)sizeof(_os_fmt_buf)) OS_LOG_PRAGMA_POP; \
})

__BEGIN_DECLS

extern struct mach_header __dso_handle;

__END_DECLS

#endif // !__OS_TRACE_BASE_H__
