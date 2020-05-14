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
 * Defines additions to the Mach family of APIs.
 */
#ifndef __DARWIN_MACH_UTILS_H
#define __DARWIN_MACH_UTILS_H

#include <os/base.h>
#include <os/api.h>
#include <os/assumes.h>

#include <sys/cdefs.h>

#include <mach/mach.h>
#include <mach/mach_init.h>
#include <mach/port.h>
#include <mach/mach_port.h>
#include <mach/kern_return.h>

#if DARWIN_TAPI
#include "tapi.h"
#endif

/*!
 * @define OS_MIG_SUBSYSTEM_MAXSIZE
 * A macro that evaluates to the maximum size of a request or reply message in
 * the given MIG subsystem.
 *
 * @param __subsystem
 * The name of the MIG subsystem, defined by the "subsystem" keyword.
 *
 * @param __serverprefix
 * The serverprefix of the MIG subsystem, defined by the "serverprefix" keyword.
 */
#define OS_MIG_SUBSYSTEM_MAXSIZE(__subsystem, __serverprefix) __extension__({ \
	union __subsystem ## _RequestReplyUnion { \
		union __RequestUnion__ ## __serverprefix ## __subsystem ## \
			_subsystem requests; \
		union __ReplyUnion__ ## __serverprefix ## __subsystem ## \
			_subsystem replies; \
	}; \
	(sizeof(union __subsystem ## _RequestReplyUnion)); \
})

__BEGIN_DECLS;

/*!
 * @function os_vm_address_from_ptr
 * Converts the given pointer to a vm_address_t.
 *
 * @param p
 * The pointer to convert.
 *
 * @result
 * The pointer as a vm_address_t.
 */
DARWIN_API_AVAILABLE_20190830
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline vm_address_t
os_vm_address_from_ptr(const void *p)
{
	return (vm_address_t)(uintptr_t)p;
}

/*!
 * @function os_mach_vm_address_from_ptr
 * Converts the given pointer to a mach_vm_address_t.
 *
 * @param p
 * The pointer to convert.
 *
 * @result
 * The pointer as a mach_vm_address_t.
 */
DARWIN_API_AVAILABLE_20190830
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline mach_vm_address_t
os_mach_vm_address_from_ptr(const void *p)
{
	return (mach_vm_address_t)(uintptr_t)p;
}

/*!
 * @function os_mach_msg_get_trailer
 * Obtains the trailer for the received Mach message.
 *
 * @param hdr
 * The message header of the received Mach message.
 *
 * @result
 * A pointer to the trailer that was received with the message.
 *
 * @discussion
 * Every received Mach message has a minimal trailer which identifies its format
 * and size (cf. mach_msg_trailer_t). Currently, there is one format,
 * MACH_MSG_TRAILER_FORMAT_0. The caller is responsible for validating the
 * returned trailer's format against this known value as well as the trailer's
 * size before using any additional trailer fields.
 *
 * The result of passing a header to a message that was not received from a port
 * is undefined.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_NONNULL1
const mach_msg_trailer_t *
os_mach_msg_get_trailer(const mach_msg_header_t *hdr);

/*!
 * @function os_mach_msg_get_audit_trailer
 * Obtains the audit trailer for the received Mach message.
 *
 * @param hdr
 * The message header of the received Mach message.
 *
 * @result
 * A pointer to the audit trailer that was received with the message. If the
 * message was not received with an audit trailer (by passing
 * MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT) in the mach_msg() options),
 * NULL is returned.
 *
 * @discussion
 * The result of passing a header to a message that was not received from a port
 * is undefined.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_NONNULL1
const mach_msg_audit_trailer_t *
os_mach_msg_get_audit_trailer(const mach_msg_header_t *hdr);

/*!
 * @function os_mach_msg_get_context_trailer
 * Obtains the context trailer for the received Mach message.
 *
 * @param hdr
 * The message header of the received Mach message.
 *
 * @result
 * A pointer to the context trailer that was received with the message. If the
 * message was not received with an context trailer (by passing
 * MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_CTX) in the mach_msg() options),
 * NULL is returned.
 *
 * @discussion
 * The result of passing a header to a message that was not received from a port
 * is undefined.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_NONNULL1
const mach_msg_context_trailer_t *
os_mach_msg_get_context_trailer(const mach_msg_header_t *hdr);

/*!
 * @const os_mach_msg_copy_description
 * A routine for obtaining a human-readable description of a Mach message.
 *
 * @param msg
 * The message for which to obtain the description.
 *
 * @result
 * A human-readable string describing the given message header. This string is
 * not intended to be machine-parseable, and the exact output format is not
 * stable.
 *
 * The implementation does not attempt to introspect the contents of the
 * message. If the implementation detects that the message is complex, it will
 * examine the first 4 bytes past the header to obtain the descriptor count, as
 * is specified by the Mach APIs. Therefore, you should not pass a complex
 * header that is not part of a valid message buffer, or the output may contain
 * garbage information.
 *
 * The caller is responsible for free(3)ing this string when it is no longer
 * required.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_COLD OS_WARN_RESULT OS_MALLOC OS_NONNULL1
char *
os_mach_msg_copy_description(const mach_msg_header_t *msg);

/*!
 * @function os_mach_msg_trailer_copy_description
 * A routine for obtaining a human-readable description of the trailer of a Mach
 * message.
 *
 * @param tlr
 * The trailer for which to obtain the description.
 *
 * @result
 * A human-readable string describing the given message trailer. This string is
 * not intended to be machine-parseable, and the exact output format is not
 * stable.
 *
 * The caller is responsible for free(3)ing this string when it is no longer
 * required.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_COLD OS_WARN_RESULT OS_MALLOC OS_NONNULL1
char *
os_mach_msg_trailer_copy_description(const mach_msg_trailer_t *tlr);

/*!
 * @function os_mach_port_copy_description
 * A routine for obtaining a human-readable description string of a port right
 * handle.
 *
 * @param port
 * The port right for which to obtain the description.
 *
 * @result
 * A human-readable string describing the given port right. This string is not
 * intended to be machine-parseable, and the exact output format is not stable.
 *
 * The caller is responsible for free(3)ing this string when it is no longer
 * required.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_COLD OS_WARN_RESULT OS_MALLOC
char *
os_mach_port_copy_description(mach_port_t port);

__END_DECLS;

#endif // __DARWIN_MACH_UTILS_H
