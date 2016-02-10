/*
 * Copyright (c) 2010 Apple Inc. All Rights Reserved.
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

#include "CommonDigest.h"
#include "CommonDigestPriv.h"
#include "CommonDigestSPI.h"
#include "tomcrypt.h"
#include "skein_ltc.h"

#define SELECT_HASH(X,Y,Z) case X: \
	ptr->hashIndex = register_hash(&Y); \
	break
	
	
int 
CCDigestInit(CCDigestAlg algorithm, CCDigestRef ctx)
{
	CCDigestCtxPtr ptr = (CCDigestCtxPtr) ctx;
	
	switch(algorithm) {
	SELECT_HASH(kCCDigestMD2,md2_desc,7);
	SELECT_HASH(kCCDigestMD4,md4_desc,6);
	SELECT_HASH(kCCDigestMD5,ltc_md5_desc,3);
	SELECT_HASH(kCCDigestRMD128,rmd128_desc,8);
	SELECT_HASH(kCCDigestRMD160,rmd160_desc,9);
	SELECT_HASH(kCCDigestRMD256,rmd256_desc,11);
	SELECT_HASH(kCCDigestRMD320,rmd320_desc,12);
	SELECT_HASH(kCCDigestSHA1,sha1_desc,2);
	SELECT_HASH(kCCDigestSHA224,sha224_desc,10);
	SELECT_HASH(kCCDigestSHA256,sha256_desc,0);
	SELECT_HASH(kCCDigestSHA384,sha384_desc,4);
	SELECT_HASH(kCCDigestSHA512,sha512_desc,5);
	SELECT_HASH(kCCDigestSkein128,skein512_128_desc,SKEIN512_128_LTC_TAG);
	SELECT_HASH(kCCDigestSkein160,skein512_160_desc,SKEIN512_160_LTC_TAG);
	SELECT_HASH(kCCDigestSkein224,skein512_224_desc,SKEIN512_224_LTC_TAG);
	SELECT_HASH(kCCDigestSkein256,skein512_256_desc,SKEIN512_256_LTC_TAG);
	SELECT_HASH(kCCDigestSkein384,skein512_384_desc,SKEIN512_384_LTC_TAG);
	SELECT_HASH(kCCDigestSkein512,skein512_512_desc,SKEIN512_512_LTC_TAG);
	default: return -1;
	}
	return hash_descriptor[ptr->hashIndex].init(&ptr->md);
}

int
CCDigestUpdate(CCDigestRef ctx, const void *data, size_t len)
{
	CCDigestCtxPtr ptr = (CCDigestCtxPtr) ctx;
    uint8_t* checkedData = (const uint8_t *)data;
    if (checkedData == NULL) {
        checkedData = "";
    }
	return hash_descriptor[ptr->hashIndex].process(&ptr->md, checkedData, len);
}

int
CCDigestFinal(CCDigestRef ctx, uint8_t *output)
{
	CCDigestCtxPtr ptr = (CCDigestCtxPtr) ctx;
	return hash_descriptor[ptr->hashIndex].done(&ptr->md, output);
}

int
CCDigest(CCDigestAlg algorithm, const uint8_t *data, size_t length, uint8_t *output)
{
	CCDigestCtx_t c;
	CCDigestCtxPtr ptr = &c;
	CCDigestInit(algorithm, ptr);
    uint8_t* checkedData = data;
    if (checkedData == NULL) {
        checkedData = "";
    }
	hash_descriptor[ptr->hashIndex].process(&ptr->md, checkedData, length);
	return hash_descriptor[ptr->hashIndex].done(&ptr->md, output);
}

CCDigestRef
CCDigestCreate(CCDigestAlg algorithm)
{
    CCDigestRef ctx;
    
    if(!(ctx = (CCDigestCtx *) malloc(CC_DIGEST_SIZE))) {
        return NULL;
    }
    if(CCDigestInit(algorithm, ctx) != 0) { 
        free(ctx); 
        return NULL; 
    }
    return ctx;
}

void
CCDigestDestroy(CCDigestRef ctx)
{
    bzero(ctx, sizeof(CCDigestCtx));
    free(ctx);
}

void
CCDigestReset(CCDigestRef ctx)
{
	CCDigestCtxPtr ptr = (CCDigestCtxPtr) ctx;
    hash_descriptor[ptr->hashIndex].init(&ptr->md);
}

int
CCDigestInterrimResult(CCDigestRef ctx, uint8_t *output)
{
    CCDigestCtx tmp;
    
    memcpy(&tmp, ctx, CC_DIGEST_SIZE);
    return CCDigestFinal(&tmp, output);
}

size_t
CCDigestBlockSize(CCDigestRef ctx) 
{
    size_t retval = 0;
	CCDigestCtxPtr ptr = (CCDigestCtxPtr) ctx;
    if(ptr) retval = hash_descriptor[ptr->hashIndex].blocksize;
    return retval;
}

size_t
CCDigestOutputSize(CCDigestRef ctx) 
{
    size_t retval = 0;
	CCDigestCtxPtr ptr = (CCDigestCtxPtr) ctx;
    if(ptr) retval = hash_descriptor[ptr->hashIndex].hashsize;
    return retval;
}

