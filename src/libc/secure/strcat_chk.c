/*
 * Copyright (c) 2007-2013 Apple Inc. All rights reserved.
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
#include <stdio.h>
#include "secure.h"

char *
__strcat_chk (char *__restrict dest, const char *__restrict append,
	      size_t dstlen)
{
  size_t len1 = strlen(dest);
  size_t len2 = strlen(append);

  if (__builtin_expect (dstlen < len1 + len2 + 1, 0))
    __chk_fail_overflow ();

  if (__builtin_expect (__chk_assert_no_overlap != 0, 1))
    __chk_overlap(dest, len1 + len2 + 1, append, len2 + 1);

  memcpy(dest + len1, append, len2 + 1);
  return dest;
}
