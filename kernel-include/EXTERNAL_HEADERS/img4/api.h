/*!
 * @header
 * API definitions.
 */
#ifndef __IMG4_API_H
#define __IMG4_API_H

#ifndef __IMG4_INDIRECT
#error "Please #include <img4/img4.h> instead of this file directly"
#endif // __IMG4_INDIRECT

#if IMG4_TAPI
#include <stdint.h>
#endif

#ifndef KERNEL
#include <os/availability.h>
#endif

#if !XNU_KERNEL_PRIVATE
#include <TargetConditionals.h>
#endif

/*!
 * @const IMG4_API_VERSION
 * The API version of the library. This version will be changed in accordance
 * with new API introductions so that callers may submit code to the build that
 * adopts those new APIs before the APIs land by using the following pattern:
 *
 *     #if IMG4_API_VERSION >= 20180424
 *     img4_new_api();
 *     #endif
 *
 * In this example, the library maintainer and API adopter agree on an API
 * version of 20180424 ahead of time for the introduction of
 * img4_new_api(). When a libdarwin with that API version is submitted, the
 * project is rebuilt, and the new API becomes active.
 *
 * Breaking API changes will be both covered under this mechanism as well as
 * individual preprocessor macros in this header that declare new behavior as
 * required.
 */
#define IMG4_API_VERSION (20191001u)

#if !defined(KERNEL) && !IMG4_PROJECT_BUILD
#define IMG4_API_AVAILABLE_20180112 \
		__API_UNAVAILABLE(macos) \
		API_AVAILABLE(ios(12.0), tvos(12.0), watchos(5.0))
#define IMG4_API_AVAILABLE_20181004 \
		__API_UNAVAILABLE(macos) \
		API_AVAILABLE(ios(12.2), tvos(12.2), watchos(5.2))
#define IMG4_API_AVAILABLE_20181106 \
		__API_UNAVAILABLE(macos) \
		API_AVAILABLE(ios(12.2), tvos(12.2), watchos(5.2))
#define IMG4_API_AVAILABLE_20190125 \
		API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0))
#define IMG4_API_AVAILABLE_20191001 \
		API_AVAILABLE(macos(10.15.2), ios(13.3), tvos(13.3), watchos(6.1.1))
#else
#define IMG4_API_AVAILABLE_20180112
#define IMG4_API_AVAILABLE_20181004
#define IMG4_API_AVAILABLE_20181106
#define IMG4_API_AVAILABLE_20190125
#define IMG4_API_AVAILABLE_20191001
#endif // !defined(KERNEL) && !IMG4_PROJECT_BUILD

#if !defined(OS_CLOSED_ENUM)
#define OS_CLOSED_ENUM(_name, _type, ...) \
		OS_ENUM(_name, _type, ## __VA_ARGS__)
#endif

#if !defined(OS_CLOSED_OPTIONS)
#define OS_CLOSED_OPTIONS(_name, _type, ...) \
		OS_ENUM(_name, _type, ## __VA_ARGS__)
#endif

/*!
 * @typedef img4_struct_version_t
 * A type describing the version of a structure in the library.
 */
IMG4_API_AVAILABLE_20180112
typedef uint16_t img4_struct_version_t;

#endif // __IMG4_API_H
