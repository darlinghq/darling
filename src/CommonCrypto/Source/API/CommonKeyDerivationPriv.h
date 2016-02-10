/*
 *  CommonKeyDerivationPriv.h
 *  CommonCrypto
 *
 *  Created by Richard Murphy on 1/13/10.
 *  Copyright 2010 McKenzie-Murphy. All rights reserved.
 *
 */

#include "CommonKeyDerivation.h"

/* Uncomment this if you want to enable using a pure digest as the PRF.  There currently
 * isn't a standard way of doing this.  We prepend the "key" material in front of the data
 * and hash the result in the current implementation.
 */

// #define USE_DIGEST_PRF

/* Reserved PRF IDs */

#if defined(USE_DIGEST_PRF)

enum {
	kCCPRFSHA1 = 6,
	kCCPRFSHA224 = 7,
	kCCPRFSHA256 = 8,
	kCCPRFSHA384 = 9,
	kCCPRFSHA512 = 10
};

static u_int8_t *
prefixSalt(const char *password, size_t passwordLen, u_int8_t *salt, size_t saltLen, u_int8_t *output);

#endif /* USE_DIGEST_PRF */

static size_t
getPRFhlen(CCPseudoRandomAlgorithm prf);


static void 
PRF(CCPseudoRandomAlgorithm prf, const char *password, size_t passwordLen,
		 u_int8_t *salt, size_t saltLen, u_int8_t *output);

static uint64_t 
timer(void);

/* 
	This or some version of it might go into the public API at some point
	the idea is that if some process goes wild while someone uses CCCalibratePBKDF()
	to determine the number of rounds to use for a desired amount of delay per attack
	the resulting number can be "sniff tested" to determine if it's in the ballpark
	and a "floor" can be set for the round count.
	
	The number "MIN_ROUNDS_PER_MSEC" is the result of running CCCalibratePBKDF for a
	10 char password and salt, HMAC-SHA256 to generate a 16 byte key on a 2.5 ghz macbook pro.
	
	To use the macro you'd do the following:
	
		rounds = CCCalibratePBKDF(kCCPBKDF2, passwordLen, saltLen, kCCPRFHmacAlgSHA256, derivedKeyLen, msec);
		rounds = FLOOR_ROUNDS(msec, rounds);
*/

#define MIN_ROUNDS_PER_MSEC 410
#define FLOOR_ROUNDS(MSEC,INROUNDS) (((INROUNDS)/(MSEC)) < MIN_ROUNDS_PER_MSEC) ? ((MSEC) * MIN_ROUNDS_PER_MSEC) : INROUNDS


