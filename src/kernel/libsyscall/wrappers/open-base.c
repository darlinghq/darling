/*
 * Copyright (c) 2020 Apple Inc. All rights reserved.
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

#include <fcntl.h>
#include <stdarg.h>
#include <sys/param.h>
#include <sys/types.h>
#include <TargetConditionals.h>

#if !defined(__i386__)

#if TARGET_OS_OSX
#include <stdbool.h>

/*
 * On macOS we have support for shimming calls to open the SystemVersion plist.
 * This support is enabled for specific (generally older) binaries from
 * libSystem_initializer()/__libkernel_init_late() by populating these function pointers
 * with the corresponding functions built into libsyscall_dynamic (see open-compat-shim.c).
 */
extern bool (*system_version_compat_check_path_suffix)(const char *orig_path);
extern int (*system_version_compat_open_shim)(int opened_fd, int openat_fd, const char *orig_path, int oflag, mode_t mode,
    int (*close_syscall)(int), int (*open_syscall)(const char *, int, mode_t),
    int (*openat_syscall)(int, const char *, int, mode_t),
    int (*fcntl_syscall)(int, int, long));
#endif /* TARGET_OS_OSX */

#ifdef VARIANT_CANCELABLE
int __open(const char *path, int oflag, mode_t mode);
int __openat(int fd, const char *path, int oflag, mode_t mode);

#define OPEN_SYSCALL __open
#define OPENAT_SYSCALL __openat

#if TARGET_OS_OSX
int __fcntl(int fd, int cmd, long arg);
int close(int fd);

#define FCNTL_SYSCALL __fcntl
#define CLOSE_SYSCALL close
#endif /* TARGET_OS_OSX */

#else /* VARIANT_CANCELABLE */
int __open_nocancel(const char *path, int oflag, mode_t mode);
int __openat_nocancel(int fd, const char *path, int oflag, mode_t mode);

#define OPEN_SYSCALL __open_nocancel
#define OPENAT_SYSCALL __openat_nocancel

#if TARGET_OS_OSX
int __fcntl_nocancel(int fd, int cmd, long arg);
int __close_nocancel(int fd);

#define FCNTL_SYSCALL __fcntl_nocancel
#define CLOSE_SYSCALL __close_nocancel
#endif /* TARGET_OS_OSX */
#endif /* VARIANT_CANCELABLE */

#ifdef VARIANT_CANCELABLE
int
open(const char *path, int oflag, ...)
#else /* VARIANT_CANCELABLE */
int
open$NOCANCEL(const char *path, int oflag, ...)
#endif
{
	int opened_fd = 0;
	mode_t mode = 0;

	if (oflag & O_CREAT) {
		va_list ap;
		va_start(ap, oflag);
		/* compiler warns to pass int (not mode_t) to va_arg */
		mode = va_arg(ap, int);
		va_end(ap);
	}

	opened_fd = OPEN_SYSCALL(path, oflag, mode);
#if !TARGET_OS_OSX
	return opened_fd;
#else /* TARGET_OS_OSX */
	if (opened_fd < 0) {
		return opened_fd;
	}

	/* check to see if system_version_compat is enabled for this process */
	if (system_version_compat_check_path_suffix == NULL) {
		return opened_fd;
	}

	/* check to see if the suffix of the path we opened matches one we are shimming */
	if (!system_version_compat_check_path_suffix(path)) {
		return opened_fd;
	}

	/* at this point we call into the version compat open shim and return values from there */
	return system_version_compat_open_shim(opened_fd, -1, path, oflag, mode, CLOSE_SYSCALL, OPEN_SYSCALL,
	           NULL, FCNTL_SYSCALL);
#endif /* TARGET_OS_OSX */
}

#ifdef VARIANT_CANCELABLE
int
openat(int fd, const char *path, int oflag, ...)
#else /* VARIANT_CANCELABLE */
int
openat$NOCANCEL(int fd, const char *path, int oflag, ...)
#endif
{
	int opened_fd = 0;
	mode_t mode = 0;

	if (oflag & O_CREAT) {
		va_list ap;
		va_start(ap, oflag);
		// compiler warns to pass int (not mode_t) to va_arg
		mode = va_arg(ap, int);
		va_end(ap);
	}

	opened_fd = OPENAT_SYSCALL(fd, path, oflag, mode);
#if !TARGET_OS_OSX
	return opened_fd;
#else
	if (opened_fd < 0) {
		return opened_fd;
	}

	/* check to see if system_version_compat is enabled for this process */
	if (system_version_compat_check_path_suffix == NULL) {
		return opened_fd;
	}

	/* check to see if the suffix of the path we opened matches one we are shimming */
	if (!system_version_compat_check_path_suffix(path)) {
		return opened_fd;
	}

	/* at this point we call into the version compat open shim and return values from there */
	return system_version_compat_open_shim(opened_fd, fd, path, oflag, mode, CLOSE_SYSCALL, NULL,
	           OPENAT_SYSCALL, FCNTL_SYSCALL);
#endif /* !TARGET_OS_OSX */
}
#endif /* !defined(__i386__) */
