/* 
 * Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.
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
 * CommonDigestPriv.h - private typedefs and defines for ComonCrypto digest routines
 */
 
#ifndef	_COMMON_DIGEST_PRIV_H_
#define _COMMON_DIGEST_PRIV_H_

#include <CommonCrypto/CommonCryptoPriv.h>
#include <CommonCrypto/CommonDigest.h>
#include "tomcrypt.h"

/* MD2 */

#define MD2_DIGEST_LENGTH	CC_MD2_DIGEST_LENGTH
#define MD2_BLOCK			CC_MD2_BLOCK_LONG
typedef	CC_MD2_CTX			MD2_CTX;
typedef CC_LONG				MD2_INT;

/* MD4 */

#define MD4_DIGEST_LENGTH 	CC_MD4_DIGEST_LENGTH
#define MD4_CBLOCK			CC_MD4_BLOCK_BYTES
#define MD4_LBLOCK			(MD4_CBLOCK/4)
#define MD4_LONG_LOG2 		3
typedef CC_MD4_CTX			MD4_CTX;
typedef CC_LONG				MD4_LONG;

/* MD5 */

#define MD5_DIGEST_LENGTH 	CC_MD5_DIGEST_LENGTH
#define MD5_CBLOCK			CC_MD5_BLOCK_BYTES
#define MD5_LBLOCK			(MD5_CBLOCK/4)
typedef CC_MD5_CTX			MD5_CTX;
typedef CC_LONG				MD5_LONG;

/* SHA1 */

#define SHA_DIGEST_LENGTH 	CC_SHA1_DIGEST_LENGTH
#define SHA_CBLOCK			CC_SHA1_BLOCK_BYTES
#define SHA_LBLOCK			CC_SHA1_BLOCK_LONG
#define SHA_LONG_LOG2 		2
#define SHA_LAST_BLOCK  	(SHA_CBLOCK-8)
typedef CC_SHA1_CTX			SHA_CTX;
typedef CC_LONG				SHA_LONG;

/* 
 * Macro to make an algorithm-specific one shot.
 */
#define CC_DIGEST_ONE_SHOT(fcnName, ctxName, initFcn, updateFcn, finalFcn)	\
unsigned char * fcnName (const void *data, CC_LONG len, unsigned char *md) \
{									\
	ctxName ctx;					\
	if(md == NULL) {				\
		return NULL;				\
	}								\
	initFcn(&ctx);					\
	updateFcn(&ctx, data, len);		\
	finalFcn(md, &ctx);				\
	return md;						\
}

typedef struct ccDigest_s {
	hash_state	md;
	int hashIndex;
} CCDigestCtx_t, *CCDigestCtxPtr;

#endif	/* _COMMON_DIGEST_PRIV_H_ */
