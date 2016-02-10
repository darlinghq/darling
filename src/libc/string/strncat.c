/*
 * Copyright (c) 2011 Apple, Inc. All rights reserved.
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

#include <string.h>

char *
strncat(char *restrict dst, const char *restrict src, size_t maxlen) {
	const size_t dstlen = strlen(dst);
	const size_t srclen = strnlen(src, maxlen);
    //  The strcat() and strncat() functions append a copy of the null-
    //  terminated string src to the end of the null-terminated string dst,
    //  then add a terminating '\0'.  The string dst must have sufficient
    //  space to hold the result.
    //
    //  The strncat() function appends not more than maxlen characters
    //  from src, and then adds a terminating '\0'.
    const size_t cpylen = srclen < maxlen ? srclen : maxlen;
    memcpy(dst+dstlen, src, cpylen);
    dst[dstlen+cpylen] = '\0';
    //  The strcat() and strncat() functions return dst.
    return dst;
}
