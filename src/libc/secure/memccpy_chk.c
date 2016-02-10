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

void *
__memccpy_chk (void *dest, const void *src, int c, size_t len, size_t dstlen)
{
  void *retval;

  if (__builtin_expect (dstlen < len, 0))
    __chk_fail_overflow ();

  /* retval is NULL if len was copied, otherwise retval is the
   * byte *after* the last one written.
   */
  retval = memccpy (dest, src, c, len);

  if (retval != NULL) {
    len = (uintptr_t)retval - (uintptr_t)dest;
  }

  __chk_overlap(dest, len, src, len);

  return retval;
}
