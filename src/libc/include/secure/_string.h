/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#ifndef _STRING_H_
# error "Never use <secure/_string.h> directly; include <string.h> instead."
#endif

#ifndef _SECURE__STRING_H_
#define _SECURE__STRING_H_

#include <Availability.h>
#include <sys/cdefs.h>
#include <secure/_common.h>

#if _USE_FORTIFY_LEVEL > 0

#ifndef __has_builtin
#define _undef__has_builtin
#define __has_builtin(x) 0
#endif

/* <rdar://problem/12622659> */
#if defined(__clang__) && \
    ((defined(__apple_build_version__) && __apple_build_version__ >= 4260006) || \
     (!defined(__apple_build_version__) && (__clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 3))))
#define __HAS_FIXED_CHK_PROTOTYPES 1
#else
#define __HAS_FIXED_CHK_PROTOTYPES 0
#endif

/* memccpy, memcpy, mempcpy, memmove, memset, strcpy, strlcpy, stpcpy,
   strncpy, stpncpy, strcat, strlcat, and strncat */

#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 70000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 1090
#if __has_builtin(__builtin___memccpy_chk) && __HAS_FIXED_CHK_PROTOTYPES
#undef memccpy
#define memccpy(dest, src, c, len)                                  \
  __builtin___memccpy_chk (dest, src, c, len, __darwin_obsz0 (dest))
#endif
#endif

#if __has_builtin(__builtin___memcpy_chk) || defined(__GNUC__)
#undef memcpy
#define memcpy(dest, src, len)					\
  __builtin___memcpy_chk (dest, src, len, __darwin_obsz0 (dest))
#endif

#if __has_builtin(__builtin___memmove_chk) || defined(__GNUC__)
#undef memmove
#define memmove(dest, src, len)					\
  __builtin___memmove_chk (dest, src, len, __darwin_obsz0 (dest))
#endif

#if __has_builtin(__builtin___memset_chk) || defined(__GNUC__)
#undef memset
#define memset(dest, val, len)					\
  __builtin___memset_chk (dest, val, len, __darwin_obsz0 (dest))
#endif

#if __has_builtin(__builtin___strcpy_chk) || defined(__GNUC__)
#undef strcpy
#define strcpy(dest, src)					\
  __builtin___strcpy_chk (dest, src, __darwin_obsz (dest))
#endif

#if __DARWIN_C_LEVEL >= 200809L
#if __has_builtin(__builtin___stpcpy_chk) || defined(__GNUC__)
#undef stpcpy
#define stpcpy(dest, src)					\
  __builtin___stpcpy_chk (dest, src, __darwin_obsz (dest))
#endif

#if __has_builtin(__builtin___stpncpy_chk) || __APPLE_CC__ >= 5666 || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
#undef stpncpy
#define stpncpy(dest, src, len)					\
  __builtin___stpncpy_chk (dest, src, len, __darwin_obsz (dest))
#endif
#endif /* _DARWIN_C_LEVEL >= 200809L */

#if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 70000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 1090
#if __has_builtin(__builtin___strlcpy_chk) && __HAS_FIXED_CHK_PROTOTYPES
#undef strlcpy
#define strlcpy(dest, src, len)                                 \
  __builtin___strlcpy_chk (dest, src, len, __darwin_obsz (dest))
#endif

#if __has_builtin(__builtin___strlcat_chk) && __HAS_FIXED_CHK_PROTOTYPES
#undef strlcat
#define strlcat(dest, src, len)                                 \
  __builtin___strlcat_chk (dest, src, len, __darwin_obsz (dest))
#endif
#endif /* __IPHONE_OS_VERSION_MIN_REQUIRED >= 70000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 1090 */
#endif /* __DARWIN_C_LEVEL >= __DARWIN_C_FULL */

#if __has_builtin(__builtin___strncpy_chk) || defined(__GNUC__)
#undef strncpy
#define strncpy(dest, src, len)					\
  __builtin___strncpy_chk (dest, src, len, __darwin_obsz (dest))
#endif

#if __has_builtin(__builtin___strcat_chk) || defined(__GNUC__)
#undef strcat
#define strcat(dest, src)					\
  __builtin___strcat_chk (dest, src, __darwin_obsz (dest))
#endif

#if ! (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && __IPHONE_OS_VERSION_MIN_REQUIRED < 32000)
#if __has_builtin(__builtin___strncat_chk) || defined(__GNUC__)
#undef strncat
#define strncat(dest, src, len)					\
  __builtin___strncat_chk (dest, src, len, __darwin_obsz (dest))
#endif
#endif

#ifdef _undef__has_builtin
#undef _undef__has_builtin
#undef __has_builtin
#endif

#undef __HAS_FIXED_CHK_PROTOTYPES

#endif /* _USE_FORTIFY_LEVEL > 0 */
#endif /* _SECURE__STRING_H_ */
