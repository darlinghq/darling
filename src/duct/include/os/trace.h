/*
 * Copyright (c) 2013-2015 Apple Inc. All rights reserved.
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

#ifndef __OS_TRACE_H__
#define __OS_TRACE_H__

#include <Availability.h>
#include <os/base.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#if __has_include(<xpc/xpc.h>)
#include <xpc/xpc.h>
#else
typedef void *xpc_object_t;
#endif

#if !__GNUC__
#error "must be GNU C compatible"
#endif

__BEGIN_DECLS

extern void *__dso_handle;

OS_ALWAYS_INLINE
static inline void
_os_trace_verify_printf(const char *msg, ...) __attribute__((format(printf, 1, 2)))
{
#pragma unused(msg)
}

#if !defined OS_COUNT_ARGS
#define OS_COUNT_ARGS(...) OS_COUNT_ARGS1(, ##__VA_ARGS__, _8, _7, _6, _5, _4, _3, _2, _1, _0)
#define OS_COUNT_ARGS1(z, a, b, c, d, e, f, g, h, cnt, ...) cnt
#endif

/* use old macros for anything less than iOS 10 and MacOS 10.12 */
#if (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) &&  __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_10_0) \
     || (defined(__WATCH_OS_VERSION_MIN_REQUIRED) &&  __WATCH_OS_VERSION_MIN_REQUIRED < __WATCHOS_3_0) \
     || (defined(__TV_OS_VERSION_MIN_REQUIRED) && __TV_OS_VERSION_MIN_REQUIRED < __TVOS_10_0) \
     || (defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_12)

#define _os_trace_0(_l, _m, _t) __extension__({ \
	_os_trace_verify_printf(_l); \
	_os_trace_with_buffer(&__dso_handle, _m, _t, NULL, 0, NULL); \
	__asm__(""); /* avoid tailcall */ \
})

#define _os_trace_1(_l, _m, _t, _1) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	_os_trace_verify_printf(_l, _c1); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		uint8_t _s[1]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._cnt = 1, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_2(_l, _m, _t, _1, _2) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	_os_trace_verify_printf(_l, _c1, _c2); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		uint8_t _s[2]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._cnt = 2, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_3(_l, _m, _t, _1, _2, _3) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	const __typeof__(_3) _c3 = _3; \
	_os_trace_verify_printf(_l, _c1, _c2, _c3); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		__typeof__(_c3) _f3; \
		uint8_t _s[3]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._f3 = _c3, ._s[2] = sizeof(_c3), \
		._cnt = 3, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_4(_l, _m, _t, _1, _2, _3, _4) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	const __typeof__(_3) _c3 = _3; \
	const __typeof__(_4) _c4 = _4; \
	_os_trace_verify_printf(_l, _c1, _c2, _c3, _c4); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		__typeof__(_c3) _f3; \
		__typeof__(_c4) _f4; \
		uint8_t _s[4]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._f3 = _c3, ._s[2] = sizeof(_c3), \
		._f4 = _c4, ._s[3] = sizeof(_c4), \
		._cnt = 4, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_5(_l, _m, _t, _1, _2, _3, _4, _5) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	const __typeof__(_3) _c3 = _3; \
	const __typeof__(_4) _c4 = _4; \
	const __typeof__(_5) _c5 = _5; \
	_os_trace_verify_printf(_l, _c1, _c2, _c3, _c4, _c5); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		__typeof__(_c3) _f3; \
		__typeof__(_c4) _f4; \
		__typeof__(_c5) _f5; \
		uint8_t _s[5]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._f3 = _c3, ._s[2] = sizeof(_c3), \
		._f4 = _c4, ._s[3] = sizeof(_c4), \
		._f5 = _c5, ._s[4] = sizeof(_c5), \
		._cnt = 5, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_6(_l, _m, _t, _1, _2, _3, _4, _5, _6) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	const __typeof__(_3) _c3 = _3; \
	const __typeof__(_4) _c4 = _4; \
	const __typeof__(_5) _c5 = _5; \
	const __typeof__(_6) _c6 = _6; \
	_os_trace_verify_printf(_l, _c1, _c2, _c3, _c4, _c5, _c6); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		__typeof__(_c3) _f3; \
		__typeof__(_c4) _f4; \
		__typeof__(_c5) _f5; \
		__typeof__(_c6) _f6; \
		uint8_t _s[6]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._f3 = _c3, ._s[2] = sizeof(_c3), \
		._f4 = _c4, ._s[3] = sizeof(_c4), \
		._f5 = _c5, ._s[4] = sizeof(_c5), \
		._f6 = _c6, ._s[5] = sizeof(_c6), \
		._cnt = 6, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_7(_l, _m, _t, _1, _2, _3, _4, _5, _6, _7) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	const __typeof__(_3) _c3 = _3; \
	const __typeof__(_4) _c4 = _4; \
	const __typeof__(_5) _c5 = _5; \
	const __typeof__(_6) _c6 = _6; \
	const __typeof__(_7) _c7 = _7; \
	_os_trace_verify_printf(_l, _c1, _c2, _c3, _c4, _c5, _c6, _c7); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		__typeof__(_c3) _f3; \
		__typeof__(_c4) _f4; \
		__typeof__(_c5) _f5; \
		__typeof__(_c6) _f6; \
		__typeof__(_c7) _f7; \
		uint8_t _s[7]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._f3 = _c3, ._s[2] = sizeof(_c3), \
		._f4 = _c4, ._s[3] = sizeof(_c4), \
		._f5 = _c5, ._s[4] = sizeof(_c5), \
		._f6 = _c6, ._s[5] = sizeof(_c6), \
		._f7 = _c7, ._s[6] = sizeof(_c7), \
		._cnt = 7, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), NULL); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_with_payload_1(_l, _m, _t, _payload) __extension__({ \
	_os_trace_verify_printf(_l); \
	_os_trace_with_buffer(&__dso_handle, _m, _t, NULL, 0, _payload); \
	__asm__(""); /* avoid tailcall */ \
})

#define _os_trace_with_payload_2(_l, _m, _t, _1, _payload) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	_os_trace_verify_printf(_l, _c1); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		uint8_t _s[1]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._cnt = 1, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_with_payload_3(_l, _m, _t, _1, _2, _payload) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	_os_trace_verify_printf(_l, _c1, _c2); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		uint8_t _s[2]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
                ._cnt = 2, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_with_payload_4(_l, _m, _t, _1, _2, _3, _payload) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	const __typeof__(_3) _c3 = _3; \
	_os_trace_verify_printf(_l, _c1, _c2, _c3); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		__typeof__(_c3) _f3; \
		uint8_t _s[3]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._f3 = _c3, ._s[2] = sizeof(_c3), \
                ._cnt = 3, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_with_payload_5(_l, _m, _t, _1, _2, _3, _4, _payload) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	const __typeof__(_3) _c3 = _3; \
	const __typeof__(_4) _c4 = _4; \
	_os_trace_verify_printf(_l, _c1, _c2, _c3, _c4); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		__typeof__(_c3) _f3; \
		__typeof__(_c4) _f4; \
		uint8_t _s[4]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._f3 = _c3, ._s[2] = sizeof(_c3), \
		._f4 = _c4, ._s[3] = sizeof(_c4), \
		._cnt = 4, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_with_payload_6(_l, _m, _t, _1, _2, _3, _4, _5, _payload) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	const __typeof__(_3) _c3 = _3; \
	const __typeof__(_4) _c4 = _4; \
	const __typeof__(_4) _c5 = _5; \
	_os_trace_verify_printf(_l, _c1, _c2, _c3, _c4, _c5); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		__typeof__(_c3) _f3; \
		__typeof__(_c4) _f4; \
		__typeof__(_c5) _f5; \
		uint8_t _s[5]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._f3 = _c3, ._s[2] = sizeof(_c3), \
		._f4 = _c4, ._s[3] = sizeof(_c4), \
		._f5 = _c5, ._s[4] = sizeof(_c5), \
		._cnt = 5, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_with_payload_7(_l, _m, _t, _1, _2, _3, _4, _5, _6, _payload) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	const __typeof__(_3) _c3 = _3; \
	const __typeof__(_4) _c4 = _4; \
	const __typeof__(_5) _c5 = _5; \
	const __typeof__(_6) _c6 = _6; \
	_os_trace_verify_printf(_l, _c1, _c2, _c3, _c4, _c5, _c6); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		__typeof__(_c3) _f3; \
		__typeof__(_c4) _f4; \
		__typeof__(_c5) _f5; \
		__typeof__(_c6) _f6; \
		uint8_t _s[6]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._f3 = _c3, ._s[2] = sizeof(_c3), \
		._f4 = _c4, ._s[3] = sizeof(_c4), \
		._f5 = _c5, ._s[4] = sizeof(_c5), \
		._f6 = _c6, ._s[5] = sizeof(_c6), \
		._cnt = 6, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define _os_trace_with_payload_8(_l, _m, _t, _1, _2, _3, _4, _5, _6, _7, _payload) __extension__({ \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wpacked\"") \
	const __typeof__(_1) _c1 = _1; \
	const __typeof__(_2) _c2 = _2; \
	const __typeof__(_3) _c3 = _3; \
	const __typeof__(_4) _c4 = _4; \
	const __typeof__(_5) _c5 = _5; \
	const __typeof__(_6) _c6 = _6; \
	const __typeof__(_7) _c7 = _7; \
	_os_trace_verify_printf(_l, _c1, _c2, _c3, _c4, _c5, _c6, _c7); \
	const struct __attribute__((packed)) { \
		__typeof__(_c1) _f1; \
		__typeof__(_c2) _f2; \
		__typeof__(_c3) _f3; \
		__typeof__(_c4) _f4; \
		__typeof__(_c5) _f5; \
		__typeof__(_c6) _f6; \
		__typeof__(_c7) _f7; \
		uint8_t _s[7]; \
                uint8_t _cnt; \
	} _buf = { \
		._f1 = _c1, ._s[0] = sizeof(_c1), \
		._f2 = _c2, ._s[1] = sizeof(_c2), \
		._f3 = _c3, ._s[2] = sizeof(_c3), \
		._f4 = _c4, ._s[3] = sizeof(_c4), \
		._f5 = _c5, ._s[4] = sizeof(_c5), \
		._f6 = _c6, ._s[5] = sizeof(_c6), \
		._f7 = _c7, ._s[6] = sizeof(_c7), \
                ._cnt = 7, \
	}; \
	_os_trace_with_buffer(&__dso_handle, _m, _t, &_buf, sizeof(_buf), _payload); \
	__asm__(""); /* avoid tailcall */ \
        _Pragma("clang diagnostic pop") \
})

#define OS_TRACE_CALL(format, _m, _t, ...) __extension__({                              \
    _Pragma("clang diagnostic push")                                                    \
    _Pragma("clang diagnostic ignored \"-Wc++98-compat-pedantic\"")                     \
    OS_CONCAT(_os_trace, OS_COUNT_ARGS(__VA_ARGS__))(format, _m, _t, ##__VA_ARGS__);    \
    _Pragma("clang diagnostic pop")                                                     \
})

#else

// Use a new layout in Mac OS 10.12+ and iOS 10.0+
#define OS_TRACE_CALL(_l, _m, _t, ...) __extension__({                          \
    uint8_t buf[1024];                                                          \
    _os_trace_verify_printf(_l, ##__VA_ARGS__);                                 \
    size_t buf_size = _os_trace_encode(buf, sizeof(buf), _m, ##__VA_ARGS__);    \
    _os_trace_internal(&__dso_handle, _t, _m, buf, buf_size, NULL);             \
    __asm__(""); /* avoid tailcall */                                           \
})

#define _os_trace_with_payload_1(_l, _m, _t, _payload) __extension__({  \
    _os_trace_verify_printf(_l);                                        \
    _os_trace_internal(&__dso_handle, _t, _m, NULL, 0, _payload);       \
    __asm__(""); /* avoid tailcall */                                   \
})

#define _os_trace_with_payload_2(_l, _m, _t, _1, _payload) __extension__({  \
    _os_trace_verify_printf(_l, _1);                                        \
    uint8_t buf[1024];                                                      \
    size_t buf_size = _os_trace_encode(buf, sizeof(buf), _m, _1);           \
    _os_trace_internal(&__dso_handle, _t, _m, buf, buf_size, _payload);     \
    __asm__(""); /* avoid tailcall */                                       \
})

#define _os_trace_with_payload_3(_l, _m, _t, _1, _2, _payload) __extension__({  \
    _os_trace_verify_printf(_l, _1, _2);                                        \
    uint8_t buf[1024];                                                          \
    size_t buf_size = _os_trace_encode(buf, sizeof(buf), _m, _1, _2);           \
    _os_trace_internal(&__dso_handle, _t, _m, buf, buf_size, _payload);         \
    __asm__(""); /* avoid tailcall */                                           \
})

#define _os_trace_with_payload_4(_l, _m, _t, _1, _2, _3, _payload) __extension__({  \
    _os_trace_verify_printf(_l, _1, _2, _3);                                        \
    uint8_t buf[1024];                                                              \
    size_t buf_size = _os_trace_encode(buf, sizeof(buf), _m, _1, _2, _3);           \
    _os_trace_internal(&__dso_handle, _t, _m, buf, buf_size, _payload);             \
    __asm__(""); /* avoid tailcall */                                               \
})

#define _os_trace_with_payload_5(_l, _m, _t, _1, _2, _3, _4, _payload) __extension__({  \
    _os_trace_verify_printf(_l, _1, _2, _3, _4);                                        \
    uint8_t buf[1024];                                                                  \
    size_t buf_size = _os_trace_encode(buf, sizeof(buf), _m, _1, _2, _3, _4);           \
    _os_trace_internal(&__dso_handle, _t, _m, buf, buf_size, _payload);                 \
    __asm__(""); /* avoid tailcall */                                                   \
})

#define _os_trace_with_payload_6(_l, _m, _t, _1, _2, _3, _4, _5, _payload) __extension__({  \
    _os_trace_verify_printf(_l, _1, _2, _3, _4, _5);                                        \
    uint8_t buf[1024];                                                                      \
    size_t buf_size = _os_trace_encode(buf, sizeof(buf), _m, _1, _2, _3, _4, _5);           \
    _os_trace_internal(&__dso_handle, _t, _m, buf, buf_size, _payload);                     \
    __asm__(""); /* avoid tailcall */                                                       \
})

#define _os_trace_with_payload_7(_l, _m, _t, _1, _2, _3, _4, _5, _6, _payload) __extension__({  \
    _os_trace_verify_printf(_l, _1, _2, _3, _4, _5, _6);                                        \
    uint8_t buf[1024];                                                                          \
    size_t buf_size = _os_trace_encode(buf, sizeof(buf), _m, _1, _2, _3, _4, _5, _6);           \
    _os_trace_internal(&__dso_handle, _t, _m, buf, buf_size, _payload);                         \
    __asm__(""); /* avoid tailcall */                                                           \
})

#define _os_trace_with_payload_8(_l, _m, _t, _1, _2, _3, _4, _5, _6, _7, _payload) __extension__({  \
    _os_trace_verify_printf(_l, _1, _2, _3, _4, _5, _6, _7);                                        \
    uint8_t buf[1024];                                                                              \
    size_t buf_size = _os_trace_encode(buf, sizeof(buf), _m, _1, _2, _3, _4, _5, _6, _7);           \
    _os_trace_internal(&__dso_handle, _t, _m, buf, buf_size, _payload);                             \
    __asm__(""); /* avoid tailcall */                                                               \
})

#endif /* if Mac OS >= 10.12 or iPhone OS >= 10.0 */

/*!
 *
 * @abstract
 * Hashtags in trace messages
 *
 * @discussion
 * Developers are encouraged to include hashtags in log messages, regardless of what API you use.
 * A hashtag is composed of a hash (#) symbol, followed by at least three non-whitespace characters,
 * terminated by whitespace or the end of the message. Hashtags may not begin with a number.
 *
 * Below is the list of predefined tags:
 *   #System     - Message in the context of a system process.
 *   #User       - Message in the context of a user process.
 *   #Developer  - Message in the context of software development. For example, deprecated APIs and debugging messages.
 *   #Attention  - Message that should be investigated by a system administrator, because it may be a sign of a larger issue.
 *     For example, errors from a hard drive controller that typically occur when the drive is about to fail.
 *   #Critical   - Message in the context of a critical event or critical failure.
 *   #Error      - Message that is a noncritical error.
 *   #Comment    - Message that is a comment.
 *   #Marker     - Message that marks a change to divide the messages around it into those before and those after the change.
 *   #Clue       - Message containing extra key/value pairs with additional information to help reconstruct the context.
 *   #Security   - Message related to security concerns.
 *   #Filesystem - Message describing a file system related event.
 *   #Network    - Message describing a network-related event.
 *   #Hardware   - Message describing a hardware-related event.
 *   #CPU        - Message describing CPU related event, e.g., initiating heavy work load
 *   #State      - Message describing state changed, e.g., global state, preference, etc.
 *   #Graphics   - Message describing significant graphics event
 *   #Disk       - Message describing disk activity
 *
 */

#pragma mark - Other defines

/*!
 * @define OS_TRACE_TYPE_RELEASE
 * Trace messages to be captured on a typical user install.  These should be
 * limited to things which improve diagnosis of a failure/crash/hang. Trace
 * buffers are generally smaller on a production system.
 */
#define OS_TRACE_TYPE_RELEASE (1u << 0)

/*!
 * @define OS_TRACE_TYPE_DEBUG
 * Trace messages to be captured while debugger or other development tool is
 * attached to the originator.
 */
#define OS_TRACE_TYPE_DEBUG (1u << 1)

/*!
 * @define OS_TRACE_TYPE_INFO
 * Trace messages that are captured when a debugger is attached, system or
 * Application mode has been increased to include additional information.
 */
#define OS_TRACE_TYPE_INFO (1u << 2)

/*!
 * @define OS_TRACE_TYPE_ERROR
 * Trace the message as an error and force a collection as a failure may be
 * imminent.
 */
#define OS_TRACE_TYPE_ERROR ((1u << 6) | (1u << 0))

/*!
 * @define OS_TRACE_TYPE_FAULT
 * Trace the message as a fatal error which forces a collection and a diagnostic
 * to be initiated.
 */
#define OS_TRACE_TYPE_FAULT ((1u << 7) | (1u << 6) | (1u << 0))

/*!
 * @typedef os_trace_payload_t
 * A block that populates an xpc_object_t of type XPC_TYPE_DICTIONARY to represent
 * complex data. This block will only be invoked under conditions where tools
 * have attached to the process. The payload can be used to send arbitrary data 
 * via the trace call. Tools may use the data to validate state for integration 
 * tests or provide other introspection services. No assumptions are made about 
 * the format or structure of the data.
 */
typedef void (^os_trace_payload_t)(xpc_object_t xdict);

#pragma mark - function declarations

/*!
 * @function os_trace
 *
 * @abstract
 * Always inserts a trace message into a buffer pool for later decoding.
 *
 * @discussion
 * Trace message that will be recorded on a typical user install. These should
 * be limited to things which help diagnose a failure during postmortem
 * analysis. Trace buffers are generally smaller on a production system.
 *
 * @param format
 * A printf-style format string to generate a human-readable log message when
 * the trace line is decoded.  Only scalar types are supported, attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 *		os_trace("network event: %ld, last seen: %ld, avg: %g", event_id, last_seen, avg);
 */
#define os_trace(format, ...) __extension__({                                           \
    _Static_assert(__builtin_constant_p(format), "format must be a constant string");   \
    __attribute__((section("__TEXT,__os_trace"))) static const char _m[] = format;      \
   OS_TRACE_CALL(format, _m, OS_TRACE_TYPE_RELEASE, ##__VA_ARGS__);                     \
})


#if (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) &&  __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_10_0) \
     || (defined(__WATCH_OS_VERSION_MIN_REQUIRED) &&  __WATCH_OS_VERSION_MIN_REQUIRED >= __WATCHOS_3_0) \
     || (defined(__TV_OS_VERSION_MIN_REQUIRED) && __TV_OS_VERSION_MIN_REQUIRED >= __TVOS_10_0) \
     || (defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_12)

/*!
 * @function os_trace_info
 *
 * @abstract
 * Optionally inserts a trace message containing additional information into a
 * buffer pool for later decoding.
 *
 * @discussion
 * Trace messages that will be captured when additional information is needed
 * and are not captured by default.  They will only be captured if the
 * system/process/activity mode has been increased or if a Development tool has
 * been attached to the process.
 *
 * @param format
 * A printf-style format string that represents a human-readable message when
 * the trace line is decoded.  Only scalar types are supported, attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 *		os_trace_info("network interface status %ld", status);
 */
#define os_trace_info(format, ...) __extension__({                                      \
    _Static_assert(__builtin_constant_p(format), "format must be a constant string");   \
    __attribute__((section("__TEXT,__os_trace"))) static const char _m[] = format;      \
   OS_TRACE_CALL(format, _m, OS_TRACE_TYPE_INFO, ##__VA_ARGS__);                        \
})

#endif

/*!
 * @function os_trace_debug
 *
 * @abstract
 * Insert debug trace message into a buffer pool for later decoding.
 *
 * @discussion
 * Debug trace message to be recorded while debugger or other development tool is
 * attached to the originator.  This is transported interprocess to help
 * diagnose the entire call chain including external helpers.
 *
 * @param format
 * A printf-style format string that represents a human-readable message when
 * the trace line is decoded.  Only scalar types are supported, attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 *		os_trace_debug("network interface status %ld", status);
 */
#define os_trace_debug(format, ...) __extension__({                                     \
    _Static_assert(__builtin_constant_p(format), "format must be a constant string");   \
    __attribute__((section("__TEXT,__os_trace"))) static const char _m[] = format;      \
   OS_TRACE_CALL(format, _m, OS_TRACE_TYPE_DEBUG, ##__VA_ARGS__);                       \
})

/*!
 * @function os_trace_info_enabled
 *
 * @abstract
 * Avoid unnecessary work for a trace point by checking if additional information
 * is enabled.
 *
 * @discussion
 * Avoid unnecessary work for a trace point by checking if additional information
 * is enabled. Generally trace points should not involve expensive operations, but some
 * circumstances warrant it.  Use this function to avoid doing the work unless
 * debug level trace messages are requested.
 *
 *	if (os_trace_info_enabled()) {
 *		os_trace_info("value = %d, average = %d",
 *                            [[dict objectForKey: @"myKey"] intValue],
 *			      (int) [self getAverage: dict]);
 *	}
 *
 * @result
 * Returns true if development mode is enabled.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
bool
os_trace_info_enabled(void);

/*!
 * @function os_trace_debug_enabled
 *
 * @abstract
 * Avoid unnecessary work for a trace point by checking if debug level is enabled.
 *
 * @discussion
 * Avoid unnecessary work for a trace point by checking if debug level is enabled.
 * Generally trace points should not involve expensive operations, but some
 * circumstances warrant it.  Use this function to avoid doing the work unless
 * debug level trace messages are requested.
 *
 *	if (os_trace_debug_enabled()) {
 *		os_trace_debug("value = %d, average = %d",
 *				[[dict objectForKey: @"myKey"] intValue],
 *				(int) [self getAverage: dict]);
 *	}
 *
 * @result
 * Returns true if debug mode is enabled.
 */
__OSX_AVAILABLE(10.10) __IOS_AVAILABLE(8.0) __WATCHOS_AVAILABLE(1.0) __TVOS_AVAILABLE(9.0)
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
bool
os_trace_debug_enabled(void);

/*!
 * @function os_trace_error
 *
 * @abstract
 * Trace the message as an error and force a collection of the trace buffer as a
 * failure may be imminent.
 *
 * @discussion
 * Trace the message as an error and force a collection of the trace buffer as a
 * failure may be imminent.
 *
 * @param format
 * A printf-style format string to generate a human-readable log message when
 * the trace line is decoded.  Only scalar types are supported, attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 *		os_trace_error("socket %d connection timeout %ld", fd, secs);
 */
#define os_trace_error(format, ...) __extension__({                                     \
    _Static_assert(__builtin_constant_p(format), "format must be a constant string");   \
    __attribute__((section("__TEXT,__os_trace"))) static const char _m[] = format;      \
   OS_TRACE_CALL(format, _m, OS_TRACE_TYPE_ERROR, ##__VA_ARGS__);                       \
})

/*!
 * @function os_trace_fault
 *
 * @abstract
 * Trace the message as a fault which forces a collection of the trace buffer
 * and diagnostic of the activity.
 *
 * @discussion
 * Trace the message as a fault which forces a collection of the trace buffer
 * and diagnostic of the activity.
 *
 * @param format
 * A printf-style format string to generate a human-readable log message when
 * the trace line is decoded.  Only scalar types are supported, attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 *		os_trace_fault("failed to lookup uid %d - aborting", uid);
 */
#define os_trace_fault(format, ...) __extension__({                                     \
    _Static_assert(__builtin_constant_p(format), "format must be a constant string");   \
    __attribute__((section("__TEXT,__os_trace"))) static const char _m[] = format;      \
    OS_TRACE_CALL(format, _m, OS_TRACE_TYPE_FAULT, ##__VA_ARGS__);                      \
})

#if __has_include(<xpc/xpc.h>)
/*!
 * @function os_trace_with_payload
 *
 * @abstract
 * Add a trace entry containing the provided values and call the block if
 * appropriate.
 *
 * @discussion
 * Will insert a trace entry into a limited ring buffer for an activity or
 * process.  Trace points are for recording interesting data that would improve
 * diagnosis of unexpected crashes, failures and hangs.  The block will only be
 * called under the required conditions.
 *
 * @param trace_msg
 * A printf-style format string to generate a human-readable log message when
 * the trace line is decoded.  Only scalar types are supported. Attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 * The final parameter must be a block of type os_trace_payload_t.
 *
 *   os_trace_with_payload("network event %ld", event, ^(xpc_object_t xdict) {
 *
 *		// validate the network interface and address where what was expected
 *		xpc_dictionary_set_string(xdict, "network", ifp->ifa_name);
 *		xpc_dictionary_set_string(xdict, "ip_address", _get_address(ifp));
 *   });
 */
#define os_trace_with_payload(format, ...) __extension__({                                                              \
    _Static_assert(__builtin_constant_p(format), "format must be a constant string");                                   \
    __attribute__((section("__TEXT,__os_trace"))) static const char _m[] = format;                                      \
    OS_CONCAT(_os_trace_with_payload, OS_COUNT_ARGS(__VA_ARGS__))(format, _m, OS_TRACE_TYPE_RELEASE, ##__VA_ARGS__);    \
})

#define os_trace_info_with_payload(format, ...) __extension__({                                                     \
    _Static_assert(__builtin_constant_p(format), "format must be a constant string");                               \
    __attribute__((section("__TEXT,__os_trace"))) static const char _m[] = format;                                  \
    OS_CONCAT(_os_trace_with_payload, OS_COUNT_ARGS(__VA_ARGS__))(format, _m, OS_TRACE_TYPE_INFO, ##__VA_ARGS__);   \
})

#define os_trace_debug_with_payload(format, ...) __extension__({                                                    \
    _Static_assert(__builtin_constant_p(format), "format must be a constant string");                               \
    __attribute__((section("__TEXT,__os_trace"))) static const char _m[] = format;                                  \
    OS_CONCAT(_os_trace_with_payload, OS_COUNT_ARGS(__VA_ARGS__))(format, _m, OS_TRACE_TYPE_DEBUG, ##__VA_ARGS__);  \
})

#define os_trace_error_with_payload(format, ...) __extension__({                                                    \
    _Static_assert(__builtin_constant_p(format), "format must be a constant string");                               \
    __attribute__((section("__TEXT,__os_trace"))) static const char _m[] = format;                                  \
    OS_CONCAT(_os_trace_with_payload, OS_COUNT_ARGS(__VA_ARGS__))(format, _m, OS_TRACE_TYPE_ERROR, ##__VA_ARGS__);  \
})

#define os_trace_fault_with_payload(format, ...) __extension__({                                                    \
    _Static_assert(__builtin_constant_p(format), "format must be a constant string");                               \
    __attribute__((section("__TEXT,__os_trace"))) static const char _m[] = format;                                  \
    OS_CONCAT(_os_trace_with_payload, OS_COUNT_ARGS(__VA_ARGS__))(format, _m, OS_TRACE_TYPE_FAULT, ##__VA_ARGS__);  \
})

#endif // __has_include(<xpc/xpc.h>)

// TODO: change this once we have compiler support
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW
size_t
_os_trace_encode(uint8_t *buf, size_t buf_size, const char *format, ...);

__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW
void
_os_trace_internal(void *dso, uint8_t type, const char *format, const uint8_t *buf, size_t buf_size, os_trace_payload_t payload);

/*!
 * @function _os_trace_with_buffer
 *
 * @abstract
 * Internal function to support pre-encoded buffer.
 */
__OSX_AVAILABLE(10.10) __IOS_AVAILABLE(8.0) __WATCHOS_AVAILABLE(1.0) __TVOS_AVAILABLE(9.0)
OS_EXPORT OS_NOTHROW
void
_os_trace_with_buffer(void *dso, const char *message, uint8_t type, const void *buffer, size_t buffer_size, os_trace_payload_t payload);

__END_DECLS

#endif // __OS_TRACE_H__
