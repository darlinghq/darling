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
 * CommonCryptor.c - common crypto context.
 *
 */

#include "CommonCryptor.h"
#include "CommonCryptorSPI.h"
#include "CommonCryptorPriv.h"
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <stddef.h>			/* for offsetof() */
#include "tomcrypt.h"
#include "rc4.h"
#include "pkcs7pad.h"
#include "ccdebug.h"

/* 
 * CommonCryptor's portion of a CCCryptorRef. 
 */


#define DEFAULT_CRYPTOR_MALLOC 4096

#define OPENCRYPT "Encrypt"
#define OPDECRYPT "Decrypt"

#define NOPADDING "OFF"
#define PKCS7PADDING "PKCS7"

/*
 * Convert a CommonCrypto alg into an LTC index, register LTC alg as a side-effect.
 */
 
static int
ccGetAlgorithm(CCAlgorithm alg)
{
	switch(alg) {
	case kCCAlgorithmAES128:
		if(register_cipher(&aesedp_desc) == -1) return -1;
		return find_cipher("aesedp");
	case kCCAlgorithmDES:	
		if(register_cipher(&des_desc) == -1) return -1;
		return find_cipher("des");
	case kCCAlgorithm3DES:	
		if(register_cipher(&des3_desc) == -1) return -1;
		return find_cipher("3des");
	case kCCAlgorithmCAST:
		if(register_cipher(&cast5_desc) == -1) return -1;
		return find_cipher("cast5");
	case kCCAlgorithmRC2:
		if(register_cipher(&rc2_desc) == -1) return -1;
		return find_cipher("rc2");
	default:
		return -1;
	}
}


static CCCryptorStatus
ccLTCErr(uint32_t err)
{
	switch(err) {
	case CRYPT_OK: return kCCSuccess;
	case CRYPT_ERROR: return -1;
	case CRYPT_NOP: return kCCSuccess;

	case CRYPT_INVALID_KEYSIZE: return kCCParamError;
	case CRYPT_INVALID_ROUNDS: return kCCParamError;
	case CRYPT_FAIL_TESTVECTOR: return -1;

	case CRYPT_BUFFER_OVERFLOW: return kCCBufferTooSmall;
	case CRYPT_INVALID_PACKET: return kCCParamError;

	case CRYPT_INVALID_PRNGSIZE: return kCCParamError;
	case CRYPT_ERROR_READPRNG: return kCCParamError;

	case CRYPT_INVALID_CIPHER: return kCCParamError;
	case CRYPT_INVALID_HASH: return kCCParamError;
	case CRYPT_INVALID_PRNG: return kCCParamError;

	case CRYPT_MEM: return kCCBufferTooSmall;

	case CRYPT_PK_TYPE_MISMATCH: return kCCParamError;
	case CRYPT_PK_NOT_PRIVATE: return kCCParamError;

	case CRYPT_INVALID_ARG: return kCCParamError;
	case CRYPT_FILE_NOTFOUND: return kCCParamError;

	case CRYPT_PK_INVALID_TYPE: return kCCParamError;
	case CRYPT_PK_INVALID_SYSTEM: return kCCParamError;
	case CRYPT_PK_DUP: return kCCParamError;
	case CRYPT_PK_NOT_FOUND: return kCCParamError;
	case CRYPT_PK_INVALID_SIZE: return kCCParamError;

	case CRYPT_INVALID_PRIME_SIZE: return kCCParamError;
	case CRYPT_PK_INVALID_PADDING: return kCCParamError;

	case CRYPT_HASH_OVERFLOW: return kCCParamError;
	default: return -1;
	}
}


/*
 Generally IVs are the same size as the blocksize of the cipher in use.
 This routine will use that value unless it's going to blow past the end
 of the cryptor IV buffer.  This is just a safety measure in case some
 upper level code is blindly triggering an IV reset for a cipher/mode
 combo that really wouldn't use the IV and may have a huge blocksize.
 */

#define CC_XZEROMEM(p, n)	memset((p), 0, (n))
#define CC_XMEMCPY(s1, s2, n) memcpy((s1), (s2), (n))

static void
ccSetIV(CCCryptor *cryptor, void *iv)
{
	uint32_t ivSize;
    
    ivSize = (cryptor->blocksize > CCMAXBUFFERSIZE) ? CCMAXBUFFERSIZE: cryptor->blocksize;
	if(iv == NULL) {
		CC_XZEROMEM(cryptor->iv, ivSize);
	} else {
    	CC_XMEMCPY(cryptor->iv, iv, ivSize);
    }
}

CCCryptorStatus CCCryptorCreate(
	CCOperation op,             /* kCCEncrypt, etc. */
	CCAlgorithm alg,            /* kCCAlgorithmDES, etc. */
	CCOptions options,          /* kCCOptionPKCS7Padding, etc. */
	const void *key,            /* raw key material */
	size_t keyLength,	
	const void *iv,             /* optional initialization vector */
	CCCryptorRef *cryptorRef)  /* RETURNED */
{
	CCCryptorStatus err;
	CCCompatCryptor *compat_cryptor = NULL;
	size_t dataUsed = 0;
	
	if((compat_cryptor = (CCCompatCryptor *)malloc(sizeof(CCCompatCryptor))) == NULL) return kCCMemoryFailure;
	err = CCCryptorCreateFromData( op, alg,  options, key,  keyLength, iv, compat_cryptor, sizeof(CCCompatCryptor), cryptorRef, &dataUsed); 
	if(err != kCCSuccess)  free(compat_cryptor);
	else compat_cryptor->weMallocd = true;
	return err;
}

CCCryptorStatus CCCryptorCreateFromData(
	CCOperation op,             /* kCCEncrypt, etc. */
	CCAlgorithm alg,            /* kCCAlgorithmDES, etc. */
	CCOptions options,          /* kCCOptionPKCS7Padding, etc. */
	const void *key,            /* raw key material */
	size_t keyLength,	
	const void *iv,             /* optional initialization vector */
	const void *data,			/* caller-supplied memory */
	size_t dataLength,			/* length of data in bytes */
	CCCryptorRef *cryptorRef,   /* RETURNED */
	size_t *dataUsed)			/* optional, RETURNED */
{
	CCMode			mode;
    CCPadding		padding;		
	const void 		*tweak;
	size_t 			tweakLength;	
	int				numRounds;
	CCModeOptions 	modeOptions;
    
	/* Determine mode from options - old call only supported ECB and CBC 
     we treat RC4 as a "mode" in that it's the only streaming cipher
     currently supported 
     */
    
	mode = kCCModeCBC;
    if(alg == kCCAlgorithmRC4) mode = kCCModeRC4;
    else if(options & kCCOptionECBMode) mode = kCCModeECB;
    
	/* Determine padding from options - only PKCS7 was available */
    padding = ccDefaultPadding;
	if(options & kCCOptionPKCS7Padding) padding = ccPKCS7Padding;
    
	/* No tweak was ever used */
   	tweak = NULL;
    tweakLength = 0;
    
	/* default rounds */
    numRounds = 0;
    
	/* No mode options needed */
    modeOptions = 0;
    
	return CCCryptorCreateFromDataWithMode(op, mode, alg, padding, iv, key, keyLength, tweak, tweakLength, numRounds, modeOptions, data, dataLength, cryptorRef, dataUsed);
    
}

CCCryptorStatus CCCryptorRelease(
	CCCryptorRef cryptorRef)
{
	uint32_t err;
    CCCompatCryptor *compat_cryptor = cryptorRef;
    CCCryptor *cryptor;
	uint32_t weMallocd;
    
	if(cryptorRef == NULL) return kCCSuccess;
    cryptor = compat_cryptor->cryptor;
	weMallocd = compat_cryptor->weMallocd;
	
	if(cryptor->modeptr->mode_done) cryptor->modeptr->mode_done(cryptor->ctx);
    
	memset(cryptor, 0, CCCRYPTOR_SIZE);
    free(cryptor);
    if(weMallocd) free(compat_cryptor);

	return kCCSuccess;
}


CCCryptorStatus CCCryptorUpdate(CCCryptorRef cryptorRef, const void *dataIn, size_t dataInLength, void *dataOut, size_t dataOutAvailable, size_t *dataOutMoved)
{
	CCCryptorStatus	retval;
	char *inp, *outp, *bufp;
	size_t inlen, outlen, movecnt, blocksize;
	int willneed;
	int	done = 0;
    CCCompatCryptor *compat_cryptor = cryptorRef;
    CCCryptor	*cryptor;
	
	if(compat_cryptor == NULL)  return kCCParamError;
    cryptor = compat_cryptor->cryptor;
    
	if(cryptor == NULL)  return kCCParamError;
    void *checkedDataIn = dataIn;
    if (checkedDataIn == NULL) checkedDataIn = "";
	if(cryptor->bufStrat == ccStreaming) {		
		if(dataOutAvailable < dataInLength) return kCCBufferTooSmall;
		if(cryptor->op == kCCEncrypt) {
			if((retval = ccLTCErr(cryptor->modeptr->mode_encrypt(dataIn, dataOut, dataInLength, cryptor->ctx))) != kCCSuccess) return retval;
		} else {
			if((retval = ccLTCErr(cryptor->modeptr->mode_decrypt(dataIn, dataOut, dataInLength, cryptor->ctx))) != kCCSuccess) return retval;
		}
		cryptor->bytesProcessed += dataInLength;
		if(dataOutMoved) *dataOutMoved = dataInLength;
		return kCCSuccess;
	}
	
	if(!(cryptor->bufStrat & ccStreamMode)) {	
			return kCCParamError; // ZZZ Need an invalid mode error before this goes public.
	}
	
	blocksize = cryptor->blocksize;
	willneed = ((cryptor->bufferPos + dataInLength) / blocksize) * blocksize;
	if(cryptor->padding == ccPKCS7Padding) willneed -= blocksize;
	if(willneed < 0) willneed = 0;
	if(dataOutAvailable < willneed) {
		ccdebug(ASL_LEVEL_ERR, "Pos %d InLength %d Needed %d Provided %d\n", cryptor->bufferPos, dataInLength, willneed, dataOutAvailable); 
		return kCCBufferTooSmall;
	}
	
	inp = (char *) dataIn;
	outp = (char *) dataOut;
	inlen = dataInLength;
	outlen = 0;
	bufp = cryptor->buffptr;
    
    if(cryptor->bufferPos == 0 && cryptor->padding != ccPKCS7Padding 
       /* && (((size_t) dataIn % 4) == 0) && (((size_t) dataOut % 4) == 0) */  ) { 
        // No Padding, not buffering, aligned pointers don't appear necessary for the entry into the x86 optimized routines. */
        
        size_t chunklen;
                
        chunklen = (dataInLength / cryptor->blocksize) * cryptor->blocksize;
        
        if(chunklen) {
            if(cryptor->op == kCCEncrypt) {
                if((retval = ccLTCErr(cryptor->modeptr->mode_encrypt(inp, outp, chunklen, cryptor->ctx))) != kCCSuccess) return retval;
            } else {
                if((retval = ccLTCErr(cryptor->modeptr->mode_decrypt(inp, outp, chunklen, cryptor->ctx))) != kCCSuccess) return retval;
            }
            inp += chunklen;
            outp += chunklen;
            inlen -= chunklen;
            outlen += chunklen;
            done = (inlen == 0);
        }
        if(done) {
            *dataOutMoved = outlen;
            return kCCSuccess;
        }
    }
    
    /* fprintf(stderr, "CRYPTORUPDATE BufferPos %d Padding is %s dataIn %16llx dataOut %16llx %lld\n",
           (int) cryptor->bufferPos, (cryptor->padding != ccPKCS7Padding) ? "None": "PKCS7",
           (unsigned long) dataIn, (unsigned long) dataOut,
            (unsigned long) dataInLength); */


	while(!done) {
		// determine how much to move to fill buffer

		movecnt = blocksize - cryptor->bufferPos;
		if(movecnt > inlen) movecnt = inlen;
		
		if(movecnt || cryptor->bufferPos == blocksize) {
			memmove(bufp + cryptor->bufferPos, inp, movecnt);
			inp += movecnt;
			inlen -= movecnt;
			cryptor->bufferPos += movecnt;
			
			if(cryptor->op == kCCEncrypt) {
				if(cryptor->bufferPos == blocksize) {
					if((retval = ccLTCErr(cryptor->modeptr->mode_encrypt(bufp, outp, blocksize, cryptor->ctx))) != kCCSuccess) return retval;
					cryptor->bytesProcessed += blocksize;
					cryptor->bufferPos = 0;
					outp += blocksize;
					outlen += blocksize;
					dataOutAvailable -= blocksize;
				} else {
					done = 1;
				}
			} else {
				if(cryptor->bufferPos == blocksize) {
					/* Need to delay by one block to know when we're processing a padded block if padding is enabled */
					if(cryptor->padding == ccPKCS7Padding && inlen == 0) done = 1; 
					else {	
						if((retval = ccLTCErr(cryptor->modeptr->mode_decrypt(bufp, outp, blocksize, cryptor->ctx))) != kCCSuccess) return retval;
						cryptor->bytesProcessed += blocksize;
						cryptor->bufferPos = 0;
						outp += blocksize;
						outlen += blocksize;
						dataOutAvailable -= blocksize;
					}
				}
			}
		} else done = 1;
	}
	*dataOutMoved = outlen;
	return kCCSuccess;
}



CCCryptorStatus CCCryptorFinal(
	CCCryptorRef cryptorRef,
	void *dataOut,					/* data RETURNED here */
	size_t dataOutAvailable,
	size_t *dataOutMoved)		/* number of bytes written */
{
	CCCryptorStatus	retval;
	char *bufp;
	size_t blocksize, moved;
	uint8_t padval;
	char tmpbuf[ccMaxCipherBlockSize];
    CCCompatCryptor *compat_cryptor = cryptorRef;
    CCCryptor	*cryptor;

    if(compat_cryptor == NULL)  return kCCParamError;
    cryptor = compat_cryptor->cryptor;
	if(cryptor == NULL) return kCCParamError;
	if(dataOutMoved) *dataOutMoved = 0;
	if(cryptor->bufStrat == ccStreaming) return kCCSuccess;

	blocksize = cryptor->blocksize;
	bufp = cryptor->buffptr;

	if(cryptor->op == kCCEncrypt) {
		if(cryptor->padding == ccPKCS7Padding) {
			padval = pkcs7_pad(bufp, blocksize, cryptor->bufferPos);
			moved = cryptor->bufferPos + padval;
			if(dataOutAvailable < moved) return kCCBufferTooSmall;
			if((retval = ccLTCErr(cryptor->modeptr->mode_encrypt(bufp, (char *) dataOut, moved, cryptor->ctx))) != kCCSuccess) return retval;
			*dataOutMoved = moved;
			cryptor->bytesProcessed += moved;
		}
		cryptor->bufferPos = 0;
	} else if(cryptor->op == kCCDecrypt) {
	  if(cryptor->padding == ccPKCS7Padding) {
			if(cryptor->bufferPos != blocksize) {
				return kCCAlignmentError;
			}
		   if((retval = ccLTCErr(cryptor->modeptr->mode_decrypt(bufp, tmpbuf, blocksize, cryptor->ctx))) != kCCSuccess) return retval;
			if((moved = pkcs7_unpadlen(tmpbuf, blocksize)) == -1) {
				return kCCAlignmentError;
			}
			if(dataOutAvailable < moved) return kCCBufferTooSmall;
			*dataOutMoved = moved;
			cryptor->bytesProcessed += *dataOutMoved;
			memmove((char *) dataOut, tmpbuf, *dataOutMoved);
		}
	} else /* Block I/O */ {
		// Nothing for now.  Block I/O will go here.
	}
	return kCCSuccess;
}

size_t CCCryptorGetOutputLength(
	CCCryptorRef cryptorRef,
	size_t inputLength,
	bool final)
{
	size_t retval;
	size_t totallength;
    CCCompatCryptor *compat_cryptor = cryptorRef;
    CCCryptor	*cryptor;
	
    if(compat_cryptor == NULL)  return kCCParamError;
    cryptor = compat_cryptor->cryptor;
	
	if(cryptor == NULL) return kCCParamError;
	totallength = cryptor->bufferPos + inputLength;
	if((cryptor->bufStrat == ccStreaming) || (cryptor->padding == 0)) retval = totallength;
	else if(cryptor->op == kCCEncrypt) retval = (totallength/cryptor->blocksize + 1) * cryptor->blocksize;
	else retval = totallength;
	ccdebug(ASL_LEVEL_ERR, " InputLength %d  Final is %s  OutputLength %d\n", inputLength, (final = true) ? "TRUE": "FALSE", retval);
	return retval;
}

/*
 * This routine needs to reset both the buffer position and the IV.
 * If the IV isn't used, don't attempt to reset it (RC4 is an example).
 */
 
CCCryptorStatus CCCryptorReset(
	CCCryptorRef cryptorRef,
	const void *iv)
{
    CCCompatCryptor *compat_cryptor = cryptorRef;
    CCCryptor	*cryptor;
    
    if(compat_cryptor == NULL)  return kCCParamError;
    cryptor = compat_cryptor->cryptor;
    
	/* <rdar://problem/8552731> need to reset buffering position. */
	cryptor->bufferPos = 0;
    cryptor->bytesProcessed = 0;
	/* <rdar://problem/8833770> (and others) Don't reset the IV if a streaming cipher is being used. */ 
	if(cryptor->blocksize == 1) return CRYPT_OK; // RC4
    ccSetIV(cryptor, iv);
    
    /* Make sure the mode has a "setter" for the IV - if not, we're done. */
    if(cryptor->modeptr->mode_setiv == NULL) 
    	return kCCSuccess;

	return ccLTCErr(cryptor->modeptr->mode_setiv(cryptor->iv, cryptor->ctx->cbcContext.blocklen, cryptor->ctx));
}


/* 
 * One-shot is mostly service provider independent, except for the
 * dataOutLength check.
 */
CCCryptorStatus CCCrypt(
	CCOperation op,			/* kCCEncrypt, etc. */
	CCAlgorithm alg,		/* kCCAlgorithmAES128, etc. */
	CCOptions options,		/* kCCOptionPKCS7Padding, etc. */
	const void *key,
	size_t keyLength,
	const void *iv,			/* optional initialization vector */
	const void *dataIn,		/* optional per op and alg */
	size_t dataInLength,
	void *dataOut,			/* data RETURNED here */
	size_t dataOutAvailable,
	size_t *dataOutMoved)	
{
	CCCryptorRef cryptor = NULL;
	CCCryptorStatus retval;
    uint8_t *outp;
	size_t outputSize;
	size_t used = 0;
	size_t moved;

	if(retval = CCCryptorCreate(op, alg, options, key, keyLength, iv, &cryptor)) {
		return retval;
	}
		
	if((outputSize = CCCryptorGetOutputLength(cryptor, dataInLength, false)) > dataOutAvailable) {
		CCCryptorRelease(cryptor);
		if(dataOutMoved != NULL) *dataOutMoved = outputSize;
		return kCCBufferTooSmall;
	}
    
    outp = (uint8_t *) dataOut;
	
	if(retval = CCCryptorUpdate(cryptor, dataIn, dataInLength, outp, dataOutAvailable, &moved)) {
		CCCryptorRelease(cryptor);
		return retval;
	}
	outp += moved;
	used += moved;
	dataOutAvailable  -= moved;
	if(retval = CCCryptorFinal(cryptor, outp, dataOutAvailable, &moved)) {
		ccdebug(ASL_LEVEL_ERR, "Final Error\n", 0);
	} else {
		used += moved;
		if(dataOutMoved != NULL) *dataOutMoved = used;
	}
	
	CCCryptorRelease(cryptor);
	return retval;
}



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
{
    CCCompatCryptor *compat_cryptor = NULL;
	CCCryptor *cryptor = NULL;
	uint32_t err;
    size_t needed;

	/* validate pointers */
	if((data == NULL) || (cryptorRef == NULL) || (key == NULL)) {
		ccdebug(ASL_LEVEL_ERR, "bad arguments\n", 0);
		return kCCParamError;
	}
	
    if((size_t)data % 4) return kCCAlignmentError;

    compat_cryptor = data;
    
    needed = sizeof(CCCompatCryptor);
    if(dataUsed != NULL) *dataUsed = needed;
    
	if(needed > dataLength) {
		return kCCBufferTooSmall;
	}
		
    compat_cryptor->weMallocd = false;
    if((cryptor = (CCCryptor *)malloc(DEFAULT_CRYPTOR_MALLOC)) == NULL) return kCCMemoryFailure;
	compat_cryptor->cryptor = cryptor;
    
	cryptor->mode = mode;
	cryptor->modeptr = mode_descriptor[cryptor->mode];
	
	cryptor->ctx = (mode_context *) cryptor->modeContext;
	
	ccdebug(ASL_LEVEL_ERR, "Entered - data (%016llx) cryptor (%016llx) ctx (%016llx)\n", (uint64_t) data, (uint64_t) cryptor, (uint64_t) cryptor->ctx);

	/* Setup the easy parts of the Cryptor - some will be used in other APIs */
	cryptor->op = op;
	cryptor->mallocAddress = NULL;
	cryptor->cipher = alg;
	cryptor->bytesProcessed = cryptor->bufferPos = 0;
	cryptor->bufferPos = 0;

	cryptor->padding = padding;
		
	if(alg == kCCAlgorithmRC4) {  // Only pure streaming algorithm currently supported.
		cryptor->blocksize = 1;
		cryptor->bufStrat = ccStreaming;
		cryptor->ltcAlgIndex = -1;
	} else {
		if((cryptor->ltcAlgIndex = ccGetAlgorithm(alg)) == -1) return kCCParamError;
		if(cipher_descriptor[cryptor->ltcAlgIndex].keysize && 
           cipher_descriptor[cryptor->ltcAlgIndex].keysize(&keyLength) != CRYPT_OK) 
            return kCCParamError;
		cryptor->blocksize = cipher_descriptor[cryptor->ltcAlgIndex].block_length;
		cryptor->bufStrat = ccStreaming;
		if(cryptor->mode == kCCModeECB) cryptor->bufStrat = ccBlockMode | ccStreamMode;
		if(cryptor->mode == kCCModeCBC) cryptor->bufStrat = ccBlockMode | ccStreamMode;
		if(cryptor->mode == kCCModeXTS) cryptor->bufStrat = ccBlockMode;
	}
	
	ccSetIV(cryptor, iv);	
	*cryptorRef = compat_cryptor;

	/* once the cryptor is all setup - initialize the context */
	
    if(cryptor->modeptr->mode_setup) {
        err = cryptor->modeptr->mode_setup(cryptor->ltcAlgIndex, cryptor->iv, 
                                    key, keyLength, tweak, tweakLength,
                                    numRounds, options, cryptor->ctx);
    } else {
        /* There *should* be no cases where this occurs. */
        return kCCUnimplemented;
    }
	
	return ccLTCErr(err);
}

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
{
	CCCryptorStatus err;
	CCCompatCryptor *compat_cryptor = NULL;
	size_t dataUsed = 0;
	
	if((compat_cryptor = (CCCompatCryptor *)malloc(sizeof(CCCompatCryptor))) == NULL) return kCCMemoryFailure;
	err = CCCryptorCreateFromDataWithMode( op, mode, alg, padding, iv, key,  keyLength, tweak, tweakLength, numRounds, options, compat_cryptor, DEFAULT_CRYPTOR_MALLOC, cryptorRef, &dataUsed); 
	if(err != kCCSuccess)  free(compat_cryptor);
	else compat_cryptor->weMallocd = true;
	return err;
}

CCCryptorStatus CCCryptorEncryptDataBlock(
	CCCryptorRef cryptorRef,
	const void *iv,
	const void *dataIn,
	size_t dataInLength,
	void *dataOut)
{
    CCCompatCryptor *compat_cryptor = cryptorRef;
    CCCryptor	*cryptor;
    
    if(compat_cryptor == NULL)  return kCCParamError;
    cryptor = compat_cryptor->cryptor;
    
	if((cryptor->bufStrat & ccBlockMode) == 0) return kCCParamError;
    
	if(iv && cryptor->modeptr->mode_encrypt_tweaked != NULL && cryptor->modeptr->mode_encrypt_tweaked != unimp_mode_encrypt_tweaked)
		return ccLTCErr(cryptor->modeptr->mode_encrypt_tweaked(dataIn, dataInLength, dataOut, (const unsigned char *) iv, cryptor->ctx));

	if(cryptor->modeptr->mode_encrypt != NULL && cryptor->modeptr->mode_encrypt != unimp_mode_encrypt)
        return ccLTCErr(cryptor->modeptr->mode_encrypt(dataIn, dataOut, dataInLength, cryptor->ctx));
    
	return kCCUnimplemented;
}


CCCryptorStatus CCCryptorDecryptDataBlock(
	CCCryptorRef cryptorRef,
	const void *iv,
	const void *dataIn,
	size_t dataInLength,
	void *dataOut)
{
    CCCompatCryptor *compat_cryptor = cryptorRef;
    CCCryptor	*cryptor;
    
    if(compat_cryptor == NULL)  return kCCParamError;
    cryptor = compat_cryptor->cryptor;

	if((cryptor->bufStrat & ccBlockMode) == 0) return kCCParamError;

	if(iv && cryptor->modeptr->mode_decrypt_tweaked != NULL && cryptor->modeptr->mode_decrypt_tweaked != unimp_mode_decrypt_tweaked)
		return ccLTCErr(cryptor->modeptr->mode_decrypt_tweaked(dataIn, dataInLength, dataOut, (const unsigned char *) iv, cryptor->ctx));

	if(cryptor->modeptr->mode_decrypt != NULL && cryptor->modeptr->mode_decrypt != unimp_mode_decrypt)
        return ccLTCErr(cryptor->modeptr->mode_decrypt(dataIn, dataOut, dataInLength, cryptor->ctx));
    
	return kCCUnimplemented;
}


