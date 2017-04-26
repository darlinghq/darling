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

#ifndef	_CC_CryptorSPI_H_
#define _CC_CryptorSPI_H_

#include <sys/types.h>
#include <sys/param.h>
#include <stdint.h>

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <Availability.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
	This is an SPI header.  It includes some work in progress implementation notes that
	will be removed when this is promoted to an API set.
*/

/*
	Cipher Modes
*/

enum {
	kCCModeECB		= 1,
	kCCModeCBC		= 2,
	kCCModeCFB		= 3,
	kCCModeCTR		= 4,
	kCCModeF8		= 5, // Unimplemented for now (not included)
	kCCModeLRW		= 6, // Unimplemented for now (not included)
	kCCModeOFB		= 7,
	kCCModeXTS		= 8,
	kCCModeRC4		= 9, // RC4 as a streaming cipher is handled internally as a mode.
	kCCModeCFB8		= 10,
};
typedef uint32_t CCMode;

/*
	Padding for block ciphers
*/

enum {
	ccDefaultPadding	= 0,
	ccPKCS7Padding		= 1,
	ccANSIx923Padding	= 2, // Unimplemented for now (not included)
	ccISO10126Padding	= 3, // Unimplemented for now (not included)
};
typedef uint32_t CCPadding;

/*
	Mode options - so far only used for CTR mode
*/

enum {
	kCCModeOptionCTR_LE	= 0x0001, // CTR Mode Little Endian
	kCCModeOptionCTR_BE = 0x0002  // CTR Mode Big Endian
};

typedef uint32_t CCModeOptions;

/*
	Supports a mode call of 
	int mode_setup(int cipher, const unsigned char *IV, const unsigned char *key, int keylen, 
		const unsigned char *tweak, int tweaklen, int num_rounds, int options, mode_context *ctx);
*/

/* User supplied space for the CryptorRef */

CCCryptorStatus CCCryptorCreateFromDataWithMode(
	CCOperation 	op,				/* kCCEncrypt, kCCEncrypt, kCCBoth (default for BlockMode) */
	CCMode			mode,
	CCAlgorithm		alg,
	CCPadding		padding,		
	const void 		*iv,			/* optional initialization vector */
	const void 		*key,			/* raw key material */
	size_t 			keyLength,	
	const void 		*tweak,			/* raw tweak material */
	size_t 			tweakLength,	
	int				numRounds,
	CCModeOptions 	options,
	const void		*data,			/* caller-supplied memory */
	size_t			dataLength,		/* length of data in bytes */
	CCCryptorRef	*cryptorRef,	/* RETURNED */
	size_t			*dataUsed)		/* optional, RETURNED */
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/* This version mallocs the CCCryptorRef */

CCCryptorStatus CCCryptorCreateWithMode(
	CCOperation 	op,				/* kCCEncrypt, kCCEncrypt, kCCBoth (default for BlockMode) */
	CCMode			mode,
	CCAlgorithm		alg,
	CCPadding		padding,		
	const void 		*iv,			/* optional initialization vector */
	const void 		*key,			/* raw key material */
	size_t 			keyLength,	
	const void 		*tweak,			/* raw tweak material */
	size_t 			tweakLength,	
	int				numRounds,		/* 0 == default */
	CCModeOptions 	options,
	CCCryptorRef	*cryptorRef)	/* RETURNED */
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*
	Assuming we can use existing CCCryptorCreateFromData for all modes serviced by these:
	int mode_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, mode_context *ctx);
	int mode_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, mode_context *ctx);
*/

/*
	Block mode encrypt and decrypt interfaces for IV tweaked blocks (XTS and CBC)
	
	int mode_encrypt_tweaked(const unsigned char *pt, unsigned long len, unsigned char *ct, const unsigned char *tweak, mode_context *ctx);
	int mode_decrypt_tweaked(const unsigned char *ct, unsigned long len, unsigned char *pt, const unsigned char *tweak, mode_context *ctx);
*/

CCCryptorStatus CCCryptorEncryptDataBlock(
	CCCryptorRef cryptorRef,
	const void *iv,
	const void *dataIn,
	size_t dataInLength,
	void *dataOut)
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);


CCCryptorStatus CCCryptorDecryptDataBlock(
	CCCryptorRef cryptorRef,
	const void *iv,
	const void *dataIn,
	size_t dataInLength,
	void *dataOut)
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*
	Assuming we can use the existing CCCryptorRelease() interface for 
	int mode_done(mode_context *ctx);
*/

/*
	Not surfacing these other than with CCCryptorReset()
	
	int mode_setiv(const unsigned char *IV, unsigned long len, mode_context *ctx);
	int mode_getiv(const unsigned char *IV, unsigned long *len, mode_context *ctx);
*/
    
/*
    DES key utilities
*/
    
CCCryptorStatus CCDesIsWeakKey(
                               void *key,
                               size_t Length)
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

void CCDesSetOddParity(
                       void *key,
                       size_t Length)
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

uint32_t CCDesCBCCksum(void *input, void *output,
                       size_t length, void *key, size_t keylen,
                       void *ivec)
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);
    
    

#ifdef __cplusplus
}
#endif

#endif /* _CC_CryptorSPI_H_ */
