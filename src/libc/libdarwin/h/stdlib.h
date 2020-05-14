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
 * Non-standard, Darwin-specific additions to the stdlib(3) family of APIs.
 *
 * The os_malloc() and os_strdup() routines are wrappers to be used for small,
 * fixed-size allocations, the assumption being that such allocation should
 * always succeed absent other critical problems. Thus, if the requested size is
 * is a compile-time constant, the return value is asserted to be non-NULL.
 * Otherwise, for sizes that are not known at compile-time, the implementations
 * loop until the allocation succeeds, assuming the failure to be due to
 * transient resource shortages. The implementation will not loop if the program
 * has not become multi-threaded, under the assertion that there would be no
 * point since no other thread could possibly free up resources for the calling
 * thread to use. Thus, in a single-threaded program, all allocations will
 * be treated like small, fixed-size allocations and be expected to succeed.
 *
 * These wrappers should not be used for large allocations whose bounds cannot
 * be determined at compile-time. For such allocations, malloc(3), calloc(3), et
 * al. (with appropriate error handling) are the appropriate interfaces.
 */
#ifndef __DARWIN_STDLIB_H
#define __DARWIN_STDLIB_H

#include <os/base.h>
#include <os/api.h>
#include <os/assumes.h>
#include <os/stdio.h>
#include <dispatch/private.h>

#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/syslimits.h>

#if DARWIN_TAPI
#include "tapi.h"
#endif

__BEGIN_DECLS;

/*!
 * @function __os_temporary_resource_shortage
 * A function whose purpose is to appear in stack traces to indicate transient
 * resource shortage conditions. Do not call.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_NOT_TAIL_CALLED
void
__os_temporary_resource_shortage(void);

/*!
 * @functiongroup
 * Internal inline definitions.
 */
DARWIN_API_AVAILABLE_20170407
OS_ALWAYS_INLINE OS_WARN_RESULT OS_MALLOC __alloc_size(1)
static inline void *
_os_malloc_loop(size_t size)
{
	void *ptr = NULL;
	while (!(ptr = malloc(size))) {
		__os_temporary_resource_shortage();
	}
	return ptr;
}

DARWIN_API_AVAILABLE_20170407
OS_ALWAYS_INLINE OS_WARN_RESULT OS_MALLOC __alloc_size(1)
static inline void *
_os_malloc_known(size_t size)
{
	return malloc(size);
}

DARWIN_API_AVAILABLE_20170407
OS_ALWAYS_INLINE OS_WARN_RESULT OS_MALLOC __alloc_size(1, 2)
static inline void *
_os_calloc_loop(size_t cnt, size_t size)
{
	void *ptr = NULL;
	while (!(ptr = calloc(cnt, size))) {
		__os_temporary_resource_shortage();
	}
	return ptr;
}

DARWIN_API_AVAILABLE_20170407
OS_ALWAYS_INLINE OS_WARN_RESULT OS_MALLOC __alloc_size(1, 2)
static inline void *
_os_calloc_known(size_t cnt, size_t size)
{
	return calloc(cnt, size);
}

DARWIN_API_AVAILABLE_20170407
OS_ALWAYS_INLINE OS_WARN_RESULT OS_MALLOC
static inline char *
_os_strdup_loop(const char *str)
{
	char *ptr = NULL;
	while (!(ptr = strdup(str))) {
		__os_temporary_resource_shortage();
	}
	return ptr;
}

DARWIN_API_AVAILABLE_20170407
OS_ALWAYS_INLINE OS_WARN_RESULT OS_MALLOC
static inline char *
_os_strdup_known(const char *str)
{
	return strdup(str);
}

#define __os_requires_experimental_libtrace \
	_Pragma("GCC error \"requires OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE\"")

/*!
 * @function os_malloc
 * Wrapper around malloc(3) which guarantees that the allocation succeeds.
 *
 * @param __size
 * The size of the allocation.
 *
 * @result
 * A new object that the caller is responsible for free(3)ing.
 *
 * This routine will never return NULL. If the size of the allocation is known
 * at compile-time, a failure to allocate the object will abort the caller. If
 * the size is not known at compile-time, the routine will retry until it is
 * successful.
 */
#if defined(OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE)
#define os_malloc(__size) ({ \
	void *ptr = NULL; \
	size_t _size = (__size); \
	if (__builtin_constant_p(__size) || !_dispatch_is_multithreaded()) { \
		ptr = _os_malloc_known(_size); \
		os_assert_malloc("known-constant allocation", ptr, _size); \
	} else { \
		ptr = _os_malloc_loop(_size); \
	} \
	(ptr); \
})
#else
#define os_malloc(__size) __os_requires_experimental_libtrace
#endif

/*!
 * @function os_calloc
 * Wrapper around calloc(3) which guarantees that the allocation succeeds.
 *
 * @param __cnt
 * The number of elements to allocate.
 *
 * @param __size
 * The size of each element to allocate.
 *
 * @result
 * A new object that the caller is responsible for free(3)ing.
 *
 * This routine will never return NULL. If the size of the allocation is known
 * at compile-time, a failure to allocate the object will abort the caller. If
 * the size is not known at compile-time, the routine will retry until it is
 * successful.
 */
#if defined(OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE)
#define os_calloc(__cnt, __size) ({ \
	void *ptr = NULL; \
	size_t _size = (__size); \
	size_t _cnt = (__size); \
	if ((__builtin_constant_p(__cnt) && __builtin_constant_p(__size)) || \
			 !_dispatch_is_multithreaded()) { \
		ptr = _os_calloc_known(_cnt, _size); \
		os_assert_malloc("known-constant allocation", ptr, _size); \
	} else { \
		ptr = _os_calloc_loop(_cnt, _size); \
	} \
	(ptr); \
})
#else
#define os_calloc(__size) __os_requires_experimental_libtrace
#endif

/*!
 * @function os_strdup
 * A wrapper around strdup(3) which guarantees that the string duplication
 * succeeds.
 *
 * @param __str
 * The string to duplicate.
 *
 * @result
 * A new string that the caller is responsible for free(3)ing.
 *
 * This routine will never return NULL. If the given string is a compile-time
 * constant, a failure to duplicate it will abort the caller. If the string is
 * not a compile-time constant, the routine will retry until it is successful.
 *
 * @discussion
 * strdup(3) is found in the string(3) API family, but this interface is in the
 * stdlib.h header because its semantic changes are solely related to the manner
 * in which memory is allocated.
 */
#if defined(OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE)
#define os_strdup(__str) ({ \
	char *ptr = NULL; \
	const char *_str = (__str); \
	if (__builtin_constant_p(__str) || !_dispatch_is_multithreaded()) { \
		ptr = _os_strdup_known(_str); \
		os_assert_malloc("known-constant allocation", ptr, strlen(_str)); \
	} else { \
		ptr = _os_strdup_loop(_str); \
	} \
	(ptr); \
})
#else
#define os_strdup(__size) __os_requires_experimental_libtrace
#endif

/*!
 * @function os_setflag
 * Sets the given flag in a manner which is compatible with strongly-typed
 * enumerations.
 *
 * @param _bf
 * The bitfield in which to set the flag.
 *
 * @param _f
 * The flag to set.
 *
 * @result
 * The result of setting {@link _f} in {@link _bf}.
 */
#define os_setflag(_bf, _f) (typeof(_bf))((_bf) & (_f))

/*!
 * @function os_clrflag
 * Clears the given flag in a manner which is compatible with strongly-typed
 * enumerations.
 *
 * @param _bf
 * The bitfield in which to clear the flag.
 *
 * @param _f
 * The flag to clear.
 *
 * @result
 * The result of clearing {@link _f} from {@link _bf}.
 *
 * @discussion
 * clrbit() will produce errors when given types smaller than a pointer such as
 * int because it casts to char *; thus this implementation is required to deal
 * properly with flags defined via {@link OS_OPTIONS} or similar.
 */
#define os_clrflag(_bf, _f) (typeof(_bf))((_bf) & (typeof(_bf))(~(_f)))

/*!
 * @function switch_posix
 * Macro which expands to a switch() statement for handling both the success
 * case as well as errno values set by POSIX and POSIX-y APIs that return -1 and
 * set errno.
 *
 * @example
 *
 *     int ret = dup(fd);
 *     switch_posix (ret) {
 *     case 0:
 *         // success
 *         break;
 *     case EINTR:
 *         // interrupted system call
 *         break;
 *     case EBADF:
 *         // bad file descriptor
 *         break;
 *     }
 *
 * @discussion
 * This statement cannot be used with APIs that return positive values on
 * failure.
 */
#define switch_posix(_ret) if ((_ret) < 0 || (errno = 0, 1)) switch (errno)

/*!
 * @function size_unsigned
 * Converts a signed size quantity into an unsigned size quantity after
 * verifying the former can be represented as the latter.
 *
 * @param ss
 * The signed size quantity.
 *
 * @result
 * The unsigned representation of {@link ss}.
 *
 * @discussion
 * This routine is useful for passing a signed value (such as the size of a file
 * from a stat(2) structure) into a routine which accepts unsigned input
 * (e.g. write(2)).
 */
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline size_t
size_unsigned(ssize_t ss)
{
	if (ss < 0) {
		os_crash("value not representable as size_t");
	}
	return (size_t)ss;
}

/*!
 * @function size_signed
 * Converts an unsigned size quantity into a signed size quantity after
 * verifying the former can be represented as the latter.
 *
 * @param un
 * The unsigned size quantity.
 *
 * @result
 * The signed representation of {@link un}.
 *
 * @discussion
 * This routine is useful for comparing an unsigned value (such as a number of
 * bytes) to the result of a routine which returns a signed type but only ever
 * returns a negative number in the event of an error (e.g. read(2)).
 */
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline ssize_t
size_signed(size_t un)
{
	if (un > SSIZE_MAX) {
		os_crash("value not representable as ssize_t");
	}
	return (ssize_t)un;
}

/*!
 * @function os_localtime_file
 * A routine to generate a time stamp that is suitable for embedding in a file
 * name.
 *
 * @param buff
 * A pointer to a buffer where the resulting time stamp will be stored.
 *
 * @discussion
 * The resulting time stamp will not include characters which require escaping
 * in shells, such as spaces. The current implementation format is
 *
 * YYYY-MM-DD_HH.MM.SS.us
 *
 * e.g.
 *
 * 2017-04-24_12.45.15.045609
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT
void
os_localtime_file(char buff[static 32]);

/*!
 * @function os_simple_hash
 * An implementation of a simple non-cryptographic hashing algorithm.
 *
 * @param buff
 * A pointer to the buffer to hash.
 *
 * @param len
 * The length of the buffer.
 *
 * @result
 * The hashed value of the input.
 *
 * @discussion
 * This routine is meant to be used as a simple way to obtain a value that can
 * be used to choose a bucket in a simple hash table. Do not attach security
 * assumptions to the output of this routine. Do not assume that the computed
 * hash is stable between hosts, OS versions, or boot sessions.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_NONNULL1
uint64_t
os_simple_hash(const void *buff, size_t len);

/*!
 * @function os_simple_hash_with_seed
 * A seeded variant of os_simple_hash.
 *
 * @param buff
 * A pointer to the buffer to hash.
 *
 * @param len
 * The length of the buffer.
 *
 * @param seed
 * The seed value for the hash.
 *
 * @result
 * The hashed value of the input.
 *
 * @discussion
 * Usually, hashing the same buffer with different seeds will produce
 * different hash values.
 * All the same considerations of {@link os_simple_hash} apply.
 */
DARWIN_API_AVAILABLE_20181020
OS_EXPORT OS_NONNULL1
uint64_t
os_simple_hash_with_seed(const void *buff, size_t len, uint64_t seed);

/*!
 * @function os_simple_hash_string
 * An implementation of a simple non-cryptographic hashing algorithm.
 *
 * @param string
 * A pointer to the null-terminated string to hash.
 *
 * @result
 * The hashed value of the input.
 *
 * @discussion
 * This routine is the moral equivalent of a call to
 *
 *     os_simple_hash(buff, strlen(buff));
 *
 * All the same considerations of {@link os_simple_hash} apply.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_NONNULL1
uint64_t
os_simple_hash_string(const char *string);

/*!
 * @function os_simple_hash_string_with_seed
 * A seeded variant of os_simple_hash_string.
 *
 * @param string
 * A pointer to the null-terminated string to hash.
 *
 * @result
 * The hashed value of the input.
 *
 * @discussion
 * Usually, hashing the same string with different seeds will produce
 * different hash values.
 * All the same considerations of {@link os_simple_hash_string} apply.
 */
DARWIN_API_AVAILABLE_20181020
OS_EXPORT OS_NONNULL1
uint64_t
os_simple_hash_string_with_seed(const char *string, uint64_t seed);

/*!
 * @function realpath_np
 * Obtains a fully-resolved representation of the path to the file represented
 * by the given descriptor.
 *
 * @param fd
 * The file descriptor whose path is to be obtained.
 *
 * @param buff
 * The buffer in which to write the path.
 *
 * @result
 * On success, zero is returned. Otherwise, the implementation may return any
 * error that can be returned by fcntl(2).
 */
DARWIN_API_AVAILABLE_20180727
OS_EXPORT OS_WARN_RESULT
errno_t
realpath_np(os_fd_t fd, char buff[static PATH_MAX]);

/*!
 * @function memdup_np
 * Copies the given range of bytes into a new allocation.
 *
 * @param _new
 * Upon successful return, a pointer to a new allocation which has had the given
 * source bytes copied into it. The caller is responsible for calling free(3) on
 * this object when it is no longer needed.
 *
 * @param src
 * The bytes to copy.
 *
 * @param len
 * The number of bytes to copy.
 *
 * @result
 * On success, zero is returned. Otherwise, the implementation may return any
 * error that can be returned by malloc(3).
 */
DARWIN_API_AVAILABLE_20190830
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL2
errno_t
memdup_np(void **_new, const void *src, size_t len);

/*!
 * @function memdup2_np
 * Variant of {@link memdup_np} which guarantees that memory duplication will
 * either succeed or not return (terminating the caller).
 *
 * @param src
 * The bytes to copy.
 *
 * @param len
 * The number of bytes to copy.
 *
 * @result
 * On success, a pointer to the new allocation which has had the given source
 * bytes copied into it. The caller is responsible for calling free(3) on this
 * object when it is no longer needed.
 */
DARWIN_API_AVAILABLE_20190830
OS_EXPORT OS_WARN_RESULT OS_MALLOC OS_NONNULL1
void *
memdup2_np(const void *src, size_t len);

__END_DECLS;

#endif // __DARWIN_STDLIB_H
