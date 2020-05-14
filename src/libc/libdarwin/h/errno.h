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

/*
 * @header
 * Non-standard, Darwin-specific additions to the error codes enumerated in
 * intro(2) and sys/errno.h.
 */
#ifndef __DARWIN_ERRNO_H
#define __DARWIN_ERRNO_H

#include <os/base.h>
#include <os/api.h>
#include <sys/errno.h>
#include <sys/cdefs.h>
#include <sysexits.h>

#if DARWIN_TAPI
#include "tapi.h"
#endif

/*!
 * @enum
 * Additional POSIX-flavor error codes that are meaningful to Darwin. These
 * definitions all have a "_NP" suffix to distinguish them from POSIX error
 * codes (meaning "not POSIX").
 */
#define EBASE_NP 200
#define _ENPERR(__c) (EBASE_NP + __c)

/*!
 * @const ENOTENTITLED_NP
 * The remote process lacked a required entitlement to perform the operation.
 */
#define ENOTENTITLED_NP _ENPERR(0)

/*!
 * @const ENOTPLATFORM_NP
 * The operation may only be invoked by platform binaries.
 */
#define ENOTPLATFORM_NP _ENPERR(1)

/*!
 * @const EROOTLESS_NP
 * The operation was denied by System Integrity Protection.
 */
#define EROOTLESS_NP _ENPERR(2)

/*!
 * @const ETAINTED_NP
 * The operation may only be invoked by processes which are not tainted by
 * debugging and introspection functionality (e.g. dyld(1) environment
 * variables, debugger attachment, etc.).
 */
#define ETAINTED_NP _ENPERR(3)

/*!
 * @const EQUARANTINE_NP
 * The operation is not permitted on quarantined file.
 */
#define EQUARANTINE_NP _ENPERR(4)

/*!
 * @const EBADUSER_NP
 * The operation referenced a user name or identifier that was invalid.
 */
#define EBADUSER_NP _ENPERR(5)

/*!
 * @const EBADGROUP_NP
 * The operation referenced a group name or identifier that was invalid.
 */
#define EBADGROUP_NP _ENPERR(6)

/*!
 * @const EOWNERSHIP_NP
 * Ownership or access permissions on a file were too permissive.
 */
#define EOWNERSHIP_NP _ENPERR(7)

/*!
 * @const ENOOOO_NP
 * A series of operations was executed in the improper order (no out-of-order
 * operations allowed).
 */
#define ENOOOO_NP _ENPERR(8)

/*!
 * @const ENOTBUNDLE_NP
 * The path given to the operation did not refer to a bundle.
 */
#define ENOTBUNDLE_NP _ENPERR(9)

/*!
 * @const EBADBUNDLE_NP
 * The path given to the operation did not refer to a valid bundle.
 */
#define EBADBUNDLE_NP _ENPERR(10)

/*!
 * @const EBADPATH_NP
 * The path given to the operation was not valid.
 */
#define EBADPATH_NP _ENPERR(11)

/*!
 * @const EBADPLIST_NP
 * The plist given to the operation was invalid.
 */
#define EBADPLIST_NP _ENPERR(12)

/*!
 * @const EBADKEY_NP
 * A key in the given plist was unrecognized.
 */
#define EBADKEY_NP _ENPERR(13)

/*!
 * @const EBADVAL_NP
 * The value for a key in the given plist was either not present (and was
 * required to be) or was present but not of the appropriate type.
 */
#define EBADVAL_NP _ENPERR(14)

/*!
 @const EBADSUBSYS_NP
 * The request referenced a subsystem that did not exist.
 */
#define EBADSUBSYS_NP _ENPERR(15)

/*!
 * @const E2BIMPL_NP
 * The operation has not yet been implemented.
 */
#define E2BIMPL_NP _ENPERR(16)

/*!
 * @const EDEPRECATED_NP
 * The operation has been deprecated.
 */
#define EDEPRECATED_NP _ENPERR(17)

/*!
 * @const EREMOVED_NP
 * The operation has been removed from the implementation.
 */
#define EREMOVED_NP _ENPERR(18)

/*!
 * @const EDROPPED_NP
 * The request referenced a data structure that will never achieve the state
 * required to perform the operation.
 */
#define EDROPPED_NP _ENPERR(19)

/*!
 * @const EDEFERRED_NP
 * The request referenced a data structure that was not in the state required
 * to perform the operation, and the request has been pended until the object
 * reaches the required state. This code is meant to be used for control flow
 * purposes in the server and should not be returned to the caller.
 */
#define EDEFERRED_NP _ENPERR(20)

/*!
 * @const EUSAGE_NP
 * Improper command line usage. This code is meant to be used for control flow
 * purposes in a command line tool so that routines may return an error code
 * indicating improper usage without having to use EX_USAGE, which collides with
 * the POSIX error space. It should not be passed to exit(3), _exit(2), et al.
 * and should instead be translated into EX_USAGE either directly or with
 * {@link darwin_sysexit}.
 */
#define EUSAGE_NP _ENPERR(21)

/*!
 * @const EUNKNOWN_NP
 * An error occurred in a subsystem, and insufficient detail as to the nature of
 * the failure was available to translate it into a more descriptive error code.
 */
#define EUNKNOWN_NP _ENPERR(22)
#define __ELAST_NP _ENPERR(22)

/*!
 * @const EX_BADRECEIPT_NP
 * An exit code indicating that the program could not verify its purchase
 * receipt from the Mac App Store. This exit code is inspected by the system to
 * trigger a re-validation of the purchase receipt. It must be passed to the
 * exit(3) Libc API and not to _exit(2) system call. This exit code is only
 * relevant to the macOS variant of Darwin.
 */
#define EX_BADRECEIPT_NP (173)

__BEGIN_DECLS;

/*!
 * @function sysexit_np
 * Translates a {@link errno_t} or POSIX error code into an exit code
 * defined in sysexits(3).
 *
 * @param code
 * The error code to translate.
 *
 * @result
 * The sysexits(3) exit code most appropriate for the given error. If no
 * appropriate exit code could be determined, the designated ¯\_(ツ)_/¯ exit
 * code, EX_UNAVAILABLE, is returned (cf. sysexits(3)).
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_CONST
int
sysexit_np(int code);

__END_DECLS;

#endif // __DARWIN_ERRNO_H
