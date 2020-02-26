/*
 * Copyright (c) 2013-2016 Apple Inc. All rights reserved.
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

#ifndef __OS_TRACE_H__
#define __OS_TRACE_H__

#include <os/trace_base.h>
#if __has_include(<xpc/xpc.h>)
#include <xpc/xpc.h>
typedef xpc_object_t os_trace_payload_object_t;
#else
typedef void *os_trace_payload_object_t;
#endif

#if !__GNUC__
#error "must be GNU C compatible"
#endif

__BEGIN_DECLS

/* We need at least clang 7.3 or later due to bugs in os_log_format parsing */
#if __has_builtin(__builtin_os_log_format) && (__clang_major__ > 7 || (__clang_major__ == 7 && __clang_minor__ >= 3))

OS_ALWAYS_INLINE __attribute__((format(os_trace, 1, 2)))
static inline void
_os_trace_verify_printf(const char *msg, ...)
{
#pragma unused(msg)
}

/* Previous OSes must go through older style...
 *
 * Format:
 *      Items: [ ]
 *      Item sizes: [
 *          8 bits * count
 *      ]
 *      Item count: 8 bits
 */

#define OS_TRACE_CALL(_t, _p, _f, ...) __extension__({ \
    if (os_trace_type_enabled(_t)) { \
        _os_trace_verify_printf(_f, ##__VA_ARGS__); \
        OS_LOG_STRING(TRACE, __f, _f); \
        uint32_t __size = (uint32_t)__builtin_os_log_format_buffer_size(_f, ##__VA_ARGS__); \
        uint8_t _buf[__size]; \
        __builtin_os_log_format(_buf, _f, ##__VA_ARGS__); \
        uint32_t tz = 0; \
        uint8_t tb[__size]; \
        uint8_t *buff = _buf; \
        uint8_t *p = ++buff; \
        uint8_t count = *p++; \
        uint8_t trailer[count + 1]; \
        trailer[count] = count; \
        for (uint8_t ii = 0; ii < count; ii++) { \
            uint8_t desc = *p++; \
            uint8_t size = *p++; \
            uint8_t *value = p; \
            p += size; \
            if ((desc >> 4) || (desc & 0x1)) { \
                size = 0;\
            }\
            if (size) {\
                memcpy(&tb[tz], value, size);\
                tz += size;\
            }\
            trailer[ii] = size;\
        }\
        memcpy(&tb[tz], trailer, sizeof(trailer));\
        tz += sizeof(trailer);\
        _os_trace_with_buffer(&__dso_handle, __f, _t, tb, tz, _p);\
    } \
})
#else
#define OS_TRACE_CALL(...)
#endif

// macros to re-order arguments so we can call log function
#define _os_trace_with_payload_1(_t, _f, _p) OS_TRACE_CALL(_t, _p, _f)
#define _os_trace_with_payload_2(_t, _f, _1, _p) OS_TRACE_CALL(_t, _p, _f, _1)
#define _os_trace_with_payload_3(_t, _f, _1, _2, _p) OS_TRACE_CALL(_t, _p, _f, _1, _2)
#define _os_trace_with_payload_4(_t, _f, _1, _2, _3, _p) OS_TRACE_CALL(_t, _p, _f, _1, _2, _3)
#define _os_trace_with_payload_5(_t, _f, _1, _2, _3, _4, _p) OS_TRACE_CALL(_t, _p, _f, _1, _2, _3, _4)
#define _os_trace_with_payload_6(_t, _f, _1, _2, _3, _4, _5, _p) OS_TRACE_CALL(_t, _p, _f, _1, _2, _3, _4, _5)
#define _os_trace_with_payload_7(_t, _f, _1, _2, _3, _4, _5, _6, _p) OS_TRACE_CALL(_t, _p, _f, _1, _2, _3, _4, _5, _6)
#define _os_trace_with_payload_8(_t, _f, _1, _2, _3, _4, _5, _6, _7, _p) OS_TRACE_CALL(_t, _p, _f, _1, _2, _3, _4, _5, _6, _7)

#define _os_trace_call_n(_t, _f, ...) OS_TRACE_CALL(_t, NULL, _f, ##__VA_ARGS__)

/*!
 *
 * @abstract
 * Hashtags in trace messages
 *
 * @discussion
 * Developers are encouraged to include hashtags in log messages, regardless of
 * what API you use.
 * A hashtag is composed of a hash (#) symbol, followed by at least three
 * non-whitespace characters, terminated by whitespace or the end of the
 * message. Hashtags may not begin with a number.
 *
 * Below is the list of predefined tags:
 *   #System     - Message in the context of a system process.
 *   #User       - Message in the context of a user process.
 *   #Developer  - Message in the context of software development. For example,
 *                 deprecated APIs and debugging messages.
 *   #Attention  - Message that should be investigated by a system
 *                 administrator, because it may be a sign of a larger issue.
 *                 For example, errors from a hard drive controller that
 *                 typically occur when the drive is about to fail.
 *   #Critical   - Message in the context of a critical event or failure.
 *   #Error      - Message that is a noncritical error.
 *   #Comment    - Message that is a comment.
 *   #Marker     - Message that marks a change to divide the messages around it
 *                 into those before and those after the change.
 *   #Clue       - Message containing extra key/value pairs with additional
 *                 information to help reconstruct the context.
 *   #Security   - Message related to security concerns.
 *   #Filesystem - Message describing a file system related event.
 *   #Network    - Message describing a network-related event.
 *   #Hardware   - Message describing a hardware-related event.
 *   #CPU        - Message describing CPU related event, e.g., initiating heavy
 *                 work load
 *   #State      - Message describing state changed, e.g., global state,
 *                 preference, etc.
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
 *
 * @abstract
 * A block that populates an xpc_object_t of type XPC_TYPE_DICTIONARY to
 * represent complex data.
 *
 * @discussion
 * This block will only be invoked under conditions where tools have attached to
 * the process. The payload can be used to send arbitrary data via the trace
 * call. Tools may use the data to validate state for integration tests or
 * provide other introspection services. No assumptions are made about the
 * format or structure of the data.
 */
typedef void (^os_trace_payload_t)(os_trace_payload_object_t xdict);

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
 * <code>
 *     os_trace("network event: %ld, last seen: %ld, avg: %g",
 *             event_id, last_seen, avg);
 * </code>
 */
#define os_trace(format, ...) OS_TRACE_CALL(OS_TRACE_TYPE_RELEASE, NULL, format, ##__VA_ARGS__)

#if OS_LOG_TARGET_HAS_10_12_FEATURES
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
 * <code>
 *      os_trace_info("network interface status %ld", status);
 * </code>
 */
#define os_trace_info(format, ...) \
        OS_TRACE_CALL(OS_TRACE_TYPE_INFO, NULL, format, ##__VA_ARGS__)

#endif

/*!
 * @function os_trace_debug
 *
 * @abstract
 * Insert debug trace message into a buffer pool for later decoding.
 *
 * @discussion
 * Debug trace message to be recorded while debugger or other development tool
 * is attached to the originator.  This is transported interprocess to help
 * diagnose the entire call chain including external helpers.
 *
 * @param format
 * A printf-style format string that represents a human-readable message when
 * the trace line is decoded.  Only scalar types are supported, attempts
 * to pass arbitrary strings will store a pointer that is unresolvable and
 * will generate an error during decode.
 *
 * <code>
 *     os_trace_debug("network interface status %ld", status);
 * </code>
 */
#define os_trace_debug(format, ...) \
        OS_TRACE_CALL(OS_TRACE_TYPE_DEBUG, NULL, format, ##__VA_ARGS__)

/*!
 * @function os_trace_info_enabled
 *
 * @abstract
 * Avoid unnecessary work for a trace point by checking if additional
 * information is enabled.
 *
 * @discussion
 * Avoid unnecessary work for a trace point by checking if additional
 * information is enabled. Generally trace points should not involve expensive
 * operations, but some circumstances warrant it.  Use this function to avoid
 * doing the work unless debug level trace messages are requested.
 *
 * <code>
 *     if (os_trace_info_enabled()) {
 *         os_trace_info("value = %d, average = %d",
 *                 [[dict objectForKey: @"myKey"] intValue],
 *                 (int)[self getAverage:dict]);
 *     }
 * </code>
 *
 * @result
 * Returns true if info types are enabled.
 */
API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
bool
os_trace_info_enabled(void);

/*!
 * @function os_trace_debug_enabled
 *
 * @abstract
 * Avoid unnecessary work for a trace point by checking if debug level is
 * enabled.
 *
 * @discussion
 * Avoid unnecessary work for a trace point by checking if debug level is
 * enabled.  Generally trace points should not involve expensive operations, but
 * some circumstances warrant it.  Use this function to avoid doing the work
 * unless debug level trace messages are requested.
 *
 * <code>
 *     if (os_trace_debug_enabled()) {
 *         os_trace_debug("value = %d, average = %d",
 *                 [[dict objectForKey: @"myKey"] intValue],
 *                 (int)[self getAverage:dict]);
 *     }
 * </code>
 *
 * @result
 * Returns true if debug mode is enabled.
 */
API_AVAILABLE(macosx(10.10), ios(8.0), watchos(2.0), tvos(8.0))
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
bool
os_trace_debug_enabled(void);

/*!
 * @function os_trace_type_enabled
 *
 * @abstract
 * Avoid unnecessary work for a trace point by checking a specific type
 *
 * @discussion
 * Avoid unnecessary work for a trace point by checking a specific type
 *
 * @result
 * Returns true if type is enabled.
 */
API_AVAILABLE(macosx(10.10), ios(8.0), watchos(2.0), tvos(8.0))
OS_NOTHROW OS_WARN_RESULT OS_ALWAYS_INLINE
static inline bool
os_trace_type_enabled(uint8_t type)
{
    switch (type) {
    case OS_TRACE_TYPE_INFO:
        return os_trace_info_enabled();
    case OS_TRACE_TYPE_DEBUG:
        return os_trace_debug_enabled();
    }
    return true;
}

/*!
 * @function os_trace_error
 *
 * @abstract
 * Trace the message as an error and force a collection of the trace buffer as
 * a failure may be imminent.
 *
 * @discussion
 * Trace the message as an error and force a collection of the trace buffer as
 * a failure may be imminent.
 *
 * @param format
 * A printf-style format string to generate a human-readable log message when
 * the trace line is decoded.  Only scalar types are supported, attempts to pass
 * arbitrary strings will store a pointer that is unresolvable and will generate
 * an error during decode.
 *
 * <code>
 *     os_trace_error("socket %d connection timeout %ld", fd, secs);
 * </code>
 */
#define os_trace_error(format, ...) \
        OS_TRACE_CALL(OS_TRACE_TYPE_ERROR, NULL, format, ##__VA_ARGS__)

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
 * <code>
 *     os_trace_fault("failed to lookup uid %d - aborting", uid);
 * </code>
 */
#define os_trace_fault(format, ...) \
        OS_TRACE_CALL(OS_TRACE_TYPE_FAULT, NULL, format, ##__VA_ARGS__)

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
 * <code>
 *     os_trace_with_payload("network event %ld", event, ^(xpc_object_t xdict) {
 *
 *         // validate the network interface and address where what was expected
 *         xpc_dictionary_set_string(xdict, "network", ifp->ifa_name);
 *         xpc_dictionary_set_string(xdict, "ip_address", _get_address(ifp));
 *     });
 * </code>
 */
#define os_trace_with_payload(format, ...) \
    OS_CONCAT(_os_trace_with_payload, OS_COUNT_ARGS(__VA_ARGS__))(\
            OS_TRACE_TYPE_RELEASE, format, ##__VA_ARGS__)

#if OS_LOG_TARGET_HAS_10_12_FEATURES

#define os_trace_info_with_payload(format, ...) \
    OS_CONCAT(_os_trace_with_payload, OS_COUNT_ARGS(__VA_ARGS__))(\
            OS_TRACE_TYPE_INFO, format, ##__VA_ARGS__)

#else

API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW OS_NOT_TAIL_CALLED
void
os_trace_info_with_payload(const char *format, ...);

#endif // !OS_LOG_TARGET_HAS_10_12_FEATURES

#define os_trace_debug_with_payload(format, ...) \
    OS_CONCAT(_os_trace_with_payload, OS_COUNT_ARGS(__VA_ARGS__))(\
            OS_TRACE_TYPE_DEBUG, format, ##__VA_ARGS__)

#define os_trace_error_with_payload(format, ...) \
    OS_CONCAT(_os_trace_with_payload, OS_COUNT_ARGS(__VA_ARGS__))(\
            OS_TRACE_TYPE_ERROR, format, ##__VA_ARGS__)

#define os_trace_fault_with_payload(format, ...) \
    OS_CONCAT(_os_trace_with_payload, OS_COUNT_ARGS(__VA_ARGS__))(\
            OS_TRACE_TYPE_FAULT, format, ##__VA_ARGS__)

#endif // __has_include(<xpc/xpc.h>)

/*!
 * @function _os_trace_with_buffer
 *
 * @abstract
 * Internal function to support pre-encoded buffer.
 */
API_AVAILABLE(macosx(10.10), ios(8.0), watchos(2.0), tvos(8.0))
OS_EXPORT OS_NOTHROW OS_NOT_TAIL_CALLED
void
_os_trace_with_buffer(void *dso, const char *message, uint8_t type,
        const void *buffer, size_t buffer_size, os_trace_payload_t payload);

__END_DECLS

#endif // !__OS_TRACE_H__
