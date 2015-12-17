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

#include <unistd.h>
#include <stdint.h>
#include "tomcrypt.h"
#include "rc4.h"

typedef union ltcModeContext {
	symmetric_ECB 	ecbContext;
	symmetric_CBC		cbcContext;
	symmetric_CFB		cfbContext;
	symmetric_CTR		ctrContext;
	symmetric_OFB 	ofbContext;
	symmetric_LRW 	lrwContext;
	symmetric_F8 	f8Context;
	symmetric_xts 	xtsContext;
	char rc4ctx[sizeof(RC4_KEY)];
} mode_context;

typedef struct ltc_mode_descriptor {
	/** name of mode */
	char *name;
	/** internal ID */
	unsigned char ID;
	/** default number of rounds */
	uint32_t default_rounds;
	/** size of the context */
	size_t	ctxsize;
	/** Setup the mode
	 @param cipher				The index of the LTC Cipher - must be registered
	 @param IV						The initial vector
	 @param key						The input symmetric key
	 @param keylen				The length of the input key (octets)
	 @param tweak					The input tweak or salt
	 @param tweaklen			The length of the tweak or salt (if variable) (octets)
	 @param options				Mask for any mode options
	 @param num_rounds		The requested number of rounds (0==default)
	 @param ctx						[out] The destination of the mode context
	 @return CRYPT_OK if successful
	 */
	int  (*mode_setup)(int cipher, const unsigned char *IV, const unsigned char *key, int keylen, const unsigned char *tweak, int tweaklen, int num_rounds, int options, mode_context *ctx);
	/** Encrypt a block
	 @param pt						The plaintext
	 @param ct						[out] The ciphertext
	 @param len						the length of data (in == out) octets
	 @param ctx						The mode context
	 @return CRYPT_OK if successful
	 */
	int (*mode_encrypt)(const unsigned char *pt, unsigned char *ct, unsigned long len, mode_context *ctx);
	/** Decrypt a block
	 @param ct						The ciphertext
	 @param pt						[out] The plaintext
	 @param len						the length of data (in == out) octets
	 @param ctx						The mode context
	 @return CRYPT_OK if successful
	 */
	int (*mode_decrypt)(const unsigned char *ct, unsigned char *pt, unsigned long len, mode_context *ctx);
	/** Encrypt a block with a tweak (XTS mode currently)
	 @param pt						The plaintext
	 @param ct						[out] The ciphertext
	 @param len						the length of data (in == out) octets
	 @param tweak					The 128--bit encryption tweak (e.g. sector number)
	 @param ctx						The mode context
	 @return CRYPT_OK if successful
	 */
	int (*mode_encrypt_tweaked)(const unsigned char *pt, unsigned long len, unsigned char *ct, const unsigned char *tweak, mode_context *ctx);
	/** Decrypt a block with a tweak (XTS mode currently)
	 @param ct						The ciphertext
	 @param pt						[out] The plaintext
	 @param len						the length of data (in == out) octets
	 @param ctx						The mode context
	 @return CRYPT_OK if successful
	 */
	int (*mode_decrypt_tweaked)(const unsigned char *ct, unsigned long len, unsigned char *pt, const unsigned char *tweak, mode_context *ctx);
	/** Terminate the mode
	 @param ctx						[out] The mode context
	 @return CRYPT_OK if successful
	 */
	int (*mode_done)(mode_context *ctx);
	/** Set an Initial Vector
	 @param IV						The initial vector
	 @param len						The length of the initial vector
	 @param ctx						The mode context
	 @return CRYPT_OK if successful
	 */
	int (*mode_setiv)(const unsigned char *IV, unsigned long len, mode_context *ctx);
	/** Get an Initial Vector
	 @param IV						[out] The initial vector
	 @param len						The length of the initial vector
	 @param ctx						The mode context
	 @return CRYPT_OK if successful
	 */
	int (*mode_getiv)(const unsigned char *IV, unsigned long *len, mode_context *ctx);
	
	
} *mode_descriptor_ptr;

extern mode_descriptor_ptr mode_descriptor[];

int unimp_mode_setup(int cipher, const unsigned char *IV, const unsigned char *key, int keylen, 
const unsigned char *tweak, int tweaklen, int num_rounds, int options, mode_context *ctx);
int unimp_mode_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, mode_context *ctx);
int unimp_mode_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, mode_context *ctx);
int unimp_mode_encrypt_tweaked(const unsigned char *pt, unsigned long len, unsigned char *ct, const unsigned char *tweak, mode_context *ctx);
int unimp_mode_decrypt_tweaked(const unsigned char *ct, unsigned long len, unsigned char *pt, const unsigned char *tweak, mode_context *ctx);
int unimp_mode_done(mode_context *ctx);
int unimp_mode_setiv(const unsigned char *IV, unsigned long len, mode_context *ctx);
int unimp_mode_getiv(const unsigned char *IV, unsigned long *len, mode_context *ctx);

int rc4_stream_setup(int cipher, const unsigned char *IV, const unsigned char *key, int keylen, 
const unsigned char *tweak, int tweaklen, int num_rounds, int options, RC4_KEY *ctx);
int rc4_stream_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, mode_context *ctx);
int rc4_stream_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, mode_context *ctx);
int rc4_stream_done(mode_context *ctx);

extern const struct ltc_mode_descriptor	modeUNIMP_desc;
extern const struct ltc_mode_descriptor	modeECB_desc;
extern const struct ltc_mode_descriptor	modeCBC_desc;
extern const struct ltc_mode_descriptor	modeCFB_desc;
extern const struct ltc_mode_descriptor	modeCFB8_desc;
extern const struct ltc_mode_descriptor	modeCTR_desc;
extern const struct ltc_mode_descriptor	modef8_desc;
extern const struct ltc_mode_descriptor	modeLRW_desc;
extern const struct ltc_mode_descriptor	modeOFB_desc;
extern const struct ltc_mode_descriptor	modeXTS_desc;
extern const struct ltc_mode_descriptor	modeRC4_desc;

