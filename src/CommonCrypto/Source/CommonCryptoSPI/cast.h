/* crypto/cast/cast.h */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#ifndef CC_CAST_H
#define CC_CAST_H

#include <Availability.h>
#include <AvailabilityMacros.h>

#ifdef  __cplusplus
extern "C" {
#endif

#include <CommonCrypto/CommonCryptoPriv.h>
#include <stdint.h>

#ifdef	_APPLE_COMMON_CRYPTO_
/* avoid symbol collision with libSystem & libcrypto */
#define CAST_set_key		CC_CAST_set_key
#define CAST_ecb_encrypt	CC_CAST_ecb_encrypt
#define CAST_encrypt		CC_CAST_encrypt
#define CAST_decrypt		CC_CAST_decrypt
#endif	/* _APPLE_COMMON_CRYPTO_ */

#ifdef NO_CAST
#error CAST is disabled.
#endif

#define CAST_ENCRYPT	1
#define CAST_DECRYPT	0

#ifdef	_APPLE_COMMON_CRYPTO_
#define CAST_LONG uint32_t
#else
#define CAST_LONG unsigned long
#endif	/* _APPLE_COMMON_CRYPTO_ */

#define CAST_BLOCK			8			/* block size in bytes */
#define CAST_KEY_LENGTH		16			/* MAX key size in bytes */
#define CAST_MIN_KEY_LENGTH	5			/* MIN key size in bytes */
typedef struct cast_key_st
	{
	CAST_LONG data[32];
	int short_key;	/* Use reduced rounds for short key */
	} CAST_KEY;
 
__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
void CAST_set_key(CAST_KEY *key, int len, const unsigned char *data);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
void CAST_ecb_encrypt(const unsigned char *in,unsigned char *out,CAST_KEY *key,
		      int enc);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
void CAST_encrypt(CAST_LONG *data,CAST_KEY *key);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
void CAST_decrypt(CAST_LONG *data,CAST_KEY *key);
#ifndef	_APPLE_COMMON_CRYPTO_

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
void CAST_cbc_encrypt(const unsigned char *in, unsigned char *out, long length,
		      CAST_KEY *ks, unsigned char *iv, int enc);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
void CAST_cfb64_encrypt(const unsigned char *in, unsigned char *out,
			long length, CAST_KEY *schedule, unsigned char *ivec,
			int *num, int enc);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_7, __IPHONE_NA, __IPHONE_NA)
void CAST_ofb64_encrypt(const unsigned char *in, unsigned char *out, 
			long length, CAST_KEY *schedule, unsigned char *ivec,
			int *num);
#endif	/* _APPLE_COMMON_CRYPTO_ */
#ifdef  __cplusplus
}
#endif

#endif	/* CC_CAST_H */
