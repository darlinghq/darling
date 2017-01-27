/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef __os_log_h
#define __os_log_h

#include <os/object.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_attribute(not_tail_called)
#define OS_LOG_NOTAILCALL __attribute__((not_tail_called))
#define OS_LOG_NOTAILCALL_MARKER
#else
#define OS_LOG_NOTAILCALL
#define OS_LOG_NOTAILCALL_MARKER __asm__("")
#endif

__BEGIN_DECLS

extern void *__dso_handle;

OS_ALWAYS_INLINE static inline void _os_log_verify_format_str(__unused const char *msg, ...) __attribute__((format(os_trace, 1, 2)));
OS_ALWAYS_INLINE static inline void _os_log_verify_format_str(__unused const char *msg, ...) { /* placeholder */ }

#if OS_OBJECT_USE_OBJC
OS_OBJECT_DECL(os_log);
#else
typedef struct os_log_s *os_log_t;
#endif /* OS_OBJECT_USE_OBJC */

/*!
 * @const OS_LOG_DISABLED
 *
 * @discussion
 * Use this to disable a specific log message.
 */
#define OS_LOG_DISABLED NULL

/*!
 * @const OS_LOG_DEFAULT
 *
 * @discussion
 * Use this to log a message in accordance with current system settings.
 */
#define OS_LOG_DEFAULT OS_OBJECT_GLOBAL_OBJECT(os_log_t, _os_log_default)
__OSX_AVAILABLE_STARTING(__MAC_10_12,__IPHONE_10_0)
OS_EXPORT
struct os_log_s _os_log_default;

/*!
 * @enum os_log_type_t
 *
 * @discussion
 * Supported log message types.
 *
 * @constant OS_LOG_TYPE_DEFAULT
 * Equivalent type for "os_log()" messages, i.e., default messages that are always
 * captured to memory or disk.
 *
 * @constant OS_LOG_TYPE_INFO
 * Equivalent type for "os_log_info()" messages, i.e., Additional informational messages.
 *
 * @constant OS_LOG_TYPE_DEBUG
 * Equivalent type for "os_log_debug()" messages, i.e., Debug messages.
 *
 * @constant OS_LOG_TYPE_ERROR
 * Equivalent type for "os_log_error()" messages, i.e., local process error messages.
 *
 * @constant OS_LOG_TYPE_FAULT
 * Equivalent type for "os_log_fault()" messages, i.e., a system error that involves
 * potentially more than one process, usually used by daemons and services.
 */
OS_ENUM(os_log_type, uint8_t,
        OS_LOG_TYPE_DEFAULT = 0x00,
        OS_LOG_TYPE_INFO    = 0x01,
        OS_LOG_TYPE_DEBUG   = 0x02,
        OS_LOG_TYPE_ERROR   = 0x10,
        OS_LOG_TYPE_FAULT   = 0x11);

/*!
 * @function os_log_create
 *
 * @abstract
 * Creates a log object to be used with other log related functions.
 *
 * @discussion
 * Creates a log object to be used with other log related functions.  The
 * log object serves two purposes:  (1) tag related messages by subsystem
 * and category name for easy filtering, and (2) control logging system
 * behavior for messages.
 *
 * A log object may customize logging system behavior for its messages by
 * adding a configuration file in /Library/LogPreferences. Most options 
 * accept 3 values: "Default", "Yes" or "No" as strings, where "Default"
 * signifies follow system behavior for the level of messages.
 *
 * For log:
 *
 *      os_log_create("com.company.mysubsystem", "connections");
 *
 * System-provided preferences are located in /System/Library/LogPreferences/<subsystem>.plist
 *
 *	<dict>
 *
 *          <!-- Default options applied to message types in each category, which can be overriden. -->
 *          <key>DEFAULT-OPTIONS</key>
 *          <dict>
 *              <key>Enabled</key>              <!-- Enabled state follows system defaults -->
 *              <string>Default</string>
 *              <key>Persist</key>              <!-- Do not persist to disk, use memory-only buffer if enabled -->
 *              <string>No</string>
 *              <key>TTL</key>          	<!-- Follow system default behavior if persistence is enabled -->
 *              <string>Default</string>        <!-- Can specify in days with "d" or hours "h" (e.g., "4h" = 4 hours) -->
 *          </dict>
 *
 *          <!-- category named “connections” -->
 *          <key>connections</key>
 *          <dict>
 *
 *              <!-- Options that control "os_log()" behavior.  The "Enabled" option is ignored. -->
 *              <key>Default</key>
 *              <dict>
 *                  <key>Persist</key>          <!-- Always persist to disk -->
 *                  <string>Yes</string>
 *                  <key>TTL</key>              <!-- Store default messages for maximum 4 days -->
 *                  <integer>4d</integer>
 *              </dict>
 *
 *              <!-- Subdictionary of options that control "os_log_info()" behavior -->
 *              <key>Info</key>
 *              <dict>
 *                  <key>Persist</key>          <!-- If enabled persist to disk -->
 *                  <string>Yes</string>
 *                  <key>TTL</key>          	<!-- Store Info messages for 2 days -->
 *                  <string>2d</string>
 *              </dict>
 *
 *              <!-- Subdictionary of options that control "os_log_debug()" behavior -->
 *              <key>Debug</key>
 *              <dict>
 *                  <key>Enabled</key>          <!-- Not enabled, must be enabled specifically -->
 *                  <string>No</string>
 *              </dict>
 *          </dict>
 *      </dict>
 *
 * All other preferences and system-overrides are stored in /Library/LogPreferences/.
 *
 * @param subsystem
 * The identifier of the given subsystem should be in reverse DNS form
 * (i.e., com.company.mysubsystem).  This string must be a constant string,
 * not dynamically generated.
 *
 * @param category
 * The category within the given subsystem that specifies the settings for
 * the log object.  This string must be a constant string, not dynamically
 * generated.
 *
 * @result
 * Returns an os_log_t value to be passed to other os_log API calls.  This
 * should be called once at log initialization and rely on system to detect
 * changes to settings.  This object should be released when no longer used
 * via os_release or -[release] method.
 *
 * A value will always be returned to allow for dynamic enablement.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12,__IPHONE_10_0)
OS_EXPORT OS_NOTHROW OS_WARN_RESULT OS_OBJECT_RETURNS_RETAINED
os_log_t
os_log_create(const char *subsystem, const char *category);

/*!
 * @function os_log_info_enabled
 *
 * @abstract
 * Returns if development log messages are enabled for a particular log object.
 *
 * @discussion
 * Returns if development log messages are enabled for a particular log object.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @result
 * Returns ‘true’ if debug log messages are enabled.
 */
__WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
bool
os_log_info_enabled(os_log_t log);

/*!
 * @function os_log_debug_enabled
 *
 * @abstract
 * Returns if debug log messages are enabled for a particular log object.
 *
 * @discussion
 * Returns if debug log messages are enabled for a particular log object.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @result
 * Returns ‘true’ if debug log messages are enabled.
 */
__WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
bool
os_log_debug_enabled(os_log_t log);

/*!
 * @function os_log
 *
 * @abstract
 * Insert a log message into the Unified Logging and Tracing system.
 *
 * @discussion
 * Insert a log message into the Unified Logging and Tracing system in 
 * accordance with the preferences specified by the provided log object.
 * These messages cannot be disabled and therefore always captured either
 * to memory or disk.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 256 bytes per entry for dynamic content,
 * i.e., %s and %@, that can be written to the persistence store.  As such,
 * all content exceeding the limit will be truncated before written to disk.
 * Live streams will continue to show the full content.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log(log, format, ...) __extension__({                                                \
    _Static_assert(__builtin_constant_p(format), "format string must be constant");             \
    __attribute__((section("__TEXT,__os_log"))) static const char _os_log_fmt[] = format;       \
    _os_log_verify_format_str(format, ##__VA_ARGS__);                                           \
    _os_log_internal(&__dso_handle, log, OS_LOG_TYPE_DEFAULT, _os_log_fmt, ##__VA_ARGS__);      \
    __asm__(""); /* avoid tailcall */                                                           \
})

/*!
 * @function os_log_info
 *
 * @abstract
 * Insert a development log message into the Unified Logging and Tracing system.
 *
 * @discussion
 * Insert a log message into the Unified Logging and Tracing system in 
 * accordance with the preferences specified by the provided log object.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 256 bytes per entry for dynamic content,
 * i.e., %s and %@, that can be written to the persistence store.  As such,
 * all content exceeding the limit will be truncated before written to disk.
 * Live streams will continue to show the full content.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log_info(log, format, ...) __extension__({                                       \
    _Static_assert(__builtin_constant_p(format), "format string must be constant");         \
    __attribute__((section("__TEXT,__os_log"))) static const char _os_log_fmt[] = format;   \
    _os_log_verify_format_str(format, ##__VA_ARGS__);                                       \
    _os_log_internal(&__dso_handle, log, OS_LOG_TYPE_INFO, _os_log_fmt, ##__VA_ARGS__);     \
    __asm__(""); /* avoid tailcall */                                                       \
})

/*!
 * @function os_log_debug
 *
 * @abstract
 * Insert a debug log message into the Unified Logging and Tracing system.
 *
 * @discussion
 * Insert a debug log message into the Unified Logging and Tracing system in
 * accordance with the preferences specified by the provided log object.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 256 bytes per entry for dynamic content,
 * i.e., %s and %@, that can be written to the persistence store.  As such,
 * all content exceeding the limit will be truncated before written to disk.
 * Live streams will continue to show the full content.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log_debug(log, format, ...) __extension__({                                          \
    _Static_assert(__builtin_constant_p(format), "format string must be constant");             \
    __attribute__((section("__TEXT,__os_log"))) static const char _os_log_fmt[] = format;       \
    _os_log_verify_format_str(format, ##__VA_ARGS__);                                           \
    _os_log_internal(&__dso_handle, log, OS_LOG_TYPE_DEBUG, _os_log_fmt, ##__VA_ARGS__);        \
    __asm__(""); /* avoid tailcall */                                                           \
})

/*!
 * @function os_log_error
 *
 * @abstract
 * Insert an error log message into the Unified Logging and Tracing system.
 *
 * @discussion
 * Insert an error log message into the Unified Logging and Tracing system.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 256 bytes per entry for dynamic content,
 * i.e., %s and %@, that can be written to the persistence store.  As such,
 * all content exceeding the limit will be truncated before written to disk.
 * Live streams will continue to show the full content.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log_error(log, format, ...) __extension__({                                          \
    _Static_assert(__builtin_constant_p(format), "format string must be constant");             \
    __attribute__((section("__TEXT,__os_log"))) static const char _os_log_fmt[] = format;       \
    _os_log_verify_format_str(format, ##__VA_ARGS__);                                           \
    _os_log_internal(&__dso_handle, log, OS_LOG_TYPE_ERROR, _os_log_fmt, ##__VA_ARGS__);        \
    __asm__(""); /* avoid tailcall */                                                           \
})

/*!
 * @function os_log_fault
 *
 * @abstract
 * Insert a fault log message into the Unified Logging and Tracing system.
 *
 * @discussion
 * Log a fault message issue into the Unified Logging and Tracing system
 * signifying a multi-process (i.e., system error) related issue, either
 * due to interaction via IPC or some other.  Faults will gather information 
 * from the entire process chain and record it for later inspection.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 256 bytes per entry for dynamic content,
 * i.e., %s and %@, that can be written to the persistence store.  As such,
 * all content exceeding the limit will be truncated before written to disk.
 * Live streams will continue to show the full content.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log_fault(log, format, ...) __extension__({                                          \
    _Static_assert(__builtin_constant_p(format), "format string must be constant");             \
    __attribute__((section("__TEXT,__os_log"))) static const char _os_log_fmt[] = format;       \
    _os_log_verify_format_str(format, ##__VA_ARGS__);                                           \
    _os_log_internal(&__dso_handle, log, OS_LOG_TYPE_FAULT, _os_log_fmt, ##__VA_ARGS__);        \
    __asm__(""); /* avoid tailcall */                                                           \
})

/*!
 * @function os_log_with_type
 *
 * @abstract
 * Log a message using a specific type.
 *
 * @discussion
 * Will log a message with the provided os_log_type_t.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param type
 * Pass a valid type from os_log_type_t.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log_with_type(log, type, format, ...) __extension__({                            \
    _Static_assert(__builtin_constant_p(format), "format string must be constant");         \
    __attribute__((section("__TEXT,__os_log"))) static const char _os_log_fmt[] = format;   \
    _os_log_verify_format_str(format, ##__VA_ARGS__);                                       \
    _os_log_internal(&__dso_handle, log, type, _os_log_fmt, ##__VA_ARGS__);                 \
    __asm__(""); /* avoid tailcall */                                                       \
})

/*!
 * @function os_log_sensitive_debug
 *
 * @abstract
 * Insert a debug log message containing sensitive content (i.e., personal
 * identifying information).
 *
 * @discussion
 * Insert a debug log message containing sensitive content (i.e., personal
 * identifying information) in accordance with the preferences specified by
 * the provided log object.
 *
 * All strings are considered potentially sensitive, though this call
 * specifically signifies the message as containing sensitive content.
 * The message will be stored separately from other messages.
 *
 * When an os_activity_id_t is present, the log message will also be scoped by
 * that identifier.  Activities provide granular filtering of log messages
 * across threads and processes.
 *
 * There is a physical cap of 256 bytes per entry for dynamic content,
 * i.e., %s and %@, that can be written to the persistence store.  As such,
 * all content exceeding the limit will be truncated before written to disk.
 * Live streams will continue to show the full content.
 *
 * @param log
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  This string must be a constant string, not dynamically
 * generated.  Supports all standard printf types and %@ (objects).
 */
#define os_log_sensitive_debug(log, format, ...) __extension__({                                                      \
    _Static_assert(__builtin_constant_p(format), "format string must be constant");                                   \
    __attribute__((section("__TEXT,__os_log_sens"))) static const char _os_log_fmt[] = format;                        \
    _os_log_verify_format_str(format, ##__VA_ARGS__);                                                                 \
    _os_log_sensitive(&__dso_handle, log, OS_LOG_TYPE_DEBUG, _os_log_fmt, ##__VA_ARGS__);                             \
    __asm__(""); /* avoid tailcall */                                                                                 \
})

/*!
 * @function _os_log_internal
 *
 * @abstract
 * Internal function used by macros.
 */
__WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
OS_EXPORT OS_NOTHROW
void
_os_log_internal(void *dso, os_log_t log, os_log_type_t type, const char *message, ...);

__END_DECLS

#endif /* __os_log_h */
