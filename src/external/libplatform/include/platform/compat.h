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

#ifndef _PLATFORM_COMPAT_H_
#define _PLATFORM_COMPAT_H_

#include <platform/string.h>

__BEGIN_DECLS

/* Helpers for other common non-primitive routines */

__header_always_inline
size_t
_platform_strlen(const char *s) {
	const char *t = _platform_memchr(s, '\0', SIZE_MAX);
	return (uintptr_t)t - (uintptr_t)s;
}

__header_always_inline
size_t
_platform_strlcpy(char * restrict dst, const char * restrict src, size_t maxlen) {
	const size_t srclen = _platform_strlen(src);
	if (srclen < maxlen) {
		_platform_memmove(dst, src, srclen+1);
	} else if (maxlen != 0) {
		_platform_memmove(dst, src, maxlen-1);
		dst[maxlen-1] = '\0';
	}
	return srclen;
}

__END_DECLS

/* Compat macros for primitives */
#define bzero            _platform_bzero
#define memchr           _platform_memchr
#define memcmp           _platform_memcmp
#define memmove          _platform_memmove
#define memccpy          _platform_memccpy
#define memset           _platform_memset
#define memset_pattern4  _platform_memset_pattern4
#define memset_pattern8  _platform_memset_pattern8
#define memset_pattern16 _platform_memset_pattern16
#define strchr           _platform_strchr
#define strcmp           _platform_strcmp
#define strncmp          _platform_strncmp

/* Compat macros for non-primitive helpers */
#define strlcpy          _platform_strlcpy
#define strlen           _platform_strlen

#endif /* _PLATFORM_COMPAT_H_ */
