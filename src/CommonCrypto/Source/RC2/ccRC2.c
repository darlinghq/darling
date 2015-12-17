/* 
 * Copyright (c) 2006 Apple Computer, Inc. All Rights Reserved.
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

#include "rc2.h"
#include "ccRC2.h"
#include <sys/types.h>

int rc2_cc_set_key(
	RC2_Schedule *cx, 
	const void *rawKey, 
	size_t keyLength)
{
	rc2_keyschedule(cx, rawKey, keyLength, keyLength*8);
	return 0;
}

void rc2_cc_encrypt(RC2_Schedule *cx, const void *blockIn, void *blockOut)
{
	rc2_encrypt(cx, (const unsigned char *)blockIn, (unsigned char *)blockOut);
}

void rc2_cc_decrypt(RC2_Schedule *cx, const void *blockIn, void *blockOut)
{
	rc2_decrypt(cx, (unsigned char *)blockOut, (const unsigned char *)blockIn);
}

