/* Copyright (c) 2012, 2012 Apple Inc. All rights reserved.
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

#ifndef __OS_ASSUMES_H__
#define __OS_ASSUMES_H__

#include <sys/cdefs.h>
#include <stdalign.h>

__BEGIN_DECLS

#include <Availability.h>
#include <TargetConditionals.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <_simple.h>
#include <errno.h>
#include <os/base_private.h>
#include <stdint.h>

#if __GNUC__
#define os_constant(x) __builtin_constant_p((x))
#define os_hardware_trap() __asm__ __volatile__ (""); __builtin_trap()
#define __OS_COMPILETIME_ASSERT__(e) __extension__({ \
	char __compile_time_assert__[(e) ? 1 : -1];	\
	(void)__compile_time_assert__; \
})
#else /* __GNUC__ */
#define os_constant(x) ((long)0)
#define os_hardware_trap() abort()
#define __OS_COMPILETIME_ASSERT__(e) (e)
#endif /* __GNUC__ */

#pragma mark os_crash

/*
 * os_crash() is like os_hardware_trap(), except you get to pass in a crash
 * message, and it can be redirected to a callback function using
 * os_set_crash_callback()
 */

#define __os_crash_simple(msg) \
	({		 \
		_os_crash(msg);			\
		os_hardware_trap();		\
	})

#if defined(OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE)
#include <os/log_private.h>

#define __os_crash_fmt(...) \
	({ \
		const size_t size = os_log_pack_size(__VA_ARGS__); \
		uint8_t buf[size] __attribute__((aligned(alignof(os_log_pack_s)))); \
		os_log_pack_t pack = (os_log_pack_t)&buf; \
		os_log_pack_fill(pack, size, errno, __VA_ARGS__); \
		_os_crash_fmt(pack, size); \
		os_hardware_trap(); \
	})

#define __os_crash_N(msg) __os_crash_simple(msg)
#define __os_crash_Y(...) __os_crash_fmt(__VA_ARGS__)

// Returns Y if >1 argument, N if just one argument.
#define __thirty_second_argument(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, \
		_12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, \
		_26, _27, _28, _29, _30, _31, _32, ...) _32
#define __has_more_than_one_argument(...) __thirty_second_argument(__VA_ARGS__, \
		Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, \
		Y, Y, Y, Y, Y, Y, Y, N, EXTRA)

#define __os_crash_invoke(variant, ...) \
        OS_CONCAT(__os_crash_, variant)(__VA_ARGS__)

#define os_crash(...) \
	__os_crash_invoke(__has_more_than_one_argument(__VA_ARGS__), __VA_ARGS__)

OS_COLD
extern void
_os_crash_fmt(os_log_pack_t, size_t);

/*!
 * @function os_assert_sprintf
 * A routine to assert the result of a call to snprintf(3) or vsnprintf(3).
 *
 * @param ret
 * The return value from {v}snprintf(3).
 *
 * @param buff_size
 * The size of the buffer given to {v}snprintf(3).
 *
 * @discussion
 * If ret is less than zero or greater than size, the routine will abort the
 * caller with a message indicating the nature of the failure in the Application
 * Specific Information section of the resulting crash log.
 *
 * This routine is useful for printing paths that are expected to succeed with a
 * statically-sized buffer.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_ALWAYS_INLINE OS_COLD
static inline void
os_assert_sprintf(int ret, size_t buff_size)
{
	if (ret < 0) {
		os_crash("error printing buffer: %s", strerror(errno));
	}

	if ((size_t)ret > buff_size) {
		os_crash("buffer too small: needed = %d, actual = %lu",
				ret, buff_size);
	}
}

/*!
 * @function os_assert_asprintf
 * A routine to assert the result of a call to {v}asprintf(3).
 *
 * @param ret
 * The return value from {v}asnprintf(3).
 *
 * @discussion
 * If ret is less than zero, the routine will abort the caller with a message
 * indicating the nature of the failure in the Application Specific Information
 * section of the resulting crash log.
 */
API_AVAILABLE(macos(10.15.2), ios(13.3), tvos(13.3), watchos(6.1.1))
OS_ALWAYS_INLINE OS_COLD
static inline void
os_assert_asprintf(int ret)
{
	if (ret < 0) {
		os_crash("error printing buffer: %s", strerror(errno));
	}
}

/*!
 * @function os_assert_malloc
 * A routine to assert the result of allocations which may fail.
 *
 * @param desc
 * A string describing the object whose allocation was attempted.
 *
 * @param p
 * The result of a call to malloc(3), calloc(3), et al.
 *
 * @param alloc_size
 * The size of the attempted allocation.
 *
 * @discussion
 * If {@link p} is NULL, the routine will abort the caller with a message
 * indicating the nature of the failure in the Application Specific Information
 * section of the resulting crash log.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_ALWAYS_INLINE
static inline void
os_assert_malloc(const char *desc, const void *const p, size_t alloc_size)
{
	if (!p) {
		os_crash("allocation failed: obj = %s, size = %lu, error = %s",
				desc, alloc_size, strerror(errno));
	}
}

/*!
 * @function os_assert_mach
 * A routine to assert the result of a Mach kernel routine.
 *
 * @param op
 * A human-readable description of the operation.
 *
 * @param kr
 * The return code.
 *
 * @discsussion
 * If {@link kr} is non-zero, this routine will abort the caller with a message
 * indicating the nature of the failure in the Application Specific Information
 * section of the resulting crash log.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_EXPORT OS_NONNULL1 OS_COLD
void
os_assert_mach(const char *op, kern_return_t kr);

#define os_assert_mach(op, kr) ({ \
		kern_return_t __kr = (kr); \
		if (os_unlikely(__kr != KERN_SUCCESS)) os_assert_mach(op, kr); \
})

/*!
 * @function os_assert_mach_port_status
 * A routine to assert the status of a Mach port.
 *
 * @param desc
 * A human-readable description of the port's purpose.
 *
 * @param p
 * The port.
 *
 * @param expected
 * A pointer to a mach_port_status_t describing the expected attributes of the
 * port. If no particular value is expected for a given field in the structure,
 * a sentinel value may be provided for each expected field to indicate that its
 * check should be elided. The sentival values are:
 *
 * mps_pset => UINT32_MAX
 * mps_seqno => UINT32_MAX
 * mps_mscount => UINT32_MAX
 * mps_qlimit => UINT32_MAX
 * mps_msgcount => UINT32_MAX
 * mps_sorights => UINT32_MAX
 * mps_srights => INT32_MAX
 * mps_pdrequest => INT32_MAX
 * mps_nsrequest => INT32_MAX
 * mps_flags => 0
 *
 * @discussion
 * If there are any mismatches in the expected and actual status of the port,
 * the implementation will abort the caller. If status cannot be obtained for
 * the given port, the implementation will abort the caller.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_EXPORT
void
os_assert_mach_port_status(const char *desc, mach_port_t p,
		mach_port_status_t *expected);

#else // OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE

#define os_crash(msg) __os_crash_simple(msg)

#endif // OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE

/*
 * An executable can register a callback to be made upon a crash using the
 * os_set_crash_callback function.  If a crash callback is not set, the symbol
 * `os_crash_function` will be called in the main binary, if it exists.
 */

typedef void (*os_crash_callback_t) (const char *);

/* private: use accessors below */
extern os_crash_callback_t _os_crash_callback;

static inline os_crash_callback_t
os_get_crash_callback() {
	return _os_crash_callback;
}

static inline void
os_set_crash_callback(os_crash_callback_t callback) {
	_os_crash_callback = callback;
}

#pragma mark os_assert

#if defined(OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE)

#define _os_assert_crash(value, expression) ({ \
		os_crash("assertion failure: \"" expression "\" -> %lld", value); \
})

#define _os_assert_crash_errno(value, expression) ({ \
		os_crash("assertion failure: \"" expression "\" -> %{errno}d", value); \
})

#else // OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE

#define _os_assert_crash(e, ...) ({ \
		char *_fail_message = _os_assert_log(e); \
		os_crash(_fail_message); \
		free(_fail_message); \
})

#define _os_assert_crash_errno(...) _os_assert_crash(__VA_ARGS__)

#endif // OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE

#define __os_assert(e) __extension__({ \
	__typeof__(e) _e = (e); \
	if (os_unlikely(!_e)) { \
		if (os_constant(e)) { __OS_COMPILETIME_ASSERT__((e)); } \
		_os_assert_crash((uint64_t)(uintptr_t)_e, #e); \
	} \
})

#define __os_assert_zero(e) __extension__({ \
	__typeof__(e) _e = (e); \
	if (os_unlikely(_e)) { \
		if (os_constant(e)) { __OS_COMPILETIME_ASSERT__(!(e)); } \
		_os_assert_crash((uint64_t)(uintptr_t)_e, #e); \
	} \
})

/*
 * This variant is for use with old-style POSIX APIs that return -1 on failure
 * and set errno. If the return code is -1, the value logged will be as though
 * os_assert_zero(errno) was used. It encapsulates the following pattern:
 *
 * int tubes[2];
 * if (pipe(tubes) == -1) {
 *     (void)os_assert_zero(errno);
 * }
 */
#define __posix_assert_zero(e) __extension__({ \
	__typeof__(e) _e = (e); \
	if (os_unlikely(_e == (__typeof__(e))-1)) { \
		_os_assert_crash_errno(errno, #e); \
	} \
})

#if defined(OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE)

#define __os_assert_msg(e, fmt, ...) __extension__({ \
	__typeof__(e) _e = (e); \
	if (os_unlikely(!_e)) { \
		os_crash("assertion failure: " fmt, ##__VA_ARGS__); \
	} \
})

#define __os_assert_zero_msg(e, fmt, ...) __extension__({ \
	__typeof__(e) _e = (e); \
	if (os_unlikely(_e)) { \
		os_crash("assertion failure (%lld): " fmt, value, ##__VA_ARGS__); \
	} \
})

#define __posix_assert_zero_msg(e, fmt, ...) __extension__({ \
	__typeof__(e) _e = (e); \
	if (os_unlikely(_e == (__typeof__(e))-1)) { \
		os_crash("assertion failure (%{errno}d): " fmt, errno, ##__VA_ARGS__); \
	} \
})

#define __os_assert_N(e) __os_assert(e)
#define __os_assert_Y(...) __os_assert_msg(__VA_ARGS__)
#define __os_assert_zero_N(e) __os_assert_zero(e)
#define __os_assert_zero_Y(...) __os_assert_zero_msg(__VA_ARGS__)
#define __posix_assert_zero_N(e) __posix_assert_zero(e)
#define __posix_assert_zero_Y(...) __posix_assert_zero_msg(__VA_ARGS__)

#define __os_assert_invoke(function, variant, ...) \
        OS_CONCAT(function, variant)(__VA_ARGS__)

#define os_assert(...) \
	__os_assert_invoke(__os_assert_, \
			__has_more_than_one_argument(__VA_ARGS__), __VA_ARGS__)
#define os_assert_zero(...) \
	__os_assert_invoke(__os_assert_zero_, \
			__has_more_than_one_argument(__VA_ARGS__), __VA_ARGS__)
#define posix_assert_zero(...) \
	__os_assert_invoke(__posix_assert_zero_, \
			__has_more_than_one_argument(__VA_ARGS__), __VA_ARGS__)

#else // OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE

#define os_assert(e) __os_assert(e)
#define os_assert_zero(e) __os_assert_zero(e)
#define posix_assert_zero(e) __posix_assert_zero(e)

#endif

#pragma mark os_assumes


#define os_assumes(e) __extension__({ \
	__typeof__(e) _e = os_fastpath(e); \
	if (!_e) { \
		if (os_constant(e)) { \
			__OS_COMPILETIME_ASSERT__(e); \
		} \
		_os_assumes_log((uint64_t)(uintptr_t)_e); \
	} \
	_e; \
})

#define os_assumes_zero(e) __extension__({ \
	__typeof__(e) _e = os_slowpath(e); \
	if (_e) { \
		if (os_constant(e)) { \
			__OS_COMPILETIME_ASSERT__(!(e)); \
		} \
		_os_assumes_log((uint64_t)(uintptr_t)_e); \
	} \
	_e; \
})

#define posix_assumes_zero(e) __extension__({ \
	__typeof__(e) _e = os_slowpath(e); \
	if (_e == (__typeof__(e))-1) { \
		_os_assumes_log((uint64_t)(uintptr_t)errno); \
	} \
	_e; \
})

#pragma mark assumes redirection

/* This is useful for clients who wish for the messages generated by assumes()
 * failures to go somewhere other than (or in addition to) the system log. If
 * you don't wish for the message to be logged to the system log, then return
 * true (to indicate that the message has been handled). If you want the default
 * behavior, return false.
 */
typedef bool (*os_redirect_t)(const char *);
struct _os_redirect_assumes_s {
	os_redirect_t redirect;
};

#define OS_ASSUMES_REDIRECT_SEG "__DATA"
#define OS_ASSUMES_REDIRECT_SECT "__os_assumes_log"

#define os_redirect_assumes(func) \
	__attribute__((__used__)) \
	__attribute__((__section__(OS_ASSUMES_REDIRECT_SEG "," OS_ASSUMES_REDIRECT_SECT))) \
	static struct _os_redirect_assumes_s _os_redirect_##func = { \
		.redirect = &func, \
	};

#pragma mark _ctx variants

/*
 * These are for defining your own assumes()-like wrapper calls so that you can
 * log additional information, such as the about-PID, sender, etc. They're
 * generally not useful for direct inclusion in your code.
 */

/*
 * The asl_message argument is a _SIMPLE_STRING that, when given to _simple_asl_send(), will
 * direct the message to the MessageTracer diagnostic messages store rather than
 * the default system log store.
 */
typedef bool (*os_log_callout_t)(_SIMPLE_STRING asl_message, void *ctx, const char *);


#define os_assumes_ctx(f, ctx, e) __extension__({ \
	__typeof__(e) _e = os_fastpath(e); \
	if (!_e) { \
		if (os_constant(e)) { \
			__OS_COMPILETIME_ASSERT__(e); \
		} \
		_os_assumes_log_ctx(f, ctx, (uintptr_t)_e); \
	} \
	_e; \
})

#define os_assumes_zero_ctx(f, ctx, e) __extension__({ \
	__typeof__(e) _e = os_slowpath(e); \
	if (_e) { \
		if (os_constant(e)) { \
			__OS_COMPILETIME_ASSERT__(!(e)); \
		} \
		_os_assumes_log_ctx((f), (ctx), (uintptr_t)_e); \
	} \
	_e; \
})

#define posix_assumes_zero_ctx(f, ctx, e) __extension__({ \
	__typeof__(e) _e = os_slowpath(e); \
	if (_e == (__typeof__(e))-1) { \
		_os_assumes_log_ctx((f), (ctx), (uintptr_t)errno); \
	} \
	_e; \
})

#define os_assert_ctx(f, ctx, e) __extension__({ \
	__typeof__(e) _e = os_fastpath(e); \
	if (!_e) { \
		if (os_constant(e)) { \
			__OS_COMPILETIME_ASSERT__(e); \
		} \
									\
		char *_fail_message = _os_assert_log_ctx((f), (ctx), (uint64_t)(uintptr_t)_e); \
		os_crash(_fail_message);	     \
		free(_fail_message); \
	} \
})

#define os_assert_zero_ctx(f, ctx, e) __extension__({ \
	__typeof__(e) _e = os_slowpath(e); \
	if (_e) { \
		if (os_constant(e)) { \
			__OS_COMPILETIME_ASSERT__(!(e)); \
		} \
\
		char *_fail_message = _os_assert_log_ctx((f), (ctx), (uint64_t)(uintptr_t)_e); \
		os_crash(_fail_message);	     \
		free(_fail_message); \
	} \
})

#define posix_assert_zero_ctx(f, ctx, e) __extension__({ \
	__typeof__(e) _e = os_slowpath(e); \
	if (_e == (__typeof__(e))-1) { \
		char *_fail_message = _os_assert_log_ctx((f), (ctx), (uint64_t)(uintptr_t)errno); \
		os_crash(_fail_message);	     \
		free(_fail_message); \
	} \
})

#pragma mark internal symbols

__OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0)
OS_COLD OS_NOT_TAIL_CALLED
extern void
_os_crash(const char *);

__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_6_0)
OS_COLD OS_NOT_TAIL_CALLED
extern void
_os_assumes_log(uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_6_0)
OS_COLD OS_NOT_TAIL_CALLED
extern char *
_os_assert_log(uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_6_0)
OS_COLD OS_NOT_TAIL_CALLED
extern void
_os_assumes_log_ctx(os_log_callout_t callout, void *ctx, uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_6_0)
OS_COLD OS_NOT_TAIL_CALLED
extern char *
_os_assert_log_ctx(os_log_callout_t callout, void *ctx, uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_6_0)
extern void
_os_avoid_tail_call(void);

__END_DECLS

#endif /* __OS_ASSUMES_H__ */
