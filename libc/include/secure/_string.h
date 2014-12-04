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

#include <sys/cdefs.h>
#include <secure/_common.h>

#if _USE_FORTIFY_LEVEL > 0

/* memcpy, mempcpy, memmove, memset, strcpy, stpcpy, strncpy, stpncpy,
   strcat, and strncat */

#undef memcpy
#undef memmove
#undef memset
#undef strcpy
#if __DARWIN_C_LEVEL >= 200809L
#undef stpcpy
#undef stpncpy
#endif
#undef strncpy
#undef strcat
#if ! (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && __IPHONE_OS_VERSION_MIN_REQUIRED < 32000)
#undef strncat
#endif

#define memcpy(dest, src, len)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___memcpy_chk (dest, src, len, __darwin_obsz0 (dest))	\
   : __inline_memcpy_chk (dest, src, len))

static __inline void *
__inline_memcpy_chk (void *__dest, const void *__src, size_t __len)
{
  return __builtin___memcpy_chk (__dest, __src, __len, __darwin_obsz0(__dest));
}

#define memmove(dest, src, len)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___memmove_chk (dest, src, len, __darwin_obsz0 (dest))	\
   : __inline_memmove_chk (dest, src, len))

static __inline void *
__inline_memmove_chk (void *__dest, const void *__src, size_t __len)
{
  return __builtin___memmove_chk (__dest, __src, __len, __darwin_obsz0(__dest));
}

#define memset(dest, val, len)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___memset_chk (dest, val, len, __darwin_obsz0 (dest))	\
   : __inline_memset_chk (dest, val, len))

static __inline void *
__inline_memset_chk (void *__dest, int __val, size_t __len)
{
  return __builtin___memset_chk (__dest, __val, __len, __darwin_obsz0(__dest));
}

#define strcpy(dest, src)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___strcpy_chk (dest, src, __darwin_obsz (dest))		\
   : __inline_strcpy_chk (dest, src))

static __inline char *
__inline_strcpy_chk (char *__restrict __dest, const char *__restrict __src)
{
  return __builtin___strcpy_chk (__dest, __src, __darwin_obsz(__dest));
}

#if __DARWIN_C_LEVEL >= 200809L
#define stpcpy(dest, src)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___stpcpy_chk (dest, src, __darwin_obsz (dest))		\
   : __inline_stpcpy_chk (dest, src))

static __inline char *
__inline_stpcpy_chk (char *__dest, const char *__src)
{
  return __builtin___stpcpy_chk (__dest, __src, __darwin_obsz(__dest));
}

#define stpncpy(dest, src, len)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___stpncpy_chk (dest, src, len, __darwin_obsz (dest))	\
   : __inline_stpncpy_chk (dest, src, len))

static __inline char *
__inline_stpncpy_chk (char *__restrict __dest, const char *__restrict __src,
		      size_t __len)
{
  return __builtin___stpncpy_chk (__dest, __src, __len, __darwin_obsz(__dest));
}
#endif

#define strncpy(dest, src, len)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___strncpy_chk (dest, src, len, __darwin_obsz (dest))	\
   : __inline_strncpy_chk (dest, src, len))

static __inline char *
__inline_strncpy_chk (char *__restrict __dest, const char *__restrict __src,
		      size_t __len)
{
  return __builtin___strncpy_chk (__dest, __src, __len, __darwin_obsz(__dest));
}

#define strcat(dest, src)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___strcat_chk (dest, src, __darwin_obsz (dest))		\
   : __inline_strcat_chk (dest, src))

static __inline char *
__inline_strcat_chk (char *__restrict __dest, const char *__restrict __src)
{
  return __builtin___strcat_chk (__dest, __src, __darwin_obsz(__dest));
}

#if ! (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && __IPHONE_OS_VERSION_MIN_REQUIRED < 32000)
#define strncat(dest, src, len)					\
  ((__darwin_obsz0 (dest) != (size_t) -1)				\
   ? __builtin___strncat_chk (dest, src, len, __darwin_obsz (dest))	\
   : __inline_strncat_chk (dest, src, len))

static __inline char *
__inline_strncat_chk (char *__restrict __dest, const char *__restrict __src,
		      size_t __len)
{
  return __builtin___strncat_chk (__dest, __src, __len, __darwin_obsz(__dest));
}
#endif

#endif
#endif
