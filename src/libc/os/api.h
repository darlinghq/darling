/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

/*!
 * @header
 * API macros. libdarwin provides APIs for low-level userspace projects in the
 * Darwin operating system.
 *
 * - C language additions
 * - POSIX and BSD API additions
 * - POSIX and BSD convenience wrappers
 * - Mach API additions and wrappers with clearer semantics
 *
 * Additions which extend the C language are not prefixed and are therefore not
 * included by default when including this header.
 *
 * Additions to API families conforming to ANSI C carry the "os_" prefix.
 *
 * Additions to API families conforming to POSIX carry the "_np" ("Not POSIX")
 * suffix.
 *
 * Additions to API families conforming to both POSIX and ANSI C carry the "_np"
 * suffix.
 *
 * Convenience wrappers for POSIX and BSD APIs carry the "os_" prefix.
 *
 * New APIs formalizing Darwin workflows carry the "os_" prefix.
 */
#ifndef __DARWIN_API_H
#define __DARWIN_API_H

#include <os/availability.h>
#include <stdint.h>

/*!
 * @const DARWIN_API_VERSION
 * The API version of the library. This version will be changed in accordance
 * with new API introductions so that callers may submit code to the build that
 * adopts those new APIs before the APIs land by using the following pattern:
 *
 * #if DARWIN_API_VERSION >= 20180424
 * darwin_new_api();
 * #endif
 *
 * In this example, the libdarwin maintainer and API adopter agree on an API
 * version of 20180424 ahead of time for the introduction of
 * darwin_new_api_call(). When a libdarwin with that API version is submitted,
 * the project is rebuilt, and the new API becomes active.
 *
 * Breaking API changes will be both covered under this mechanism as well as
 * individual preprocessor macros in this header that declare new behavior as
 * required.
 */
#define DARWIN_API_VERSION 20191015u

#if !DARWIN_BUILDING_LIBSYSTEM_DARWIN
#define DARWIN_API_AVAILABLE_20170407 \
		API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
#define DARWIN_API_AVAILABLE_20180727 \
		API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0))
#define DARWIN_API_AVAILABLE_20181020 \
		API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0))
#define DARWIN_API_AVAILABLE_20181020 \
		API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0))
#define DARWIN_API_AVAILABLE_20190830 \
		API_AVAILABLE(macos(10.15.2), ios(13.3), tvos(13.3), watchos(6.1.1))
#define DARWIN_API_AVAILABLE_20191015 \
		API_AVAILABLE(macos(10.15.2), ios(13.3), tvos(13.3), watchos(6.1.1))
#else
#define DARWIN_API_AVAILABLE_20170407
#define DARWIN_API_AVAILABLE_20180727
#define DARWIN_API_AVAILABLE_20181020
#define DARWIN_API_AVAILABLE_20190830
#define DARWIN_API_AVAILABLE_20191015
#endif

/*!
 * @typedef os_struct_magic_t
 * A type representing the magic number of a transparent structure.
 */
typedef uint32_t os_struct_magic_t;

/*!
 * @typedef os_struct_version_t
 * A type representing the version of a transparent structure.
 */
typedef uint32_t os_struct_version_t;

#endif // __DARWIN_API_H
