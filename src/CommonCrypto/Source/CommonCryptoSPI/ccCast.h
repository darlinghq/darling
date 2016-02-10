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
 * ccCast.h - shim between openssl-based CAST and CommonEncryption.
 *
 * Created 3/30/06 by Doug Mitchell. 
 */

#ifndef	_CC_CCCAST_H_
#define _CC_CCCAST_H_

#include <CommonCrypto/CommonCryptoPriv.h>
#include <CommonCrypto/cast.h>
#include <sys/types.h>

#ifdef  __cplusplus
extern "C" {
#endif

int cast_cc_set_key(
	CAST_KEY *cx, 
	const void *rawKey, 
	size_t keyLength,
	int forEncrypt);

void cast_cc_encrypt(CAST_KEY *cx, const void *blockIn, void *blockOut);
void cast_cc_decrypt(CAST_KEY *cx, const void *blockIn, void *blockOut);

#ifdef  __cplusplus
}
#endif

#endif	/* _CC_CCCAST_H_ */
