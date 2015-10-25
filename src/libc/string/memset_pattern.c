/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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


#include <stdio.h>

void
bcopy(const void *src0, void *dst0, size_t length);

void
memset_pattern4(void *b, const void *pattern4, size_t len)
{
	char * start = (char *)b;
	char * p = (char *)b;
	while ((start + len) - p >= 4) {
		bcopy(pattern4, p, 4);
		p += 4;
	}
	if ((start + len) - p != 0) {
		bcopy(pattern4, p, (start + len) - p);
	}
}

void
memset_pattern8(void *b, const void *pattern8, size_t len)
{
	char * start = (char *)b;
	char * p = (char *)b;
	while ((start + len) - p >= 8) {
		bcopy(pattern8, p, 8);
		p += 8;
	}
	if ((start + len) - p != 0) {
		bcopy(pattern8, p, (start + len) - p);
	}

}

void
memset_pattern16(void *b, const void *pattern16, size_t len)
{
	char * start = (char *)b;
	char * p = (char *)b;
	while ((start + len) - p >= 16) {
		bcopy(pattern16, p, 16);
		p += 16;
	}
	if ((start + len) - p != 0) {
		bcopy(pattern16, p, (start + len) - p);
	}
}
