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

#include <TargetConditionals.h>
#include <xpc/xpc.h>
#include <errno.h>

#include "tzlink.h"
#include "tzlink_internal.h"

errno_t
tzlink(const char *tz)
{
#if TARGET_OS_IPHONE || TARGET_OS_OSX && !TARGET_OS_SIMULATOR
	xpc_connection_t connection;
	xpc_object_t request, reply;
	errno_t e;

	if (tz == NULL) {
		return EINVAL;
	}

	connection = xpc_connection_create_mach_service(TZLINK_SERVICE_NAME, NULL, XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);
	xpc_connection_set_event_handler(connection, ^(__unused xpc_object_t event) {
	});
	xpc_connection_resume(connection);

	request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(request, TZLINK_KEY_REQUEST_TIMEZONE, tz);

	reply = xpc_connection_send_message_with_reply_sync(connection, request);
	if (xpc_get_type(reply) == XPC_TYPE_DICTIONARY) {
		e = (errno_t)xpc_dictionary_get_uint64(reply, TZLINK_KEY_REPLY_ERROR);
	} else {
		e = EIO;
	}

	xpc_release(reply);
	xpc_release(request);
	xpc_release(connection);

	return e;
#else /* !TARGET_OS_IPHONE */
#pragma unused (tz)
	return ENOTSUP;
#endif /* TARGET_OS_IPHONE */
}
