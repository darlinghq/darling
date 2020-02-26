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

#ifndef __OS_ACTIVITY_H__
#define __OS_ACTIVITY_H__

#include <os/trace_base.h>

__BEGIN_DECLS

OS_ASSUME_NONNULL_BEGIN

#pragma mark - types and globals

#if OS_LOG_TARGET_HAS_10_12_FEATURES
#define OS_ACTIVITY_OBJECT_API 1
#else
#if OS_ACTIVITY_OBJECT_API
#error Please change your minimum OS requirements because OS_ACTIVITY_OBJECT_API is not available
#endif // OS_ACTIVITY_OBJECT_API
#define OS_ACTIVITY_OBJECT_API 0
#endif

/*!
 * @enum os_activity_flag_t
 *
 * @discussion
 * Support flags for os_activity_create or os_activity_start.
 *
 * @constant OS_ACTIVITY_FLAG_DEFAULT
 * Use the default flags.
 *
 * @constant OS_ACTIVITY_FLAG_DETACHED
 * Detach the newly created activity from the provided activity (if any).  If
 * passed in conjunction with an exiting activity, the activity will only note
 * what activity "created" the new one, but will make the new activity a top
 * level activity.  This allows users to see what activity triggered work
 * without actually relating the activities.
 *
 * @constant OS_ACTIVITY_FLAG_IF_NONE_PRESENT
 * Will only create a new activity if none present.  If an activity ID is
 * already present, a new object will be returned with the same activity ID
 * underneath.
 *
 * Passing both OS_ACTIVITY_FLAG_DETACHED and OS_ACTIVITY_FLAG_IF_NONE_PRESENT
 * is undefined.
 */
OS_ENUM(os_activity_flag, uint32_t,
    OS_ACTIVITY_FLAG_DEFAULT = 0,
    OS_ACTIVITY_FLAG_DETACHED = 0x1,
    OS_ACTIVITY_FLAG_IF_NONE_PRESENT = 0x2
);

#if OS_ACTIVITY_OBJECT_API

#define OS_ACTIVITY_NULL NULL

/*!
 * @typedef os_activity_t
 * An opaque activity object.
 */
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
#if OS_OBJECT_USE_OBJC
OS_OBJECT_DECL(os_activity);
#else
typedef struct os_activity_s *os_activity_t;
#endif /* OS_OBJECT_USE_OBJC */

/*!
 * @const OS_ACTIVITY_NONE
 *
 * @discussion
 * Create activity with no current traits, this is the equivalent of a
 * detached activity.
 */
#define OS_ACTIVITY_NONE OS_OBJECT_GLOBAL_OBJECT(os_activity_t, _os_activity_none)
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT
const struct os_activity_s _os_activity_none;

/*!
 * @const OS_ACTIVITY_CURRENT
 *
 * @discussion
 * Create activity and links to the current activity if one is present.
 * If no activity is present it is treated as if it is detached.
 */
#define OS_ACTIVITY_CURRENT OS_OBJECT_GLOBAL_OBJECT(os_activity_t, _os_activity_current)
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT
const struct os_activity_s _os_activity_current;

#else // !OS_ACTIVITY_OBJECT_API

#define OS_ACTIVITY_NULL 0

/*!
 * @typedef os_activity_t
 * An opaque activity identifier.
 */
API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0))
typedef uint64_t os_activity_t;

#endif // OS_ACTIVITY_OBJECT_API

/*!
 * @typedef os_activity_id_t
 * An value representing the activity ID assigned to an newly created activity.
 */
API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0))
typedef uint64_t os_activity_id_t;

/*!
 * @typedef os_activity_scope_state_t
 * Structure that is populated by os_activity_scope_enter and restored using
 * os_activity_scope_leave.
 */
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
typedef struct os_activity_scope_state_s {
    uint64_t opaque[2];
} *os_activity_scope_state_t;

#pragma mark - Internal support functions

#if OS_ACTIVITY_OBJECT_API
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW OS_WARN_RESULT_NEEDS_RELEASE OS_NOT_TAIL_CALLED
OS_OBJECT_RETURNS_RETAINED
os_activity_t
_os_activity_create(void *dso, const char *description, os_activity_t activity,
        os_activity_flag_t flags);
#endif

/*!
 * @function _os_activity_label_useraction
 *
 * @abstract
 * Internal function for use by os_activity_label_useraction.
 *
 * @warning
 * Do not use directly.
 */
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW OS_NOT_TAIL_CALLED
void
_os_activity_label_useraction(void *dso, const char *name);

/*!
 * @function _os_activity_initiate
 *
 * @abstract
 * Do not use directly because your description will not be preserved.
 */
API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0))
OS_EXPORT OS_NOTHROW OS_NOT_TAIL_CALLED
void
_os_activity_initiate(void *dso, const char *description,
        os_activity_flag_t flags, os_block_t activity_block OS_NOESCAPE);

/*!
 * @function _os_activity_initiate_f
 *
 * @abstract
 * Do not use directly because your description will not be preserved.
 */
API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0))
OS_EXPORT OS_NOTHROW OS_NOT_TAIL_CALLED
void
_os_activity_initiate_f(void *dso, const char *description,
        os_activity_flag_t flags, void *context, os_function_t function);

#pragma mark - activity related

/*!
 * @function os_activity_initiate
 *
 * @abstract
 * Synchronously initiates an activity using provided block.
 *
 * @discussion
 * Synchronously initiates an activity using the provided block and creates
 * a tracing buffer as appropriate.  All new activities are created as a
 * subactivity of an existing activity on the current thread.
 *
 * <code>
 *     os_activity_initiate("indexing database", OS_ACTIVITY_FLAG_DEFAULT, ^(void) {
 *         // either do work directly or issue work asynchronously
 *     });
 * </code>
 *
 * @param description
 * A constant string describing the activity, e.g., "performClick" or
 * "menuSelection".
 *
 * @param flags
 * Flags to be used when initiating the activity, typically
 * OS_ACTIVITY_FLAG_DEFAULT.
 *
 * @param activity_block
 * The block to execute a given activity
 */
#define os_activity_initiate(description, flags, activity_block) __extension__({ \
    OS_LOG_STRING(ACT, __description, description); \
    _os_activity_initiate(&__dso_handle, __description, flags, activity_block); \
})

/*!
 * @function os_activity_initiate_f
 *
 * @abstract
 * Synchronously initiates an activity using the provided function.
 *
 * @discussion
 * Synchronously initiates an activity using the provided function and creates
 * a tracing buffer as appropriate.  All new activities are created as a
 * subactivity of an existing activity on the current thread.
 *
 * <code>
 *     os_activity_initiate_f("indexing database", OS_ACTIVITY_FLAG_DEFAULT, context, function);
 * </code>
 *
 * @param description
 * A constant string describing the activity, e.g., "performClick" or
 * "menuSelection".
 *
 * @param flags
 * Flags to be used when initiating the activity, typically
 * OS_ACTIVITY_FLAG_DEFAULT.
 *
 * @param context
 * An optional context that will be supplied to the activity function.
 *
 * @param activity_func
 * The function to execute for the new activity.
 */
#define os_activity_initiate_f(description, flags, context, function) __extension__({ \
    OS_LOG_STRING(ACT, __description, description); \
    _os_activity_initiate_f(&__dso_handle, __description, flags, context, function); \
})

#if OS_ACTIVITY_OBJECT_API
/*!
 * @function os_activity_create
 *
 * @abstract
 * Creates an os_activity_t object which can be passed to os_activity_apply
 * function.
 *
 * @param description
 * Pass a description for the activity.  The description must be a constant
 * string within the calling executable or library.
 *
 * @param parent_activity
 * Depending on flags will link the newly created activity to the value passed
 * or note where the activity was created.  Possible activities include:
 * OS_ACTIVITY_NONE, OS_ACTIVITY_CURRENT or any existing os_activity_t object
 * created using os_activity_create.
 *
 * @param flags
 * A valid os_activity_flag_t which will determine behavior of the newly created
 * activity.
 *
 * If the OS_ACTIVITY_FLAG_DETACHED flag is passed, the value passed to the
 * parent_activity argument is ignored, and OS_ACTIVITY_NONE is used instead.
 *
 * If the OS_ACTIVITY_FLAG_IF_NONE_PRESENT flag is passed, then passing another
 * value than OS_ACTIVITY_CURRENT to the parent_activity argument is undefined.
 *
 * @result
 * Returns an os_activity_t object which can be used with os_activity_apply.
 */
#define os_activity_create(description, parent_activity, flags) __extension__({ \
    OS_LOG_STRING(ACT, __description, description); \
    _os_activity_create(&__dso_handle, __description, parent_activity, flags); \
})

/*!
 * @function os_activity_apply
 *
 * @abstract
 * Execute a block using a given activity object.
 *
 * @param activity
 * The given activity object created with os_activity_create() or
 * OS_ACTIVITY_NONE.
 *
 * @param block
 * Pass the block to be executed within the context of the given activity.
 */
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW
void
os_activity_apply(os_activity_t activity, os_block_t block OS_NOESCAPE);

/*!
 * @function os_activity_apply_f
 *
 * @abstract
 * Execute a given function with a provided activity.
 *
 * @param activity
 * The given activity object created with os_activity_create() or
 * OS_ACTIVITY_NONE.
 *
 * @param context
 * Context to pass to the function which may be NULL.
 *
 * @param function
 * Pass the function to be executed within the context of the given activity.
 */
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW
void
os_activity_apply_f(os_activity_t activity, void *context,
        os_function_t function);

/*!
 * @function os_activity_scope_enter
 *
 * @abstract
 * Will change the current execution context to use the provided activity.
 *
 * @discussion
 * Will change the current execution context to use the provided activity.
 * An activity can be created and then applied to the current scope by doing:
 *
 * <code>
 *     struct os_activity_scope_state_s state;
 *     os_activity_t activity = os_activity_create("my new activity", 0);
 *     os_activity_scope_enter(activity, &state);
 *     ... do some work ...
 *     os_activity_scope_leave(&state);
 * </code>
 *
 * To auto-cleanup state call:
 *
 *      os_activity_scope(activity);
 *
 * @param activity
 * Pass a valid activity created with os_activity_create or any global object.
 *
 * @param state
 * A stack-based struct os_activity_scope_state_s to store the state.
 */
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW
void
os_activity_scope_enter(os_activity_t activity, os_activity_scope_state_t state);

/*!
 * @function os_activity_scope_leave
 *
 * @abstract
 * Will pop state up to the state provided.
 *
 * @discussion
 * Will leave scope using the state provided.  If state is not present an error
 * will be generated.
 *
 * @param state
 * Must be a valid value filled by os_activity_scope_enter call.
 */
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW
void
os_activity_scope_leave(os_activity_scope_state_t state);

#if defined(__GNUC__)
#define _os_activity_scope(var, activity) \
    struct os_activity_scope_state_s var __attribute__((__cleanup__(os_activity_scope_leave))); \
    os_activity_scope_enter(activity, &var)
#define os_activity_scope(activity) _os_activity_scope(OS_CONCAT(scope, __COUNTER__), activity)
#endif

#endif // OS_ACTIVITY_OBJECT_API

/*!
 * @function os_activity_get_active
 *
 * @abstract
 * Returns the stack of nested activities associated with the current thread.
 *
 * @discussion
 * Activities have a sense of nesting and therefore there could be more than
 * one activity involved on the current thread.  This should be used by
 * diagnostic tools only for making additional decisions about a situation.
 *
 * @param entries
 * Pass a buffer of sufficient size to hold the the number of os_activity_id_t
 * being requested.
 *
 * @param count
 * Pointer to the requested number of activity identifiers.
 * On output will be filled with the number of activities that are available.
 *
 * @result
 * Number of activity identifiers written to 'entries'
 */
API_DEPRECATED("No longer supported", macos(10.10, 10.12), ios(8.0, 10.0),
        watchos(2.0, 3.0), tvos(9.0, 10.0))
OS_EXPORT OS_NOTHROW
unsigned int
os_activity_get_active(os_activity_id_t *entries, unsigned int *count);

/*!
 * @function os_activity_get_identifier
 *
 * @abstract
 * Returns the current activity ID and will fill the parent_id if present.
 *
 * @discussion
 * Returns the current activity ID and will fill the parent_id if present.
 *
 * @param parent_id
 * If non-null will set the parent activity ID.
 *
 * @result
 * The identifier for the provided activity.
 */
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_NOTHROW OS_WARN_RESULT
os_activity_id_t
os_activity_get_identifier(os_activity_t activity,
        os_activity_id_t *_Nullable parent_id);

/*!
 * @function os_activity_label_useraction
 *
 * @abstract
 * Label an activity that is auto-generated by AppKit/UIKit with a name that is
 * useful for debugging macro-level user actions.
 *
 * @discussion
 * Label an activity that is auto-generated by AppKit/UIKit with a name that is
 * useful for debugging macro-level user actions.  The API should be called
 * early within the scope of the IBAction and before any sub-activities are
 * created.  The name provided will be shown in tools in additon to the
 * underlying AppKit/UIKit provided name.  This API can only be called once and
 * only on the activity created by AppKit/UIKit.  These actions help determine
 * workflow of the user in order to reproduce problems that occur.  For example,
 * a control press and/or menu item selection can be labeled:
 *
 * <code>
 *     os_activity_label_useraction("New mail message");
 *     os_activity_label_useraction("Empty trash");
 * </code>
 *
 * Where the underlying AppKit/UIKit name will be "gesture:" or "menuSelect:".
 *
 * @param name
 * A constant string that describes the the action.
 */
#define os_activity_label_useraction(label) __extension__({ \
    OS_LOG_STRING(ACT, __label, label); \
    _os_activity_label_useraction(&__dso_handle, __label); \
})

#pragma mark - deprecated function support

/*!
 * @function _os_activity_start
 *
 * @abstract
 * Internal function for activity start, do not use directly will not preserve
 * description.
 */
API_DEPRECATED("use combination of os_activity_create and os_activity_apply/os_activity_scope",
        macos(10.10, 10.12), ios(8.0, 10.0), watchos(2.0, 3.0), tvos(9.0, 10.0))
OS_EXPORT OS_NOTHROW OS_WARN_RESULT OS_NOT_TAIL_CALLED
os_activity_t
_os_activity_start(void *dso, const char *description, os_activity_flag_t flags);

/*!
 * @function os_activity_start
 *
 * @warning
 * Deprecated please use new os_activity_create and os_activity_apply.
 */
#define os_activity_start(description, flags) __extension__({ \
    OS_LOG_STRING(ACT, __description, description); \
    _os_activity_start(&__dso_handle, __description, flags); \
})

/*!
 * @function os_activity_end
 *
 * @warning
 * Deprecated please use new os_activity_create and os_activity_apply.
 */
API_DEPRECATED("use combination of os_activity_create and os_activity_apply/os_activity_scope",
        macos(10.10, 10.12), ios(8.0, 10.0), watchos(2.0, 3.0), tvos(9.0, 10.0))
OS_EXPORT OS_NOTHROW
void
os_activity_end(os_activity_t activity);

/*!
 * @typedef os_breadcrumb_t
 * An opaque value for the breadcrumb ID.
 */
API_DEPRECATED("No longer supported", macos(10.10, 10.12), ios(8.0, 10.0),
        watchos(2.0, 3.0), tvos(9.0, 10.0))
typedef uint32_t os_breadcrumb_t;

/*!
 * @function _os_activity_set_breadcrumb
 *
 * @warning
 * Deprecated, please use os_activity_label_useraction.
 */
API_DEPRECATED_WITH_REPLACEMENT("os_activity_label_useraction",
        macos(10.10, 10.12), ios(8.0, 10.0), watchos(2.0, 3.0), tvos(9.0, 10.0))
OS_EXPORT OS_NOTHROW OS_NOT_TAIL_CALLED
void
_os_activity_set_breadcrumb(void *dso, const char *name);

/*!
 * @function os_activity_set_breadcrumb
 *
 * @warning
 * Deprecated, please use os_activity_label_useraction.
 */
#define os_activity_set_breadcrumb(name) __extension__({ \
    OS_LOG_STRING(ACT, __name, name); \
    _os_activity_set_breadcrumb(&__dso_handle, __name); \
})

OS_ASSUME_NONNULL_END

__END_DECLS

#endif // __OS_ACTIVITY_H__
