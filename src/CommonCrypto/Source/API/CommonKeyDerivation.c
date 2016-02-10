/*	$OpenBSD: pbkdf2.c,v 1.1 2008/06/14 06:28:27 djm Exp $	*/
/*
 * This version was derived by generalizing the original code by Damien Bergamini
 * for use with alternate pseudo-random functions.
 */

/*-
 * Copyright (c) 2008 Damien Bergamini <damien.bergamini@free.fr>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <stdio.h>

#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonHMAC.h>
#include "CommonKeyDerivation.h"
#include "CommonKeyDerivationPriv.h"

#define kCCPRFHmacAlgSHA1hlen	CC_SHA1_DIGEST_LENGTH
#define kCCPRFHmacAlgSHA224hlen CC_SHA224_DIGEST_LENGTH
#define kCCPRFHmacAlgSHA256hlen CC_SHA256_DIGEST_LENGTH
#define kCCPRFHmacAlgSHA384hlen CC_SHA384_DIGEST_LENGTH
#define kCCPRFHmacAlgSHA512hlen CC_SHA512_DIGEST_LENGTH

#if defined(USE_DIGEST_PRF)
#define kCCPRFSHA1hlen			CC_SHA1_DIGEST_LENGTH
#define kCCPRFSHA224hlen		CC_SHA224_DIGEST_LENGTH
#define kCCPRFSHA256hlen		CC_SHA256_DIGEST_LENGTH
#define kCCPRFSHA384hlen		CC_SHA384_DIGEST_LENGTH
#define kCCPRFSHA512hlen		CC_SHA512_DIGEST_LENGTH
#define kCCPRFSkeinhlen			CC_SHA512_DIGEST_LENGTH
#define kCCPRFSHA3hlen			CC_SHA512_DIGEST_LENGTH
#endif /* USE_DIGEST_PRF */

// This is for the scratchspace - it's twice the size of the max PRF buffer + 4 to work within the pbkdf2 code we currently
// have.

#define CC_MAX_PRF_WORKSPACE 128+4

static size_t
getPRFhlen(CCPseudoRandomAlgorithm prf)
{
	switch(prf) {
		case kCCPRFHmacAlgSHA1:		return kCCPRFHmacAlgSHA1hlen;
		case kCCPRFHmacAlgSHA224:	return kCCPRFHmacAlgSHA224hlen;
		case kCCPRFHmacAlgSHA256:	return kCCPRFHmacAlgSHA256hlen;
		case kCCPRFHmacAlgSHA384:	return kCCPRFHmacAlgSHA384hlen;
		case kCCPRFHmacAlgSHA512:	return kCCPRFHmacAlgSHA512hlen;
#if defined(USE_DIGEST_PRF)
		case kCCPRFSHA1:			return kCCPRFSHA1hlen;
		case kCCPRFSHA224:			return kCCPRFSHA224hlen;
		case kCCPRFSHA256:			return kCCPRFSHA256hlen;
		case kCCPRFSHA384:			return kCCPRFSHA384hlen;
		case kCCPRFSHA512:			return kCCPRFSHA512hlen;
#endif /* USE_DIGEST_PRF */
		default: return 0;
	}
}

#if defined(USE_DIGEST_PRF)
static u_int8_t *
prefixSalt(const char *password, size_t passwordLen, u_int8_t *salt, size_t saltLen, u_int8_t *output)
{	
	memcpy(output, salt, saltLen);
	memcpy(output+saltLen, password, passwordLen);
	return output;
}
#endif /* USE_DIGEST_PRF */


static void 
PRF(CCPseudoRandomAlgorithm prf, const char *password, size_t passwordLen, u_int8_t *salt, size_t saltLen, u_int8_t *output)
{
	u_int8_t tmpbuf[1024]; // ZZZfor now
	
	switch(prf) {
		case kCCPRFHmacAlgSHA1:	
			CCHmac(kCCHmacAlgSHA1, password, passwordLen, salt, saltLen, output);
			break;
		case kCCPRFHmacAlgSHA224:
			CCHmac(kCCHmacAlgSHA224, password, passwordLen, salt, saltLen, output);
			break;
		case kCCPRFHmacAlgSHA256:
			CCHmac(kCCHmacAlgSHA256, password, passwordLen, salt, saltLen, output);
			break;
		case kCCPRFHmacAlgSHA384:
			CCHmac(kCCHmacAlgSHA384, password, passwordLen, salt, saltLen, output);
			break;
		case kCCPRFHmacAlgSHA512:
			CCHmac(kCCHmacAlgSHA512, password, passwordLen, salt, saltLen, output);
			break;
#if defined(USE_DIGEST_PRF)
		case kCCPRFSHA1:
			CC_SHA1(prefixSalt(password, passwordLen, salt, saltLen, tmpbuf), saltLen+passwordLen, output);
			break;
		case kCCPRFSHA224:
			CC_SHA224(prefixSalt(password, passwordLen, salt, saltLen, tmpbuf), saltLen+passwordLen, output);
			break;
		case kCCPRFSHA256:
			CC_SHA256(prefixSalt(password, passwordLen, salt, saltLen, tmpbuf), saltLen+passwordLen, output);
			break;
		case kCCPRFSHA384:
			CC_SHA384(prefixSalt(password, passwordLen, salt, saltLen, tmpbuf), saltLen+passwordLen, output);
			break;
		case kCCPRFSHA512:
			CC_SHA512(prefixSalt(password, passwordLen, salt, saltLen, tmpbuf), saltLen+passwordLen, output);
			break;
#endif /* USE_DIGEST_PRF */
			
	}
}


int 
CCKeyDerivationPBKDF( CCPBKDFAlgorithm algorithm, const char *password, size_t passwordLen,
					 const uint8_t *salt, size_t saltLen,
					 CCPseudoRandomAlgorithm prf, uint rounds, 
					 uint8_t *derivedKey, size_t derivedKeyLen)
{
	u_int8_t oldbuffer[CC_MAX_PRF_WORKSPACE], newbuffer[CC_MAX_PRF_WORKSPACE], 
			 saltCopy[CC_MAX_PRF_WORKSPACE+4], collector[CC_MAX_PRF_WORKSPACE];
	int rawblock, i, j, r, nblocks;
	size_t	hlen, offset;
	
	if(algorithm != kCCPBKDF2) return -1;
	
	/*
	 * Check initial parameters
	 */
	
	if (rounds < 1 || derivedKeyLen == 0)
		return -1; // bad parameters
	if (saltLen == 0 || saltLen > CC_MAX_PRF_WORKSPACE)
		return -1; // out of bounds parameters
	
	hlen = getPRFhlen(prf);
		
	/*
	 * FromSpec: Let l be the number of hLen-octet blocks in the derived key, rounding up, 
	 * and let r be the number of octets in the last block:
	 */
	
	nblocks = (derivedKeyLen+hlen-1)/hlen; // in the spec nblocks is referred to as l
	r = derivedKeyLen % hlen;
    r = (r) ? r: hlen;
		
	/*
	 * Make a copy of the salt buffer so we can concatenate the
	 * block counter for each series of rounds.
	 */
	
	memcpy(saltCopy, salt, saltLen);
	bzero(derivedKey, derivedKeyLen);
	
	/*
	 * FromSpec: 
	 *
	 * For each block of the derived key apply the function F defined below to the password P,
	 * the salt S, the iteration count c, and the block index to compute the block:
	 * 
	 *           F(P,S,c,i)=U1 \xorU2 \xor⋅⋅⋅\xorUc
	 *
	 * where
	 *				U1 =PRF(P,S||INT (i)),
	 *				U2 =PRF(P,U1), 
	 *				... 
	 *				Uc = PRF (P, Uc-1) .
	 */
	
	for(rawblock = 0; rawblock < nblocks; rawblock++) {
		int block = rawblock+1;
		int copyLen;

		offset = rawblock * hlen;
		copyLen = (block != nblocks) ? hlen: r;
		
		/*
		 * FromSpec: Here, INT (i) is a four-octet encoding of the integer i, most significant octet first.
		 */
		
		for(i=0; i<4; i++) saltCopy[saltLen+i] = (block >> 8*(3-i)) & 0xff;
		
		PRF(prf, password, passwordLen, saltCopy, saltLen+4, oldbuffer);					// Initial PRF with the modified salt
		
		memcpy(collector, oldbuffer, hlen);												// Initial value for this block of the derived key.
		
		for(i = 1; i < rounds; i++) {
			PRF(prf, password, passwordLen, oldbuffer, hlen, newbuffer);						// Subsequent PRF with the previous result as the salt
			memcpy(oldbuffer, newbuffer, hlen);
			for(j = 0; j < hlen; j++) collector[j] ^= newbuffer[j];					// Xoring the round into the collector
		}
		memcpy(derivedKey+offset, collector, copyLen);
	}
	
	/*
	 * Clear temp buffers.
	 */
	
	bzero(oldbuffer, CC_MAX_PRF_WORKSPACE);
	bzero(newbuffer, CC_MAX_PRF_WORKSPACE);
	bzero(collector, CC_MAX_PRF_WORKSPACE);
	bzero(saltCopy, CC_MAX_PRF_WORKSPACE+4);
	
	return 0;
}

#ifndef KERNEL
#include <mach/mach.h>
#include <mach/mach_time.h>
#define ROUNDMEASURE 100000
static uint64_t
timer()
{
	static mach_timebase_info_data_t    sTimebaseInfo;
    uint64_t        timeNano;

	if ( sTimebaseInfo.denom == 0 ) {
        (void) mach_timebase_info(&sTimebaseInfo);
    }
	
	timeNano = mach_absolute_time();
	return (uint64_t) (timeNano * sTimebaseInfo.numer) / (sTimebaseInfo.denom * 1000000);
}

uint
CCCalibratePBKDF(CCPBKDFAlgorithm algorithm, size_t passwordLen, size_t saltLen,
				 CCPseudoRandomAlgorithm prf, size_t derivedKeyLen, uint32_t msec)
{
	char        *password;
	uint8_t     *salt;
	uint64_t	startTime, endTime, elapsedTime;
	uint8_t     *derivedKey;
	int         i;

	if (derivedKeyLen == 0) return -1; // bad parameters
	if (saltLen == 0 || saltLen > CC_MAX_PRF_WORKSPACE) return -1; // out of bounds parameters
	if (passwordLen == 0 ) return -1; // out of bounds parameters
	if(algorithm != kCCPBKDF2) return -1;
    
	if((password = malloc(passwordLen)) == NULL) return -1;
	for(i=0; i<passwordLen; i++) password[i] = 'a';
	if((salt = malloc(saltLen)) == NULL) return -1;
	for(i=0; i<saltLen; i++) salt[i] = i%256;
	if((derivedKey = malloc(derivedKeyLen)) == NULL) return -1;

    for(elapsedTime = 0, i=0; i < 5 && elapsedTime == 0; i++) {
        startTime = timer();
        if(CCKeyDerivationPBKDF(algorithm, password, passwordLen, salt, saltLen, prf, ROUNDMEASURE, derivedKey, derivedKeyLen)) return -2;
        endTime = timer();

        elapsedTime = endTime - startTime;
	}
    
    if(elapsedTime == 0) return 123456; // arbitrary, but something is seriously wrong
    
	free(password);
	free(salt);
	free(derivedKey);
	   
	return (msec * ROUNDMEASURE)/elapsedTime;
}
#endif
