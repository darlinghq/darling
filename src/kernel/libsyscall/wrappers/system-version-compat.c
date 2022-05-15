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

#include <TargetConditionals.h>

#if TARGET_OS_OSX && !defined(__i386__)
/*
 * Support for shimming calls to open() to the SystemVersion plist on macOS for older
 * binaries. This code is only built into libsyscall_dynamic.
 */

#include <fcntl.h>
#include <stdbool.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <unistd.h>

#include "system-version-compat-support.h"

#define PLAT_PREFIX_IOS "iOS"
#define PLAT_PREFIX_MACOS ""

#define COMPAT_SUFFIX_MACOS "Compat"
#define COMPAT_SUFFIX_IOS ""

#define SYSTEM_VERSION_PLIST_FILENAME "SystemVersion.plist"
#define SYSTEM_VERSION_PLIST_PATH ("/System/Library/CoreServices/" SYSTEM_VERSION_PLIST_FILENAME)

#define SYSTEM_VERSION_COMPAT_PLIST_FILENAME(platform_prefix, compat_suffix) (platform_prefix "SystemVersion" compat_suffix ".plist")

#define SYSTEM_VERSION_PLIST_FILENAMELEN strlen(SYSTEM_VERSION_PLIST_FILENAME)

#define SYSTEM_VERSION_COMPAT_PLIST_FILENAMELEN(platform_prefix, compat_suffix) strlen(SYSTEM_VERSION_COMPAT_PLIST_FILENAME(platform_prefix, compat_suffix))

#define SYSTEM_VERSION_PLIST_PATHLEN strlen(SYSTEM_VERSION_PLIST_PATH)

extern system_version_compat_mode_t system_version_compat_mode;

/*
 * This routine determines whether the path specified matches the path of the SystemVersion plist file
 * we are shimming accesses to. If the file name suffix matches, it's expected we'll call into the
 * version_compat_open_shim() routine below which will do a full comparison on the expanded path.
 *
 * Parameters:	orig_path The path suffix that was provided to the open{at} call.
 *
 * Returns:		true if the path suffix matches the SystemVersion plist path we're shimming
 *				false otherwise
 */
__attribute__((visibility("hidden")))
bool
_system_version_compat_check_path_suffix(const char *orig_path)
{
	size_t path_str_len = strnlen(orig_path, MAXPATHLEN);
	/*
	 * If the length of the filename we're opening is shorter than
	 * SYSTEM_VERSION_PLIST_FILENAME, bail.
	 */
	if (path_str_len < SYSTEM_VERSION_PLIST_FILENAMELEN) {
		return false;
	}

	/* If the path we're accessing doesn't end in SYSTEM_VERSION_PLIST_FILENAME, bail. */
	if (strncmp(&orig_path[path_str_len - SYSTEM_VERSION_PLIST_FILENAMELEN], SYSTEM_VERSION_PLIST_FILENAME,
	    SYSTEM_VERSION_PLIST_FILENAMELEN) != 0) {
		return false;
	}

	/* If modifying the path specified would exceed MAXPATHLEN, bail */
	if (path_str_len == MAXPATHLEN) {
		return false;
	}

	size_t compat_len = (system_version_compat_mode == SYSTEM_VERSION_COMPAT_MODE_IOS) ? SYSTEM_VERSION_COMPAT_PLIST_FILENAMELEN(PLAT_PREFIX_IOS, COMPAT_SUFFIX_IOS) : SYSTEM_VERSION_COMPAT_PLIST_FILENAMELEN(PLAT_PREFIX_MACOS, COMPAT_SUFFIX_MACOS);
	if ((compat_len - SYSTEM_VERSION_PLIST_FILENAMELEN) > (MAXPATHLEN - path_str_len - 1)) {
		return false;
	}

	/* Indicate that we should */
	return true;
}

/*
 * This routine determines whether we are trying to open the SystemVersion plist at SYSTEM_VERSION_PLIST_PATH.
 * It's only used on targets that have the compatibility shim enabled (mainly older binaries).
 *
 * Note that this routine should * ABSOLUTELY NOT * be used as a general shim for accesses at all paths. We replace
 * what the developer generally expected to be one system call with multiple additional system calls. We're ok
 * with doing this here because we only do it for calls to open against files that match this very specific pattern
 * (named SystemVersion.plist), but doing so for all calls to open could result in various issues. Specifically it's
 * difficult to ensure the same cancellation semantics (around EINTR etc) that developers generally expect when replacing
 * a single system call with multiple.
 *
 * This routine should return with the same semantics as the general open system calls that it is shimming - specifically
 * it should leave errno and the return value matching what developers expect.
 *
 * It's expected that _version_compat_check_path_suffix() above was called prior to this call and returned true.
 *
 * We take the close, open and fcntl syscalls as parameters to make sure the variant we call matches the original call
 * to open{at}.
 *
 * Parameters:  opened_fd        The file descriptor that was opened in the original open{at} call
 *              openat_fd        The file descriptor passed to the original openat call (only used when use_openat is true)
 *              orig_path        The original path suffix passed to open{at}
 *              oflag            The original oflag passed to open{at}
 *              mode             The original mode passed to open{at}
 *              close_syscall    The appropriate syscall to use for closing file descriptors
 *              open_syscall     The syscall that should be used for a new call to open.
 *              fctnl_syscall    The appopriate syscall to use for fcntl.
 *
 * Returns:     The original file descriptor if the open{at} access wasn't to SYSTEM_VERSION_PLIST_PATH
 *              A new file descriptor (with the original closed) if the expanded path matches SYSTEM_VERSION_PLIST_PATH
 *              The original file descriptor if the full path suffix does not match SYSTEM_VERSION_PLIST_PATH
 *              -1 (with errno set to EINTR) if the new open or fcntl calls received EINTR (with all new fds closed)
 */
__attribute__((visibility("hidden")))
int
_system_version_compat_open_shim(int opened_fd, int openat_fd, const char *orig_path, int oflag, mode_t mode,
    int (*close_syscall)(int), int (*open_syscall)(const char *, int, mode_t),
    int (*openat_syscall)(int, const char *, int, mode_t),
    int (*fcntl_syscall)(int, int, long))
{
	/* stash the errno from the original open{at} call */
	int stashed_errno = errno;
	char new_path[MAXPATHLEN];
	size_t path_str_len = strnlen(orig_path, sizeof(new_path));

	/* Resolve the full path of the file we've opened */
	if (fcntl_syscall(opened_fd, F_GETPATH, new_path)) {
		if (errno == EINTR) {
			/* If we got EINTR, we close the file that was opened and return -1 & EINTR */
			close_syscall(opened_fd);
			errno = EINTR;
			return -1;
		} else {
			/* otherwise we return the original file descriptor that was requested */
			errno = stashed_errno;
			return opened_fd;
		}
	}

	/* Check to see whether the path matches SYSTEM_VERSION_PLIST_PATH */
	size_t newpathlen = strnlen(new_path, MAXPATHLEN);
	if (newpathlen != SYSTEM_VERSION_PLIST_PATHLEN) {
		errno = stashed_errno;
		return opened_fd;
	}

	if (strncmp(new_path, SYSTEM_VERSION_PLIST_PATH, SYSTEM_VERSION_PLIST_PATHLEN) != 0) {
		errno = stashed_errno;
		return opened_fd;
	}

	new_path[0] = '\0';

	/*
	 * It looks like we're trying to access the SystemVersion plist. Let's try to open
	 * the compatibility plist and return that instead if it exists.
	 */
	size_t prefix_str_len = path_str_len - SYSTEM_VERSION_PLIST_FILENAMELEN;
	strlcpy(new_path, orig_path, (prefix_str_len + 1));
	if (system_version_compat_mode == SYSTEM_VERSION_COMPAT_MODE_IOS) {
		strlcat(new_path, SYSTEM_VERSION_COMPAT_PLIST_FILENAME(PLAT_PREFIX_IOS, COMPAT_SUFFIX_IOS), MAXPATHLEN);
	} else {
		strlcat(new_path, SYSTEM_VERSION_COMPAT_PLIST_FILENAME(PLAT_PREFIX_MACOS, COMPAT_SUFFIX_MACOS), MAXPATHLEN);
	}

	int new_fd = -1;
	if (openat_syscall != NULL) {
		new_fd = openat_syscall(openat_fd, new_path, oflag, mode);
	} else {
		new_fd = open_syscall(new_path, oflag, mode);
	}
	if ((new_fd == -1) && (errno == ENOENT)) {
		/* The file doesn't exist, so return the original fd and errno. */
		errno = stashed_errno;
		return opened_fd;
	}

	/*
	 * Otherwise we close the first file we opened and populate errno
	 * with errno from the call to open{at}. (Note this covers the EINTR
	 * case and other failures).
	 */
	stashed_errno = errno;
	close_syscall(opened_fd);
	errno = stashed_errno;
	return new_fd;
}

#endif /* TARGET_OS_OSX && !defined(__i386__) */
