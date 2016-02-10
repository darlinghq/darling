/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include "secure.h"

size_t
__strlcat_chk (char *restrict dest, char *restrict src,
	       size_t len, size_t dstlen)
{
  size_t initial_srclen;
  size_t initial_dstlen;

  if (__builtin_expect (dstlen < len, 0))
    __chk_fail_overflow ();

  initial_srclen = strlen(src);
  initial_dstlen = strnlen(dest, len);

  if (initial_dstlen == len)
    return len+initial_srclen;

  if (initial_srclen < len - initial_dstlen) {
    __chk_overlap(dest, initial_srclen + initial_dstlen + 1, src, initial_srclen + 1);
    memcpy(dest+initial_dstlen, src, initial_srclen + 1);
  } else {
    __chk_overlap(dest, initial_srclen + initial_dstlen + 1, src, len - initial_dstlen - 1);
    memcpy(dest+initial_dstlen, src, len - initial_dstlen - 1);
    dest[len-1] = '\0';
  }

  return initial_srclen + initial_dstlen;
}
