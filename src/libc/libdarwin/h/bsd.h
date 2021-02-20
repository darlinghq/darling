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
 * Darwin-specific additions for FreeBSD APIs.
 */
#ifndef __DARWIN_BSD_H
#define __DARWIN_BSD_H

#include <os/base.h>
#include <os/api.h>
#include <sys/cdefs.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#if DARWIN_TAPI
#include "tapi.h"
#endif

__BEGIN_DECLS;

/*!
 * @function sysctl_get_data_np
 * A convenience routine for getting a sysctl(3) property whose size is not
 * known at compile-time.
 *
 * @param mib
 * An array describing the property to manipulate. This is a "management
 * information base"-style descriptor, as described in sysctl(3).
 *
 * @param mib_cnt
 * The number of items in the MIB array.
 *
 * @param buff
 * On successful return, a pointer to a newly-allocated buffer. The caller is
 * responsible for free(3)ing this buffer when it is no longer needed.
 *
 * @param buff_len
 * On successful return, the length of the returned buffer.
 *
 * @result
 * See the sysctl(3) man page for possible return codes.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL3 OS_NONNULL4
errno_t
sysctl_get_data_np(int *mib, size_t mib_cnt, void **buff, size_t *buff_len);

/*!
 * @function sysctlbyname_get_data_np
 * A convenience routine for getting a sysctl(3) property whose size is not
 * known at compile-time.
 *
 * @param mibdesc
 * An ASCII representation of the MIB vector describing the property to
 * manipulate. Each element of the vector described is separated by a '.'
 * character (e.g. "kern.ostype").
 *
 * @param buff
 * On successful return, a pointer to a newly-allocated buffer. The caller is
 * responsible for free(3)ing this buffer when it is no longer needed.
 *
 * @param buff_len
 * On successful return, the length of the returned buffer.
 *
 * @result
 * See the sysctl(3) man page for possible return codes.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL2 OS_NONNULL3
errno_t
sysctlbyname_get_data_np(const char *mibdesc, void **buff, size_t *buff_len);

/*!
 * @function os_parse_boot_arg_int
 * A routine for extracting a boot-arg as an integer value that is semantically
 * similar to the PE_parse_boot_argn() kernel routine.
 *
 * @param which
 * The name of the boot-arg whose value is to be obtained. The caller may pass
 * NULL to simply check for the existence of a boot-arg.
 *
 * @param where
 * On successful return, the integer value of the given boot-arg.
 *
 * @result
 * A Boolean indicating whether the named argument was found. If the discovered
 * argument value was not convertible to an integer according to the contract
 * in strtoll(3), the implementation will return false.
 *
 * @discussion
 * Boot-args are expressed with an '=' sign as a separator between the name and
 * value of an argument, e.g. "cs_enforcement_disable=1".
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_NONNULL1
bool
os_parse_boot_arg_int(const char *which, int64_t *where);

/*!
 * @function os_parse_boot_arg_string
 * A routine for extracting a boot-arg's string value that is semantically
 * similar to the PE_parse_boot_argn() kernel routine.
 *
 * @param which
 * The name of the boot-arg whose value is to be obtained.
 *
 * @param where
 * The buffer in which to place the extracted value on successful return. The
 * caller may pass NULL to simply check for the existence of a boot-arg.
 *
 * @param maxlen
 * The length of the {@link where} buffer. May be zero if the caller only wishes
 * to check for the existence of a boot-arg.
 *
 * @result
 * A Boolean indicating whether the named argument was found.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_NONNULL1
bool
os_parse_boot_arg_string(const char *which, char *where, size_t maxlen);

__END_DECLS;

#endif // __DARWIN_BSD_H
