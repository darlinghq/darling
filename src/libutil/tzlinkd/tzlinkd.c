/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#include <xpc/xpc.h>
#include <xpc/private.h>
#include <notify.h>
#include <tzfile.h>

#include "tzlink_internal.h"

static bool peer_entitled(xpc_connection_t);
static int set_timezone(const char *);
static int build_source_path(char *, size_t, const char *);
static int validate_source_path(const char *);

int
main(void)
{
	dispatch_queue_t queue;
	xpc_connection_t listener;

	xpc_track_activity();

	queue = dispatch_queue_create(TZLINK_SERVICE_NAME, NULL);

	listener = xpc_connection_create_mach_service(TZLINK_SERVICE_NAME, queue, XPC_CONNECTION_MACH_SERVICE_LISTENER);
	xpc_connection_set_event_handler(listener, ^(xpc_object_t peer) {
		if (xpc_get_type(peer) != XPC_TYPE_CONNECTION) {
			return;
		}

		xpc_connection_set_target_queue(peer, queue);
		xpc_connection_set_event_handler(peer, ^(xpc_object_t request) {
			xpc_object_t reply;
			const char *tz;
			int error;

			if (xpc_get_type(request) != XPC_TYPE_DICTIONARY) {
				return;
			}

			if (peer_entitled(peer)) {
				tz = xpc_dictionary_get_string(request, TZLINK_KEY_REQUEST_TIMEZONE);
				error = tz ? set_timezone(tz) : EINVAL;
			} else {
				error = EPERM;
			}

			reply = xpc_dictionary_create_reply(request);
			xpc_dictionary_set_uint64(reply, TZLINK_KEY_REPLY_ERROR, (uint64_t)error);
			xpc_connection_send_message(peer, reply);
			xpc_release(reply);
		});
		xpc_connection_resume(peer);
	});
	xpc_connection_resume(listener);

	dispatch_main();
}

static bool
peer_entitled(xpc_connection_t peer)
{
	xpc_object_t val;
	bool entitled;

	entitled = false;

	val = xpc_connection_copy_entitlement_value(peer, TZLINK_ENTITLEMENT);
	if (val != NULL) {
		entitled = xpc_bool_get_value(val);
		xpc_release(val);
	}

	return entitled;
}

static int
set_timezone(const char *tz)
{
	char srcpath[PATH_MAX];
	int error;

	error = build_source_path(srcpath, sizeof(srcpath), tz);
	if (error != 0) {
		return error;
	}

	error = validate_source_path(srcpath);
	if (error != 0) {
		return error;
	}

	(void)unlink(TZDEFAULT);
	if (symlink(srcpath, TZDEFAULT) != 0) {
		return errno ? errno : EFAULT;
	}

	/*
	 * notifyd posts "com.apple.system.timezone" automatically,
	 * but we also need post this. Sigh.
	 */
	(void)notify_post("SignificantTimeChangeNotification");

	return 0;
}

/* Create path from input. */
static int
build_source_path(char *path, size_t size, const char *tz)
{
	char *str, *str0;
	char *pathcomp;
	int error;

	if (strlcpy(path, TZDIR, size) >= size) {
		return ENAMETOOLONG;
	}

	error = 0;

	/* Attempt to validate the input; construct a clean path as we go. */
	str0 = str = strdup(tz);
	while ((pathcomp = strsep(&str, "/")) != NULL) {
		if (pathcomp[0] == '\0' || pathcomp[0] == '.') {
			error = EINVAL;
			break;
		}
		if (strlcat(path, "/", size) >= size) {
			error = ENAMETOOLONG;
			break;
		}
		if (strlcat(path, pathcomp, size) >= size) {
			error = ENAMETOOLONG;
			break;
		}
	}
	free(str0);

	return error;
}

/* Validate path. */
static int
validate_source_path(const char *path)
{
	struct stat sb;

	if (lstat(path, &sb) != 0) {
		return errno ? errno : EFAULT;
	}

	/* Ensure that the time zone file is... */

	/* ... a regular file. */
	if (!S_ISREG(sb.st_mode)) {
		return ENOENT;
	}

	/* ... owned by root:wheel */
	if (sb.st_uid != 0 || sb.st_gid != 0) {
		return ENOENT;
	}

	/* ... 0644 perms */
	if ((sb.st_mode & ACCESSPERMS) != 0644) {
		return ENOENT;
	}

	/* ... is a real tzfile (starts with TZif) */
	// TODO: Validate contents of file.

	return 0;
}
