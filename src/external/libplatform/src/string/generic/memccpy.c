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

#if !_PLATFORM_OPTIMIZED_MEMCCPY

#include <stdlib.h>

void *
_platform_memccpy(void *t, const void *f, int c, size_t n)
{
	void *last;

	if (n == 0) {
		return NULL;
	}

	last = _platform_memchr(f, c, n);

	if (last == NULL) {
		_platform_memmove(t, f, n);
		return NULL;
	} else {
		n = (char *)last - (char *)f + 1;
		_platform_memmove(t, f, n);
		return (void *)((char *)t + n);
	}
}

#if VARIANT_STATIC
void *
memccpy(void *t, const void *f, int c, size_t n)
{
	return _platform_memccpy(t, f, c, n);
}
#endif

#endif
