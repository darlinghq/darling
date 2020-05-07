/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All rights reserved.
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
/*
 * @OSF_COPYRIGHT@
 *
 */

#ifndef _MACHINE_ENDIAN_H_
#define _MACHINE_ENDIAN_H_

/*
 * Definitions for byte order,
 * according to byte significance from low address to high.
 */
#define LITTLE_ENDIAN   1234    /* least-significant byte first (vax) */
#define BIG_ENDIAN      4321    /* most-significant byte first (IBM, net) */
#define PDP_ENDIAN      3412    /* LSB first in word, MSW first in long (pdp) */

#define BYTE_ORDER      LITTLE_ENDIAN   /* byte order on i386 */
#define ENDIAN          LITTLE

/*
 * Macros for network/external number representation conversion.
 * Use GNUC support to inline the byteswappers.
 */

#if !defined(ntohs)
static __inline__ unsigned short        ntohs(unsigned short);
static __inline__
unsigned short
ntohs(unsigned short w_int)
{
	return (w_int << 8) | (w_int >> 8);
}
#endif

#if !defined(htons)
unsigned short  htons(unsigned short);
#define htons   ntohs
#endif

#if !defined(ntohl)
static __inline__ unsigned long ntohl(unsigned long);
static __inline__
unsigned long
ntohl(unsigned long value)
{
#if defined(__clang__)
	return (unsigned long)__builtin_bswap32((unsigned int)value);
#else
	unsigned long l = value;
	__asm__ volatile ("bswap %0" : "=r" (l) : "0" (l));
	return l;
#endif
}
#endif

#if !defined(htonl)
unsigned long   htonl(unsigned long);
#define htonl   ntohl
#endif

#define NTOHL(x)        (x) = ntohl((unsigned long)x)
#define NTOHS(x)        (x) = ntohs((unsigned short)x)
#define HTONL(x)        (x) = htonl((unsigned long)x)
#define HTONS(x)        (x) = htons((unsigned short)x)

#endif
