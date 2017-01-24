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

#include <platform/string.h>

#if !_PLATFORM_OPTIMIZED_MEMSET

void *
_platform_memset(void *b, int c, size_t len) {
	unsigned char pattern[4];

	pattern[0] = (unsigned char)c;
	pattern[1] = (unsigned char)c;
	pattern[2] = (unsigned char)c;
	pattern[3] = (unsigned char)c;

	_platform_memset_pattern4(b, pattern, len);
	return b;
}

#if VARIANT_STATIC
void *
memset(void *b, int c, size_t len) {
	return _platform_memset(b, c, len);
}
#endif

#endif


#if !_PLATFORM_OPTIMIZED_BZERO

void
_platform_bzero(void *s, size_t n)
{
	_platform_memset(s, 0, n);
}

#if VARIANT_STATIC
void
bzero(void *s, size_t n) {
	_platform_bzero(s, n);
}

void
__bzero(void *s, size_t n) {
	_platform_bzero(s, n);
}
#endif

#endif
