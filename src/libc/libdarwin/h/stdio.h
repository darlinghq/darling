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
 * Non-standard, Darwin-specific additions for the stdio(3) family of APIs.
 */
#ifndef __DARWIN_STDIO_H
#define __DARWIN_STDIO_H

#include <os/base.h>
#include <os/api.h>
#include <sys/cdefs.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#if __has_include(<sys/guarded.h>)
#include <sys/guarded.h>
#else
typedef uint64_t guardid_t;
#endif

#if DARWIN_TAPI
#include "tapi.h"
#endif

__BEGIN_DECLS;

/*!
 * @typedef os_fd_t
 * A type alias for a file descriptor.
 */
typedef int os_fd_t;

/*!
 * @function os_fd_valid
 * Returns whether the given integer is a valid file descriptor number.
 *
 * @param fd
 * The integer to check.
 *
 * @result
 * A Boolean indicating whether the integer is a valid file descriptor number,
 * that is, greater than or equal to zero.
 */
DARWIN_API_AVAILABLE_20180727
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline bool
os_fd_valid(os_fd_t fd)
{
	return (fd >= STDIN_FILENO);
}

/*!
 * @function os_guardid_from_ptr
 * Converts the given pointer to a guardid_t.
 *
 * @param p
 * The pointer to convert.
 *
 * @result
 * The pointer as a guardid_t.
 */
DARWIN_API_AVAILABLE_20190830
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline guardid_t
os_guardid_from_ptr(const void *p)
{
	return (guardid_t)(uintptr_t)p;
}

/*!
 * @function fcheck_np
 * Checks the status of an fread(3) or fwrite(3) operation to a FILE.
 *
 * @param f
 * The file on which the operation was performed.
 *
 * @param n
 * The return value of the operation.
 *
 * @param expected
 * The expected return value of the operation.
 *
 * @result
 * One of the following integers:
 *
 *     0     The operation succeeded
 *     EOF   The operation encountered the end of the FILE stream before it
 *           could complete
 *     1     There was an error
 */
DARWIN_API_AVAILABLE_20180727
OS_EXPORT OS_WARN_RESULT OS_NONNULL1
int
fcheck_np(FILE *f, size_t n, size_t expected);

/*!
 * @function dup_np
 * Variant of dup(2) that guarantees the dup(2) operation will either succeed or
 * not return.
 *
 * @param fd
 * The descriptor to dup(2).
 *
 * @result
 * A new file descriptor number that is functionally equivalent to what the
 * caller passed.
 *
 * @discussion
 * The implementation will retry if the operation was interrupted by a signal.
 * If the operation failed for any other reason, the implementation will
 * terminate the caller.
 */
DARWIN_API_AVAILABLE_20180727
OS_EXPORT OS_WARN_RESULT
os_fd_t
dup_np(os_fd_t fd);

/*!
 * @function claimfd_np
 * Claims the given file descriptor for the caller's exclusive use by applying a
 * guard and invalidating the given storage.
 *
 * @param fdp
 * A pointer to the storage for the descriptor to claim. Upon return, a known-
 * invalid value is written into this memory.
 *
 * @param gdid
 * The optional guard value to enforce the caller's claim on the descriptor.
 *
 * @param gdflags
 * The guard flags to enforce the caller's claim on the descriptor.
 *
 * @result
 * The given descriptor with the guard applied.
 */
DARWIN_API_AVAILABLE_20190830
OS_EXPORT OS_WARN_RESULT OS_NONNULL1
os_fd_t
claimfd_np(os_fd_t *fdp, const guardid_t *gdid, u_int gdflags);

/*!
 * @function xferfd_np
 * Transfers ownership from the given file descriptor back to the general public
 * by clearing the guard associated with it.
 *
 * @param fdp
 * A pointer to the storage for the descriptor to claim. Upon return, a known-
 * invalid value is written into this memory.
 *
 * @param gdid
 * The optional guard value to reliquish ownership on the descriptor.
 *
 * @param gdflags
 * The guard flags to relinquish.
 *
 * @result
 * The given descriptor with the guard cleared. This descriptor is suitable for
 * claiming with {@link claimfd_np}.
 */
DARWIN_API_AVAILABLE_20190830
OS_EXPORT OS_WARN_RESULT OS_NONNULL1
os_fd_t
xferfd_np(os_fd_t *fdp, const guardid_t *gdid, u_int gdflags);

/*!
 * @function close_drop_np
 * Variant of close(2) which transfers ownership from the caller and performs
 * the close(2) operation. These semantics are useful for ensuring that a
 * descriptor is not erroneously re-used after it has been closed. To achieve
 * these semantics, this variant will clear the memory in which the descriptor
 * resides and replace it with a known-invalid value before returning.
 *
 * @param fdp
 * A pointer to the storage for the descriptor to close. Upon return, a known-
 * invalid value is written into this memory.
 *
 * @param gdid
 * The optional guard. If the descriptor is not guarded, pass NULL.
 *
 * @discussion
 * If the implementation encounters a failure to close a valid descriptor
 * number, the caller will be terminated.
 */
OS_EXPORT OS_NONNULL1
void
close_drop_np(os_fd_t *fdp, const guardid_t *gdid);

/*!
 * @function close_drop_optional_np
 * Variant of {@link close_drop} which will not attempt to close an invalid
 * descriptor. Otherwise all semantics are the same.
 *
 * @param fdp
 * A pointer to the storage for the descriptor to close. Upon return, a known-
 * invalid value is written into this memory.
 *
 * @param gdid
 * The optional guard. If the descriptor is not guarded, pass NULL.
 *
 * @discussion
 * If the implementation encounters a failure to close a valid descriptor
 * number, the caller will be terminated. The implementation will not attempt to
 * close the descriptor if its value is -1.
 */
OS_EXPORT OS_NONNULL1
void
close_drop_optional_np(os_fd_t *fdp, const guardid_t *gdid);

/*!
 * @function zsnprintf_np
 * snprintf(3) variant which returns the numnber of bytes written less the null
 * terminator.
 *
 * @param buff
 * The buffer in which to write the string.
 *
 * @param len
 * The length of the buffer.
 *
 * @param fmt
 * The printf(3)-like format string.
 *
 * @param ...
 * The arguments corresponding to the format string.
 *
 * @result
 * The number of bytes written into the buffer, less the null terminator. This
 * routine is useful for successive string printing that may be lossy, as it
 * will simply return zero when there is no space left in the destination
 * buffer, i.e. enables the following pattern:
 *
 * char *cur = buff;
 * size_t left = sizeof(buff);
 * for (i = 0; i < n_strings; i++) {
 *     size_t n_written = zsnprintf_np(buff, left, "%s", strings[i]);
 *     cur += n_written;
 *     left -= n_written;
 * }
 *
 * This loop will safely terminate without any special care since, as soon as
 * the buffer's space is exhausted, all further calls to zsnprintf_np() will
 * write nothing and return zero.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL3 OS_FORMAT_PRINTF(3, 4)
size_t
zsnprintf_np(char *buff, size_t len, const char *fmt, ...);

/*!
 * @function crfprintf_np
 * fprintf(3) variant that appends a new line character to the output.
 *
 * @param f
 * The file to which the output should be written.
 *
 * @param fmt
 * The printf(3)-like format string.
 *
 * @param ...
 * The arguments corresponding to the format string.
 */
DARWIN_API_AVAILABLE_20181020
OS_EXPORT OS_NONNULL1 OS_NONNULL2 OS_FORMAT_PRINTF(2, 3)
void
crfprintf_np(FILE *f, const char *fmt, ...);

/*!
 * @function vcrfprintf_np
 * vfprintf(3) variant that appends a new line character to the output.
 *
 * @param f
 * The file to which the output should be written.
 *
 * @param fmt
 * The printf(3)-like format string.
 *
 * @param ap
 * The argument list corresponding to the format string.
 */
DARWIN_API_AVAILABLE_20181020
OS_EXPORT OS_NONNULL1 OS_NONNULL2 OS_NONNULL3
void
vcrfprintf_np(FILE *f, const char *fmt, va_list ap);

/*!
 * @function wfprintf_np
 * fprintf(3) variant which wraps the output to the specified column width,
 * inserting new lines as necessary. Output will be word-wrapped with a trivial
 * algorithm.
 *
 * @param f
 * The file to which the output should be written.
 *
 * @param initpad
 * The number of spaces that should be inserted prior to the first line of
 * output. If a negative value is given, the implementation will assume that an
 * amount of spaces equal to the absolute value of the parameter has already
 * been written, and therefore it will only use the parameter to compute line-
 * wrapping information and not insert any additional spaces on the first line
 * of output.
 *
 * @param pad
 * The number of spaces that should be inserted prior to every line of output
 * except the first line.
 *
 * @param width
 * The maximum number of columns of each line of output. Pass zero to indicate
 * that there is no maximum.
 *
 * @param fmt
 * The printf(3)-like format string.
 *
 * @param ...
 * The arguments corresponding to the format string.
 *
 * @discussion
 * This routine will silently fail to print to the desired output stream if
 * there was a failure to allocate heap memory.
 */
DARWIN_API_AVAILABLE_20181020
OS_EXPORT OS_NONNULL1 OS_NONNULL5 OS_NONNULL6
void
wfprintf_np(FILE *f, ssize_t initpad, size_t pad, size_t width,
		const char *fmt, ...);

/*!
 * @function vwfprintf_np
 * vfprintf(3) variant which wraps the output to the specified column width,
 * inserting new lines as necessary. Output will be word-wrapped with a trivial
 * algorithm.
 *
 * @param f
 * The file to which the output should be written.
 *
 * @param initpad
 * The number of spaces that should be inserted prior to the first line of
 * output. If a negative value is given, the implementation will assume that an
 * amount of spaces equal to the absolute value of the parameter has already
 * been written, and therefore it will only use the parameter to compute line-
 * wrapping information and not insert any additional spaces on the first line
 * of output.
 *
 * @param pad
 * The number of spaces that should be inserted prior to every line of output
 * except the first line.
 *
 * @param width
 * The maximum number of columns of each line of output. Pass zero to indicate
 * that there is no maximum.
 *
 * @param fmt
 * The printf(3)-like format string.
 *
 * @param ap
 * The argument list corresponding to the format string.
 *
 * @discussion
 * This routine will silently fail to print to the desired output stream if
 * there was a failure to allocate heap memory.
 */
DARWIN_API_AVAILABLE_20181020
OS_EXPORT OS_NONNULL1 OS_NONNULL5 OS_NONNULL6
void
vwfprintf_np(FILE *f, ssize_t initpad, size_t pad, size_t width,
		const char *fmt, va_list ap);

__END_DECLS;

#endif // __DARWIN_STDIO_H
