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
 * CommonHMAC.h - Keyed Message Authentication Code (HMAC) functions.
 *
 * Created 3/27/2006 by Doug Mitchell.
 */

#include <CommonCrypto/CommonHMAC.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>

#ifndef	NDEBUG
#define ASSERT(s)
#else
#define ASSERT(s)	assert(s)
#endif

/* 
 * Callouts for digest ops.
 * The void *ctx pointers are needed to accommodate different underlying
 * digest context types. 
 */
typedef void (*ccDigestInit)(void *ctx);
typedef void (*ccDigestUpdate)(void *ctx, const void *data, CC_LONG len);
typedef void (*ccDigestFinal)(unsigned char *md, void *ctx);

#define HMAC_MAX_BLOCK_SIZE		CC_SHA512_BLOCK_BYTES
#define HMAC_MAX_DIGEST_SIZE	CC_SHA512_DIGEST_LENGTH

/* 
 * This is what a CCHmacContext actually points to.
 */
typedef struct {
	uint32_t			digestLen;
	uint32_t			blockLen;
	union {
		CC_MD5_CTX		md5Ctx;
		CC_SHA1_CTX		sha1Ctx;
		CC_SHA256_CTX 	sha256Ctx;
		CC_SHA512_CTX 	sha512Ctx;
	} digest;
	uint8_t				k_opad[HMAC_MAX_BLOCK_SIZE];	/* max block size */
	
	ccDigestInit		digestInit;
	ccDigestUpdate		digestUpdate;
	ccDigestFinal		digestFinal;
} _CCHmacContext;

void CCHmacInit(
	CCHmacContext *ctx, 
	CCHmacAlgorithm algorithm,	/* kCCHmacSHA1, kCCHmacMD5 */
	const void *key,
	size_t keyLength)			/* length of key in bytes */
{
	_CCHmacContext	*hmacCtx = (_CCHmacContext *)ctx;
	uint8_t			tk[HMAC_MAX_DIGEST_SIZE];
	uint8_t			*keyP;
	uint32_t		byte;
	uint8_t			k_ipad[HMAC_MAX_BLOCK_SIZE]; 

	/* if this fails, it's time to adjust CC_HMAC_CONTEXT_SIZE */
	ASSERT(sizeof(_CCHmacContext) < sizeof(CCHmacContext));
	
	if(hmacCtx == NULL) {
		return;
	}
	
	memset(hmacCtx, 0, sizeof(*hmacCtx));
	
	switch(algorithm) {
		case kCCHmacAlgMD5:
			hmacCtx->digestLen    = CC_MD5_DIGEST_LENGTH;
			hmacCtx->blockLen     = CC_MD5_BLOCK_BYTES;
			hmacCtx->digestInit   = (void *)CC_MD5_Init;
			hmacCtx->digestUpdate = (void *)CC_MD5_Update;
			hmacCtx->digestFinal  = (void *)CC_MD5_Final;
			break;
		case kCCHmacAlgSHA1:
			hmacCtx->digestLen    = CC_SHA1_DIGEST_LENGTH;
			hmacCtx->blockLen     = CC_SHA1_BLOCK_BYTES;
			hmacCtx->digestInit   = (void *)CC_SHA1_Init;
			hmacCtx->digestUpdate = (void *)CC_SHA1_Update;
			hmacCtx->digestFinal  = (void *)CC_SHA1_Final;
			break;
		case kCCHmacAlgSHA224:
			hmacCtx->digestLen    = CC_SHA224_DIGEST_LENGTH;
			hmacCtx->blockLen     = CC_SHA224_BLOCK_BYTES;
			hmacCtx->digestInit   = (void *)CC_SHA224_Init;
			hmacCtx->digestUpdate = (void *)CC_SHA224_Update;
			hmacCtx->digestFinal  = (void *)CC_SHA224_Final;
			break;
		case kCCHmacAlgSHA256:
			hmacCtx->digestLen    = CC_SHA256_DIGEST_LENGTH;
			hmacCtx->blockLen     = CC_SHA256_BLOCK_BYTES;
			hmacCtx->digestInit   = (void *)CC_SHA256_Init;
			hmacCtx->digestUpdate = (void *)CC_SHA256_Update;
			hmacCtx->digestFinal  = (void *)CC_SHA256_Final;
			break;
		case kCCHmacAlgSHA384:
			hmacCtx->digestLen    = CC_SHA384_DIGEST_LENGTH;
			hmacCtx->blockLen     = CC_SHA384_BLOCK_BYTES;
			hmacCtx->digestInit   = (void *)CC_SHA384_Init;
			hmacCtx->digestUpdate = (void *)CC_SHA384_Update;
			hmacCtx->digestFinal  = (void *)CC_SHA384_Final;
			break;
		case kCCHmacAlgSHA512:
			hmacCtx->digestLen    = CC_SHA512_DIGEST_LENGTH;
			hmacCtx->blockLen     = CC_SHA512_BLOCK_BYTES;
			hmacCtx->digestInit   = (void *)CC_SHA512_Init;
			hmacCtx->digestUpdate = (void *)CC_SHA512_Update;
			hmacCtx->digestFinal  = (void *)CC_SHA512_Final;
			break;
		default:
			return;
	}
	
	hmacCtx->digestInit(&hmacCtx->digest);
	
	/* If the key is longer than block size, reset it to key=digest(key) */
	if (keyLength <= hmacCtx->blockLen)
		keyP = (uint8_t *)key;
	else {
		hmacCtx->digestUpdate(&hmacCtx->digest, key, keyLength);
		hmacCtx->digestFinal(tk, &hmacCtx->digest);
		keyP = tk;
		keyLength = hmacCtx->digestLen;
		hmacCtx->digestInit(&hmacCtx->digest);
	}
	
	/* The HMAC_<DIG> transform looks like:
	   <DIG> (K XOR opad || <DIG> (K XOR ipad || text))
	   Where K is a n byte key
	   ipad is the byte 0x36 repeated 64 times.
	   opad is the byte 0x5c repeated 64 times.
	   text is the data being protected.
	  */
	/* Copy the key into k_ipad and k_opad while doing the XOR. */
	for (byte = 0; byte < keyLength; byte++)
	{
		k_ipad[byte] = keyP[byte] ^ 0x36;
		hmacCtx->k_opad[byte] = keyP[byte] ^ 0x5c;
	}
	/* Fill the remainder of k_ipad and k_opad with 0 XORed with the appropriate value. */
	if (keyLength < hmacCtx->blockLen)
	{
		memset (k_ipad + keyLength, 0x36, hmacCtx->blockLen - keyLength);
		memset (hmacCtx->k_opad + keyLength, 0x5c, hmacCtx->blockLen - keyLength);
	}
	hmacCtx->digestUpdate(&hmacCtx->digest, k_ipad, hmacCtx->blockLen);
}

void CCHmacUpdate(
	CCHmacContext *ctx, 
	const void *dataIn,
	size_t dataInLength)			/* length of data in bytes */
{
	_CCHmacContext	*hmacCtx = (_CCHmacContext *)ctx;
	hmacCtx->digestUpdate(&hmacCtx->digest, dataIn, dataInLength);
}

void CCHmacFinal(
	CCHmacContext *ctx, 
	void *macOut)
{
	_CCHmacContext	*hmacCtx = (_CCHmacContext *)ctx;
	hmacCtx->digestFinal(macOut, &hmacCtx->digest);
	hmacCtx->digestInit(&hmacCtx->digest);
	/* Perform outer digest */
	hmacCtx->digestUpdate(&hmacCtx->digest, hmacCtx->k_opad, hmacCtx->blockLen);
	hmacCtx->digestUpdate(&hmacCtx->digest, macOut, hmacCtx->digestLen);
	hmacCtx->digestFinal(macOut, &hmacCtx->digest);
}

/*
 * Stateless, one-shot HMAC function. 
 * Output is written to caller-spullied buffer, as in CCHmacFinal().
 */
void CCHmac(
	CCHmacAlgorithm algorithm,	/* kCCHmacSHA1, kCCHmacMD5 */
	const void *key,
	size_t keyLength,			/* length of key in bytes */
	const void *data,
	size_t dataLength,			/* length of data in bytes */
	void *macOut)				/* MAC written here */
{
	CCHmacContext ctx;
	
	CCHmacInit(&ctx, algorithm, key, keyLength);
	CCHmacUpdate(&ctx, data, dataLength);
	CCHmacFinal(&ctx, macOut);
}
