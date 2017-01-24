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

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <machine/cpu_capabilities.h>

#include <_simple.h>

#include <platform/string.h>
#include <platform/compat.h>

__attribute__ ((visibility ("hidden")))
uintptr_t commpage_pfz_base=0;

__attribute__ ((visibility ("hidden")))
void
__pfz_setup(const char *apple[])
{
    const char *p = _simple_getenv(apple, "pfz");
    if (p != NULL) {
        const char *q;

        /* We are given hex starting with 0x */
        if (p[0] != '0' || p[1] != 'x') {
            goto __pfz_setup_clear;
        }

        for (q = p + 2; *q; q++) {
            commpage_pfz_base <<= 4; // *= 16

            if ('0' <= *q && *q <= '9') {
                commpage_pfz_base += *q - '0';
            } else if ('a' <= *q && *q <= 'f') {
                commpage_pfz_base += *q - 'a' + 10;
            } else if ('A' <= *q && *q <= 'F') {
                commpage_pfz_base += *q - 'A' + 10;
            } else {
                commpage_pfz_base=0;
                goto __pfz_setup_clear;
            }
        }

__pfz_setup_clear:
        bzero((void *)((uintptr_t)p - 4), strlen(p) + 4);
    }

    if (commpage_pfz_base == 0) {
        commpage_pfz_base = _COMM_PAGE_TEXT_START;
    }
}
