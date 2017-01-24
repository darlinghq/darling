/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef _PLATFORM_STRING_H_
#define _PLATFORM_STRING_H_

#include <sys/cdefs.h>
#include <Availability.h>
#include <TargetConditionals.h>

#include <stdint.h>
#include <sys/types.h>

#define _PLATFORM_OPTIMIZED_BZERO 0
#define _PLATFORM_OPTIMIZED_MEMCCPY 0
#define _PLATFORM_OPTIMIZED_MEMCHR 0
#define _PLATFORM_OPTIMIZED_MEMCMP 0
#define _PLATFORM_OPTIMIZED_MEMMOVE 0
#define _PLATFORM_OPTIMIZED_MEMSET 0
#define _PLATFORM_OPTIMIZED_MEMSET_PATTERN4 0
#define _PLATFORM_OPTIMIZED_MEMSET_PATTERN8 0
#define _PLATFORM_OPTIMIZED_MEMSET_PATTERN16 0
#define _PLATFORM_OPTIMIZED_STRCHR 0
#define _PLATFORM_OPTIMIZED_STRCMP 0
#define _PLATFORM_OPTIMIZED_STRNCMP 0

/* Primitives used to implement C memory and string routines */

__BEGIN_DECLS

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
void
_platform_bzero(void *s, size_t n);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
void *
_platform_memchr(const void *s, int c, size_t n);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
int
_platform_memcmp(const void *s1, const void *s2, size_t n);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
void *
_platform_memmove(void *dst, const void *src, size_t n);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
void *
_platform_memccpy(void *restrict dst, const void *restrict src, int c, size_t n);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
void *
_platform_memset(void *b, int c, size_t len);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
void
_platform_memset_pattern4(void *b, const void *pattern4, size_t len);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
void
_platform_memset_pattern8(void *b, const void *pattern8, size_t len);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
void
_platform_memset_pattern16(void *b, const void *pattern16, size_t len);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
char *
_platform_strchr(const char *s, int c);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
int
_platform_strcmp(const char *s1, const char *s2);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
int
_platform_strncmp(const char *s1, const char *s2, size_t n);

__END_DECLS

#endif /* _PLATFORM_STRING_H_ */
