/*
 * Copyright (c) 1999-2012 Apple Computer, Inc. All rights reserved.
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

#if !defined(KERNEL)
#include <MacTypes.h>
#endif  /* !KERNEL */

#ifndef _OS_OSTYPES_H
#define _OS_OSTYPES_H

#define OSTYPES_K64_REV         2

typedef unsigned int       UInt;
typedef signed int         SInt;

#if defined(KERNEL)

typedef unsigned char      UInt8;
typedef unsigned short     UInt16;
#if __LP64__
typedef unsigned int       UInt32;
#else
typedef unsigned long      UInt32;
#endif
typedef unsigned long long UInt64;
#if             defined(__BIG_ENDIAN__)
typedef struct __attribute__((deprecated)) UnsignedWide {
	UInt32          hi;
	UInt32          lo;
}                                                       UnsignedWide __attribute__((deprecated));
#elif           defined(__LITTLE_ENDIAN__)
typedef struct __attribute__((deprecated)) UnsignedWide {
	UInt32          lo;
	UInt32          hi;
}                                                       UnsignedWide __attribute__((deprecated));
#else
#error Unknown endianess.
#endif

typedef signed char        SInt8;
typedef signed short       SInt16;
#if __LP64__
typedef signed int         SInt32;
#else
typedef signed long        SInt32;
#endif
typedef signed long long   SInt64;

typedef SInt32                          OSStatus;

#ifndef ABSOLUTETIME_SCALAR_TYPE
#define ABSOLUTETIME_SCALAR_TYPE    1
#endif
typedef UInt64          AbsoluteTime;

typedef UInt32                          OptionBits __attribute__((deprecated));

#if defined(__LP64__)
/*
 * Use intrinsic boolean types for the LP64 kernel, otherwise maintain
 * source and binary backward compatibility.  This attempts to resolve
 * the "(x == true)" vs. "(x)" conditional issue.
 */
#ifdef __cplusplus
typedef bool Boolean;
#else   /* !__cplusplus */
#if defined(__STDC_VERSION__) && ((__STDC_VERSION__ - 199901L) > 0L)
/* only use this if we are sure we are using a c99 compiler */
typedef _Bool Boolean;
#else   /* !c99 */
/* Fall back to previous definition unless c99 */
typedef unsigned char Boolean;
#endif  /* !c99 */
#endif  /* !__cplusplus */
#else   /* !__LP64__ */
typedef unsigned char Boolean;
#endif  /* !__LP64__ */

#endif  /* KERNEL */

#include <sys/_types/_os_inline.h>

#endif /* _OS_OSTYPES_H */
