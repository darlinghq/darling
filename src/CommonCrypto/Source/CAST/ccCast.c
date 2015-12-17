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

/*
 * ccCAST.c - shim between openssl-based CAST and CommonEncryption.
 *
 * Created 3/30/06 by Doug Mitchell. 
 */

#include <CommonCrypto/ccCast.h>
#include <sys/types.h>

int cast_cc_set_key(
	CAST_KEY *cx, 
	const void *rawKey, 
	size_t keyLength,
	int forEncrypt)
{
	CAST_set_key(cx, keyLength, rawKey);
	return 0;
}

void cast_cc_encrypt(CAST_KEY *cx, const void *blockIn, void *blockOut)
{
	CAST_ecb_encrypt((const unsigned char *)blockIn, (unsigned char *)blockOut,
		cx, CAST_ENCRYPT);
}

void cast_cc_decrypt(CAST_KEY *cx, const void *blockIn, void *blockOut)
{
	CAST_ecb_encrypt((const unsigned char *)blockIn, (unsigned char *)blockOut,
		cx, CAST_DECRYPT);
}

