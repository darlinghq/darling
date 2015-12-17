/*
 ---------------------------------------------------------------------------
 Copyright (c) 2003, Dr Brian Gladman, Worcester, UK.   All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products
      built using this software without specific written permission.

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue 28/01/2004

 This file contains the definitions required to use AES in C. See aesopt.h
 for optimisation details.
*/

#if !defined( _CC_AES_H_ )
#define _CC_AES_H_

// Generate nothing if this file has been included in an assembly language file.
#if !__ASSEMBLER__

#include <Availability.h>
#include <AvailabilityMacros.h>
#include <CommonCrypto/CommonCryptoPriv.h>

/*  This include is used to find 8 & 32 bit unsigned integer types  */
#include <machine/limits.h>

#include <TargetConditionals.h>
#if TARGET_OS_EMBEDDED && __arm__
#define CC_AES_USE_HARDWARE				1
#endif

#if CC_AES_USE_HARDWARE
#define CC_AES_MAX_KEYSIZE				32 //32 bytes or 256 bits
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

/*	CommonCrypto-specific mods. 

	_APPLE_COMMON_CRYPTO_, when defined, enables the following:
	
	 -- IV/chain buffer stored in the aes_{en,de}crypt_ctx (though it 
	    can still be passed to the encrypt/decrypt routines)
	 -- ECB/CBC controlled per ctx->cbcEnable
	 -- common SPI functions 
	 -- disables AES_VAR
*/

#define AES_128     /* define if AES with 128 bit keys is needed    */
#define AES_192     /* define if AES with 192 bit keys is needed    */
#define AES_256     /* define if AES with 256 bit keys is needed    */
#ifndef		_APPLE_COMMON_CRYPTO_
#define AES_VAR     /* define if a variable key size is needed      */
#endif

/* The following must also be set in assembler files if being used  */

#define AES_ENCRYPT /* if support for encryption is needed          */
#define AES_DECRYPT /* if support for decryption is needed          */
//#define AES_ERR_CHK /* for parameter checks & error return codes    */

#if UCHAR_MAX == 0xff                   /* an unsigned 8 bit type   */
  typedef unsigned char      aes_08t;
#else
#  error Please define aes_08t as an 8-bit unsigned integer type in aes.h
#endif

#if UINT_MAX == 4294967295              /* an unsigned 32 bit type  */
  typedef   unsigned int     aes_32t;
#elif ULONG_MAX == 4294967295ul
  typedef   unsigned long    aes_32t;
#else
#  error Please define aes_32t as a 32-bit unsigned integer type in aes.h
#endif

#define AES_BLOCK_SIZE  16  /* the AES block size in bytes          */
#define N_COLS           4  /* the number of columns in the state   */

/* The key schedule length is 11, 13 or 15 16-byte blocks for 128,  */
/* 192 or 256-bit keys respectively. That is 176, 208 or 240 bytes  */
/* or 44, 52 or 60 32-bit words. For simplicity this code allocates */
/* the maximum 60 word array for the key schedule for all key sizes */

#if defined( AES_VAR ) || defined( AES_256 )
#define KS_LENGTH       60
#elif defined( AES_192 )
#define KS_LENGTH       52
#else
#define KS_LENGTH       44
#endif

#if defined( AES_ERR_CHK )
#define aes_ret     int
#define aes_good    0
#define aes_error  -1
#else
#define aes_ret     void
#endif

#if !defined( AES_DLL )                 /* implement normal/DLL functions   */
#define aes_rval    aes_ret
#else
#define aes_rval    aes_ret __declspec(dllexport) _stdcall
#endif

typedef struct
{   aes_32t ks[KS_LENGTH];
    aes_32t rn;
	#ifdef	_APPLE_COMMON_CRYPTO_
	unsigned char chainBuf[AES_BLOCK_SIZE];
	aes_32t cbcEnable;
	#if CC_AES_USE_HARDWARE
	unsigned char keyBytes[CC_AES_MAX_KEYSIZE];
	aes_32t keyLength;
	#endif
	#endif
} aes_encrypt_ctx;

typedef struct
{   aes_32t ks[KS_LENGTH];
    aes_32t rn;
	#ifdef	_APPLE_COMMON_CRYPTO_
	unsigned char chainBuf[AES_BLOCK_SIZE];
	aes_32t cbcEnable;
	#if CC_AES_USE_HARDWARE
	unsigned char keyBytes[CC_AES_MAX_KEYSIZE];
	aes_32t keyLength;
	#endif
	#endif
} aes_decrypt_ctx;

typedef struct
{   
	aes_decrypt_ctx decrypt;
    aes_encrypt_ctx encrypt;
} aes_ctx;


/* This routine must be called before first use if non-static       */
/* tables are being used                                            */

void gen_tabs(void);

/* The key length (klen) is input in bytes when it is in the range  */
/* 16 <= klen <= 32 or in bits when in the range 128 <= klen <= 256 */

#if defined( AES_ENCRYPT )

#if defined(AES_128) || defined(AES_VAR)
__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
aes_rval aes_encrypt_key128(const unsigned char *in_key, aes_encrypt_ctx cx[1]);
#endif

#if defined(AES_192) || defined(AES_VAR)
__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
aes_rval aes_encrypt_key192(const unsigned char *in_key, aes_encrypt_ctx cx[1]);
#endif

#if defined(AES_256) || defined(AES_VAR)
__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
aes_rval aes_encrypt_key256(const unsigned char *in_key, aes_encrypt_ctx cx[1]);
#endif

#if defined(AES_VAR)
__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
aes_rval aes_encrypt_key(const unsigned char *in_key, int key_len, aes_encrypt_ctx cx[1]);
#endif

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
aes_rval aes_encrypt_cbc(const unsigned char *in_blk, const unsigned char *in_iv, unsigned int num_blk,
					 unsigned char *out_blk, aes_encrypt_ctx cx[1]);
#endif

#if defined( AES_DECRYPT )

#if defined(AES_128) || defined(AES_VAR)
__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
aes_rval aes_decrypt_key128(const unsigned char *in_key, aes_decrypt_ctx cx[1]);
#endif

#if defined(AES_192) || defined(AES_VAR)
__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
aes_rval aes_decrypt_key192(const unsigned char *in_key, aes_decrypt_ctx cx[1]);
#endif

#if defined(AES_256) || defined(AES_VAR)
__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
aes_rval aes_decrypt_key256(const unsigned char *in_key, aes_decrypt_ctx cx[1]);
#endif

#if defined(AES_VAR)
__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
aes_rval aes_decrypt_key(const unsigned char *in_key, int key_len, aes_decrypt_ctx cx[1]);
#endif

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
aes_rval aes_decrypt_cbc(const unsigned char *in_blk, const unsigned char *in_iv, unsigned int num_blk,
					 unsigned char *out_blk, aes_decrypt_ctx cx[1]);
#endif

#ifdef	_APPLE_COMMON_CRYPTO_

typedef union
{   
	aes_decrypt_ctx decrypt;
    aes_encrypt_ctx encrypt;
} aes_cc_ctx;

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
int aes_cc_set_key(aes_cc_ctx *cx, const void *rawKey, aes_32t keyLength, 
	int forEncrypt);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
void aes_cc_set_iv(aes_cc_ctx *cx, int forEncrypt, const void *iv);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
void aes_cc_encrypt(aes_cc_ctx *cx, const void *blocksIn, aes_32t numBlocks,
	void *blocksOut);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
void aes_cc_decrypt(aes_cc_ctx *cx, const void *blocksIn, aes_32t numBlocks,
	void *blocksOut);
#endif

#if defined(__cplusplus)
}
#endif

#endif	// !__ASSEMBLER__

#endif	/* _CC_AES_H_ */
