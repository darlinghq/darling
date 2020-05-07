/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _NET_FLOWHASH_H_
#define _NET_FLOWHASH_H_

#include <sys/types.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * If 32-bit hash value is too large, use this macro to truncate
 * it to n-bit; masking is a faster operation than modulus.
 */
#define HASHMASK(n)     ((1UL << (n)) - 1)

/*
 * Returns 32-bit hash value.  Hashes which are capable of returning
 * more bits currently have their results truncated to 32-bit.
 */
typedef u_int32_t net_flowhash_fn_t(const void *, u_int32_t, const u_int32_t);

extern net_flowhash_fn_t *net_flowhash;
extern net_flowhash_fn_t net_flowhash_mh3_x86_32;
extern net_flowhash_fn_t net_flowhash_mh3_x64_128;
extern net_flowhash_fn_t net_flowhash_jhash;
#ifdef  __cplusplus
}
#endif

#endif /* _NET_FLOWHASH_H_ */
