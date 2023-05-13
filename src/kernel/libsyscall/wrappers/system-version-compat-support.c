/*
 * Copyright (c) 2020 Apple Inc. All rights reserved.
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
#include <TargetConditionals.h>

#if TARGET_OS_OSX && !defined(__i386__)

/*
 * Support for the open compatibilty shim for macOS. These NULL
 * function pointers need to be built into libsyscall_static. They
 * are hooked up to the actual functions from libsyscall_dynamic
 * if/when they are used (generally only for older binaries where we
 * need to shim the version information).
 */

#include "system-version-compat-support.h"
#include <stdbool.h>
#include <sys/param.h>
#include <sys/types.h>

__attribute__((visibility("hidden")))
bool (*system_version_compat_check_path_suffix)(const char *orig_path) = NULL;
system_version_compat_mode_t system_version_compat_mode = SYSTEM_VERSION_COMPAT_MODE_DISABLED;

__attribute__((visibility("hidden")))
int (*system_version_compat_open_shim)(int opened_fd, int openat_fd, const char *orig_path, int oflag, mode_t mode,
    int (*close_syscall)(int), int (*open_syscall)(const char *, int, mode_t),
    int (*openat_syscall)(int, const char *, int, mode_t),
    int (*fcntl_syscall)(int, int, long)) = NULL;
#endif /* TARGET_OS_OSX && && !defined(__i386__) */
