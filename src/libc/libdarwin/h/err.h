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
 * Non-standard, Darwin-specific additions to the err(3) family of APIs. The
 * following additions are provided:
 *
 *    - support for the error code additions in os/errno.h
 *    - automatic conversion of an error code into a sysexits(3) code
 *    - a debug_*() family of routines
 *
 * There are no direct equivalents for err(3), warn(3), verr(3), or vwarn(3)
 * because those routines query errno for error information, and it is not valid
 * to store error codes from os/error.h in errno. Therefore when evaluating the
 * output of normal POSIX routines, use the err(1) family of routines. When
 * evaluating the output of routines that can return the error codes in
 * os/error.h, you may use these routines and always explicitly pass the error
 * code.
 *
 * There are no equivalents for errx(3), warnx(3), verrx(3), or vwarnx(3)
 * because those routines do not query error information.
 */
#ifndef __DARWIN_ERR_H
#define __DARWIN_ERR_H

#include <os/base.h>
#include <os/api.h>

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <uuid/uuid.h>

#include <stdarg.h>
#include <stdio.h>

#if DARWIN_TAPI
#include "tapi.h"
#endif

__BEGIN_DECLS;

/*!
 * @function err_np
 * Writes a formatted error message to stderr(4) and exits the program.
 *
 * @param code
 * The error code to query in the output.
 *
 * @param fmt
 * The printf(3)-like format string representing the message to print.
 *
 * @param ...
 * The arguments corresponding to the format string.
 *
 * @discussion
 * This routine will look up an appropriate sysexits(3) code using the given
 * error code using {@link darwin_sysexit} and pass it to exit(3).
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_NORETURN OS_COLD OS_NONNULL2 OS_FORMAT_PRINTF(2, 3)
void
err_np(errno_t code, const char *fmt, ...);

/*!
 * @function errc_np
 * Writes a formatted error message to stderr(4) and exits the program.
 *
 * @param eval
 * The code to pass to exit(3). This should be a code defined in sysexits(3).
 *
 * @param code
 * The error code to query in the output.
 *
 * @param fmt
 * The printf(3)-like format string representing the message to print.
 *
 * @param ...
 * The arguments corresponding to the format string.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_NORETURN OS_COLD OS_NONNULL3 OS_FORMAT_PRINTF(3, 4)
void
errc_np(int eval, errno_t code, const char *fmt, ...);

/*!
 * @function warn_np
 * Writes a formatted warning message to stderr(4).
 *
 * @param code
 * The error code to query in the output.
 *
 * @param fmt
 * The printf(3)-like format string representing the message to print.
 *
 * @param ...
 * The arguments corresponding to the format string.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_COLD OS_NONNULL2 OS_FORMAT_PRINTF(2, 3)
void
warn_np(errno_t code, const char *fmt, ...);

/*!
 * @function verr_np
 * Writes a formatted error message to stderr(4) and exits the program.
 *
 * @param code
 * The error code to query in the output.
 *
 * @param fmt
 * The printf(3)-like format string representing the message to print.
 *
 * @param ap
 * The argument pointer corresponding to the format string.
 *
 * @discussion
 * This routine will look up an appropriate sysexits(3) code using the given
 * error code using {@link darwin_sysexit} and pass it to exit(3).
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_NORETURN OS_COLD OS_NONNULL2 OS_NONNULL3
void
verr_np(errno_t code, const char *fmt, va_list ap);

/*!
 * @function verrc_np
 * Writes a formatted error message to stderr(4) and exits the program.
 *
 * @param eval
 * The code to pass to exit(3). This should be a code defined in sysexits(3).
 *
 * @param code
 * The error code to query in the output.
 *
 * @param fmt
 * The printf(3)-like format string representing the message to print.
 *
 * @param ap
 * The argument pointer corresponding to the format string.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_NORETURN OS_COLD OS_NONNULL3 OS_NONNULL4
void
verrc_np(int eval, errno_t code, const char *fmt, va_list ap);

/*!
 * @function vwarn_np
 * Writes a formatted warning message to stderr(4).
 *
 * @param code
 * The error code to query in the output.
 *
 * @param fmt
 * The printf(3)-like format string representing the message to print.
 *
 * @param ap
 * The arguments corresponding to the format string.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_COLD OS_NONNULL2 OS_NONNULL3
void
vwarn_np(errno_t code, const char *fmt, va_list ap);

__END_DECLS;

#endif // __DARWIN_ERR_H
