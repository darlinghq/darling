/*
 * Copyright (c) 2006-2010 Apple Inc. All rights reserved.
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
#ifndef _SANDBOX_H_
#define _SANDBOX_H_

#include <sys/cdefs.h>
#include <stdint.h>
#include <unistd.h>

__BEGIN_DECLS
/*
 * @function sandbox_init
 * Places the current process in a sandbox with a profile as
 * specified.  If the process is already in a sandbox, the new profile
 * is ignored and sandbox_init() returns an error.
 *
 * @param profile (input)   The Sandbox profile to be used.  The format
 * and meaning of this parameter is modified by the `flags' parameter.
 *
 * @param flags (input)   Must be SANDBOX_NAMED.  All other
 * values are reserved.
 *
 * @param errorbuf (output)   In the event of an error, sandbox_init
 * will set `*errorbuf' to a pointer to a NUL-terminated string
 * describing the error. This string may contain embedded newlines.
 * This error information is suitable for developers and is not
 * intended for end users.
 *
 * If there are no errors, `*errorbuf' will be set to NULL.  The
 * buffer `*errorbuf' should be deallocated with `sandbox_free_error'.
 *
 * @result 0 on success, -1 otherwise.
 */
int sandbox_init(const char *profile, uint64_t flags, char **errorbuf);

/*
 * @define SANDBOX_NAMED  The `profile' argument specifies a Sandbox
 * profile named by one of the kSBXProfile* string constants.
 */
#define SANDBOX_NAMED		0x0001

#ifdef __APPLE_API_PRIVATE

/* The following flags are reserved for Mac OS X.  Developers should not
 * depend on their availability.
 */

/*
 * @define SANDBOX_NAMED_BUILTIN   The `profile' argument specifies the
 * name of a builtin profile that is statically compiled into the
 * system.
 */
#define SANDBOX_NAMED_BUILTIN	0x0002

/*
 * @define SANDBOX_NAMED_EXTERNAL   The `profile' argument specifies the
 * pathname of a Sandbox profile.  The pathname may be abbreviated: If
 * the name does not start with a `/' it is treated as relative to
 * /usr/share/sandbox and a `.sb' suffix is appended.
 */
#define SANDBOX_NAMED_EXTERNAL	0x0003

/*
 * @define SANDBOX_NAMED_MASK   Mask for name types: 4 bits, 15 possible
 * name types, 3 currently defined.
 */
#define SANDBOX_NAMED_MASK	0x000f

#endif /* __APPLE_API_PRIVATE */

/*
 * Available Sandbox profiles.
 */

/* TCP/IP networking is prohibited. */
extern const char kSBXProfileNoInternet[];

/* All sockets-based networking is prohibited. */
extern const char kSBXProfileNoNetwork[];

/* File system writes are prohibited. */
extern const char kSBXProfileNoWrite[];

/* File system writes are restricted to temporary folders /var/tmp and
 * confstr(_CS_DARWIN_USER_DIR, ...).
 */
extern const char kSBXProfileNoWriteExceptTemporary[];

/* All operating system services are prohibited. */
extern const char kSBXProfilePureComputation[];

/*
 * @function sandbox_free_error
 * Deallocates an error string previously allocated by sandbox_init.
 *
 * @param errorbuf (input)   The buffer to be freed.  Must be a pointer
 * previously returned by sandbox_init in the `errorbuf' argument, or NULL.
 *
 * @result void
 */
void sandbox_free_error(char *errorbuf);


#ifdef __APPLE_API_PRIVATE

/* The following definitions are reserved for Mac OS X.  Developers should not
 * depend on their availability.
 */

int sandbox_init_with_parameters(const char *profile, uint64_t flags, const char *const parameters[], char **errorbuf);

int sandbox_init_with_extensions(const char *profile, uint64_t flags, const char *const extensions[], char **errorbuf);

enum sandbox_filter_type {
	SANDBOX_FILTER_NONE,
	SANDBOX_FILTER_PATH,
	SANDBOX_FILTER_GLOBAL_NAME,
	SANDBOX_FILTER_LOCAL_NAME,
	SANDBOX_FILTER_APPLEEVENT_DESTINATION,
	SANDBOX_FILTER_RIGHT_NAME,
};

extern const enum sandbox_filter_type SANDBOX_CHECK_NO_REPORT __attribute__((weak_import));

enum sandbox_extension_flags {
	FS_EXT_DEFAULTS =              0,
	FS_EXT_FOR_PATH =       (1 << 0),
	FS_EXT_FOR_FILE =       (1 << 1),
	FS_EXT_READ =           (1 << 2),
	FS_EXT_WRITE =          (1 << 3),
	FS_EXT_PREFER_FILEID =  (1 << 4),
};

int sandbox_check(pid_t pid, const char *operation, enum sandbox_filter_type type, ...);

int sandbox_note(const char *note);

int sandbox_suspend(pid_t pid);
int sandbox_unsuspend(void);

int sandbox_issue_extension(const char *path, char **ext_token);
int sandbox_issue_fs_extension(const char *path, uint64_t flags, char **ext_token);
int sandbox_issue_fs_rw_extension(const char *path, char **ext_token);
int sandbox_issue_mach_extension(const char *name, char **ext_token);

int sandbox_consume_extension(const char *path, const char *ext_token);
int sandbox_consume_fs_extension(const char *ext_token, char **path);
int sandbox_consume_mach_extension(const char *ext_token, char **name);

int sandbox_release_fs_extension(const char *ext_token);

int sandbox_container_path_for_pid(pid_t pid, char *buffer, size_t bufsize);

int sandbox_wakeup_daemon(char **errorbuf);

const char *_amkrtemp(const char *);

#endif /* __APPLE_API_PRIVATE */

__END_DECLS
#endif /* _SANDBOX_H_ */
